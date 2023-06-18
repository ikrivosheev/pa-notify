#include <glib.h>
#include <glib/gprintf.h>
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <locale.h>
#include <math.h>
#include <pulse/pulseaudio.h>
#include <signal.h>

#define PROGRAM_NAME "pa-notify"
#define DEFAULT_DEBUG FALSE

typedef struct _Context
{
    pa_mainloop* loop;
    pa_mainloop_api* api;
    pa_context* context;
    NotifyNotification* notification;
    gint volume;

} Context;

static struct config
{
    gboolean debug;
    gint timeout;
} config = {
    DEFAULT_DEBUG,
    NOTIFY_EXPIRES_DEFAULT,
};

static GOptionEntry option_entries[] = {
    { "debug", 'd', 0, G_OPTION_ARG_NONE, &config.debug, "Enable/disable debug information", NULL },
    { "timeout",
      't',
      0,
      G_OPTION_ARG_INT,
      &config.timeout,
      "Notification timeout in seconds (-1 - default notification timeout, 0 - notification never "
      "expires)",
      NULL },
    { NULL }
};

void
context_init(Context* context)
{
    context->loop = NULL;
    context->api = NULL;
    context->context = NULL;
    context->notification = notify_notification_new(NULL, NULL, NULL);
    context->volume = -2;
}

void
context_free(Context* context)
{
    if (context != NULL) {
        pa_context_unref(context->context);
    }

    if (context->loop) {
        pa_signal_done();
        pa_mainloop_free(context->loop);
    }
}

void
context_exit(Context* context, int retval)
{
    if (context->api) {
        context->api->quit(context->api, retval);
    }
}

static gchar*
notify_icon(gint volume)
{
    if (volume < 0) {
        return "audio-volume-muted";
    } else if (volume >= 66) {
        return "audio-volume-high";
    } else if (volume >= 33) {
        return "audio-volume-medium";
    } else {
        return "audio-volume-low";
    }
}

static void
notify_message(NotifyNotification* notification,
               const gchar* summary,
               NotifyUrgency urgency,
               const gchar* icon,
               gint timeout,
               gint volume)
{
    notify_notification_update(notification, summary, NULL, icon);
    notify_notification_set_timeout(notification, timeout);
    notify_notification_set_urgency(notification, urgency);
    if (volume >= 0) {
        GVariant* g_volume = g_variant_new_int32(volume);
        notify_notification_set_hint(notification, "value", g_volume);
    } else {
        notify_notification_set_hint(notification, "value", NULL);
    }

    notify_notification_show(notification, NULL);
}

static void
sink_info_callback(pa_context* c, const pa_sink_info* i, int eol, void* userdata)
{
    static gchar summery[255];
    float f_volume;
    gint volume;
    Context* context = (Context*)userdata;

    g_debug("Sink info");
    if (i) {
        if (i->mute) {
            volume = -1;
            g_sprintf(summery, "Volume muted");
        } else {
            f_volume = (float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM;
            f_volume *= 100.0f;
            volume = (int)ceil(f_volume);
            g_sprintf(summery, "Volume (%d%%)", volume);
        }

        if (context->volume != volume) {
            notify_message(context->notification,
                           summery,
                           NOTIFY_URGENCY_LOW,
                           notify_icon(volume),
                           config.timeout,
                           volume);
            context->volume = volume;
        }
    }
}

static void
subscribe_callback(pa_context* c, pa_subscription_event_type_t type, uint32_t idx, void* userdata)
{
    pa_operation* op = NULL;
    unsigned facility = type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

    g_debug("New event");
    switch (facility) {
        case PA_SUBSCRIPTION_EVENT_SINK:
            op = pa_context_get_sink_info_by_index(c, idx, sink_info_callback, userdata);
            break;
        default:
            g_debug("Unexpected event");
            break;
    }

    if (op) {
        pa_operation_unref(op);
    }
}

static void
context_state_callback(pa_context* c, void* userdata)
{
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
            g_info("PulseAudio connection established.\n");
            // Subscribe to sink events from the server. This is how we get
            // volume change notifications from the server.
            pa_context_set_subscribe_callback(c, subscribe_callback, userdata);
            pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
            break;

        case PA_CONTEXT_TERMINATED:
            g_warning("PulseAudio connection terminated.\n");
            context_exit(userdata, 0);
            break;

        case PA_CONTEXT_FAILED:
        default:
            g_warning("Connection failure: %s\n", pa_strerror(pa_context_errno(c)));
            context_exit(userdata, 1);
            break;
    }
}

static void
exit_signal_callback(pa_mainloop_api* api, pa_signal_event* e, int sig, void* userdata)
{
    if (api) {
        api->quit(api, 0);
    }
}

gboolean
pa_init(Context* c)
{
    if (!(c->loop = pa_mainloop_new())) {
        g_warning("pa_mainloop_new failed");
        return FALSE;
    }
    g_debug("pa_mainloop_new");

    c->api = pa_mainloop_get_api(c->loop);
    if (pa_signal_init(c->api) != 0) {
        g_warning("pa_signal_init failed");
        return FALSE;
    }
    g_debug("pa_mainloop_get_api");

    if (!pa_signal_new(SIGINT, exit_signal_callback, c)) {
        g_warning("pa_signal_new SIGINT failed");
        return FALSE;
    }
    if (!pa_signal_new(SIGTERM, exit_signal_callback, c)) {
        g_warning("pa_signal_new SIGTERN failed");
        return FALSE;
    }
    signal(SIGPIPE, SIG_IGN);
    g_debug("pa_signal_new SIGINT SIGTERM");

    if (!(c->context = pa_context_new(c->api, PROGRAM_NAME))) {
        g_warning("pa_context_new failed");
        return FALSE;
    }
    g_debug("pa_context_new");

    if (pa_context_connect(c->context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
        g_warning("pa_context_connect ");
        return FALSE;
    }
    g_debug("pa_context_connect");

    pa_context_set_state_callback(c->context, context_state_callback, c);
    g_debug("pa_context_set_state_callback");

    return TRUE;
}

static gboolean
options_init(int argc, char* argv[])
{

    GError* error = NULL;
    GOptionContext* option_context;

    option_context = g_option_context_new(NULL);
    g_option_context_add_main_entries(option_context, option_entries, PROGRAM_NAME);

    if (g_option_context_parse(option_context, &argc, &argv, &error) == FALSE) {
        g_warning("Cannot parse command line arguments: %s", error->message);
        g_error_free(error);
        return FALSE;
    }

    g_option_context_free(option_context);
    if (config.debug == TRUE)
        g_log_set_handler(NULL, G_LOG_LEVEL_DEBUG, g_log_default_handler, NULL);
    else
        g_log_set_handler(NULL,
                          G_LOG_LEVEL_INFO | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL |
                            G_LOG_LEVEL_ERROR,
                          g_log_default_handler,
                          NULL);

    if (config.timeout > 0) {
        config.timeout *= 1000;
    }

    return TRUE;
}

int
main(int argc, char* argv[])
{
    Context context;
    int retval = 1;

    setlocale(LC_ALL, "");

    g_return_val_if_fail(options_init(argc, argv), 1);
    g_info("Options have been initialized");

    context_init(&context);
    g_return_val_if_fail(notify_init(PROGRAM_NAME), 1);
    g_info("Notify has been initialized");

    g_return_val_if_fail(pa_init(&context), 1);
    g_info("PulseAudio has been initialized");

    g_info("Run loop");
    pa_mainloop_run(context.loop, &retval);
    g_info("Stop loop");

    notify_uninit();
    context_free(&context);

    return retval;
}

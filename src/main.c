#include <locale.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <pulse/pulseaudio.h>

#define G_LOG_DOMAIN    ((gchar*) 0)
#define APPLICATION_NAME "pa-notify"


static void subscribe_callback(
    pa_context* context, 
    pa_subscription_event_type_t t, 
    uint32_t idx, 
    void *userdata)
{
    printf("New event");
}

static void context_state_callback(pa_context *c, void *userdata)
{
    switch (pa_context_get_state(c))
    {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
            fprintf(stderr, "PulseAudio connection established.\n");
            // Subscribe to sink events from the server. This is how we get
            // volume change notifications from the server.
            pa_context_set_subscribe_callback(c, subscribe_callback, userdata);
            pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
            break;

        case PA_CONTEXT_TERMINATED:
            fprintf(stderr, "PulseAudio connection terminated.\n");
            break;

        case PA_CONTEXT_FAILED:
        default:
            fprintf(stderr, "Connection failure: %s\n", pa_strerror(pa_context_errno(c)));
            break;
    }
}

int main(int argc, char* argv[]) 
{
    pa_mainloop* loop = NULL;    
    pa_mainloop_api* api = NULL;
    pa_context* context = NULL;
    int error, retval;

    setlocale (LC_ALL, "");

    loop = pa_mainloop_new();
    api = pa_mainloop_get_api(loop);
    context = pa_context_new(api, APPLICATION_NAME);
    if ((error = pa_context_connect(context, NULL, 0, NULL)) != 0) 
    {
        g_error("pa_context_connect returned NULL");
        return FALSE;
    }

    pa_context_set_state_callback(context, context_state_callback, NULL);

    g_info("Run loop");
    pa_mainloop_run(loop, &retval);

    pa_mainloop_free(loop);
    return 0;
}

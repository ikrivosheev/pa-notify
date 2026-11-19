# pa-notify

<a href="https://aur.archlinux.org/packages/pa-notify/"><img src="https://raw.githubusercontent.com/themix-project/oomox/master/packaging/download_aur.png" height="54"></a>

### Description

PulseAudio or PipeWire volume notification. Automatically reconnects if the sound server daemon is restarted.

### Usage

```
pa-notify [OPTION…]
```

### Options

#### Help Options:

- `-h`, `--help` - Show help options

#### Application Options:

- `-d`, `--debug` - Enable/disable debug information
- `-t`, `--timeout` - Notification timeout in seconds (-1 - default notification timeout, 0 - notification never expires)
- `-u`, `--urgency` - Notification urgency (0 - low, 1 - normal, 2 - critical)

### Examples

- `pa-notify` - run with default settings
- `pa-notify -t 2 -u 1` - show each notification for two seconds with normal urgency

### Build

Dependencies (development packages): `glib-2.0`, `libpulse`, `libnotify`; plus `cmake` and a C compiler.

```
cmake -B build
cmake --build build
sudo cmake --install build
```

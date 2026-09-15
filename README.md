# Echo

Audio device input/output filters for [OBS Studio](https://obsproject.com/) — send audio
from OBS straight out to hardware, and bring hardware audio back into OBS, without a
virtual cable or a second application in the path.

Echo is a standalone extraction of the Device IO feature from the
[atkAudio Plugin for OBS](https://github.com/atkAudio/PluginForObsRelease). See
[Credits](#credits).

## Filters

Both filters are audio filters: add them to any OBS source that has audio (including a
dummy/mixer source) via **Filters → + → Echo Device IO**.

### Echo Device IO

Single-device routing. Pick one input and one output device in the settings window and the
filter sends the source's audio to it and (optionally) mixes the device's input back into
the OBS chain.

- **Mix Input** — mix the device's input into the filter's output instead of replacing it
- **Follow Source Volume/Mute** — track the parent source's fader, mute and monitoring type
- **Follow Scene** — fade out when the parent source is not in the current scene, timed to
  the active transition
- **Input Gain** / **Output Gain** — ±30 dB, applied to the device signal only
- **Output Delay** — 0–10000 ms, smoothed delay line for lip-sync or latency alignment

### Echo Device IO 2

Multi-device routing. The settings window exposes a channel routing matrix: any device
channel on any driver can be mapped to any filter channel, in either direction, across
several devices at once. Sample-rate conversion and drift compensation between devices and
OBS are handled internally. The filter properties are the same as above, minus **Mix
Input** (routing replaces it).

## Supported audio drivers

| Platform | Drivers |
|---|---|
| Windows | WASAPI, DirectSound, ASIO (see below) |
| macOS | CoreAudio |
| Linux | ALSA, JACK |

ASIO requires the Steinberg ASIO SDK, which cannot be redistributed with this repository.
To build with ASIO support, download the SDK and configure with
`-DASIO_SDK_DIR=<path to asiosdk>` (or set the `ASIOSDK_DIR` environment variable). Builds
without it fall back to WASAPI/DirectSound and log which drivers are available.

## Installation

Requires **OBS Studio 31.1.1 or newer**. The plugin refuses to load on older versions.

Download the installer or portable archive for your platform from the
[releases page](https://github.com/voidscape-development/echo/releases). For portable
Linux installs, extract the archive and copy the `echo` directory into
`~/.config/obs-studio/plugins/`.

Echo can be installed alongside the original atkAudio plugin: it uses its own module name,
its own filter IDs and its own settings file. Be aware that the two plugins each open
audio hardware independently, so they cannot share a device that is opened exclusively
(most notably ASIO devices) — whichever plugin opens it first wins.

## Building

Dependencies: CMake 3.28+, a C++23 compiler, Qt 6, the OBS sources/prebuilt dependencies
listed in `buildspec.json`, and [JUCE](https://github.com/juce-framework/JUCE) 9.0.2
(fetched automatically at configure time into `_deps/`).

```console
git clone https://github.com/voidscape-development/echo
cd echo
cmake --preset ubuntu-x86_64
cmake --build --preset ubuntu-x86_64
```

See `CMakePresets.json` for the Windows and macOS presets. On Linux, install the JUCE
build dependencies first:

```console
sudo apt install qt6-base-dev libasound2-dev libfreetype-dev libfontconfig1-dev \
  libxrandr-dev libxinerama-dev libxcursor-dev libgtk-3-dev
```

## Repository layout

| Path | Contents |
|---|---|
| `src/plugin-main.cpp` | Module entry point, Tools menu settings dialog |
| `src/core/device_io.cpp`, `src/core/device_io2.cpp` | OBS filter definitions and property UI |
| `src/core/atkaudio/DeviceIo*/` | The filters' audio processing and settings windows |
| `src/core/atkaudio/ModuleInfrastructure/` | Device broker, synchronised buffers, OBS↔JUCE device bridge |
| `src/core/atkaudio/` (rest) | JUCE runtime lifecycle, message pump, logging, settings |

Everything under `src/core/` is vendored from upstream and deliberately kept close to its
original form (including its own `.clang-format`) so upstream fixes stay easy to diff in.

## Credits

Echo is derived from the [atkAudio Plugin for OBS](https://github.com/atkAudio/PluginForObsRelease)
by atkAudio, used under the terms of the AGPLv3. The Device IO filters, the audio device
server and the OBS↔JUCE bridge are atkAudio's work; this repository packages them as a
standalone plugin. If you find Echo useful, please consider
[supporting atkAudio](https://www.paypal.com/donate/?hosted_button_id=ERBKC76F55HZW).

Built with the [JUCE framework](https://juce.com/) and based on the
[OBS plugin template](https://github.com/obsproject/obs-plugintemplate).

## License

[GNU Affero General Public License v3.0](LICENSE), inherited from upstream.

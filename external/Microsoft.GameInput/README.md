# GameInput  (https://aka.ms/gameinput)

GameInput is a next-generation input API that exposes input devices of all kinds through a single consistent interface. It's designed with a simple programming model that makes it easy to use. GameInput is built from the ground up for the best possible performance. Key features of the GameInput API are as follows.

## Overview

*   Availability:

    GameInput is available on Xbox via the GDK and PC via NuGet. Older versions of Windows going back to Windows 10 19H1 (May 2019 update) are supported.

*   Consistency:

    GameInput exposes input from keyboards, mice, gamepads, and other game controllers via a single unified input model, synchronized to a common time base. The code for handling input from these devices is nearly identical, using many of the same functions but with different filters applied. This makes it easy to add support for additional input devices, without major changes to input code.

*   Functionality:

    GameInput is a functional superset of all legacy input APIs—XInput, DirectInput, Raw Input, Human Interface Device (HID), and WinRT APIs—in addition to adding new features of its own. GameInput's functionality ranges from simple fixed-format gamepad state to detailed low-level raw device access. Input can be obtained via polling or callbacks in an event-driven way. Haptics and force feedback are fully supported, and third-party device SDKs can easily be written on top of GameInput to provide access to custom device features.

*   Performance:

    GameInput is built around an entirely new direct memory access (DMA) architecture for the lowest possible input latency and resource usage. Nearly all API functions are lock-free with strict performance guarantees, while still being 100 percent thread-safe. This makes them safe to call from time-sensitive contexts such as render threads. Advanced applications can take direct control of scheduling GameInput's internal asynchronous work queues, controlling which thread does the work and how often.

*   Usability

    GameInput was designed with ease of use as a top priority. Most common input tasks can be implemented with just a few lines of code.

## Companion helpers

* [DualSense helper](companion/DualSense/README.md) — header-only helper for sending raw DualSense output reports through GameInput.

## Version History

### Version 4.0

*   Added support for device lighting.
*   Added support for touch devices.
*   Added support for Nintendo&#174; Switch Pro&#174; controllers.
*   Added support for viewing device XInput indices (where applicable).
*   Added new labels for axes, primary and secondary fire, and paddle label fields.
*   Added LSB/RSB button flags and corresponding labels to GameInputArcadeStickInfo.
*   Added side-by-side binaries for Windows and Xbox to the NuGet package.
*   Grouped various IGameInputDevice callbacks into a single event notification.
*   Replaced GameInputDeviceInfo and related structures with getters on IGameInputDevice.
*   Added limited support for x86 architecture.
*   Improved support for devices in remote access applications.
*   Improved support for rumble on HID devices.
*   Stability and performance improvements.
*   Increased API version to 4.

"Switch Pro" is a registered trademark or trademark of Nintendo of America Inc.

### Version 3.5

*   Added support for Agility SDK style side-by-side deployment.
*   Added support for XInput and background GIP raw device reports.
*   Added companion header for PlayStation&#174;5 DualSense&#174; features (see https://github.com/microsoftconnect/GameInput).
*   Updated public header and static library (lib) license to MIT.
*   Fixed issues with loss of input after suspend/resume on Xbox.
*   Fixed missing display names for input devices on Xbox.
*   Fixed PlayStation&#174;5 DualSense Edge&#174; input duplication for some Windows.Gaming.Input games.
*   Stability and performance improvements.

"PlayStation" is a registered trademark or trademark of Sony Interactive Entertainment Inc.

"DualSense Edge" is a registered trademark or trademark of Sony Interactive Entertainment Inc.

### Version 3.4

*   Added support for raw HID reports.
*   Fixed keyboard layout changes not being detected.
*   Fixed compatibility issue with accessing readings for disconnected devices.
*   Stability and performance improvements.

### Version 3.3

*   Added support for controller paddles.
*   Added support for PlayStation&#174;5 DualSense&#174; gyroscope.
*   Added support for automatic device grouping by PNP container ID.
*   Added native support for ARM.
*   Fixed a bug where the original Xbox One Elite controller reported as multiple devices.
*   Improved controller support for console/terminal applications.
*   Enable CFG for exception handling continuations.
*   Stability and performance improvements.
*   Fixed an issue on some devices where sensor axis orientation was incorrect, causing accelerometer and gyrometer values to be rotated. 
*   Previously, this NuGet package only impacted x64 builds of a project. Starting with this version, referencing this package will now impact console builds as well. This package will override the GameInput version distributed with the GDK and use the new GameInput runtime on console.  

**NOTE:** For this and future GameInput NuGet releases, if you also build for console, GDK 2510 or newer is required to use this package.  If your console build requires a GDK older than 2510, do not upgrade to this or future NuGet packages.

**NOTE:** The PC redistributable included in this and future packages can and should always be packaged with your title if you use GameInput on PC.

"PlayStation" is a registered trademark or trademark of Sony Interactive Entertainment Inc.

"DualSense" is a registered trademark or trademark of Sony Interactive Entertainment Inc.

### Version 3.2

*   Added new device registry mappings.
*   Added limited protection for applications that mismanage reference counts.
*   Fixed compatibility issue for the `GAMEINPUT_CURRENT_CALLBACK_TOKEN_VALUE` value.
*   Removed implicit dependency on `initguid.h` for GUID constants.
*   Stability and performance improvements.

### Version 3.1

*   Fixed incorrect button, axis, and switch counts reported in v0 - v2 of the API.
*   Compatibility improvements for applications relying on `GameInputBlockingEnumeration`.
*   Compatibility improvements for wireless devices.
*   Stability and performance improvements.

### Version 3.0

*   Added support for aggregate devices.
*   Added support for querying device input mappings.
*   Added support for querying gamepads of varying capabilities.
*   Added support for GIP raw device reports in the latest header.
*   Added VID/PID values for game controllers on Linux-based systems.
*   Added `GameInputInitialize` for specifying API version during creation.
*   Added ability to query unmapped inputs for accessing extra axes and buttons.
*   Added some mapped gamepad buttons for flight sticks and racing wheels.
*   Removed support for UI navigation; it is still available via old headers.
*   Fixed VID/PID not being reported correctly for some wireless controllers.
*   Fixed performance regression for Xbox One controllers on Windows 10.
*   Fixed hardware and firmware version not being exposed via old headers.
*   Stability and performance improvements.
*   Increased API version to 3.

### Version 2.2

*   Fixed mice and keyboards not being detected after being unplugged and re-plugged into same port.
*   Fixed issue which caused rumble to become unresponsive on certain devices when under load.
*   Fixed background input not working on gamepads plugged in after the input policy is specified.
*   Fixed haptic devices not getting detected when plugged in after application start.
*   Updated `GameInput.lib` for improved cross-platform support; source code is now also included.
*   Miscellaneous plug and play reliability improvements for HID and GIP devices.

### Version 2.1

*   Fixed force feedback not working correctly in applications with multiple windows.
*   Fixed missing mouse and keyboard readings in applications using RawInput outside of GameInput.
*   Fixed the `supportedSystemButtons` field not being correctly populated for some devices.
*   Removed unnecessary `OutputDebugString` logging.
*   Improved support for UWP applications.
*   Stability and performance improvements.

### Version 2.0

*   Added support for advanced haptics for audio-driven devices including the PlayStation&#174;5 DualSense&#174; controller.
*   Added support for sensors supported by the Windows sensor stack.
*   Added support for DirectInput driver `DirectInputEscape` call.
*   Added support for running GameInput in the WINE environment on Linux-based systems.
*   Added support for optional background gamepad input across gamepad drivers.
*   Added `revisionNumber`, `hardwareVersion`, `firmwareVersion`, and `containerId` to `GameInputDeviceInfo`.
*   Fixed GIP devices reporting incorrect PNP paths in `GameInputDeviceInfo`.
*   Fixed an issue where GIP devices would not report raw readings in older versions of the API.
*   Increased API version to 2.

"PlayStation" is a registered trademark or trademark of Sony Interactive Entertainment Inc.

"DualSense" is a registered trademark or trademark of Sony Interactive Entertainment Inc.

### Version 1.2

*   Fix inverted axis mapping on multiple controllers.
*   Fix two second timeout on process shutdown for games that leak the `IGameInput` instance.
*   GameInput will now automatically reconnect to the GameInput service when it is upgraded.
*   Increase the callback backlog to provide more time for games to process GameInput notifications.

### Version 1.1

*   Applications no longer have to call `CoInitialize`.
*   Fixed missing device disconnect notifications.
*   Fixed `FindDeviceFromId` method not returning requested device.
*   Fixed compiler compatibility issues in header.
*   Stability and performance improvements.

### Version 1.0

*   Added support for reading callbacks.
*   Added support for trackpads.
*   Added support for absolute mouse coordinates.
*   Added support for remote sessions.
*   Exposed additional device information, including PnP path and display name.
*   Removed deprecated APIs, fields, and constants.

While this version of GameInput is applicable to PC only, if you are using a previous version of GameInput on PC, or sharing code with GameInput on Xbox using the GDK, please note that many unimplemented functions and corresponding enums and constants have been removed. Additionally, the API has been placed in the `GameInput::v1` namespace to facilitate versioning. Future versions of GameInput will be in their own versioned namespaces. Due to these changes, you may experience compilation errors when building that code using this (and future) versions. Notable changes include:

1. `IGameInputDevice::GetDeviceInfo` previously returned the resulting `IGameInputDeviceInfo` struct as the function's return value. This struct is now returned as an out parameter on the function, and the function's return value is now an `HRESULT`.

2. `IGameInput::UnregisterCallback` previously took a timeout value as its second parameter, however this parameter has been removed. In these two cases, along with the touch API used for XCloud on console, use the `GAMEINPUT_API_VERSION` define to conditionally compile (or exclude) code between PC and console.

3. `IGameInputReading::GetSequenceNumber` has been removed. Please use `IGameInputReading::GetTimestamp` instead.

Example:

```cpp
#include <GameInput.h>
#ifndef GAMEINPUT_API_VERSION
#define GAMEINPUT_API_VERSION 0
#endif

#if GAMEINPUT_API_VERSION == 1
using namespace GameInput::v1;
#elif GAMEINPUT_API_VERSION == 2
using namespace GameInput::v2;
#elif GAMEINPUT_API_VERSION == 3
using namespace GameInput::v3;
#endif

// ...

#if GAMEINPUT_API_VERSION >= 2
    device->GetDeviceInfo(&deviceInfo);
#else
    deviceInfo = device->GetDeviceInfo();
#endif
```

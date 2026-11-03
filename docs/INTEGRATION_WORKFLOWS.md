# Host integration workflows

This repository can continuously build the portable library for Linux, Windows and Android arm64. Host application workflows must run in the host repositories because only those repositories own their Gradle/CMake projects, SDK pins, signing configuration and DXVK fork revisions.

Templates are in `integrations/workflows/`. Copy the matching file into the host repository's `.github/workflows/` directory and adjust the documented source paths/build command to that repository. The templates intentionally do not claim to build a complete host until the concrete adapter sources are integrated there.

## Skyrim_Android

Use `skyrim-android.yml` after adding the SnapFrame CMake target to the app's native build and wiring the DXVK hooks. It builds the Android app with the host Gradle wrapper. Keep signing secrets out of pull-request workflows.

## Winlator

Use `winlator-dxvk.yml` only after selecting the exact Winlator DXVK fork/ref and adding the adapter target to its native build. The workflow must build the same fork used by the runtime; upstream DXVK success is not a substitute.

## WinNative

Use `winnative.yml` after implementing the WinNative compositor adapter and adding its CMake target. The build must use the host's supported Windows SDK, architecture and Vulkan loader/runtime assumptions.

## CI security

- Pin third-party actions to reviewed major versions or immutable SHAs according to host policy.
- Do not store game files, APK signing keys or personal device logs in public artifacts.
- Upload only build outputs and sanitized test logs.
- Keep hardware/device validation as a separate self-hosted or manual test stage.

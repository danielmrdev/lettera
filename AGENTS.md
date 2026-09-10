# Project Context

Lettera is a focused plain-text writing surface for Arch Linux running Omarchy v4. The app uses C++17 with Qt 6 Qt Quick/QML, and integrates with Omarchy's theme, text scale, and desktop portals.

## Tooling

- Build system: qmake via `lettera.pro`; use `qmake6` for manual qmake commands.
- Build: `./bin/build`
- Test: `./bin/test` (QtTest, using the offscreen platform)
- QML lint: `qmllint src/Main.qml src/SettingsPanel.qml`
- Arch package build/install: `./bin/install` (runs `makepkg -fsi`; run only when explicitly requested).

## Guidance

- Keep the supported runtime Omarchy v4 on Arch Linux. Preserve plain UTF-8 `.txt` editing, atomic saves, autosave/recovery, external-change handling, system file dialogs, and live Omarchy theme integration.
- Keep the C++/QML boundary clear: C++ owns document, file, persistence, and system integration; QML owns presentation and interaction. Keep settings validation and persistence in `WritingSettings`, not in QML.
- After C++ or QML changes, run `./bin/build` and `./bin/test`. Avoid adding support for platforms or document formats outside the Omarchy plain-text scope.

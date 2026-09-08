# Omawrite

A focused writing surface for Omarchy v4.

Omawrite is a small, open-source plain-text writer for Arch Linux systems running Omarchy v4. It is intentionally not a general cross-platform editor: the supported environment is Omarchy's Arch, Hyprland, Quickshell, XDG desktop portal, and system theme.

## Behaviour

- Writes ordinary UTF-8 `.txt` files.
- Saves an opened file automatically after a short idle pause.
- Uses atomic replacement for saves.
- Recovers edits after an abnormal exit.
- Watches the active file and warns about external changes.
- Opens files through the system XDG file dialog, filtered to `.txt`.
- Remembers the last directory used for opening files.
- Follows the active Omarchy theme and text scale.
- Opens directly into the writing surface without an in-app toolbar or title bar.

`Ctrl+O` opens a text file. `Ctrl+S` forces an immediate save. `Ctrl+Shift+S` chooses a new text file.

## Writing preferences

`Ctrl+,` toggles the settings panel. `Esc` closes it and returns focus to your text.

- Choose an installed font (default: bundled iA Writer Mono S).
- Set font size (12–40 logical px), maximum writing width (480–1200 logical px), and line height (1.00–2.20×).
- Follow Omarchy's text scale, or use your chosen size without the system multiplier.
- Optionally show a word count, hidden by default.
- Reset writing preferences without changing remembered folders or window geometry.

Preferences persist locally and apply immediately, **except line height**, which applies when you next open a document or window. The panel shows pending line-height changes. This preserves the current document's undo/redo history; Qt treats live paragraph spacing changes as document edits.

These settings never change `.txt` contents. Autosave stays enabled and colours continue to follow Omarchy. Long documents scroll within the writing surface.

## Scope

This project is deliberately Omarchy-specific. It does not promise support for macOS, Windows, other Linux distributions, Markdown, rich text, databases, cloud sync, accounts, plugins, or document hierarchies.

## Build

Dependencies:

- Arch Linux / Omarchy v4
- Qt 6: `qt6-base`, `qt6-declarative`, `qt6-quickcontrols2`
- `xdg-desktop-portal` and an active portal backend

```sh
./bin/build
./bin/test
```

## Origin

This repository is an independent copy of [omacom-io/omawrite](https://github.com/omacom-io/omawrite), not a fork. The copied upstream revision is recorded in the initial commit message.

## License

MIT. The bundled iA Writer Mono font remains under its original SIL Open Font License 1.1; see `fonts/OFL.txt`.

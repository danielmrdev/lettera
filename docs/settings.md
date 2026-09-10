# Writing settings

## Approved scope (2026-09-08)

Extend the independent Omawrite copy, not the pre-pivot application. Keep plain UTF-8 `.txt`, autosave, system file dialogs, and Omarchy colours. No toolbar or title bar.

- Right-hand modal settings panel; Ctrl+, toggles, Escape closes and restores editor focus. Ctrl+N starts a new document; unsaved changes get Save/Discard/Cancel.
- Installed font picker plus bundled iA Writer Mono S; size 12–40 logical px, maximum writing width 480–1200 logical px, proportional line height 100–220%.
- Optional system text scale (default on), optional word count (default off), scoped reset.
- Local persistence via the existing QSettings identity, isolated under `writing/`.
- Apply preferences immediately without changing text, triggering saves or damaging undo/redo.
- Explicitly approved exception: line height applies on next document/window open. Show pending state. Existing and newly inserted paragraphs retain the current document's active line height until then.

## Implementation boundary

`WritingSettings` owns validation, defaults and persistence. `SettingsPanel.qml` owns controls; `Main.qml` binds presentation preferences and handles editor focus/shortcuts. `Backend` snapshots line height during its existing document typography pass. Live settings never call that pass.

Qt 6 TextArea has no public presentation-only line-height property. `QTextCursor::mergeBlockFormat` creates undoable commands; disabling undo clears history. Avoid both for live preferences. A custom text layout is outside this change.

## Verification

- `./bin/build`
- `./bin/test`: defaults, persistence, reset isolation, invalid values, actual QML loading, keyboard panel toggle/Escape, slider keyboard change, narrow window, long-document overflow and cursor visibility, font/system scaling, no QML warnings, undo/redo preservation, unchanged file bytes, idle autosave, pending line height and new paragraphs.
- Optional screenshot: `LETTERA_TEST_SCREENSHOT=/tmp/lettera-settings.png ./bin/test`.

### Result (2026-09-08)

Build passed; Qt 6.11.2 offscreen suite: 10 passed, 0 failed, including the long-document scrolling check. `git diff --check` passed. Independent standards review found no issues; spec review requested an overflow test, now added and passing. Screenshot captured at `/tmp/lettera-settings.png` (temporary, reproducible via command above). Build logs include a GCC 16 warning in Qt's `qbitarray.h`, not an application warning; no QML engine warnings occurred.

## Follow-up

Native Hyprland visual acceptance remains a manual check, particularly at large Omarchy interface scales. A future improvement is a dedicated QML visual regression harness; source-string tests alone cannot validate editor behaviour. Live line height would require a separate design decision, not silently clearing undo history.

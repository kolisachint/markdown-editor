# Omawrite

A dead-simple Markdown writing app built with Qt Quick and C++ that automatically follows system dark/light mode.

<img width="2948" height="3227" alt="screenshot-2026-06-23_15-24-08" src="https://github.com/user-attachments/assets/4e930c0d-edda-4046-b444-a59eff523329" />
<img width="2948" height="3227" alt="screenshot-2026-06-23_15-23-23" src="https://github.com/user-attachments/assets/8ced7c26-961b-4ded-b263-84403001a951" />


## Install

### Linux

Install via the Omarchy Package Repository via the `omawrite` package. It's installed by default in new installations of Omarchy (from Quattro forward).

### macOS

Download `Omawrite-macos.zip` from the [releases](https://github.com/kolisachint/markdown-editor/releases), unzip it and move `Omawrite.app` to Applications. It is ad-hoc signed, so the first launch may need a right-click → Open.

To build it yourself you need Qt 6 (`brew install qt`):

```sh
./bin/test         # run the test suite
./bin/build-macos  # build build/Omawrite.app
```

## Shortcuts

On Linux these are `Ctrl` chords; on macOS Qt maps `Ctrl` to Command, so the
same chords are `⌘`. The in-app reference (`⌘?` / `Ctrl+?`) shows whichever
applies.

- `Ctrl+S` saves. Unsaved documents use the XDG desktop portal file picker.
- `Ctrl+Shift+S` saves as.
- `Ctrl+O` opens a Markdown file through the portal picker.
- `Ctrl+P` opens the system print dialog.
- `Ctrl+N` opens a new Omawrite window.
- `Ctrl+Z`, `Ctrl+Shift+Z`, and `Ctrl+Y` handle undo and redo.
- `Super+F` toggles fullscreen. Qt maps this key as `Meta+F`, which is `Ctrl+F` on macOS.
- `Ctrl+F` searches the document. Use `Enter` or `Ctrl+G` for the next match and `Shift+Enter` for the previous match.
- `Ctrl+H` opens find and replace.
- `Ctrl+B`, `Ctrl+I`, and `Ctrl+K` insert bold, italic, and link Markdown.
- `Ctrl+?` shows the keyboard shortcut reference.

On macOS the file dialogs and the print dialog are the native ones, and double
clicking a `.md` file in Finder opens it in Omawrite.

Unsaved drafts are recovered after an abnormal exit. Omawrite also watches open files
and warns before an external change can replace local work.

The font button in the footer picks the writing font from any installed text font,
and Omawrite remembers the choice. IBM Plex Mono is the default.

Text follows the desktop text size — `omarchy display text size`, or GNOME's
`text-scaling-factor` — and re-flows without a restart. The default of 12px leaves
Omawrite at the size it is designed around; larger and smaller sizes scale from there.

## Requirements

- Qt 6.5 or newer: `qtbase`, `qtdeclarative`, `qtquickcontrols2`. The 6.5
  floor comes from `QStyleHints::colorScheme()`, which reads the system
  appearance on macOS as well as on Linux.
- `xdg-desktop-portal` and a portal backend, on Linux

The IBM Plex Mono font is bundled under the SIL Open Font License 1.1; see
`fonts/OFL.txt`. The font is copyright IBM Corp.

## Porting to another platform

The app asks the desktop how it looks through one seam, `src/platformtheme.h`,
which answers four questions: dark mode, apparent text size, the desktop's own
palette, and what to watch for changes. Nothing else in the app checks the
platform, so a new OS is a packaging job unless its desktop reports something
Qt does not already expose.

`src/platformtheme.cpp` answers all four from Qt's style hints, which is enough
on any desktop: on macOS Qt reads the system Appearance setting on its own, and
text size stays at 1.0 because only GNOME has a desktop-wide knob. A platform
that reports nothing more — Windows, for instance — therefore needs no C++ at
all.

A desktop that does report more needs one file, `src/platformtheme_<os>.cpp`,
defining `darkMode`, `textScale`, `palette`, `palettePaths` and
`watchPortalSettings`. `src/platformtheme_linux.cpp` is the worked example: the
XDG portal for dark mode and text size, and the omarchy theme for the colors.
Add it to `omawrite.pri` under a scope for the OS, and guard the two blocks in
`platformtheme.cpp` that it replaces — the one there is guarded with
`#ifndef Q_OS_LINUX` — so only one copy is ever compiled.

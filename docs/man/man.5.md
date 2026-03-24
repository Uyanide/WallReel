---
title: WALLREEL
section: 5
header: File Formats Manual
footer: WallReel 2.0.2
date: 2026-03-24
---

# NAME

wallreel-config - configuration format for wallreel

# SYNOPSIS

`~/.config/wallreel/config.json`

# DESCRIPTION

WallReel reads configuration from a JSON document. The root object is divided into
five sections:

- `wallpaper`
- `theme`
- `action`
- `style`
- `cache`

For complete machine-readable validation details, refer to `config.schema.json`.

# WALLPAPER SECTION

Defines where WallReel looks for images and what to exclude.

If both `paths` and `dirs` are empty or omitted, WallReel defaults to recursively
scanning the user's Pictures directory and treating all supported image files as
wallpaper candidates.

`paths` (array of string, default: `[]`)
: Exact paths to specific image files.

`dirs` (array of object, default: `[]`)
: Directories to scan for images.

Each item has:

- `path` (string)
- `recursive` (boolean)

`excludes` (array of string, default: `[]`)
: Exclude patterns as regular expressions.

# THEME SECTION

Configures color palettes.

A dominant color is extracted from each wallpaper. If a palette is selected,
WallReel picks the closest palette color as the primary color.

`palettes` (array of object, default: `[]`)
: Custom palette definitions.

Each palette has:

- `name` (string)
- `colors` (array)

Each color item has:

- `name` (string)
- `value` (hex string, for example `"#89b4fa"`)

# ACTION SECTION

Configures commands executed for preview, selection, and restore behavior.

`previewDebounceTime` (integer, default: `300`)
: Debounce interval in milliseconds for preview actions.

`printSelected` (boolean, default: `true`)
: Print selected wallpaper path to stdout on confirmation.

`printPreview` (boolean, default: `false`)
: Print previewed wallpaper path to stdout on preview.

`onSelected` (string, default: `""`)
: Command executed when a wallpaper is confirmed.

`onPreview` (string, default: `""`)
: Command executed when a wallpaper is previewed.

`saveState` (array of object, default: `[]`)
: Commands for capturing system values before changing wallpaper.

Each item has:

- `key` (placeholder key)
- `fallback` (default value)
- `command` (stdout-mapped command)
- `timeout` (milliseconds)

`onRestore` (string, default: `""`)
: Command executed on restore. Saved state keys are usable as placeholders.

`quitOnSelected` (boolean, default: `false`)
: Exit application immediately after confirming a selection.

`restoreOnClose` (boolean, default: `true`)
: Run `onRestore` when application closes without a final selection.

## ACTION PLACEHOLDERS

The following placeholders are available in `onSelected`, `onPreview`, and
`onRestore` (where applicable):

`{{ path }}`
: Full path of selected or previewed wallpaper.

`{{ name }}`
: File name of selected or previewed wallpaper.

`{{ size }}`
: Size in bytes of selected or previewed wallpaper.

`{{ palette }}`
: Selected palette name (`"null"` if none).

`{{ colorName }}`
: Chosen primary color name (`"null"` if none).

`{{ colorHex }}`
: Chosen primary color hex (`"null"` if none).

`{{ domColorHex }}`
: Dominant color hex extracted from the wallpaper.

`{{ <key> }}`
: Value of a saved state item with matching key.

# STYLE SECTION

Controls window layout and thumbnail dimensions.

`image_width` (integer, default: `320`)
: Width of each thumbnail.

`image_height` (integer, default: `180`)
: Height of each thumbnail.

`image_focus_scale` (number, default: `1.5`)
: Focus scale multiplier for highlighted thumbnail.

`window_width` (integer, default: `750`)
: Initial window width.

`window_height` (integer, default: `500`)
: Initial window height.

# CACHE SECTION

Controls persisted UI state.

`saveSortMethod` (boolean, default: `true`)
: Persist sort method and direction.

`savePalette` (boolean, default: `true`)
: Persist selected palette.

`maxImageEntries` (integer, default: `1000`)
: Maximum number of image cache entries. Older entries are evicted.

# EXAMPLE

```json
{
  "$schema": "https://raw.githubusercontent.com/Uyanide/WallReel/refs/heads/master/config.schema.json",
  "wallpaper": {
    "paths": ["/home/user/Pictures/favorite.jpg"],
    "dirs": [
      {
        "path": "/home/user/Pictures/Wallpapers",
        "recursive": true
      }
    ],
    "excludes": ["\\.gif$"]
  },
  "theme": {
    "palettes": [
      {
        "name": "Dark",
        "colors": [
          { "name": "blue", "value": "#89b4fa" },
          { "name": "red", "value": "#f38ba8" }
        ]
      }
    ]
  },
  "action": {
    "previewDebounceTime": 500,
    "quitOnSelected": true,
    "onPreview": "swww img {{ path }}",
    "onSelected": "cp {{ path }} ~/.config/wallpaper/current/ && swww img {{ path }}",
    "saveState": [
      {
        "key": "current_wp",
        "fallback": "/home/user/Pictures/default.jpg",
        "command": "find ~/.config/wallpaper/current -type f | head -n 1",
        "timeout": 1000
      }
    ],
    "onRestore": "swww img {{ current_wp }}"
  },
  "style": {
    "image_width": 640,
    "image_height": 400,
    "image_focus_scale": 1.2,
    "window_width": 1280,
    "window_height": 720
  },
  "cache": {
    "saveSortMethod": true,
    "savePalette": true,
    "maxImageEntries": 300
  }
}
```

# SEE ALSO

**wallreel**(1)

# AUTHOR

Uyanide <github.com/Uyanide>

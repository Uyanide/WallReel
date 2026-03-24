---
title: WALLREEL
section: 1
header: User Commands
footer: WallReel 2.0.2
date: 2026-03-24
---

# NAME

wallreel - Choose and set desktop wallpapers with customizable themes and actions

# SYNOPSIS

**wallreel** [*options*]

# DESCRIPTION

**wallreel** is a Qt6 application for browsing wallpaper images, previewing candidates,
and applying a selected image.

Configuration is loaded from a JSON file. CLI options are available for logging,
one-shot operations, and runtime overrides.

# OPTIONS

**-h, --help**
: Display help for command-line options.

**-v, --version**
: Display version information.

**-V, --verbose**
: Set log level to DEBUG (default is INFO).

**-C, --clear-cache**
: Clear image cache and exit.

**-q, --quiet**
: Suppress log output.

**-d, --append-dir** _dir_
: Append an additional wallpaper search directory.

This option can be provided multiple times.

**-c, --config-file** _file_
: Use a custom configuration file.

**-D, --disable-actions**
: Disable actions defined in the configuration file.

**-a, --apply** _file_
: Apply the specified image as wallpaper and exit.

In this mode, the configuration is still parsed. Action placeholders are resolved
from the selected image and any captured state values.

# BEHAVIOR NOTES

- CLI options are generally optional; configuration is the preferred customization path.
- Some options are mutually exclusive (for example `--verbose` and `--quiet`).
- With `--apply`, WallReel executes configured selection actions without opening the UI.

# FILES

`~/.config/wallreel/config.json`
: Default configuration file location.

`~/.cache/wallreel/`
: Runtime cache location.

# EXAMPLES

Run with default configuration:

```bash
wallreel
```

Use a custom configuration file:

```bash
wallreel --config-file ~/.config/wallreel/config.json
```

Append additional search directories:

```bash
wallreel --append-dir ~/Pictures/Wallpapers --append-dir ~/Art
```

Apply a wallpaper and exit:

```bash
wallreel --apply ~/Pictures/wallpaper.jpg
```

# EXIT STATUS

Returns `0` on success. Returns a non-zero value on failure.

# SEE ALSO

**wallreel**(5)

# AUTHOR

Uyanide <github.com/Uyanide>

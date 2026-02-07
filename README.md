## What is this

It might not be that worthy to write a QtWidget application for such a small feature, but I kind of enjoy the pain... So here it is.

<img src="https://github.com/Uyanide/backgrounds/blob/master/screenshots/desktop-alt.jpg?raw=true"/>

## How to build

1. Make sure you have Qt6 libraries, CMake and a C++ compiler installed.

   e.g. On Arch-based systems:

   ```bash
   sudo pacman -S --needed qt6-base cmake gcc
   ```

2. Clone the repository:

   ```bash
   git clone https://github.com/Uyanide/Wallpaper_Carousel.git --depth 1 && \
   cd Wallpaper_Carousel
   ```

3. Run [build script](app/install.sh):

   ```bash
   app/install.sh
   ```

   or if you prefer a prefix other than `/usr/local`, e.g. `$HOME/.local`:

   ```bash
   PREFIX=$HOME/.local ./app/install.sh
   ```

> [!Warning]
>
> This script will ask for root permissions if the prefix is set to a system directory like `/usr/local`. Please make sure you have read and trust the script before proceeding.

## How to use

The config file should be placed in `~/.config/wallpaper-carousel/config.json`. Refer to [config.example.json](config.example.json) and [config.h](src/config.h) for specific entries.

A minimum config should at least contain the path(s) to wallpapers, e.g.

```json
{
  "wallpaper": {
    "dirs": ["/path/to/your/wallpapers"]
  }
}
```

By default, the path of the selected wallpaper will be output to stdout. If you want to apply the selected wallpaper automatically after selection, the `action.confirm` entry should be set, e.g.

```json
{
  "wallpaper": {
    "dirs": ["/path/to/your/wallpapers"]
  },
  "action": {
    "confirm": "awww img \"%1\""
  }
}
```

`action.confirm` should be a executable followed by a couple of arguments, where `%1` will be replaced by the path of the selected wallpaper.

## CLI

```
Usage: wallpaper-carousel [options]

Options:
  -h, --help                Displays help on commandline options.
  -v, --version             Displays version information.
  -V, --verbose             Set log level to DEBUG (default is INFO)
  -q, --quiet               Suppress all log output
  -d, --append-dir <dir>    Append an additional wallpaper search directory
  -c, --config-file <file>  Specify a custom configuration file
```

A few things to notice:

- It's generally not necessary to provide any CLI arguments, I would recommend using the config file to customize the behavior instead. However, it is still possible to control some essential options via CLI.

- All logs are directed to stderr by default. Only the full path of the selected wallpaper (if any) will be sent to stdout. This allows easy piping of the output to other programs.

- The `--append-dir` option can be used multiple times to add multiple directories.

- It is quite obvious that some options are conflicting with each other (e.g. `--verbose` and `--quiet`). If mutually exclusive options are provided together, the behavior is undefined and can be changed without notice in future versions.

- Paths passed via CLI options are tested before any further operation is performed. That is to say, if an invalid path is provided, the program will exit with an error before any further action, and you won't even have a chance to see a window.

  On the contrary, paths provided in the config file are only tested when they are actually used (e.g. when searching for wallpapers). And most errors will be ignored silently (with a warning log).

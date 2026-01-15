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
   git clone https://github.com/Uyanide/Wallpaper_Carousel.git --depth 1
   cd Wallpaper_Carousel
   ```

3. Run build script:

   ```bash
   app/install.sh
   ```

   or if you prefer a prefix other than `/usr/local`, e.g. `$HOME/.local`:

   ```bash
   PREFIX=$HOME/.local ./app/install.sh
   ```

> [!NOTE]
>
> This script will ask for `sudo` permission if the prefix is set to a system directory like `/usr/local`. Please make sure you have read and trust the script before proceeding.

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

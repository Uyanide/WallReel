#!/bin/env bash

path="$(dirname "$(readlink -f "$0")")"

cmake -S "$path/.." -B "$path/../build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$HOME"/.local || exit 1

cmake --build "$path/../build" --target install || exit 1

cp "$path/wallpaper-carousel.desktop" "$HOME"/.local/share/applications/wallpaper-carousel.desktop

echo "Exec=$HOME/.local/bin/wallpaper-carousel" >>"$HOME"/.local/share/applications/wallpaper-carousel.desktop

if command -v update-desktop-database &>/dev/null; then
    update-desktop-database "$HOME"/.local/share/applications/
fi


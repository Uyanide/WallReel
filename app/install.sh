#!/bin/env bash

set -euo pipefail

prefix=${PREFIX:-/usr/local}

path="$(dirname "$(readlink -f "$0")")"

cmake -S "$path/.." -B "$path/../build" \
    -DCMAKE_INSTALL_PREFIX="$prefix"

cmake --build "$path/../build"

if [ ! -w "$prefix" ] && [ "$(id -u)" -ne 0 ]; then
    sudo cmake --install "$path/../build"
else
    cmake --install "$path/../build"
fi

if command -v update-desktop-database &>/dev/null; then
    echo "Updating desktop database..."
    update-desktop-database "$prefix"/share/applications/ || true
fi

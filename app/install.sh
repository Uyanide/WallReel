#!/usr/bin/env bash

set -euo pipefail

prefix=${PREFIX:-/usr/local}

path="$(dirname "$(readlink -f "$0")")"

cmake -S "$path/.." -B "$path/../build" \
    -DCMAKE_INSTALL_PREFIX="$prefix"

cmake --build "$path/../build" --config Release -- -j"$(nproc)"

if [ ! -w "$prefix" ] && [ "$(id -u)" -ne 0 ]; then
    echo "Elevated permissions are required to install to $prefix, enter root's password to continue."
    su -m -c "cmake --install '$path/../build' --config Release" root
else
    cmake --install "$path/../build" --config Release
fi

#ifndef WALLREEL_PALETTES_PREDEFINED_HPP
#define WALLREEL_PALETTES_PREDEFINED_HPP

#include "data.hpp"

// License of Catppuccin - MIT
/*
MIT License

Copyright (c) 2021 Catppuccin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

namespace WallReel::Core::Palette {

inline const QList<PaletteItem> preDefinedPalettes = {

    {
        .name   = "Catppuccin Latte",
        .colors = {

            {"rosewater", "#dc8a78"},
            {"flamingo", "#dd7878"},
            {"pink", "#ea76cb"},
            {"mauve", "#8839ef"},
            {"red", "#d20f39"},
            {"maroon", "#e64553"},
            {"peach", "#fe640b"},
            {"yellow", "#df8e1d"},
            {"green", "#40a02b"},
            {"teal", "#179299"},
            {"sky", "#04a5e5"},
            {"sapphire", "#209fb5"},
            {"blue", "#1e66f5"},
            {"lavender", "#7287fd"},
        },
    },
    {
        .name   = "Catppuccin Frappe",
        .colors = {
            {"rosewater", "#f2d5cf"},
            {"flamingo", "#eebebe"},
            {"pink", "#f4b8e4"},
            {"mauve", "#ca9ee6"},
            {"red", "#e78284"},
            {"maroon", "#ea999c"},
            {"peach", "#ef9f76"},
            {"yellow", "#e5c890"},
            {"green", "#a6d189"},
            {"teal", "#81c8be"},
            {"sky", "#99d1db"},
            {"sapphire", "#85c1dc"},
            {"blue", "#8caaee"},
            {"lavender", "#babbf1"},
        },
    },
    {
        .name   = "Catppuccin Macchiato",
        .colors = {
            {"rosewater", "#f4dbd6"},
            {"flamingo", "#f0c6c6"},
            {"pink", "#f5bde6"},
            {"mauve", "#c6a0f6"},
            {"red", "#ed8796"},
            {"maroon", "#ee99a0"},
            {"peach", "#f5a97f"},
            {"yellow", "#eed49f"},
            {"green", "#a6da95"},
            {"teal", "#8bd5ca"},
            {"sky", "#91d7e3"},
            {"sapphire", "#7dc4e4"},
            {"blue", "#8aadf4"},
            {"lavender", "#b7bdf8"},
        },
    },
    {
        .name   = "Catppuccin Mocha",
        .colors = {

            {"rosewater", "#f5e0dc"},
            {"flamingo", "#f2cdcd"},
            {"pink", "#f5c2e7"},
            {"mauve", "#cba6f7"},
            {"red", "#f38ba8"},
            {"maroon", "#eba0ac"},
            {"peach", "#fab387"},
            {"yellow", "#f9e2af"},
            {"green", "#a6e3a1"},
            {"teal", "#94e2d5"},
            {"sky", "#89dceb"},
            {"sapphire", "#74c7ec"},
            {"blue", "#89b4fa"},
            {"lavender", "#b4befe"},
        },
    },
};

}  // namespace WallReel::Core::Palette

#endif  // WALLREEL_PALETTES_PREDEFINED_HPP

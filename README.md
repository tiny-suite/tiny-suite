# tiny-suite

A collection of tiny, dependency-light tools that try to replace heavier software.
Every project lives in its own repository under the [tiny-suite](https://github.com/tiny-suite) organization.

## components

| project | what it is | replaces | repo |
| --- | --- | --- | --- |
| tinytile | tiny tiling wayland compositor | dwl / sway | [tiny-suite/tinytile](https://github.com/tiny-suite/tinytile) |
| tinyctl | control center for the suite (TUI + CLI) | manual scripts | [tiny-suite/tinyctl](https://github.com/tiny-suite/tinyctl) |
| tinyfetch | tiny system info | neofetch / fastfetch | [tiny-suite/tinyfetch](https://github.com/tiny-suite/tinyfetch) |
| tinytop | tiny process viewer | top / htop | [tiny-suite/tinytop](https://github.com/tiny-suite/tinytop) |
| tinymake | tiny build tool | make | [tiny-suite/tinymake](https://github.com/tiny-suite/tinymake) |
| tinyget | tiny HTTP client | curl | [tiny-suite/tinyget](https://github.com/tiny-suite/tinyget) |
| tinyvcs | tiny version control | git (for small projects) | [tiny-suite/tinyvcs](https://github.com/tiny-suite/tinyvcs) |

## registry

The [registry](registry) file maps each component to its repository. tinyctl uses it to find and install components.

## using the suite

Clone every component into one folder so tinyctl can discover them:

    git clone https://github.com/tiny-suite/tinytile
    git clone https://github.com/tiny-suite/tinyctl
    git clone https://github.com/tiny-suite/tinyfetch
    git clone https://github.com/tiny-suite/tinytop
    git clone https://github.com/tiny-suite/tinymake
    git clone https://github.com/tiny-suite/tinyget
    git clone https://github.com/tiny-suite/tinyvcs

Then:

    cd tinyctl
    make
    ./tinyctl          # interactive tui
    ./tinyctl list      # list components
    ./tinyctl doctor    # check dependencies
    ./tinyctl build tinytop

## building a single component

    git clone https://github.com/tiny-suite/tinytop
    cd tinytop
    make && sudo make install

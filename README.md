# wc-themegen

wc-themegen is a command-line GTK-3-based utile for managing Wine ui color palette.

Wine is a compatibility layer capable of running Windows applications on several POSIX-compliant operating systems, such as Linux, macOS, & BSD.



Usage wc-themegen:

--prefix, -p PATH - path to prefix

--not-run-winecfg, -w - not run winecfg after

--loader-dir, -l DIR- custom wine loader dir, e.g. "/opt/wine-staging/bin"

--set-default, -d - set default color theme

--main-color, -m COLOR - widgets background color in a hex triplet format, e.g. "#fa4500"

--highlight-color, -h COLOR - hilighted widgets background color 

--active-color, -a COLOR - active window title color 

--inactive-color, -i COLOR - inactive window title color

--text-color, -t COLOR - text color 

--contrast, -c VALUE - contrast value. May be from 0.1 to 2.0, default 1.0. 

--help, -? - print brief reference and quit



Usage wc-fast:

WC_LOADER=/some/wine/loader wc-fast WINE_ARGS

E. g.:

WC_LOADER=/opt/wine-staging/bin/wine WINEPREFIX=~/temp_wineprefix wc-fast winecfg

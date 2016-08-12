evhz
====

Display the current mouse refresh rate under linux with evdev.

For information about polling rate, see https://wiki.archlinux.org/index.php/Mouse_Polling_Rate

The program is just evhz.c. Compile and run:

    gcc -o evhz evhz.c
    sudo ./evhz

Nonverbose mode = -n

Maintainer: Ian Kelling <ian@iankelling.org>

Bugs, patches, requests, feedback are welcome.

Last tested 7/2016 on Debian testing. Still working great after several
years.

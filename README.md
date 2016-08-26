# evhz

Display the current mouse refresh rate under linux with evdev

For information about polling rate, see https://wiki.archlinux.org/index.php/Mouse_Polling_Rate

The program is just evhz.c. Compile and run:

    gcc -o evhz evhz.c
    sudo ./evhz

Nonverbose mode = -n

There exists a kernel bug for USB ports which use the uhci_hcd driver,
where a 1000 hz mouse will only read at 500hz, causing jittering:
https://bugzilla.kernel.org/show_bug.cgi?id=60586. A workaround is to
use USB ports that use the ehci-pci driver.

Maintainer: Ian Kelling <ian@iankelling.org>

Bugs, patches, requests, feedback are welcome.

Last tested 7/2016 on Debian testing. Still working great after several
years.

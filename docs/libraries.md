# Library Hacking
## Overview
Telink SDK provides three libraries which handle End Device, Router and Coordinator Zigbee functionality but each library provides an identically named interface.

We need to be able to run **both** Coordinator and End Device functionality in the same image but the standard libraries would cause a name clash and linking would fail.

[lief] appears to provide the answer as it should alllow us to create modified libraries where the three libraries now export differnet named functions e.g. function `zigbee_function()` can be exported as `en_sigbee_function()` and `rt_zigbee_function()` and `co_zigbee_function()` and now the linker will be happy with all three (we actually only use two) being present in the same image.

> lief cannot do this because these are archives (collections of objects) and not libraries.

## Take 2 - Rename In Object?
See [objcopy] which allows objects to be copied and functions renamed.

The issue might be figuring out what functions are external functions and which are internal and can be hidden inside a library.

> Suggestion: Write a program or compile an existing program and see which symbols are required; all other symbols should be ignored.

## Limiting size
It might also be possible to create a smaller initial image by removing duplicate code however the savings look to be quite small.

> **DANGER** It looks like even just a single ilbrary will be too large to allow OTA updates.
>
> ** Can this really be true? **

In the Zigbee2MQT for development board project, the firmware sizes are around 190KB!

# So OTA?
Can we abuse the OTA and have one image be the _Coordinator_ and the second image be the _End Device_?  Since the OTA seems to be driven by the serial port, it appears to be totally at the control of the user.  We could switch between the two and reboot the TLSR8258?

Or maybe we just have to try building an image and see if we really do use all this functionality.  Perhaps we use very little of it?

[lief]: https://pypi.org/project/lief/
[objcopy]: https://linux.die.net/man/1/objcopy

# Library Hacking
## Overview
Telink SDK provides three libraries which handle End Device, Router and Coordinator Zigbee functionality but each library provides an identically named interface.

We need to be able to run **both** Coordinator and End Device functionality in the same image but the standard libraries would cause a name clash and linking would fail.

[lief] appears to provide the answer as it should alllow us to create modified libraries where the three libraries now export differnet named functions e.g. function `zigbee_function()` can be exported as `en_sigbee_function()` and `rt_zigbee_function()` and `co_zigbee_function()` and now the linker will be happy with all three (we actually only use two) being present in the same image.

[lief]: https://pypi.org/project/lief/

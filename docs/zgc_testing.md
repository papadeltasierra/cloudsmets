# Testing Plan

---
# Building
- Remember to switch board from 48-pin dongle to 32-pin dongle in `board_2858_dongle.h`
- Remember to enable ZBHCI_UART in `app_cfg.h`
- You **cannot** cnotrol the zbhci using the USB connector; you must either pass-through to the ESP32-C3 or redirect the tlsr8258 UART to other pins.
---


## Index
- [What we Want To Know](#what-we-want-to-know)

## What We Want To Know?
1. How do we read and write attributes such as `time` or `localtime`?
1. How do we receive indications/notifications?  Do these even exist or does ZigBee always poll?
1. If we poll, how do we react to received responses?
1. Can we trigger the initial _connection-to-controller_ flow?
1. Can we trigger the _connection-to-established-controller_ flow?

## Testing
### Initial Connection and Sniffing
1. Install the [ESP-C3] serial port passthrough on all boards; this is required so that we can _talk_ to all thr boars without needing three of the [LilyGo T-U2T] adapters.
    - Remember to turn up the tlsr8258 power line in the sketch!
    - Note that board #1 is considered my target board so we do very little with it here
    - Install the Arduino version for now; would like an ESP-IDF version but might have to do this later.
1. Install the `sampleGW` application (a ZigBee Controller) on the second T-ZigBee board (#2)
1. Install the `sampleSwitch` (a Zigbee Endpoint) on the third T-ZigBee board (#3)
1. Query the (probable) ZigBee (MAC) addresses on each board and note down here:

|Board|Address|
|-|-|
|1|`C8 22 1E 0E 0A 0E A1 0C`|
|2|`1C FE 82 82 4D 21 4C 4F`|
|3|`22 E9 4B F9 EB F9 AB OC`|

> Strange, none of these contain the Telink vendor ID `A4 C1 C8`!

5. Update the MAC addresses so that all are unique and note down new ones here.
1. Flip the DIP switches on the T-ZigBee boards to enable [ESP-C3]/[tlsr8258] serial connections.
    - We could, for amusement, see if we can install [tlsr8258] images over this bridge!
1. Install the `zgc` application on two PCs (using `poetry`)
1. Install the [ZigBee sniffer]
1. Launch the [ZigBee sniffer], find a relatively clear channel and listen on it, filtering on the anticipated MAC addresses
1. Launch `zgc` on the `sampleGW`/controller, set the channel to the clear channel, set a ??? encryption key ??? and allow connections from the `sampleSwitch`/endpoint ?? MAC address ??
1. launch `zgc` on the `sampleSwitch`/endpoint, set an ?? encryption key ?? and join to the `sampleGW`/controller
1. Stop sniffing and configure the `sampleSwitch`/controller ?? encryption key ??
1. Confirm that we can identify the flow that carries the **sampleGW/controller** ?? encryption key ?? (?? what message is this ??) and that we can then sniffer the entire ZigBee flows with decoded messages.

# Repeated Connection and Sniffing
1. Start the ZigBee sniffer and configure:
    1. Add filtering for the ZigBee controller and endpoint ?? MAC addresses ??
    1. Configure decoding using the ZigBee controller and endpoint ?? encryption keys ??
1. Configure the `smartGW`/controller to _know_ the `smarthSwitch`/endpoint
1. Configure the `smartSwitch`/endpoint to _know_ the `smartGW`/comtroller
1. Prove that we can now _reconnect_ (?? ZigBee terminology??) the `smartSwitch`/endpoint to the `smartGW`/controller
1. Check the ZigBee sniffer logs and confirm that the expected flows happened.

# Writing/Reading an Attribute
1. Set up and configure the ZigBee sniffer as per above
1. Reconnect (?? terminology ??) the `smartSwitch`/endpoint to the `sampleGW`/controller
1. Identifiy a lone attribute expoed by the `smartGW`/controller
1. Read the attribute from the 'smartSwitch`/endpoint
1. Check the ZigBee sniffer to confirm the ZigBee flows for the attribute read
1. Check the `smartSwitch`/endpoint `zgc` instance to see that the attribute value was read correctly
1. Change the attribute value on the `sampleGW`/controller and repeat the read test.

[ESP-C3]: http://espressif.com
[tlsr8258]: http://telink.com
[LilyGo T-U2T]: https://www.lilygo.cc/products/t-u2t?_pos=1&_psq=T-U2T&_ss=e&_v=1.0
[ZigBee sniffer]: https://sniffer.com
[poetry] https://poetry.org

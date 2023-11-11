# Sniffing ZigBee Notes

## Initial Testing
Initial sniffing is being performed using a [TI] (Texas instruments) [CC2531 USB stick], preloaded with the ZBOSS/Wireshark sniffer package.  instructions can be found in the [Sniff Zigbee traffic] section of the [Zigbee2MQTT] website.

Data was successfully captured off channel 11.

- ZBOSS starts a little GUI program which is where you can set the channel and indicate where your `wireshark.exe` is located
- Wireshark is then launched, listening off a pipe created by ZBOSS
- All data appears to be flowing from the IHD to the Hub, which cannot be correct so presumably traffic from the Hub to the IHD is flowing on a different channel
- As expected, all data is encrypted

~~~
Frame 164: 79 bytes on wire (632 bits), 77 bytes captured (616 bits) on interface \\.\pipe\zboss_sniffer_COM3, id 0
IEEE 802.15.4 Data, Dst: 0x55b1, Src: 0x60e4
    Frame Control Field: 0x8861, Frame Type: Data, Acknowledge Request, PAN ID Compression, Destination Addressing Mode: Short/16-bit, Frame Version: IEEE Std 802.15.4-2003, Source Addressing Mode: Short/16-bit
    Sequence Number: 73
    Destination PAN: 0x5805
    Destination: 0x55b1
    Source: 0x60e4
    [Extended Source: Chameleo_00:00:56:aa:98 (0c:a2:f4:00:00:56:aa:98)]
    [Origin: 6]
ZigBee Network Layer Data, Dst: 0x55b1, Src: 0x60e4
    Frame Control Field: 0x3248, Frame Type: Data, Discover Route: Enable, Security, Extended Source, End Device Initiator Data
        .... .... .... ..00 = Frame Type: Data (0x0)
        .... .... ..00 10.. = Protocol Version: 2
        .... .... 01.. .... = Discover Route: Enable (0x1)
        .... ...0 .... .... = Multicast: False
        .... ..1. .... .... = Security: True
        .... .0.. .... .... = Source Route: False
        .... 0... .... .... = Destination: False
        ...1 .... .... .... = Extended Source: True
        ..1. .... .... .... = End Device Initiator: True
    Destination: 0x55b1
    Source: 0x60e4
    Radius: 30
    Sequence Number: 164
    Extended Source: Chameleo_00:00:56:aa:98 (0c:a2:f4:00:00:56:aa:98)
    ZigBee Security Header
        Security Control Field: 0x28, Key Id: Network Key, Extended Nonce
        Frame Counter: 45412939
        Extended Source: Chameleo_00:00:56:aa:98 (0c:a2:f4:00:00:56:aa:98)
        Key Sequence Number: 11
        Message Integrity Code: 2d8b16f2
        [Expert Info (Warning/Undecoded): Encrypted Payload]
    Data (34 bytes)
~~~

- Umm, the MAC address and GUID are expected to be the same but they do not match the GUID on my IHD; was I listening to someone else's?
  - Almost certainly!  Need to keep a look out for my specific GUID/MAC address.
  - Note that `0c:a2:f4` is the MAC/GUID prefix for [Chameleon Technology] who make my and many other IHDs.

[TI]: https://ti.com
[CC2531 USB Stick]: https://www.ti.com/tool/CC2531USB-RD?keyMatch=&tisearch=search-everything&usecase=partmatches
[ZBOSS]: https://dsr-iot.com/downloads/tools#upper-header
[Wireshark]: https://wireshark.org
[Sniff Zigbee traffic]: https://www.zigbee2mqtt.io/advanced/zigbee/04_sniff_zigbee_traffic.html
[Zigbee2MQTT]: https://www.zigbee2mqtt.io
[Chameleon Technology]: https://chameleontechnology.co.uk/
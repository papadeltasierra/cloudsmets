# LilyGo Development Board
- This does **NOT** contain a TTL/USB serial port adapter!
  - A [T-U2T] dongle is required that converts the TTL to/from USB
  - **WARNING** You must ensure that the DIP switches have disonnected the RX/TX lines before you add power in the _normal manner_ via the USB socket
- What ports get mapped?  Presumably the standard ones?
  - The ports that are mapped to the "USB-C" port are not exposed through pins so we cannot use connect to them.  There are other UART ports available but presumably they are not expecting to receive a programming signal
- The standard connectivity is:
  TLSR8258 TX/RX to ESP32 RX/TXD1
- Switching the DIP switches connects the _USB-C_ to either TLSR8258 RX/TX or the ESP32's RT/TX(0).

> You pretty much have to have the T-U2T connector to program this board, which is something that is not clear from the docs!

  [T-U2T]: https://www.lilygo.cc/products/t-u2t
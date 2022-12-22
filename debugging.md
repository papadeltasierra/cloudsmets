# Debugging
How will we debug CloudSMETS running on the Zigbee development board?

> The USB-C connector on the Zigbee board does **NOT** provide a proper USB/Serial port and cannot be used for debugging

## Pins Functions
The [ESP32-C3] supports two standard UARTs (UART0 and UART1) and a _USB Serial/JTAG controller_ (hitherto referrred to as a _USB_ port).

The Zigbee board uses the USB port as the programing port which means that both UART0 and UART1 should be avialable for use by suitable programming of the GPIO pins.

The [TLSR8258] uses two sets of pins, PB1/PB7 as programming pin and only one further RX port is exposed, PC2 which is paired TX port PC3 .

The following debugging scheme is therefore defined:

- The [TLSR8258] will generate debug information on the PC3 port
- The PC3 port is connected to the [ESP32-C3]'s GPIO8 which is programmed to be the UART1 RX port
- The [ESP32-C3] will generate debug information on the GPIO21/TX port acting as UART0

> The UARTs functions need to be programmed as part of the ESP32-C3 start-up code.  Do this ASAP after starting so that start-up logs can be obtained.
>
> Copythe existing Zigbee2MQTT project's set-up for UARTs etc.

- Debug information received from the [TLSR8258] on UART1 is relayed out of UART0 so that both [ESP32-C3] and [TLSR8258] debugging information is available on the same UART

> Supposedly both the [ESP32-C3] and [TLSR8258] can achieve >4Mbps baud rates.  Possibly this is something that we might set as an output parameter for debugging?

- Initial UART configuration is 115200,8,1,N.

~~~mermaid
---
title: Serial Based Debugging Output
---
flowchart LR
    T[TLSR8258];
    E[ESP32-C3];
    D[UART debugger];
    T -->|PC3,TX/GPIO8,RX| E;
    E -->|GPIO21,TX/FTDI| D;
~~~

- **Additionally** A network debugging interface may be defined that sends the same messages out to a specified IP address and port.  This will permit a simple program, for example a Python script, to obtain debugging information from a Zigbee board without the need for a physical wired UART connection; this could be helpful when debugging customer issues.

> A sample Python script should be created as part of this project or maybe a simple C program.

## Configuaration
The website will permit configuration of:
- Debugging globally enabled/disabled
- ESP32-C3 debugging level set to Error/Info/Debug
- TLSR8258 debugging level set to Error/Info/Debug
- UART debugging output enabled
- Network debugging output enabled
- Network debugging server IP address
- Network debugging server IP port

[ESP32-C3]: https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf
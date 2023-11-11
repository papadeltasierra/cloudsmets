# Ideas for "Hacking" SMETS Exchanges

## Background
- SMETS exchanges are encrypted
- We don't kow the Network Key (Coordinator/Hub) or device Keys (IHD)
- So...

## CloudSMETS Functions
1. IHD key; learning the security key from the IHD (end device)
2. Hub ??; learning the ?? from the hub (coordinator)
3. Cloud; quietly listening on the network, catching intresting frames directed to the hub and passing the data to our cloud server(s).

## Learning IHD Key
- Create a ZigBee coordinator using a cheap ZigBee device
- Turn off the IHD
- Take the IHD, ZigBeen coordinator far away from the hub
- Start the coordinator
- Start the IHD and see if it will try connect to the coordinator and tell us its key

## Learning the Network Key
- First sniff traffic between the IHD and the hub and see if the standard ZigBee network key and the learnt IHD key can decode the traffic
  - If it can then we are done - hurrah
- If not...
  - Note down the GUID (IEEE address) of the IHD
  - Turn off the IHD
  - Configure the ZigBee device as an end node
  - Give the ZigBee device the same GUID as the IHD (we're pretending to be the IHD)
  - See if the ZigBee device can pair to the IDH
  - If yes, read the network key or see it from sniffing the exchange
  - Turn off the ZigBee device
  - Turn the IHD back on
  - Can we sniff the IHD/hub traffic now?

## A Permanent Sniffer?
- The TLSR8258 doesn't seem to be able to sniff ZigBee traffic
- Can we rework the TI CC2531 USB stick firmware to output via the output pins and then wire it to an ESP32?
- Ideally we want to add as much filtering as possible on the CC2531
- Ideally we want to architect the same interface as the ZigBee2MQTT code uses (same serial port based protocol)
- Most functions do nothing
- Maybe some new functions but probably OK
- Same data indication being passed to the ESP32-C3
- How do we decode it and pass along?  The ZigBee2MQTT code should explain all
- Can we build a simple sniffer to do this, or perhaps get a board designed that does all we need?  Is there a market?

## Telink SDK Notes
- 'b85m' includs the TLSR8258
- There is a `__PROJECT_TL_SNIFFER__` flag in the SDK
- There is an `irq_sniffer_handler()` that becomes available for a _sniffer_ project
- `irq_sniffer_handler()` is called from `irq_handler()` instead of the RX or TX handlers (which would have been called based on flags and what was happening)
  - not clear what triggers the IRQ as it is called from cstartup_8285.S machine code
  - so what sort of processor are we looking at here?  What is the core?
  - See [Ghisdra TELink]: _The TC32 is essentially a clone of the 16-bit ARM9 Thumb instruction set._
  - See the [Thumb 16-bit Instruction Set Quick Reference Card] however there are clearly some differents; perhaps te [Ghisdra TElink] project could clarify them if required
- Not clear what the _sniffer_ IRQ is as opposed to say just the _rx_ IRQ.
- `ZB_RADIO_TRX_CFG` enables both RX and TX IRQs

### Working up from IRQ
- `cstartup_8278.S` has code to call `irq_handler`
- `irq_handler` calls various secondary handlers
  - oddly there seem to be _BOOTLOADER_ and _SNIFFER_ special build options; not sure what these are for or do.
  - `drv_uart_tx/rx_irq_handler()` is called from the `drv_uart.c` module
  - **There is no implementation of the sniffer `irq_sniffer_handler()`.**
  - The _standard_, i.e. not _BOOTLOADER_ or _SNIFFER_, IRQ handler passes the IRQ off to different handlers depending on what is flagged in the `rf_iqr_src_get()`
    - The _standard_ handler can pass off to the `drv_uart_tx/rx_handler()` which implies that the _BOOTLOADER_ is a special _single-function_ codebase somehow
    - My guess is that the _BOOTLOADER_ is a special build, small program that is run before the make program is launched
    - It looks like the _SNIFFER_ is a special program that we do not have the source code to.

**Question:/** Can we work up from the standard code and disable most of the [IEEE 802.15.4] and [ZigBee] functionality to create a custom sniffer?

- Following `rf_rx_iqr_handler()`to see how nicoming frames are handled...
- Implemented in `mac_phys.c`; this implements the [IEEE 802.15.4] function
- The incoming frame is filtered based on destination MAC (IEEE address)
  - We will still want this for _cloud_ function
- The IEEE ACK is sent here - we will not do this for _cloud_ function
- Received data is pass to `zb_macDataRecvHeader()`
- **There is no implementation of `zb_macDataRecvHeader()`**
  - `zb_macDataRecvHeader()` is implemented in one of three libraries, which implement _End Device_, _Router_ or _Coordinator_ ZigBee function namely `libzb_ed.a`, `libzb_router.a`, `libzb_coordinator.a`
  - So to avoid responses etc, we need to implement our own `libzb_cloud.a` and somehow hook things together such that we have a _switch_ that allows us to select the library that we want to use
  - Could be tricky as presumably these libraries also implement the SDK functions - **Correct**

> We may be able to use the _End Device_ library for the _cloud_ function and just catch and ignore any attempt to send anything.
>
> So we implement an extension _set mode_ command which disables the loweer level send function, probably throwing a log error if someone tries to send something.

### Wrapping the _EP_ and _Coordinator_ Libraries
- Wrap each in a library (A) that exposes the functions such that `a_function()` becomes `ep_a_function() or `co_a_function()`
- Another library (B) wraps these and exposes `a_function()`
- Library (B) also exposes a _set mode_ function which causes a call to `a_function()` to be routed to `ep_a_function()` or `co_a_function()` (but neve both) as appropriate.
- There are probably callbacks that happen too which need to be registered so that received frames and indications can be passed up
- The _set mode_ function will also disable all sending of frames if set to _cloud_ mode (_End Device_ but with no sending)); this includes frames being caused by [ZigBee] and also by [IEEE 802.15.4].


Question:/ what is the `zbhci_mgmtCmdHandler()`?

- `zbchi_mgmtCmdhandler()` is one of a range or similar functions called from the zhbciCmdHandler()`
- There are multiple implementations of the `zhbciCmdhandler()` and it looks like this implements the serial port API (via the UART implementation) that is described elsewhere
  - So this is where we might add the extension _set mode_ command



[Ghisdra TELink]: https://github.com/rgov/Ghidra_TELink_TC32
[Thumb 16-bit Instruction Set Quick Reference Card]: https://developer.arm.com/documentation/qrc0006/e
[IEEE 802.15.4]: https://en.wikipedia.org/wiki/IEEE_802.15.4
[ZigBee]: https://en.wikipedia.org/wiki/Zigbee
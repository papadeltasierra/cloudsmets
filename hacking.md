# Ideas for "Hacking" SMETS Exchanges

## Background
- SMETS exchanges are encrypted
- We don't kow the Network Key (Coordinator/Hub) or device Keys (IHD)
- So...

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

[Ghisdra TELink]: https://github.com/rgov/Ghidra_TELink_TC32
[Thumb 16-bit Instruction Set Quick Reference Card]: https://developer.arm.com/documentation/qrc0006/e
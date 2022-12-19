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

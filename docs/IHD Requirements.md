# IHD Requirements

See section 6 of the Smart Metering Equipment Technical Specifications Version 2

- ZigBee SEP v1 communications
  - SMART Enery Profile v1
  - DLSM etc.  See IEC 62056 - Probably not the one!
- Connect to one ESME and one Gas Proxy Device
- Pricing and consumption from the ESME (Electricity Smart Metering Equipment)
- Pricing and consumption from the Gas Proxy Device (probably the ESME!)

This specification does not appear to define the protocol.

So implies that the IHD has to support the SEP.

- `ZCL_METERING_SUPPORT`
- `ZCL_METERING` aka "Smart Energy"
- `zcl_metering_attr.c` defines attributes available for smart metering.
    - Need to enable the individual attributes (maybe start with all then cut down if not seen?)
- Functions are provided that secode (?) the signals.
    - Functions appear to be implemented in libraries.  Not sure if we can trim down to avoid using methods we don't want.

So how is this all established?

Question:/ What is "retention" in the `user_init()` method?


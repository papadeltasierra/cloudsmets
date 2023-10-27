# TODO

1. Add support for the price command callbacks to both ESME and CloudSMETS
1. Add support to get ESME to publish price information (timed or ZGC?)

1. Extend/constrict the ZGC application to handle the price commands

1. How do we know where cluster X is? Find the Gas and electricity meters?
    1. See ZigBee Specification, 2.4.2.7 etc.
       1. Device Discovery
       1. Service Discovery
       1. Network Managment requests
       1. No!! `ZBHCI_CMD_DISCOVERY_SIMPLE_DESC_REQ` is what we need but we first need to find devices.  How do we know what partner devices are in the network?
       1. `ZBHCI_CMD_DISCOVERY_MATCH_DESC_REQ` is the one we want because it can be broadcast; the question is, can we limit it to the appropriate PANID?  Yes, use the special addresses shown in 3.6.5 (`0xfffd`).
           1. We can indicate the profile and all the clusters and hopefully both Gas and Electricity will respond.

    1. `ZBHCI_CMD_DISCOVERY_NWK_ADDR_REQ` and similar seems to be the way to go.  But does this just look for coordinators to connect to?  Is does not appear to take cluster types as parameters.
    1. Also what does `ZBHCI_CMD_GET_LOCAL_NWK_INFO_RSP` do?
        1. See `zbhciCmdProcess.c`, line 975.  Really does return local data so there is precedence.  Returns...
            1. Node type
            1. Capability
            1. nodeIsOnANetwork?
            1. The PANID (2-bytes)
            1. Extended PANID, EUI64  (8-bytes)
            1. Local address (2-bytes)
            1. Local EUI64 (8-bytes)
            1. Note that is **DOES NOT** return the encryption key.
    1. Or `ZBHCI_CMD_MAC_ADDR_IND`?
        1. Looks like this returns the MAC address of a device joining to the coordinator, so no good for CloudSMETS.

1. Extend zbhci to print published price.
1. Print link key as simple hex.

1. Can we query the join information, including the keys, over the `zbhci`?
    1. Doesn't look like it.  Cannot find an interface that has access to it.
    1. MAYBE - see zdo_ssInfoKeyGet() which seems to query the keys.  Can we extend this to read and set the key?
1. Can we clear the firmware over the `zbhci` and then force a new join?
    1. Yes, `ZBHCI_CMD_BDB_FACTORY_RESET`
1. Can we restore join infomraiton over the `zbhci` and then force a rejoin
    1. Doesn't look like it.
    1. MAYBE - see above.

1. Is the ESP32-C3 serial pass-through working?

1. How does HCI OTA work?  Can we use this to update the TLSR8258?
    1. I'm guessing so if we can make it happen and have space on the ESP32-C3 to download the OTA.


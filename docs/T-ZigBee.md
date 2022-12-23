# Notes on the T-ZigBee Implementation of ZigBee2MQTT

Ref:
- [ZigBee development board]
- [ZigBee2MQTT for development board]

## Overview
- Not sure if the TLSR8258 can be switched into "sniffer" mode or not
- For testing of "can we bind the IHD", the TLSR8258 code is almost certainly sufficient and we might be able to read the key for the device
- ESP32-C3 code is only useful for the `zbhci` interface definition, which appears to support an OTA update but it is not clear
- Write new ESP-C3 firmware to add OTA updates, cnofig stores, web service etc.

#### ZigBee Notes
- The [Hub] above the electriicity meter is a [ZigBee] [Coordinator] and is the node
that stores network information, including security keys
- The [IHD] is an [End Device] and can talk to a coordinator or router but not to other end devices
- [Routers] just relay information between endpoints and coordinators.

> So in theory we might have been able to place a [Router] between the [IHD] and [Hub], but the [IHD] is too close to the [Hub] to need/use a [Router] and the [Router] would almost certainly not be allowed to contact the [Hub] anyway as the _device list_ on the [Hub] is locked down.

- ZigBee only specifies Intra-[PAN] (Personal Area Network) communications; In SMETS/Smart Metering terms, a [HAN] (Home Area Network) is a [PAN].
- Multi-octet values are sent over the network using  _byte containing the lowest-numbers bits_ first ordering
- A byte/octet is a binary string (values `{0,1}`) of length 8
- Expect to see ZigBee Protocol Version `0x03` (ZigBee PRO)


## zbhci Notes
- `zbhci` is a protocol between the Espressif ESP32-C3 and the Telink [TLSR8258] processors
- communcation is via zbhci_Tx()
    - then uart_send()
- zbhci commands use a CRC8 checksum calculated over the data being sent
- requests(?)/responses from the TLS??? are handled via the `zbhci_task()` which spins calling `uart_recv()` and then unpacking using zbhci_CmdUnpack()
   - `zbhci_CmdUnpack()` does 6 steps, checking the CRC8 as the final step and if that is OK, accepting the request.
   - It is not clear how the protocol recovers from an error as it seems to run through states 0-6 every single time.  It might be possible to improve this or perhaps not!
- `zhbi_Cmdhandler()` processes the message and then passes it along using `xQueueSend()`
    - There are lots of commands, but what do they do?
- `hci_uart.h` seems to be used as the UART - this is written by T-ZigBee and not an Espressif module

## hci_uart Notes
_Pretty standard ESP32 processing_
- Has `init()`/`send()`/`recv()`/`deinit()` methods
- Uses UART_NUM_1
    - **We should be using UART_NUM_2 (0?) to generate debugging and listening in using the serial adapter**
- The usual 115200,8,1,n and not flow control
    - using GPIOs 18 and 19
- Using an ESP32 task to receive data and then send it as an event to the `zbhci` task.

## hci_display Notes
`hci_display` functions are just logging functions that log the responses and indications recieve over the `hci_uart` interface.  they should probably be `zbhci_display` methods since they all related to zbhci responses or indications.

The process for receipt of a response or indication is:
- received via `hci_uart`
- unpacked and passed to the `zhbci_Cmdhandler()`, which really processes responses and indications not requests
- individual unpack methods for each message type e.g. `zbhci_UnpackNetworkStateRspPayload()`
    - data is unpacked into a _response payload_ e.g. `ts_MsgNetworkStateRspPayload`
    - the payload is the data poassed along when the event is sent along using the `xQueueSend()` reuqest from `zbhci_task()`
    - the queue to send to is an argument to `zbhci_Init()`
    - the queue is set by the various (Arduino) examples
    - the (Arduino) examples handle the responses and then do more stuff.

## zhbi Commands
It is assumed that zhbi is supporting the _ZigBee PRO_ form of [ZigBee].
Message types are below.

_These probably relate to ZigBee concepts so we need to read up on the [ZigBee] protocol._

### Commisioning
- `Formation` and `Steer` take no parameters
- `TouchLink` and `FindBind` take an `enum` of `Initiator` or `Target`


|Command|Method?|Purpose|
|-|-|-|
|ZBHCI_CMD_BDB_COMMISSION_FORMATION|Y|This appears to be related to _binding_, namely how nodes find each other and "register" themselves with the coordinator.<br/>Actually that might be wrong; commisioning might be how the speicfic node is configured.  See section `2.5.4.5.6.1` of the ZigBee specifications.
|ZBHCI_CMD_BDB_COMMISSION_STEER|Y|
|ZBHCI_CMD_BDB_COMMISSION_TOUCHLINK|Y
|ZBHCI_CMD_BDB_COMMISSION_FINDBIND|Y
|ZBHCI_CMD_BDB_FACTORY_RESET|Y|Wipes all [NV] configuration, may send a `leave` command and restores the TLSR8258 firmware to factory settings
|ZBHCI_CMD_BDB_PRE_INSTALL_CODE|Y|Configures the device address and unique [link key] for this device.<br/>**We don't expect to use this command as we are going to try and act as a Sniffer.**
|ZBHCI_CMD_BDB_CHANNEL_SET|Y|channels can only be `11` to `26` which says that the TLS8258 only operates in the 2450MHz frequency band.<br/>Selecting a band seems to be application level function so presumably the system has to "find" a free band.<br/>Our device has to search for the band that the [IHD] and [Hub] are using the communicate.
|ZBHCI_CMD_BDB_DONGLE_WORKING_MODE_SET|Y|Takes _Get MAC address_ or _Normal_ mode as the sole parameter; may be _set MAC address_ or _generate random MAC address_ which the [TLS8258] supports.
|ZBHCI_CMD_BDB_NODE_DELETE|Y|(Presumably) If working as a [Coordinator], forget all information about this [Node].  The [Node] is identified by its 64-bit device address
|ZBHCI_CMD_BDB_TX_POWER_SET|Y|Set transmission power<br/>VANT and VBAT may be _regular Bluetooth_ and _ANT_ modes sinc ethis board can also support Bluetooth.
|ZBHCI_CMD_ACKNOWLEDGE|
|ZBHCI_CMD_BDB_COMMISSION_FORMATION_RSP|
|ZBHCI_CMD_NETWORK_STATE_REQ|Y
|ZBHCI_CMD_NETWORK_STATE_RSP|
|ZBHCI_CMD_NETWORK_STATE_REPORT|
|ZBHCI_CMD_DISCOVERY_NWK_ADDR_REQ|Y
|ZBHCI_CMD_DISCOVERY_IEEE_ADDR_REQ|Y
|ZBHCI_CMD_DISCOVERY_NODE_DESC_REQ|Y
|ZBHCI_CMD_DISCOVERY_SIMPLE_DESC_REQ|Y
|ZBHCI_CMD_DISCOVERY_MATCH_DESC_REQ|Y
|ZBHCI_CMD_DISCOVERY_ACTIVE_EP_REQ|Y
|ZBHCI_CMD_DISCOVERY_LEAVE_REQ|Y
|ZBHCI_CMD_DISCOVERY_NWK_ADDR_RSP|
|ZBHCI_CMD_DISCOVERY_IEEE_ADDR_RSP|
|ZBHCI_CMD_DISCOVERY_NODE_DESC_RSP|
|ZBHCI_CMD_DISCOVERY_SIMPLE_DESC_RSP|
|ZBHCI_CMD_DISCOVERY_MATCH_DESC_RSP|
|ZBHCI_CMD_DISCOVERY_ACTIVE_EP_RSP|
|ZBHCI_CMD_BINDING_REQ|Y
|ZBHCI_CMD_UNBINDING_REQ|Y
|ZBHCI_CMD_BINDING_RSP|
|ZBHCI_CMD_UNBINDING_RSP|
|ZBHCI_CMD_MGMT_LQI_REQ|Y
|ZBHCI_CMD_MGMT_BIND_REQ|Y
|ZBHCI_CMD_MGMT_LEAVE_REQ|Y
|ZBHCI_CMD_MGMT_DIRECT_JOIN_REQ|Y
|ZBHCI_CMD_MGMT_PERMIT_JOIN_REQ|Y
|ZBHCI_CMD_MGMT_NWK_UPDATE_REQ|Y
|ZBHCI_CMD_MGMT_LQI_RSP|
|ZBHCI_CMD_MGMT_BIND_RSP|
|ZBHCI_CMD_MGMT_LEAVE_RSP|
|ZBHCI_CMD_MGMT_DIRECT_JOIN_RSP|
|ZBHCI_CMD_MGMT_PERMIT_JOIN_RSP|
|ZBHCI_CMD_MGMT_NWK_UPDATE_RSP|
|ZBHCI_CMD_NODES_JOINED_GET_REQ|Y
|ZBHCI_CMD_NODES_TOGLE_TEST_REQ|Y
|ZBHCI_CMD_TXRX_PERFORMANCE_TEST_REQ|Y
|ZBHCI_CMD_AF_DATA_SEND_TEST_REQ|Y
|ZBHCI_CMD_NODES_JOINED_GET_RSP|
|ZBHCI_CMD_NODES_TOGLE_TEST_RSP|
|ZBHCI_CMD_TXRX_PERFORMANCE_TEST_RSP|
|ZBHCI_CMD_NODES_DEV_ANNCE_IND|
|ZBHCI_CMD_AF_DATA_SEND_TEST_RSP|
|ZBHCI_CMD_LEAVE_INDICATION|
|ZBHCI_CMD_ZCL_ATTR_READ|Y
|ZBHCI_CMD_ZCL_ATTR_WRITE|Y
|ZBHCI_CMD_ZCL_CONFIG_REPORT|Y
|ZBHCI_CMD_ZCL_READ_REPORT_CFG|Y
|ZBHCI_CMD_ZCL_LOCAL_ATTR_READ|Y
|ZBHCI_CMD_ZCL_LOCAL_ATTR_WRITE|Y
|ZBHCI_CMD_ZCL_SEND_REPORT_CMD|Y
|ZBHCI_CMD_ZCL_ATTR_READ_RSP|
|ZBHCI_CMD_ZCL_ATTR_WRITE_RSP|
|ZBHCI_CMD_ZCL_CONFIG_REPORT_RSP|
|ZBHCI_CMD_ZCL_READ_REPORT_CFG_RSP|
|ZBHCI_CMD_ZCL_REPORT_MSG_RCV|
|ZBHCI_CMD_ZCL_LOCAL_ATTR_READ_RSP|
|ZBHCI_CMD_ZCL_LOCAL_ATTR_WRITE_RSP|
|ZBHCI_CMD_ZCL_ATTR_WRITE_RCV|
|ZBHCI_CMD_ZCL_BASIC|
|ZBHCI_CMD_ZCL_BASIC_RESET|Y
|ZBHCI_CMD_ZCL_GROUP|
|ZBHCI_CMD_ZCL_GROUP_ADD|Y
|ZBHCI_CMD_ZCL_GROUP_VIEW|Y
|ZBHCI_CMD_ZCL_GROUP_GET_MEMBERSHIP|Y
|ZBHCI_CMD_ZCL_GROUP_REMOVE|Y
|ZBHCI_CMD_ZCL_GROUP_REMOVE_ALL|Y
|ZBHCI_CMD_ZCL_GROUP_ADD_IF_IDENTIFY|Y
|ZBHCI_CMD_ZCL_GROUP_ADD_RSP|
|ZBHCI_CMD_ZCL_GROUP_VIEW_RSP|
|ZBHCI_CMD_ZCL_GROUP_GET_MEMBERSHIP_RSP|
|ZBHCI_CMD_ZCL_GROUP_REMOVE_RSP|
|ZBHCI_CMD_ZCL_IDENTIFY|
|ZBHCI_CMD_ZCL_IDENTIFY_QUERY|Y
|ZBHCI_CMD_ZCL_IDENTIFY_QUERY_RSP|
|ZBHCI_CMD_ZCL_ONOFF|
|ZBHCI_CMD_ZCL_ONOFF_ON|Y
|ZBHCI_CMD_ZCL_ONOFF_OFF|Y
|ZBHCI_CMD_ZCL_ONOFF_TOGGLE|Y
|ZBHCI_CMD_ZCL_ONOFF_CMD_RCV|
|ZBHCI_CMD_ZCL_LEVEL|
|ZBHCI_CMD_ZCL_LEVEL_MOVE2LEVEL|Y
|ZBHCI_CMD_ZCL_LEVEL_MOVE|Y
|ZBHCI_CMD_ZCL_LEVEL_STEP|Y
|ZBHCI_CMD_ZCL_LEVEL_STOP|Y
|ZBHCI_CMD_ZCL_LEVEL_MOVE2LEVEL_WITHONOFF|Y
|ZBHCI_CMD_ZCL_LEVEL_MOVE_WITHONOFF|Y
|ZBHCI_CMD_ZCL_LEVEL_STEP_WITHONOFF|Y
|ZBHCI_CMD_ZCL_LEVEL_STOP_WITHONOFF|Y
|ZBHCI_CMD_ZCL_SCENE|
|ZBHCI_CMD_ZCL_SCENE_ADD|Y
|ZBHCI_CMD_ZCL_SCENE_VIEW|Y
|ZBHCI_CMD_ZCL_SCENE_REMOVE|Y
|ZBHCI_CMD_ZCL_SCENE_REMOVE_ALL|Y
|ZBHCI_CMD_ZCL_SCENE_STORE|Y
|ZBHCI_CMD_ZCL_SCENE_RECALL|Y
|ZBHCI_CMD_ZCL_SCENE_GET_MEMBERSHIP|Y
|ZBHCI_CMD_ZCL_SCENE_ADD_RSP|
|ZBHCI_CMD_ZCL_SCENE_VIEW_RSP|
|ZBHCI_CMD_ZCL_SCENE_REMOVE_RSP|
|ZBHCI_CMD_ZCL_SCENE_REMOVE_ALL_RSP|
|ZBHCI_CMD_ZCL_SCENE_STORE_RSP|
|ZBHCI_CMD_ZCL_SCENE_GET_MEMBERSHIP_RSP|
|ZBHCI_CMD_ZCL_COLOR|
|ZBHCI_CMD_ZCL_COLOR_MOVE2HUE|Y
|ZBHCI_CMD_ZCL_COLOR_MOVE2COLOR|Y
|ZBHCI_CMD_ZCL_COLOR_MOVE2SAT|Y
|ZBHCI_CMD_ZCL_COLOR_MOVE2TEMP|Y
|ZBHCI_CMD_ZCL_IAS_ZONE|
|ZBHCI_CMD_ZCL_OTA_IMAGE_NOTIFY|Y
|ZBHCI_CMD_DATA_CONFIRM|
|ZBHCI_CMD_MAC_ADDR_IND|
|ZBHCI_CMD_NODE_LEAVE_IND|
|ZBHCI_CMD_AF_DATA_SEND|

???

- <span id='PAN'/>**PAN**</span>: Personal Area Network
- <span id='Coordinator'>**Coordinator**</span>: ZigBee node that manages the network and stores information such as security keys
- <span id='EndDevice'>**End Device**</span>: ZigBee node that _does the work_ but which can only communicate with the [Coordinator] or a [Router]
- <span id='Router'>**Router**</span>: ZigBee node used in some networks to relay information between [End Node]s and/or the [Coordinator]
- <span id='Node'>**Node**</span>: A ZigBee [Coordinator], [Router], [End Point]
- <span id='NV'>**NV**</span>: non-volatile; configuration information that survives a power-cycle, typically configuration such as node IDs, passwords etc
- <span id='LinkKey'>**Link Key**</span>: A security key, half of a link key pair shared between this node another node that talks to.
- <span id='HAN'>**HAN**</span>: Home Area Network (a PAN!)
- <span id='Hub'>**Hub**</span>: The ZigBee bx, typically above your electricity meter; this performs the role of the ZigBee [Coordinator]- <span id='IHD'>**IHD**</span>: In Home Device: The display device (a ZigBeen [End Device]) that shows you, the customer, your current gas and electricity usage

[TLS8258]: http://wiki.telink-semi.cn/doc/ds/PB_TLSR8258-E_Product%20Brief%20for%20Telink%20BLE%20IEEE802.15.4%20Multi-Standard%20Wireless%20SoC%20TLSR8258.pdf
[TLS8258 ZigBee Development Manual]: http://wiki.telink-semi.cn/doc/an/AN_19052900-E_Telink%20Zigbee%20SDK%20Developer%20Manual.pdf
[ZigBee development board]: https://www.cnx-software.com/2022/04/27/10-t-zigbee-board-combines-esp32-c3-and-tlsr8258-for-zigbee-3-0-wifi-and-ble-connectivity/
[ZigBee2MQTT for development board]: https://github.com/Xinyuan-LilyGO/T-ZigBee
[ZigBee]: https://zigbeealliance.org/wp-content/uploads/2019/11/docs-05-3474-21-0csg-zigbee-specification.pdf
[PAN]: #PAN
[HAN]: #HAN
[IHD]: #IHD
[Coordinator]: #Coordinator
[End Device]: #EndDevice
[Router]: #Router
[NV]: #NV
[link key]: #LinkKey
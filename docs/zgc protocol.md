# Zigbee Gateway Controller (ZGC) Protocol

See the documentation at [zbhci].

## One of Two Ways!
The ZGC processing either:
1. Processes attrbutes directly into variables or
1. Processes attributes into structures that can then be serialized and passed over the zhci interface.

Standard processing ot `time` attributes is an example of _straight to variable_ whilst processing of `scene` attributes is an example of _process to structure then serialize_.

## zbhci Processing From Top-Down
- `zbhci` requests are received over the UART via `hci_uart.c` and method `uart_data_handler()`
- A bad request results in an error acknowledgement back over the UART
- A good request is passed to method `zbhciCmdHandler()`
- `zbhciCmdHandler()` is defined in `zbhciCmdHandler.c` and the message types can be see, `ZBHCI_CMD_...`

### Notes
- `zbhci_ZcLLocalAttrWrite()` looks interesting

> Local link secret is hard coded at firmware address 0x78000 and cannot be read or changed over the zbhci unless we extend it somehow.




## Reading Attributes
Apparently in Zigbee you can request a read of multiple attributes at once so how might we do this?  Well we need this information:

|Info|Example|Purpose|
|-|-|-|
|Server address|11:22:33:44:55:66:77:88|The address to read from|
|Cluster ID|0x0702|Metering (Smart Energy)|
|Attr Id|0x0000|Current Summation Delivered|
|Attr Id|0x0001|Current Summation Received|

Note that there is no reason why you cannot read attributes from different information sets at the same time; they have distinct attribute IDs.

It appears that CloudSMETS will need to learn the address of two devices so it can read the gas and electricy seperately.  These should operate slightly differently because gas info is only available at around 30min granularity.

Use "Service Discovery" to find the endpoints.

Recommended [ZigBee Base Device Behavior Specification, v1.0] to start with :
- ZBHCI_CMD_MGMT_LQI_REQ to find the neighbours
- Match_Desc_req, ZBHCI_CMD_DISCOVERY_MATCH_DESC_REQ
- Looks like we can send this to the coordinator and it will respond with matches
- So we ask for a time server...
- ...and then we ask for all metering servers, which probably results in the ESME and the gas proxy.
- We care about clusters in and clusters out, although not clear which and in
which order!

Implies that it should be possible to define "we support the metering cluster as a client" and then everything else works from the `zbhci`.


## Time Processing from the bottom up
- There are no example of using the time attributes in the [Telink ZigBee SDK].
- 10 attributes are defined in `zcl_time.h`.
- Refer to the [ZigBee Cluster Library], section 3.12, Time.
    - Attributes are defined in 3.12.2.2, Attributes.

Since `time` only has attributes and no commands, it appears that the processing used by `scene` is not required as only a single attribute is ever requested or received.

> It is not clear how we might be expected to pass this information over the `zbhci`.

## Scheme Processing from the bottom up
`zbhciTx` sends the serialized `scene` information to the `zbhci` client over a serial port.

- zbhciTx(ZBHCI_CMD_ZCL_SCENE_REMOVE_ALL_RSP, pBuf - array, array);
    - sampleGW_zclRemoveAllSceneRspCmdHandler
        - status_t sampleGW_sceneCb
            - g_sampleGwClusterList[]
                - ZCL_CLUSTER_GEN_SCENES, = 0x0005
                  MANUFACTURER_CODE_NONE, = 0
                  0, NULL, zcl_scene_register,		sampleGW_sceneCb
                - zcl_register(
                    SAMPLE_GW_ENDPOINT,
                    SAMPLE_GW_CB_CLUSTER_NUM, g_sampleGwClusterList);

### Registering ZigBee Clusters and Decoding
- `zcl_register()` called from `user_app_init()`
- `zcl_register()` passed list of cluster definition structures of type `zcl_specClusterInfo_t`

#### Using Time as an Example...
> This works differently to _Scenes_ - why
- `time_attrTbl[]` defines time attributes and the location of storage variables
    - Type is `zclAttrInfo_t`
- `zcl_time_attrNum` defines the number of time attributes

> There appears to be no references to these variables!
>
> **But see `g_sampleGwClusterList[]` which seems to use one, or the other, ways of relating to the attributes.  Compare `Time` to `Scene`.**

- `zcl_specClusterInfo_t`
    - Uses **either** of:
        - `zclAttrInfo_t *attrTbl`, table of attributes (`Time` uses this)
        - `cluster_forAppCb_t clusterAppCb`, cluster processing commands (`Scene` uses this)

> If we want CloudSMETS to send the UTC time (and BST?) to the ESP32-C3 then we probably want to use a `cluster_forAppCb_t` instead of the attribute table.

#### Using Scenes as an Example...
> This works differently to _Time_ - why?
>
> Note that much of this appears to be available for SMETS via `zcl_metering.c` already.

- `zcl_scene_register()` is passed
    - `zcl_scene_register()` is defined in `zcl_scene.c` and registers the `Cluster Identifier` ([ZigBee Cluster Library], 3.7.1.3), the `zcl_scene_cmdHandler`,
    - `zcl_scene_cmdHandler()` is a method in zcl_scene.c and splits command processing between directions e.g. is this a response to a request sent from Service-to-Client or a request being received from Client-to-Server
- if `zcl_scene_register()` succeeds in registering the cluster, it calls `zcl_scene_updateSceneCntAttr()` which appears to reach current scene attributes.

> Does this mean that the `sampleGW_sceneCb` doesn't need to check for direction?

- `sampleGW_sceneCb` is passed
    - `sampleGW_sceneCb` is a handler for the Scenes command/cluster
    - A `cmdId` distinguishes how to decode each incoming message
    - Filtering:
        - _is this for the gateway_ (at least in the `sampleGateway`` example)
        - _is this in the Client-to-Server_ direction?
    - `cmdID`s have defined values in `zcl_scene.h`
    - `cmdID`s match the `Commands Received` definitions in the [ZigBee Cluster Library], section 3.7.2.4 and responses are the same IDs are requests.

- Incoming requesrts are passed to handlers such as `zcl_addScenePrc()` whch calls parsers such as `zcl_addSceneParse()`
- `zcl_addSceneParse()` parses the request into a structure
- Then stuff happens and we send a response.

> But what about the attribute to field references that we elsewhere?  Are there two different ways to handle data?

## Over UART...
### Packet format
- 0x55
- Type (16 bits)
- Length (16 bits)
- CRC (8 bits)
- Data (n bytes)
- 0xAA

### Data Format
- Source address (16 bits)
- Source endpoint (8 bits)
- Destination endpoint (8 bits)
- Sequence number (8 bits)
- Message data (variable)

> Can we just relay the `Message data` to Azure?  Sghouold be OK providing we teach Azure about the structure of the data that we are sending it.
>
> We will also need to make sure that we send a length field and some sort of identifier.  See the UART thing above so perhaps we forward the entire packet?

### Message Data
- Seems to be a representation of the data received over ZigBee but might not precisely match an attribute.
- Not clear how this works other than for a gateway!
- Not clear how notifications might work; but perhaps we can replicate this for a gateway?

#### Message Data Format: Scene Table as an Example
- [ZigBee Cluster Library], 3.7.2.3, defines `Scene Table`
- [Telink ZigBee SDK] defines a `viewSceneRsp_t` structure in `zcl_scene.h` that mirrors the definition of the `Scene Table`
- [Telink ZigBee SDK] `sampleGW_zclViewSceneRspCmdHandler` method in `zcl_sampleGatewayCb.c`, creates the message data from the `viewSceneRsp_t` structure, allowing for zero-length data.


[ZigBee Cluster Library]: https://zigbeealliance.org/wp-content/uploads/2021/10/07-5123-08-Zigbee-Cluster-Library.pdf
[Telink ZigBee SDK]: http://wiki.telink-semi.cn/tools_and_sdk/Zigbee/Zigbee_SDK.zip
[ESP-C3]: https://somewhere
[zbhci]: https://zbhci.readthedocs.io/en/latest/developer-guide/index.html
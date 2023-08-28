# Zigbee Gateway Controller (ZGC) Protocol

## Following Scene processing
Bottom (sending over serial port) downwards...

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
- `time_attrTbl[]` defines time attributes and the location of storage varialbles
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



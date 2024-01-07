// Decoder for received zbhci attribute messages.
//
// The zbhci format is a binary format which consists of:
//
// 0x55     - a "start-message" indicator
// 0x????   - a 2-byte request/response identifier
// 0x????   - a 2-byte payload length
// 0x??     - an 8-byt cyclic redundancy check (CRC)
// 0x??...??
//          - Variable length payload
// 0xAA - an "end-message" indicator
//
// For the purposes of this decoder, the payload is always a "read-attribute(s)
// response" which means that the structure of the payload is always this:
//
// - 0x???? - a 16-bit source address
// - 0x??   - an 8-bit source endpoint
// - 0x??   - an 8-bit destination endpoint
// - 0x??   - an 8-bit sequence number
// - 0x???? - a 16-bit cluster id.
// - 0x??   - an 8-bit number-of-attributes
// - Repeated for each attribute
//   - 0x????   - a 16-bit attribute identifier
//   - 0x??     - an 8-bit attribute state (we only care for 0x00, success)
//   - 0x??     - an 8-bit data type
//   - 0x??...??
//              - A variable number of bits indicating the length
//   - 0x??...??
//              - A variable number of data bits
//
// Note that it is important that we spot and avoid data overruns in the case
// of invalid date and also log any errors so that they cna be identified and
// corrected.

// With zero length payload, need at least 7 bytes.
#define ZBHCI_MIN_FRAME_LENGTH          7

#define ZBHCI_FRAME_START               ((byte)0x55)
#define ZBHCI_FRAME_END                 ((byte)0xAA)

#define ZBHCI_CMD_ZCL_ATTR_READ_RSP     0x8100

#define ZBHCI_OFFSET_PAYLOAD_LENGTH     3
#define ZBHCI_OFFSET_COMMMAND_ID        5
#define ZBHCI_OFFSET_CRC                6
#define ZBHCI_OFFSET_PAYLOAD            7

// With no attributes, payload must be at least 7 bytes.
#define ZBHCI_PAYLOAD_MIN_LENGTH        7

// Exceptions that may be throw whilst parsing the data.
class ZbhciShortFrameException : Exception {};
class ZbhciFramingException : Exception {};
class ZbhciCRCExcException : Exception {};
class ZbhciFrameException : Exception {};
class ZbhciAttributeStateInvalid : Exception {};
class ZbhciNotAttributesException : Exception {};
class ZbhciShortPayloadException : Exception {};
class ZbhciPayloadFrameLengthsException : Exception {};
class ZbhciShortattributeHeader : Exception {};


// Representation of a zbhci payload header.
class ZbhciPayloadHeader
{
#define ZBHCI_POFFSET_SRC_ADDR          0
#define ZBHCI_POFFSET_SRC_ENDPT         1
#define ZBHCI_POFFSET_DEST_ENDPT        2
#define ZBHCI_POFFSET_SEQ_NUM           3
#define ZBHCI_POFFSET_CLUSTER_ID        4
#define ZBHCI_POFFSET_NUM_ATTRS         6

#define ZBHCI_AOFFSET_SIMPLE_VALUE

    byte sourceAddress;
    byte sourceEndpoint
    byte destinationEndpoint;
    byte sequenceNumber;
    ushort clusterId;
    byte numberOfAttributes;

    ZbhciPayloadHeader(ILogger log, int frameLength, byte *frame, int *bytesRead)
    {
        (*bytesRead) = 0;

        // First validate the message.
        // Is the frame long enough?
        if (frameLength < ZBHCI_MIN_FRAME_LENGTH)
        {
            log.LogError(message: $"Frame length '{frameLength}' is too short.");
            throw new ZbhciShortFrameException($"Frame length '{frameLength}' is too short.");
        }

        // Are the framing bytes present?
        if ((frame[0] != ZBHCI_FRAME_START) || (frame[frameLength - 1] != ZBHCI_FRAME_END))
        {
            // TODO: How do we log anarbitary length of binary data?
            log.LogError(message: $"Frame start/end invalid: {frame[0]:2.2X}, {frame[frameLength - 1]:2.2X}.")
            throw new ZbhciFramingException($"Frame start/end invalid: {frame[0]:2.2X}, {frame[frameLength - 1]:2.2X}.");
        }

        // Is this an attribute read response?
        ushort commandId = BitConverter.ToUInt16(frame, ZNHCI_OFFSET_COMMAND_ID);
        commandId = (ushort)NetworkToHostOrder((short)commandId);
        if (commandId != ZBHCI_CMD_ZCL_ATTR_READ_RSP)
        {
            log.LogError(message: $"Frame is not a read-atrtibutes response: {commandId:4.4X}.");
            throw new ZbhciNotAttributesException($"Frame is not a read-atrtibutes response: {commandId:4.4X}.");
        }

        // Is the payload length correct?
        ushort payloadLength = BitConverter.ToUInt16(frame, ZNHCI_OFFSET_PAYLOAD_LENGTH);
        payloadLength = (ushort)NetworkToHostOrder((short)payloadLength);
        if (payloadLength < ZBHCI_PAYLOAD_MIN_LENGTH)
        {
            log.LogError(message: $"Payload is too short: {payloadLength}.")
            throw new ZbhciShortPayloadException($"Payload is too short: {payloadLength}.");
        }

        if ((payloadLength + ZBHCI_MIN_FRAME_LENGTH) != frameLength)
        {
            log.LogError(message: $"Payload/frame lengths do not match: {payloadLength}, {frameLength}.");
            throw new ZbhciPayloadFrameLengthsException($"Payload/frame lengths do not match: {payloadLength}, {frameLength}.");
        }

        // Is the CRC correct?
        byte crc = crc8Calculate(commandId, payloadLength, payload);
        if (crc != frame[ZBHCI_OFFSET_CRC])
        {
            log.LogError(message: $"CRC is incorrect: expected: {crc:2.2X}, received: {frame[ZBHCI_OFFSET_CRC]:2.2X}.");
            throw new ZbhciCRCExcException($"CRC is incorrect: expected: {crc:2.2X}, received: {frame[ZBHCI_OFFSET_CRC]:2.2X}.");
        }

        // Safe to access and store payload fields.
        this.sourceAddress = payload[ZBHCI_POFFSET_SRC_ADDR];
        this.sourceEndpoint = payload[ZBHCI_POFFSET_SRC_ENDPT];
        this.destinationEndpoint = payload[ZBHCI_POFFSET_DEST_ENDPT];
        this.sequenceNumber = payload[ZBHCI_POFFSET_SEQ_NUM];
        this.numberOfAttributes = payload[ZBHCI_POFFSET_NUM_ATTRS];
        this.clusterId = BitConverter.ToUInt16(frame, ZBHCI_POFFSET_CLUSTER_ID);
        this.clusterId = (ushort)NetworkToHostOrder((short)this.clusterId);

        // TODO: Is this the best we can do?
        (*bytesRead) = 7;
    }

    private byte Crc8Calculate(ushort commandId, ushort payloadLength, byte *payload)
    {
        byte crc8;

        crc8 = (dataType >> 0) & 0xff;
        crc8 ^= (dataType >> 8) & 0xff;
        crc8 ^= (payloadLength >> 0) & 0xff;
        crc8 ^= (payloadLength >> 8) & 0xff;

        for (ushort n = 0; n < payloadLength; n++)
        {
            crc8 ^= payload[n]
        }

        return crc8
    }

}

// - Repeated for each attribute
//   - 0x????   - a 16-bit attribute identifier
//   - 0x??     - an 8-bit attribute state (we only care for 0x00, success)
//   - 0x??     - an 8-bit data type
//   - 0x??...??
//              - A variable number of bits indicating the length
//   - 0x??...??
//              - A variable number of data bits

// This union only supports the values that we expect to receive, and converted
// to data types that we can sensibly give to the database so for example
// 48-bit data type is stored as a uint64.
union ZbhciAttributeValue
{
    ushort valueUint16;
    uint valueUint32;
    ulong valueUint64;
    string valueStr;
}

#define ZBHCI_AOFFSET_IDENTIFIER            0
#define ZBHCI_AOFFSET_STATUS                2
#define ZBHCI_AOFFSET_DATA_TYPE             3
#define ZBHCI_AOFFSET_LENGTH                4

// Note that the length field for simple values is implicit.
#define ZBHCI_AOFFSET_SIMPLE_VALUE          4
#define ZBHCI_AOFFSET_SHORT_STR_OCT         5
#define ZBHCI_AOFFSET_LONG_STR_OCT          6

class ZbhciAttribute
{
    public ushort identifier;
    public byte status;
    public byte dataType;
    public ushort length;
    public zbhciAttributeValue value;
    ZbhciAttribute(ILogger log, int dataLength, byte *data, int *bytesRead)
    {
        (*bytesRead) = 0;

        // Validate we have enough data left for an attribute read.
        if (dataLength < ZBHCI_ATTR_HDR_LEN)
        {
            log.LogError($"Insufficient data, {dataLength}, for an attribute header.");
            throw new ZbhciShortAttributeHeader($"Insufficient data, {dataLength}, for an attribute header.");
        }

        this identier = BitConverter.ToUInt16(frame, ZBHCI_AOFFSET_IDENTIFIER);
        this.identifier = (ushort)NetworkToHostOrder((short)this.identifier);
        this.status = frame[ZBHCI_AOFFSET_STATUS];
        if (this.status != ZCL_STA_SUCCESS)
        {
            log.LogError(message: $"Attribute status {this.status:2.2X} for attribute {this.identifier:4.4X} is invalid.);
            throw new ZbhciAttributeStateInvalid($"Attribute status {this.status:2.2X} for attribute {this.identifier:4.4X} is invalid.");
        }
        this.dataType = frame[ZBHCI_AOFFSET_DATA_TYPE];

        // Reduce the size of the available data.
        dataLength -= 4;
        data += 4;
        (*bytesRead) = 4;

        attributeLength = this.GetAttrSize(this.dataType, dataLength, data, &lengthBytesRead);
    }

    private GetAttrSize(byte dataType, int dataLength, byte *data, int &attributeLength, int *bytesRead)
    {
        int length = 0;

        (*bytesRead) = 0;

        switch (dataType)
        {
            case ZCL_DATA_TYPE_LONG_CHAR_STR:
            case ZCL_DATA_TYPE_LONG_OCTET_STR:
                // TODO: Write a common check length that takes a string as input etc.
                length = BitConverter.ToUInt16(frame, ZBHCI_AOFFSET_STR_LENGTH);
                length = (ushort)NetworkToHostOrder((short)length);
                (*bytesRead) = 2;
                break;

            case ZCL_DATA_TYPE_CHAR_STR:
            case ZCL_DATA_TYPE_OCTET_STR:
                length = frame(ZBHCI_AOFFSET_STR_LENGTH);
                (*bytesRead) = 1;
                break;

            case ZCL_DATA_TYPE_STRUCT:
                // Structures consist of a list of attributes, each with a length.
                byte itemNum = frame[ZBHCI_AOFFSET_STRUCT_NUM_ITEMS];
                (*bytesRead) = 1;
                while (itemNum--)
                {
                    byte itemDataType = frame[ZBHCI_S_DATA_TYPE];
                    length += getAttrSize(itemDataType, dataLength, data, ???);
                }
                break;

            case ZCL_DATA_TYPE_DATA8:
            case ZCL_DATA_TYPE_BOOLEAN:
            case ZCL_DATA_TYPE_BITMAP8:
            case ZCL_DATA_TYPE_INT8:
            case ZCL_DATA_TYPE_ENUM8:
            case ZCL_DATA_TYPE_UINT8:
                length = 1;
                break;

            case ZCL_DATA_TYPE_DATA16:
            case ZCL_DATA_TYPE_BITMAP16:
            case ZCL_DATA_TYPE_UINT16:
            case ZCL_DATA_TYPE_INT16:
            case ZCL_DATA_TYPE_ENUM16:
            case ZCL_DATA_TYPE_SEMI_PREC:
            case ZCL_DATA_TYPE_CLUSTER_ID:
            case ZCL_DATA_TYPE_ATTR_ID:
                length = 2;
                break;

            case ZCL_DATA_TYPE_DATA24:
            case ZCL_DATA_TYPE_BITMAP24:
            case ZCL_DATA_TYPE_UINT24:
            case ZCL_DATA_TYPE_INT24:
                length = 3;
                break;

            case ZCL_DATA_TYPE_DATA32:
            case ZCL_DATA_TYPE_BITMAP32:
            case ZCL_DATA_TYPE_UINT32:
            case ZCL_DATA_TYPE_INT32:
            case ZCL_DATA_TYPE_SINGLE_PREC:
            case ZCL_DATA_TYPE_DATE:
            case ZCL_DATA_TYPE_UTC:
            case ZCL_DATA_TYPE_BAC_OID:
                length = 4;
                break;

            case ZCL_DATA_TYPE_DATA40:
            case ZCL_DATA_TYPE_BITMAP40:
            case ZCL_DATA_TYPE_UINT40:
            case ZCL_DATA_TYPE_INT40:
                length = 5;
                break;

            case ZCL_DATA_TYPE_DATA48:
            case ZCL_DATA_TYPE_BITMAP48:
            case ZCL_DATA_TYPE_UINT48:
            case ZCL_DATA_TYPE_INT48:
                length = 6;
                break;

            case ZCL_DATA_TYPE_DATA56:
            case ZCL_DATA_TYPE_BITMAP56:
            case ZCL_DATA_TYPE_UINT56:
            case ZCL_DATA_TYPE_INT56:
                length = 7;
                break;

            case ZCL_DATA_TYPE_DATA64:
            case ZCL_DATA_TYPE_BITMAP64:
            case ZCL_DATA_TYPE_UINT64:
            case ZCL_DATA_TYPE_INT64:
            case ZCL_DATA_TYPE_DOUBLE_PREC:
            case ZCL_DATA_TYPE_IEEE_ADDR:
                length = 8;
                break;

            case ZCL_DATA_TYPE_128_BIT_SEC_K:
                length = 16;
                break;

            // case ZCL_DATA_TYPE_ARRAY
            // case ZCL_DATA_TYPE_SET
            // case ZCL_DATA_TYPE_BAG
            // case ZCL_DATA_TYPE_TOD
            default:
                log.LogError("Unsupported attribute type.");
                break;
        }
        return length;
    }

    protected virtual void ParseValue(ILogger log, int frameLength, byte *frame)
    {
        throw new NotImplementedException($"ParseValue must be overriden by subclass.")
    }
}

class ZbhciAttributeUint16 : ZbhciAttribute
{
    private override void ParseValue(ILogger log, int frameLength, byte *frame)
    {
        ushort value;
        value = BitConverter.ToUInt16(frame, ZBHCI_AOFFSET_SIMPLE_VALUE);
        this.value.valueUint16 = (ushort)NetworkToHostOrder((short)value);
    }
}

class ZbhciAttributeUint48 : ZbhciAttribute
{
    private override void ParseValue(ILogger log, int frameLength, byte *frame)
    {
        ushort value16;
        ulong value32;
        uint value;
        value32 = BitConverter.ToUInt32(frame, ZBHCI_AOFFSET_SIMPLE_VALUE);
        value32 = (uint)NetworkToHostOrder((int)value);
        value16 = BitConverter.ToUInt32(frame, ZBHCI_AOFFSET_SIMPLE_VALUE + 2);
        value16 = (ushort)NetworkToHostOrder((short)value16);
        this.value.valueUint64 = ((ulong)value32 << 16) + value16;
    }
}

class ZbhciAttributeUint32 : ZbhciAttribute
{
    private override void ParseValue(ILogger log, int frameLength, byte *frame)
    {
        uint value;
        value = BitConverter.ToUInt32(frame, ZBHCI_AOFFSET_SIMPLE_VALUE);
        this.value.valueUint32 = (uint)NetworkToHostOrder((int)value);
    }
}

class ZbhciAttributeUint64 : ZbhciAttribute
{
    private override void ParseValue(ILogger log, int frameLength, byte *frame)
    {
        ulong value;
        value = BitConverter.ToUInt64(frame, ZBHCI_AOFFSET_SIMPLE_VALUE);
        this.value.valueUint64 = (ulong)NetworkToHostOrder((long)value);
    }
}

class ZbhciAttributeShortStr : ZbhciAttribute
{
    private override void ParseValue(ILogger log, int frameLength, byte *frame, ushort valueLength)
    {
        this.value.valueStr = new string(frame, ZBHCI_AOFFSET_SHORT_STR_OCT, valueLength);
    }
}

class ZbhciAttributeLongStr : ZbhciAttribute
{
    private override void ParseValue(ILogger log, int frameLength, byte *frame, ushort valueLength)
    {
        this.value.valueStr = new string(frame, ZBHCI_AOFFSET_LONG_STR_OCT, valueLength);
    }
}

public static class ZbhciMessage
{
    public ZbhciPayloadHeader payloadHeader;
    public List<ZbhciAttribute> attributes = new List<ZbhciAttribute>();

    ZbhciMessage(ILogger log, int frameLength, byte *frame)
    {
        byte *framePtr = frame;
        int bytesRead;

        this.log = log;
        this.frameLength = frameLength;
        this.frame = frame;

        // Read the payload header.
        ZbhciPayloadHeader payloadHeader = new ZbhciPayloadHeader(log, frameLength, framePtr, &bytesRead);
        framePtr = framePtr + bytesRead;
        frameLength -= bytesRead;

        // Parse out the attributes.
        for (byte n = 0; n < payloadHeader.numberOfAttributes; n++)
        {
            attributes.Add(ZbhciAttribute.ReadAttribute(log, frameLength, framePtr, &bytesRead);
            framePtr += bytesRead;
            frameLength -= bytesRead;
        }
    }
}

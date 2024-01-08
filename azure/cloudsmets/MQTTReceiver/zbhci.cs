// Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
// License: Free to copy providing the author is acknowledged.namespace CloudSMETS.zbhci

using System;
using System.Collections.Generic;
using System.Net;
using System.Runtime.InteropServices;
using Microsoft.Extensions.Logging;

namespace CloudSMETS.zbhci
{
    // Representation of a zbhci payload header.
    class ZbhciPayloadHeader
    {
        readonly byte sourceAddress;
        readonly byte sourceEndpoint;
        readonly byte destinationEndpoint;
        readonly byte sequenceNumber;
        readonly ushort clusterId;
        readonly byte numberOfAttributes;

        ZbhciPayloadHeader(byte[] payload, ref int offset)
        {
            int readOffset = offset;

            ushort commandId = BitConverter.ToUInt16(payload, readOffset + ZbhciFrameConstant.OFFSET_COMMMAND_ID);
            commandId = (ushort)IPAddress.NetworkToHostOrder((short)commandId);
            if (commandId != ZbhciCommandId.ATTR_READ_RSP)
            {
                throw new ZbhciNotAttributesException($"Frame is not a read-attributes response: {commandId:4.4X}.");
            }

            // Is the payload length correct?
            ushort payloadLength = BitConverter.ToUInt16(payload, readOffset + ZbhciFrameConstant.OFFSET_PAYLOAD_LENGTH);
            payloadLength = (ushort)IPAddress.NetworkToHostOrder((short)payloadLength);
            if (payloadLength < ZbhciPayloadConstant.MIN_LENGTH)
            {
                throw new ZbhciShortPayloadException($"Payload is too short: {payloadLength}.");
            }

            if ((payloadLength + readOffset + ZbhciFrameConstant.MIN_FRAME_LENGTH) != payload.GetLength(0))
            {
                throw new ZbhciPayloadFrameLengthsException($"Payload/frame lengths do not match: {payloadLength}, {payload.GetLength(0)}.");
            }

            // Is the CRC correct?
            byte crc = Crc8Calculate(commandId, payload);
            if (crc != payload[readOffset + ZbhciFrameConstant.OFFSET_CRC])
            {
                throw new ZbhciCRCExcException($"CRC is incorrect: expected: {crc:2.2X}, received: {payload[readOffset + ZbhciMessageConstant.OFFSET_CRC]:2.2X}.");
            }

            // Safe to access and store payload fields.
            this.sourceAddress = payload[readOffset + ZbhciPayloadConstant.OFFSET_SRC_ADDR];
            this.sourceEndpoint = payload[readOffset + ZbhciPayloadConstant.OFFSET_SRC_ENDPT];
            this.destinationEndpoint = payload[readOffset + ZbhciPayloadConstant.OFFSET_DEST_ENDPT];
            this.sequenceNumber = payload[readOffset + ZbhciPayloadConstant.OFFSET_SEQ_NUM];
            this.numberOfAttributes = payload[readOffset + ZbhciPayloadConstant.OFFSET_NUM_ATTRS];
            this.clusterId = BitConverter.ToUInt16(payload, readOffset + ZbhciPayloadConstant.OFFSET_CLUSTER_ID);
            this.clusterId = (ushort)IPAddress.NetworkToHostOrder((short)this.clusterId);

            // TODO: Is this the best we can do?
            offset = readOffset;
        }

        private static byte Crc8Calculate(ushort commandId, ushort payloadLength, byte[] payload)
        {
            byte crc8;

            crc8 = (byte)((commandId >> 0) & 0xff);
            crc8 ^= (byte)((commandId >> 8) & 0xff);
            crc8 ^= (byte)((payloadLength >> 0) & 0xff);
            crc8 ^= (byte)((payloadLength >> 8) & 0xff);

            for (ushort n = 0; n < payloadLength; n++)
            {
                crc8 ^= payload[n];
            }

            return crc8;
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
    [StructLayout(LayoutKind.Explicit)]
    public struct ZbhciAttributeValue
    {
        [FieldOffset(0)] public ushort valueUint16;
        [FieldOffset(0)] public uint valueUint32;
        [FieldOffset(0)] public ulong valueUint64;
        [FieldOffset(0)] public string valueStr;
    }

    class ZbhciAttribute
    {
        public ZbhciAttributeValue value;
        ZbhciAttribute(ILogger log, ushort identifier, byte status, byte dataType, byte[] attribute, ref int offset)
        {
            throw new TypeInitializationException("Use factory method, GetAttribute().");
        }

        public static ZbhciAttribute GetAttribute(ILogger log, byte[] attribute, ref int offset)
        {
            ZbhciAttribute attribute;
            ushort identifier;
            byte status;
            byte dataType;
            ushort length;

            ushort identier = BitConverter.ToUInt16(attribute, offset + ZbhciAttributeConstant.OFFSET_IDENTIFIER);
            identifier = (ushort)IPAddress.NetworkToHostOrder((short)this.identifier);
            byte status = attribute[offset + ZbhciAttributeConstant.OFFSET_STATUS];
            if (status != ZCL_STA_SUCCESS)
            {
                throw new ZbhciAttributeStateInvalid($"Attribute status {this.status:2.2X} for attribute {this.identifier:4.4X} is invalid.");
            }
            byte dataType = attribute[offset + ZbhciAttributeConstant.OFFSET_DATA_TYPE];
            offset += ZbhciAttributeConstant.OFFSET_LENGTH;

            offset += ZbhciAttributeConstant.OFFSET_SIMPLE_VALUE;

            switch (dataType)
            {
                case ZbhciDataType.LONG_CHAR_STR:
                case ZbhciDataType.LONG_OCTET_STR:
                    // TODO: Write a common check length that takes a string as input etc.
                    length = BitConverter.ToUInt16(attribute, readOffset);
                    length = (ushort)IPAddress.NetworkToHostOrder((short)length);
                    valueOffset += ZbhciAttributeConstant.LONG_STR_LENGTH_LENGTH;
                    attribute = new ZbhciAttributeLongStr(log, identifier, status, dataType, attribute, length, offset);
                    break;

                case ZbhciDataType.CHAR_STR:
                case ZbhciDataType.OCTET_STR:
                    length = attribute[readOffset];
                    readOffset += ZbhciAttributeConstant.SHORT_STR_LENGTH_LENGTH;
                    attribute = new ZbhciAttributeShortStr(log, identifier, status, dataType, attribute, length, offset);
                    break;

                // No support for these as not required.
                // case ZbhciDataType.STRUCT:
                //    // Structures consist of a list of attributes, each with a length.
                //    byte itemNum = frame[ZBHCI_AOFFSET_STRUCT_NUM_ITEMS];
                //    (*offset) = 1;
                //    while (itemNum--)
                //    {
                //        byte itemDataType = frame[ZBHCI_S_DATA_TYPE];
                //        length += getAttrSize(itemDataType, dataLength, data, ???);
                //    }
                //    break;

                case ZbhciDataType.DATA8:
                case ZbhciDataType.BOOLEAN:
                case ZbhciDataType.BITMAP8:
                case ZbhciDataType.INT8:
                case ZbhciDataType.ENUM8:
                case ZbhciDataType.UINT8:
                    length = 1;
                    break;

                case ZbhciDataType.DATA16:
                case ZbhciDataType.BITMAP16:
                case ZbhciDataType.UINT16:
                case ZbhciDataType.INT16:
                case ZbhciDataType.ENUM16:
                case ZbhciDataType.SEMI_PREC:
                case ZbhciDataType.CLUSTER_ID:
                case ZbhciDataType.ATTR_ID:
                    length = 2;
                    attribute = new ZbhciAttributeUint16(log, identifier, status, dataType, attribute, offset);
                    break;

                // case ZbhciDataType.DATA24:
                // case ZbhciDataType.BITMAP24:
                // case ZbhciDataType.UINT24:
                // case ZbhciDataType.INT24:
                //     length = 3;
                //     break;

                case ZbhciDataType.DATA32:
                case ZbhciDataType.BITMAP32:
                case ZbhciDataType.UINT32:
                case ZbhciDataType.INT32:
                case ZbhciDataType.SINGLE_PREC:
                case ZbhciDataType.DATE:
                case ZbhciDataType.UTC:
                case ZbhciDataType.BAC_OID:
                    length = 4;
                    attribute = new ZbhciAttributeUint32(log, identifier, status, dataType, attribute, offset);
                    break;

                // case ZbhciDataType.DATA40:
                // case ZbhciDataType.BITMAP40:
                // case ZbhciDataType.UINT40:
                // case ZbhciDataType.INT40:
                //     length = 5;
                //     break;

                // case ZbhciDataType.DATA48:
                // case ZbhciDataType.BITMAP48:
                // case ZbhciDataType.UINT48:
                // case ZbhciDataType.INT48:
                //     length = 6;
                //     break;

                // case ZbhciDataType.DATA56:
                // case ZbhciDataType.BITMAP56:
                // case ZbhciDataType.UINT56:
                // case ZbhciDataType.INT56:
                //     length = 7;
                //     break;

                case ZbhciDataType.DATA64:
                case ZbhciDataType.BITMAP64:
                case ZbhciDataType.UINT64:
                case ZbhciDataType.INT64:
                case ZbhciDataType.DOUBLE_PREC:
                case ZbhciDataType.IEEE_ADDR:
                    length = 8;
                    attribute = new ZbhciAttributeUint64(log, identifier, status, dataType, attribute, offset);
                    break;

                // case ZbhciDataType.BIT_128_SEC_K:
                //     length = 16;
                //     break;

                // case ZbhciDataType.ARRAY
                // case ZbhciDataType.SET
                // case ZbhciDataType.BAG
                // case ZbhciDataType.TOD
                default:
                    throw new ZbhciUnsupportedValueTypeException($"Unsupported attribute type: {dataType:2.2X}.");
            }

            offset += length;
            return attribute;
        }

        protected virtual void ParseValue(ILogger log, int frameLength, byte[] frame)
        {
            throw new NotImplementedException($"ParseValue must be overriden by subclass.");
        }
    }

    class ZbhciAttributeUint16 : ZbhciAttribute
    {
        public ZbhciAttributeUint16(ushort identifier, byte status, byte dataType, byte[] attribute, ref int offset)
        {
            this.identifier = identifier;
            this.status = status;
            this.dataType = dataType;

            ushort value;

            value = BitConverter.ToUInt16(attribute, offset);
            this.value.valueUint16 = (ushort)IPAddress.NetworkToHostOrder((short)value);
        }
    }

    class ZbhciAttributeUint48 : ZbhciAttribute
    {
        public ZbhciAttributeUint48(ushort identifier, byte status, byte dataType, byte[] attribute, ref int offset)
        {
            this.identifier = identifier;
            this.status = status;
            this.dataType = dataType;

            ushort value16;
            ulong value32;

            value32 = BitConverter.ToUInt32(attribute, offset);
            value32 = (uint)IPAddress.NetworkToHostOrder((int)value32);
            value16 = (ushort)BitConverter.ToUInt32(attribute, offset + 2);
            value16 = (ushort)IPAddress.NetworkToHostOrder((short)value16);
            this.value.valueUint64 = ((ulong)value32 << 16) + value16;
        }
    }

    class ZbhciAttributeUint32 : ZbhciAttribute
    {
        public ZbhciAttributeUint32(ushort identifier, byte status, byte dataType, byte[] attribute, ref int offset)
        {
            this.identifier = identifier;
            this.status = status;
            this.dataType = dataType;

            uint value;
            value = BitConverter.ToUInt32(attribute, offset);
            this.value.valueUint32 = (uint)IPAddress.NetworkToHostOrder((int)value);
        }
    }

    class ZbhciAttributeUint64 : ZbhciAttribute
    {
        public ZbhciAttributeUint64(ushort identifier, byte status, byte dataType, byte[] attribute, ref int offset)
        {
            this.identifier = identifier;
            this.status = status;
            this.dataType = dataType;

            ulong value;
            value = BitConverter.ToUInt64(attribute, offset);
            this.value.valueUint64 = (ulong)IPAddress.NetworkToHostOrder((long)value);
        }
    }

    class ZbhciAttributeShortStr : ZbhciAttribute
    {
        public ZbhciAttributeShortStr(ushort identifier, byte status, byte dataType, int length, byte[] attribute, ref int offset)
        {
            this.identifier = identifier;
            this.status = status;
            this.dataType = dataType;

            this.value.valueStr = System.Text.Encoding.UTF8.GetString(attribute, offset, length);
        }
    }

    class ZbhciAttributeLongStr : ZbhciAttribute
    {
        publicZbhciAttributeLongStr(ushort identifier, byte status, byte dataType, byte[] attribute, ref int offset)
        {
            this.identifier = identifier;
            this.status = status;
            this.dataType = dataType;

            this.value.valueStr = System.Text.Encoding.UTF8.GetString(attribute, offset, length);
        }
    }

    public static class ZbhciFrame
    {

        static List<ZbhciAttribute> ZbhciMessage(ILogger log, byte[] frame)
        {
            ZbhciPayloadHeader payloadHeader;
            List<ZbhciAttribute> attributes = new();
            int offset = 0;

             // Are the framing bytes present?
            if ((frame[0] != ZbhciFrameConstant.FRAME_START) || (frame[frame.GetLength(0) - 1] != ZbhciFrameConstant.FRAME_END))
            {
                throw new ZbhciFramingException($"Frame start/end invalid: {frame[0]:2.2X}, {frame[frame.GetLength(0) - 1]:2.2X}.");
            }

           // Read the payload header, which starts after the frame start byte.
            offset = 1;
            payloadHeader = new ZbhciPayloadHeader(log, frame, offset);

            // Parse out the attributes.
            for (byte n = 0; n < payloadHeader.numberOfAttributes; n++)
            {
                attributes.Add(ZbhciAttribute.GetAttribute(log, frame, ref offset));
            }

            return attributes;
        }
    }
}
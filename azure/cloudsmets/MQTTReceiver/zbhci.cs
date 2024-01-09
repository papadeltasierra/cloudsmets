// Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
// License: Free to copy providing the author is acknowledged.namespace CloudSMETS.zbhci

using System;
using System.Collections.Generic;
using System.Net;
using System.Runtime.InteropServices;
using Microsoft.Extensions.Logging;

// The zbhci frame format is as follows.
//
// +---
// | Frame
// | - 0x55
// | - 0xhhhh  Command Id
// | - 0xhhhh  Payload length
// | - 0xhh    CRC
// | +---
// | | Payload
// | | +--
// | | | Attributes header
// | | | - 0xhhhh  Source address
// | | | - 0xhhhh  Source endpoint
// | | | - 0xhhhh  Destination endpoint
// | | | - 0xhhhh  Cluster Id
// | | | - 0xhh    Number of attributes
// | | | +--
// | | | | Attribute 1
// | | | | - 0xhhhh        Attribute Id
// | | | | - 0xhh          Data type
// | | | | - (0xhh(hh))    Data length (if data type is string)
// | | | | - 0xhh...       Attribute value
// | | | +--
// | | | ...
// | | | +--
// | | | | Attribute n
// | | | | - 0xhhhh        Attribute Id
// | | | | - 0xhh          Data type
// | | | | - (0xhh(hh))    Data length (if data type is string)
// | | | | - 0xhh...       Attribute value
// | | | +--
// | | +--
// | +---
// | - 0xAA
// +--
//
// Attributes can but structures of nested attributes too although CloudSMETS
// doesn't use these.

namespace CloudSMETS.zbhci
{
    // Representation of a zbhci payload header.
    public class ZbhciFrameHeader
    {
        const byte FRAME_START = 0x55;
        const byte FRAME_END = 0xAA;

        public ushort commandId;
        public ushort payloadLength;
        public byte crc8;

       public ZbhciFrameHeader(byte[] frame, ref int offset)
        {
             // Are the framing bytes present?
            if ((frame[0] != FRAME_START) || (frame[frame.GetLength(0) - 1] != FRAME_END))
            {
                throw new ZbhciFramingException($"Frame start/end invalid: 0x{frame[0]:X2}, 0x{frame[frame.GetLength(0) - 1]:X2}.");
            }
            // Step past the frame-start.
            offset++;

            ushort commandId = BitConverter.ToUInt16(frame, offset);
            offset += 2;
            commandId = (ushort)IPAddress.NetworkToHostOrder((short)commandId);

            // Is the command length correct?
            ushort payloadLength = BitConverter.ToUInt16(frame, offset);
            offset += 2;

            // We do not validate payload length because C# will throw an Exception
            // if we try to read beyond the end of the message.
            payloadLength = (ushort)IPAddress.NetworkToHostOrder((short)payloadLength);

            // Is the CRC correct?
            this.crc8 = Crc8Calculate(commandId, payloadLength, frame, offset);
            if (this.crc8 != frame[offset])
            {
                throw new ZbhciCRCExcException($"CRC is incorrect: expected: 0x{this.crc8:X2}, received: 0x{frame[offset]:X2}.");
            }
            offset++;
        }

        private static byte Crc8Calculate(ushort commandId, ushort payloadLength, byte[] payload, int offset)
        {
            byte crc8;

            crc8 = (byte)((commandId >> 0) & 0xff);
            int icrc8 = crc8 ^ ((commandId >> 8) & 0xff);
            crc8 = (byte)(crc8 ^ (byte)((commandId >> 8) & 0xff));
            crc8 = (byte)(crc8 ^ (byte)((payloadLength >> 0) & 0xff));
            crc8 = (byte)(crc8 ^ (byte)((payloadLength >> 8) & 0xff));

            for (ushort n = 1; n <= payloadLength; n++)
            {
                crc8 = (byte)(crc8 ^ payload[offset + n]);
            }

            return crc8;
        }
    }

    public class ZbhciCommandHeader
    {
        public readonly ushort sourceAddress;
        public readonly byte sourceEndpoint;
        public readonly byte destinationEndpoint;
        public readonly byte sequenceNumber;
        public readonly ushort clusterId;

        public ZbhciCommandHeader(byte[] command, ref int offset)
        {

            // Safe to access and store payload fields.
            this.sourceAddress = BitConverter.ToUInt16(command, offset);
            offset += 2;
            this.sourceAddress = (ushort)IPAddress.NetworkToHostOrder((short)this.sourceAddress);

            this.sourceEndpoint = command[offset++];
            this.destinationEndpoint = command[offset++];
            this.sequenceNumber = command[offset++];

            this.clusterId = BitConverter.ToUInt16(command, offset);
            offset += 2;
            this.clusterId = (ushort)IPAddress.NetworkToHostOrder((short)this.clusterId);
        }
    }

    public class ZbhciAttribute
    {
        public ushort identifier;
        public byte status;
        public byte dataType;

        public ZbhciAttribute(ushort identifier, byte status, byte dataType)
        {
            this.identifier = identifier;
            this.status = status;
            this.dataType = dataType;
        }

        public static ZbhciAttribute GetAttribute(byte[] attribute, ref int offset)
        {
            ZbhciAttribute zbhciAttribute;
            ushort identifier;
            byte status;
            byte dataType;
            ushort length;

            identifier = BitConverter.ToUInt16(attribute, offset);
            offset += 2;
            identifier = (ushort)IPAddress.NetworkToHostOrder((short)identifier);
            status = attribute[offset++];
            if (status != ZbhciStatus.SUCCESS)
            {
                throw new ZbhciAttributeStateInvalid($"Attribute status 0x{status:X2} for attribute 0x{identifier:X4} is invalid.");
            }
            dataType = attribute[offset++];

            switch (dataType)
            {
                case ZbhciDataType.LONG_CHAR_STR:
                case ZbhciDataType.LONG_OCTET_STR:
                    // TODO: Write a common check length that takes a string as input etc.
                    length = BitConverter.ToUInt16(attribute, offset);
                    offset += 2;
                    length = (ushort)IPAddress.NetworkToHostOrder((short)length);
                    zbhciAttribute = new ZbhciAttributeLongStr(identifier, status, dataType, length, attribute, ref offset);
                    break;

                case ZbhciDataType.CHAR_STR:
                case ZbhciDataType.OCTET_STR:
                    length = attribute[offset++];
                    zbhciAttribute = new ZbhciAttributeShortStr(identifier, status, dataType, length, attribute, ref offset);
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

                // case ZbhciDataType.DATA8:
                // case ZbhciDataType.BOOLEAN:
                // case ZbhciDataType.BITMAP8:
                // case ZbhciDataType.INT8:
                // case ZbhciDataType.ENUM8:
                // case ZbhciDataType.UINT8:
                //     length = 1;
                //     break;

                case ZbhciDataType.DATA16:
                case ZbhciDataType.BITMAP16:
                case ZbhciDataType.UINT16:
                case ZbhciDataType.INT16:
                case ZbhciDataType.ENUM16:
                case ZbhciDataType.SEMI_PREC:
                case ZbhciDataType.CLUSTER_ID:
                case ZbhciDataType.ATTR_ID:
                    zbhciAttribute = new ZbhciAttributeUint16(identifier, status, dataType, attribute, ref offset);
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
                    zbhciAttribute = new ZbhciAttributeUint32(identifier, status, dataType, attribute, ref offset);
                    break;

                    // case ZbhciDataType.DATA40:
                    // case ZbhciDataType.BITMAP40:
                    // case ZbhciDataType.UINT40:
                    // case ZbhciDataType.INT40:
                    //     length = 5;
                    //     break;

                case ZbhciDataType.DATA48:
                case ZbhciDataType.BITMAP48:
                case ZbhciDataType.UINT48:
                case ZbhciDataType.INT48:
                    zbhciAttribute = new ZbhciAttributeUint48(identifier, status, dataType, attribute, ref offset);
                    break;

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
                    zbhciAttribute = new ZbhciAttributeUint64(identifier, status, dataType, attribute, ref offset);
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
            return zbhciAttribute;
        }
    }

    class ZbhciAttributeUint16 : ZbhciAttribute
    {
        public ushort value;

        public ZbhciAttributeUint16(ushort identifier, byte status, byte dataType, byte[] attribute, ref int offset) : base(identifier, status, dataType)
        {
            this.value = BitConverter.ToUInt16(attribute, offset);
            offset += 2;
            this.value = (ushort)IPAddress.NetworkToHostOrder((short)this.value);
        }
    }

    class ZbhciAttributeUint48 : ZbhciAttribute
    {
        public ulong value;

        public ZbhciAttributeUint48(ushort identifier, byte status, byte dataType, byte[] attribute, ref int offset) : base(identifier, status, dataType)
        {
            ushort value16;
            ulong value32;

            value32 = BitConverter.ToUInt32(attribute, offset);
            offset += 4;
            value32 = (uint)IPAddress.NetworkToHostOrder((int)value32);
            value16 = (ushort)BitConverter.ToUInt32(attribute, offset);
            offset += 2;
            value16 = (ushort)IPAddress.NetworkToHostOrder((short)value16);
            this.value = ((ulong)value32 << 16) + value16;
        }
    }

    class ZbhciAttributeUint32 : ZbhciAttribute
    {
        public uint value;

        public ZbhciAttributeUint32(ushort identifier, byte status, byte dataType, byte[] attribute, ref int offset) : base(identifier, status, dataType)
        {
            this.value = BitConverter.ToUInt32(attribute, offset);
            offset += 4;
            this.value = (uint)IPAddress.NetworkToHostOrder((int)this.value);
        }
    }

    class ZbhciAttributeUint64 : ZbhciAttribute
    {
        public ulong value;

        public ZbhciAttributeUint64(ushort identifier, byte status, byte dataType, byte[] attribute, ref int offset) : base(identifier, status, dataType)
        {
            this.value = BitConverter.ToUInt64(attribute, offset);
            this.value = (ulong)IPAddress.NetworkToHostOrder((long)this.value);
        }
    }

    class ZbhciAttributeShortStr : ZbhciAttribute
    {
        public string value;

        public ZbhciAttributeShortStr(ushort identifier, byte status, byte dataType, int length, byte[] attribute, ref int offset) : base(identifier, status, dataType)
        {
            this.value = System.Text.Encoding.UTF8.GetString(attribute, offset, length);
        }
    }

    public class ZbhciAttributeLongStr : ZbhciAttribute
    {
        public string value;

        public ZbhciAttributeLongStr(ushort identifier, byte status, byte dataType, ushort length, byte[] attribute, ref int offset) : base(identifier, status, dataType)
        {
            this.value = System.Text.Encoding.UTF8.GetString(attribute, offset, length);
        }
    }

    public class ZbhciMessage
    {
        public ZbhciFrameHeader frameHeader;
        public ZbhciCommandHeader commandHeader;
        public List<ZbhciAttribute> attributeList = new ();

        public ZbhciMessage(byte[] frame)
        {
            int offset = 0;

            // Read the frame header.
            this.frameHeader = new ZbhciFrameHeader(frame, ref offset);

            // Read the payload header, which starts after the frame start byte.
            this.commandHeader = new ZbhciCommandHeader(frame, ref offset);

            if (this.frameHeader.commandId != ZbhciCommandId.ATTR_READ_RSP)
            {
                // We only support attribute read responses.
                throw new ZbhciNotAttributesException($"Frame is not a read-attributes response: 0x{this.frameHeader.commandId:X4}.");
            }

            // Read the number of attributes; we don't bother storing this because
            // a C# list has a count.
            int numberOfAttributes = (int)frame[offset++];

            for (int n = 0; n < numberOfAttributes; n++)
            {
                this.attributeList.Add(ZbhciAttribute.GetAttribute(frame, ref offset));
            }
        }
    }
}
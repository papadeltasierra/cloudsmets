// Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
// License: Free to copy providing the author is acknowledged.namespace CloudSMETS.zbhci

namespace CloudSMETS.zbhci
{
    public static class ZbhciFrameConstant
    {
        // The zbhci format is a binary format which consists of:
        //
        // 0x55 = - a "start-message" indicator
        // 0x???? = - a 2-byte request/response identifier
        // 0x???? = - a 2-byte payload length
        // 0x?? = - an 8-byt cyclic redundancy check (CRC)
        // 0x??...??
        // = - Variable length payload
        // 0xAA - an "end-message" indicator
        //
        // For the purposes of this decoder, the payload is always a "read-attribute(s)
        // response" which means that the structure of the payload is always this:
        //
        // - 0x???? - a 16-bit source address
        // - 0x?? = - an 8-bit source endpoint
        // - 0x?? = - an 8-bit destination endpoint
        // - 0x?? = - an 8-bit sequence number
        // - 0x???? - a 16-bit cluster id.
        // - 0x?? = - an 8-bit number-of-attributes
        // - Repeated for each attribute
        // = - 0x???? = - a 16-bit attribute identifier
        // = - 0x?? = - an 8-bit attribute state (we only care for 0x00, success)
        // = - 0x?? = - an 8-bit data type
        // = - 0x??...??
        // = - A variable number of bits indicating the length
        // = - 0x??...??
        // = - A variable number of data bits
        //
        // Note that it is important that we spot and avoid data overruns in the case
        // of invalid date and also log any errors so that they cna be identified and
        // corrected.

        public const int MIN_FRAME_LENGTH =7;
        public const byte FRAME_START = 0x55;
        public const byte FRAME_END = 0xAA;
        public const int OFFSET_PAYLOAD_LENGTH = 3;
        public const int OFFSET_COMMMAND_ID = 5;
        public const int OFFSET_CRC = 6;
        public const int OFFSET_PAYLOAD = 7;
    }

    public static class ZbhciCommandId
    {
        public const ushortint ATTR_READ_RSP = 0x8100;
    }


    public static class ZbhciPayloadConstant
    {
        // With no attributes, payload must be at least 7 bytes.
        public const int MIN_LENGTH = 7;
        public const int OFFSET_SRC_ADDR = 0;
        public const int OFFSET_SRC_ENDPT = 1;
        public const int OFFSET_DEST_ENDPT = 2;
        public const int OFFSET_SEQ_NUM = 3;
        public const int OFFSET_CLUSTER_ID = 4;
        public const int OFFSET_NUM_ATTRS = 6;
    }

    public static class ZbhciAttributeConstant
    {
        public const int OFFSET_IDENTIFIER = 0;
        public const int OFFSET_STATUS = 2;
        public const int OFFSET_DATA_TYPE = 3;

        // The value begins at this offset, unless we have to allow for length
        // bytes as per below.
        public const int OFFSET_VALUE= 4;

        // Bytes used to indicate the length of strings.  Note that other value
        // types have implicit lengths.  Structs are also of variable length.
        public const int LONG_STR_LENGTH_LENGTH = 2;
        public const int SHORT_STR_LENGTH_LENGTH = 1;
    }

    static class ZbhciDataType
    {
        public const byte NO_DATA = 0x00;
        public const byte DATA8 = 0x08;
        public const byte DATA16 = 0x09;
        public const byte DATA24 = 0x0a;
        public const byte DATA32 = 0x0b;
        public const byte DATA40 = 0x0c;
        public const byte DATA48 = 0x0d;
        public const byte DATA56 = 0x0e;
        public const byte DATA64 = 0x0f;
        public const byte BOOLEAN = 0x10;
        public const byte BITMAP8 = 0x18;
        public const byte BITMAP16 = 0x19;
        public const byte BITMAP24 = 0x1a;
        public const byte BITMAP32 = 0x1b;
        public const byte BITMAP40 = 0x1c;
        public const byte BITMAP48 = 0x1d;
        public const byte BITMAP56 = 0x1e;
        public const byte BITMAP64 = 0x1f;
        public const byte UINT8 = 0x20;
        public const byte UINT16 = 0x21;
        public const byte UINT24 = 0x22;
        public const byte UINT32 = 0x23;
        public const byte UINT40 = 0x24;
        public const byte UINT48 = 0x25;
        public const byte UINT56 = 0x26;
        public const byte UINT64 = 0x27;
        public const byte INT8 = 0x28;
        public const byte INT16 = 0x29;
        public const byte INT24 = 0x2a;
        public const byte INT32 = 0x2b;
        public const byte INT40 = 0x2c;
        public const byte INT48 = 0x2d;
        public const byte INT56 = 0x2e;
        public const byte INT64 = 0x2f;
        public const byte ENUM8 = 0x30;
        public const byte ENUM16 = 0x31;
        public const byte SEMI_PREC = 0x38;
        public const byte SINGLE_PREC = 0x39;
        public const byte DOUBLE_PREC = 0x3a;
        public const byte OCTET_STR = 0x41;
        public const byte CHAR_STR = 0x42;
        public const byte LONG_OCTET_STR = 0x43;
        public const byte LONG_CHAR_STR = 0x44;
        public const byte ARRAY = 0x48;
        public const byte STRUCT = 0x4c;
        public const byte SET = 0x50;
        public const byte BAG = 0x51;
        public const byte TOD = 0xe0;
        public const byte DATE = 0xe1;
        public const byte UTC = 0xe2;
        public const byte CLUSTER_ID = 0xe8;
        public const byte ATTR_ID = 0xe9;
        public const byte BAC_OID = 0xea;
        public const byte IEEE_ADDR = 0xf0;
        public const byte BIT_128_SEC_KEY = 0xf1;
        public const byte UNKNOWN = 0xff;
    }
}
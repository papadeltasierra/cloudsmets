// Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
// License: Free to copy providing the author is acknowledged.namespace CloudSMETS.zbhci
using System;
using System.Collections.Generic;

namespace CloudSMETS.zbhci
{
    enum AmbientConsumptionIndicator
    {
        LowEnergyUsage = 0x00,
        MediumEnergyUsage = 0x01,
        HighEnergyUsage = 0x02
    }

    public static class ZbhciSmartEnergyEnums
    {
        public static string GetEnum8(ushort attributeId, byte value)
        {
            switch (attributeId)
            {
                case 0x0207:
                    return Enum.GetName(typeof(AmbientConsumptionIndicator), value);

                default:
                    throw new System.Exception($"Unknown attribute enumeration: {attributeId:X4}.");
            }
        }
    };


    public class ZbhciAttributeNames
    {
        public Dictionary<ushort, string> Attributes;

        public ZbhciAttributeNames() { }
    }

    public class ZbhciSmartEnergyAttributes : ZbhciAttributeNames
    {
        public Dictionary<ushort, string> Attributes;

        public ZbhciSmartEnergyAttributes()
        {
            this.Attributes = new ()
            {
                { 0x0000, "CurrentSummationDelivered" },               // Uint48
                { 0x0207, "AmbientConsumptionIndicator" },             // Enum8
                { 0x0206, "CurrentMeterID" },                          // Octet string
            };
        }
    }

    public enum ZbhhiClusterId
    {
        SMART_ENERY_STANDARD = 0x0109,
    }

    public static class ZbhciClusterAttributes
    {
        public static Dictionary<ushort, ZbhciAttributeNames> ClusterAttributes = new()
        {
            { ZbhciClusterId.SMART_ENERY_STANDARD, new ZbhciSmartEnergyAttributes() },
        };
    }

    public static class ZbhciEnumMaps
    {
        public static Dictionary<ushort, Dictionary<ushort, string>> EnumMaps = new()
        {
            { ZbhcuClisterId.SMART_ENERY_STANDARD, ZbhciSmartEnergyEnums }
        };
    }

    public static class ZbhciCommandId
    {
        public const ushort ATTR_READ_RSP = 0x8100;
    }

    public static class ZbhciStatus
    {
        public const byte SUCCESS = 0x00;
    }

    public static class ZbhciDataType
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
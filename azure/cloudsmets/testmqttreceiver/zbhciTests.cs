namespace CloudSMETS.Tests
{
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using FluentAssertions;
    using CloudSMETS.zbhci;
    using System.ComponentModel.DataAnnotations;

    public class TestZbhciFrameHeader(ushort commandId, ushort payloadLength, byte crc8) : ZbhciFrameHeader(commandId, payloadLength, crc8)
    {
    }

    public class TestZbhciCommandHeader(
            ushort sourceAddress,
            byte sourceEndpoint,
            byte destinationEndpoint,
            byte sequenceNumber,
            ushort clusterId) : ZbhciCommandHeader(sourceAddress, sourceEndpoint, destinationEndpoint, sequenceNumber, clusterId)
    {
    }

    public class TestZbhciAttributeStr(ushort identifier, byte status, byte dataType, string value) : ZbhciAttributeStr(identifier, status, dataType, value)
    {
    }

    public class TestZbhciAttributeUint48(ushort identifier, byte status, byte dataType, ulong value) : ZbhciAttributeUint48(identifier, status, dataType, value)
    {
    }

    public class TestZbhciAttributeEnum8(ushort identifier, byte status, byte dataType, byte value) : ZbhciAttributeEnum8(identifier, status, dataType, value)
    {
    }


    [TestClass]
    public class ZbhciTest
    {

        [TestInitialize]
        public void TestInitialize()
        {
        }

        [TestMethod]
        public void Test_ZbhciNoFrameStart()
        {
            byte[] frame = [0x11, 0x22, 0xAA];

            var act = () => { ZbhciMessage message = new (frame); };
            act.Should().Throw<ZbhciFramingException>().
                WithMessage("Frame start/end invalid: 0x11, 0xAA.");
        }

        [TestMethod]
        public void Test_ZbhciNoFrameEnd()
        {
            byte[] frame = [0x55, 0x11, 0x22];

            var act = () => { ZbhciMessage message = new (frame); };
            act.Should().Throw<ZbhciFramingException>().
                WithMessage("Frame start/end invalid: 0x55, 0x22.");
        }

        [TestMethod]
        public void Test_ZbhciBadCrc8()
        {
            byte[] frame = [0x55, 0x01, 0x02, 0x00, 0x00, 0xCC, 0xAA];

            var act = () => { ZbhciMessage message = new (frame); };
            act.Should().Throw<ZbhciCRCExcException>().
                WithMessage("CRC is incorrect: expected: 0x03, received: 0xCC.");
        }

        [TestMethod]
        public void Test_ZbhciNotReadAttrRsp()
        {
        }

        [TestMethod]
        public void Test_ZbhciBadAttributeStatus()
        {
        }

        [TestMethod]
        public void Test_ZbhciInsufficientDataForFrame()
        {
        }

        [TestMethod]
        public void Test_ZbhciInsufficientDataForCommand()
        {
        }

        [TestMethod]
        public void Test_ZbhciInsufficientDataForAttributeHeader()
        {
        }

        [TestMethod]
        public void Test_ZbhciInsufficientDataForAttribute()
        {
        }

        [TestMethod]
        public void Test_ZbhciGoodMesssage()
        {
            byte attrRdRspHi = (byte)(ZbhciCommandId.ATTR_READ_RSP >> 8);
            byte attrRdRspLo = (byte)(ZbhciCommandId.ATTR_READ_RSP & 0xff);
            byte[] frame = [
                0x55,                       // frame start
                    attrRdRspHi, attrRdRspLo,
                                            // Command Id
                    0x00, 0x22,                 // Payload length
                    0xAD,                       // CRC
                        0x00, 0x01,                 // Source address
                        0x05,                       // Source endpoint
                        0x01,                       // Destination endpoint
                        0x98,                       // Sequence number
                        0x01, 0x09,                 // Cluster Id
                            0x03,                           // Number of attributes
                            0x02, 0x07,                     // Attribute Id
                            ZbhciStatus.SUCCESS,            // Status
                            ZbhciDataType.ENUM8,            // Data type (long string)
                            0x01,                           // Value

                            0x00, 0x00,                     // Attribute Id
                            ZbhciStatus.SUCCESS,            // Status
                            ZbhciDataType.UINT48,           // Data type
                            0x45, 0x67, 0x89,
                            0xAB, 0xCD, 0xEF,               // Value

                            0x02, 0x06,                     // Attribute Id
                            ZbhciStatus.SUCCESS,            // Status
                            ZbhciDataType.CHAR_STR,         // Data type (short string)
                            0x05,                           // Length
                            0x48, 0x65, 0x6C, 0x6C, 0x6F,   // Value "Hello"
                0xAA];                                  // frame end

            ZbhciMessage message = new (frame);

            TestZbhciFrameHeader testFrameHeader = new(0x8100, 0x0022, 0xAD);
            TestZbhciCommandHeader testCommandHeader = new(0x0001, 0x05, 0x01, 0x98, (ushort)ZbhciClusterId.SMART_ENERGY_STANDARD);

            message.frameHeader.Should().BeEquivalentTo(testFrameHeader);
            message.commandHeader.Should().BeEquivalentTo(testCommandHeader);

            // Note that the attribute gets sorted based on attribute identifier.


            // TODO: Add more checking here.  not testing values!
            List<ZbhciAttribute> attributeList = [
                new TestZbhciAttributeUint48(0x0000, ZbhciStatus.SUCCESS, ZbhciDataType.UINT48, 0x00456789ABCDEF),
                new TestZbhciAttributeStr(0x0206, ZbhciStatus.SUCCESS, ZbhciDataType.CHAR_STR, "Hello"),
                new TestZbhciAttributeEnum8(0x0207, ZbhciStatus.SUCCESS, ZbhciDataType.ENUM8, (byte)ZbhciAmbientConsumptionIndicator.LowEnergyUsage)
            ];
            attributeList.Should().BeEquivalentTo(message.attributeList, options => options.RespectingRuntimeTypes().WithAutoConversion().IncludingInternalFields());
            message.attributeList[0].AttributeId().Should().Be("CurrentSummationDelivered");
            message.attributeList[1].AttributeId().Should().Be("CurrentMeterID");
            message.attributeList[2].AttributeId().Should().Be("AmbientConsumptionIndicator");
            message.attributeList[0].AttributeValue().Should().Be("76310993685999");
            message.attributeList[1].AttributeValue().Should().Be("Hello");
            message.attributeList[2].AttributeValue().Should().Be("MediumEnergyUsage");
        }
    }
}

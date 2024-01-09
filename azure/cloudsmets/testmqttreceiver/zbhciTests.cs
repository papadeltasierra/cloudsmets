namespace CloudSMETS.Tests
{
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using FluentAssertions;
    using Moq;
    using CloudSMETS.zbhci;

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
            byte[] frame = [0x55, 0x11, 0x11, 0x00, 0x00, 0xCC, 0xAA];

            var act = () => { ZbhciMessage message = new (frame); };
            act.Should().Throw<ZbhciCRCExcException>().
                WithMessage("CRC is incorrect: expected: 0xCC, received: 0xCC.");
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
        }
    }
}

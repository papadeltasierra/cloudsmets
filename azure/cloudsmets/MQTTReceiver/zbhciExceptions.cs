// Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
// License: Free to copy providing the author is acknowledged.namespace CloudSMETS.zbhci

namespace CloudSMETS.zbhci
{
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
    class ZbhciUnsupportedValueType : Exception {};
}
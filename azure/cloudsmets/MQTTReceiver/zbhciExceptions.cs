// Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
// License: Free to copy providing the author is acknowledged.namespace CloudSMETS.zbhci
using System;

namespace CloudSMETS.zbhci
{
    // Exceptions that may be throw whilst parsing the data.
    class ZbhciShortFrameException : Exception
    {
        public ZbhciShortFrameException()
        {
        }

        public ZbhciShortFrameException(string message)
            : base(message)
        {
        }

        public ZbhciShortFrameException(string message, Exception inner)
            : base(message, inner)
        {
        }
    };

    class ZbhciFramingException : Exception
    {
        public ZbhciFramingException()
        {
        }

        public ZbhciFramingException(string message)
            : base(message)
        {
        }

        public ZbhciFramingException(string message, Exception inner)
            : base(message, inner)
        {
        }
    };

    class ZbhciCRCExcException : Exception
    {
        public ZbhciCRCExcException()
        {
        }

        public ZbhciCRCExcException(string message)
            : base(message)
        {
        }

        public ZbhciCRCExcException(string message, Exception inner)
            : base(message, inner)
        {
        }
    };

    class ZbhciFrameException : Exception {};
    {
        public ZbhciFrameException()
        {
        }

        public ZbhciFrameException(string message)
            : base(message)
        {
        }

        public ZbhciFrameException(string message, Exception inner)
            : base(message, inner)
        {
        }
    };

    class ZbhciAttributeStateInvalid : Exception
    {
        public ZbhciAttributeStateInvalid()
        {
        }

        public ZbhciAttributeStateInvalid(string message)
            : base(message)
        {
        }

        public ZbhciAttributeStateInvalid(string message, Exception inner)
            : base(message, inner)
        {
        }
    };

    class ZbhciNotAttributesException : Exception
    {
        public ZbhciNotAttributesException()
        {
        }

        public ZbhciNotAttributesException(string message)
            : base(message)
        {
        }

        public ZbhciNotAttributesException(string message, Exception inner)
            : base(message, inner)
        {
        }
    };

    class ZbhciShortPayloadException : Exception
    {
        public ZbhciShortPayloadException()
        {
        }

        public ZbhciShortPayloadException(string message)
            : base(message)
        {
        }

        public ZbhciShortPayloadException(string message, Exception inner)
            : base(message, inner)
        {
        }
    };

    class ZbhciPayloadFrameLengthsException : Exception
    {
        public ZbhciPayloadFrameLengthsException()
        {
        }

        public ZbhciPayloadFrameLengthsException(string message)
            : base(message)
        {
        }

        public ZbhciPayloadFrameLengthsException(string message, Exception inner)
            : base(message, inner)
        {
        }
    };

    class ZbhciShortattributeHeader : Exception
    {
        public ZbhciShortattributeHeader()
        {
        }

        public ZbhciShortattributeHeader(string message)
            : base(message)
        {
        }

        public ZbhciShortattributeHeader(string message, Exception inner)
            : base(message, inner)
        {
        }
    };

    class ZbhciUnsupportedValueType : Exception
    {
        public ZbhciUnsupportedValueType()
        {
        }

        public ZbhciUnsupportedValueType(string message)
            : base(message)
        {
        }

        public ZbhciUnsupportedValueType(string message, Exception inner)
            : base(message, inner)
        {
        }
    };


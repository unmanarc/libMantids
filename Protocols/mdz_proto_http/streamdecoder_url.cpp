#include "streamdecoder_url.h"
#include <mdz_hlp_functions/encoders.h>

#include <mdz_mem_vars/b_chunks.h>
#include <mdz_mem_vars/b_mem.h>

using namespace Mantids::Memory::Streams;
using namespace Mantids::Memory::Streams::Decoders;

URL::URL(Memory::Streams::StreamableObject *orig)
{
    this->orig = orig;
    filled = 0;
}

bool URL::streamTo(Memory::Streams::StreamableObject *, StreamableObject::Status &)
{
    return false;
}
StreamableObject::Status URL::write(const void *buf, const size_t &count, Status &wrStat)
{
    StreamableObject::Status cur;

    const unsigned char *data = static_cast<const unsigned char *>(buf);

    // Handle empty buffer
    if (count == 0)
    {
        if (filled!=0)
        {
            cur.succeed = false;
        }
        return cur;
    }

    switch (filled)
    {
    case 0: // Normal mode - looking for special characters
    {
        unsigned char byteDetected = 0;
        size_t plainBytes;

        // Find consecutive plain bytes (non-special characters)
        plainBytes = getPlainBytesSize(data, count, &byteDetected);

        if (plainBytes > 0)
        {
            // Transmit plain bytes
            cur.bytesWritten = plainBytes;
            if (!(orig->writeFullStream(data, plainBytes, wrStat)).succeed)
            {
                cur.succeed = false;
            }
        }
        else
        {
            // Special character detected
            if (byteDetected == '%')
            {
                // Start of URL encoding sequence
                bytes[0] = '%';
                filled = 1;
                cur.bytesWritten = 1;
            }
            else if (byteDetected == '+')
            {
                // Plus sign represents space
                bytes[0] = ' ';
                cur.bytesWritten = 1;
                if (!(orig->writeFullStream(bytes, 1, wrStat)).succeed)
                {
                    cur.succeed = false;
                }
            }
        }
    }
    break;
    case 1: // Waiting for first hex digit after %
    {
        bytes[1] = data[0];
        cur.bytesWritten = 1;

        filled = 2;
        if (!isxdigit(bytes[1]))
        {
            // Malformed - flush the % and the invalid byte as-is
            if (!(flushBytes(wrStat)).succeed)
            {
                cur.succeed = false;
            }
        }
    }
    break;
    case 2: // Waiting for second hex digit
    {
        bytes[2] = data[0];
        cur.bytesWritten = 1;

        if (!isxdigit(bytes[2]))
        {
            // Malformed - flush all 3 bytes as-is
            filled = 3;
            if (!(flushBytes(wrStat)).succeed)
            {
                cur.succeed = false;
            }
        }
        else
        {
            // Valid hex pair - decode and write
            unsigned char val;
            val = Helpers::Encoders::hexPairToByte((char *) bytes + 1);
            filled = 0;
            if (!(orig->writeFullStream(&val, 1, wrStat)).succeed)
            {
                cur.succeed = false;
            }
        }
    }

    default:
        break;
    }

    return cur;
}

size_t URL::getPlainBytesSize(const unsigned char *buf, size_t count, unsigned char *byteDetected)
{
    for (size_t i = 0; i < count; i++)
    {
        if (buf[i] == '%')
        {
            *byteDetected = '%';
            return i;
        }
        else if (buf[i] == '+')
        {
            *byteDetected = '+';
            return i;
        }
    }
    return count;
}

StreamableObject::Status URL::flushBytes(Status &wrStat)
{
    auto x = orig->writeFullStream(bytes, filled, wrStat);
    filled = 0;
    return x;
}


void URL::writeEOF(bool)
{
    // flush intermediary bytes...
    Status w;
    flushBytes(w);
}

std::string URL::decodeURLStr(const std::string &url)
{
    Mantids::Memory::Containers::B_MEM uriEncoded(url.c_str(), url.size());
    Memory::Containers::B_Chunks uriDecoded;

    // Decode URI (maybe it's url encoded)...
    Memory::Streams::Decoders::URL uriDecoder(&uriDecoded);
    Memory::Streams::StreamableObject::Status cur;
    Memory::Streams::StreamableObject::Status wrsStat;

    if ((cur += uriEncoded.streamTo(&uriDecoder, wrsStat)).succeed)
    {
        return uriDecoded.toString();
    }
    return url;
}

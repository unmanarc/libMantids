#ifndef HLP_ENCODERS_H
#define HLP_ENCODERS_H

#include <string>

namespace Mantids { namespace Helpers {

enum eURLEncodingType
{
    ENC_STRICT,
    ENC_QUOTEPRINT
};

class Encoders
{
public:
    Encoders();

    // Obfuscated encoding
    static std::string fromBase64Obf(std::string const& sB64Buf, const uint64_t & seed = 0xAA12017BEA385A7B);
    static std::string toBase64Obf(unsigned char const* buf, uint32_t count, const uint64_t & seed = 0xAA12017BEA385A7B);
    static std::string toBase64Obf(const std::string & buf, const uint64_t & seed = 0xAA12017BEA385A7B);

    // B64 Encoding
    static std::string fromBase64(std::string const& sB64Buf);
    static std::string toBase64(const std::string & buf);
    static std::string toBase64(unsigned char const* buf, uint32_t count);

    // URL Percent Encoding
    static std::string toURL(const std::string &str, const eURLEncodingType & urlEncodingType = ENC_STRICT);
    static std::string fromURL(const std::string &urlEncodedStr);

    // Hex Encoding
    static std::string toHex(const unsigned char *data, size_t len);
    static void fromHex(const std::string &hexValue, unsigned char *data, size_t maxlen);

    // Hex Helpers
    static char toHexPair(char value, char part);
    static bool isHexChar(char v);
    static char hexToValue(char v);
private:
    static bool getIfMustBeURLEncoded(char c, const eURLEncodingType &urlEncodingType);
    static size_t calcURLEncodingExpandedStringSize(const std::string &str,const eURLEncodingType & urlEncodingType);
    static const std::string b64Chars;

};

}}

#endif // HLP_ENCODERS_H

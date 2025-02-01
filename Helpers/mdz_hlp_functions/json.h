#ifndef JSON_H
#define JSON_H

#include <json/json.h>
#include <list>

typedef Json::Value json;

// jsoncpp macros:
#define JSON_ASCSTRING(j, x, def) \
((j).isObject() && (j).isMember(x) && (j)[x].isString() ? (j)[x].asCString() : (def))

#define JSON_ASSTRING(j, x, def) \
    ((j).isObject() && (j).isMember(x) && (j)[x].isString() ? (j)[x].asString() : (def))

#define JSON_ASBOOL(j, x, def) \
    ((j).isObject() && (j).isMember(x) && (j)[x].isBool() ? (j)[x].asBool() : (def))

#define JSON_ASDOUBLE(j, x, def) \
    ((j).isObject() && (j).isMember(x) && (j)[x].isDouble() ? (j)[x].asDouble() : (def))

#define JSON_ASFLOAT(j, x, def) \
    ((j).isObject() && (j).isMember(x) && (j)[x].isFloat() ? (j)[x].asFloat() : (def))

#define JSON_ASINT(j, x, def) \
    ((j).isObject() && (j).isMember(x) && (j)[x].isInt() ? (j)[x].asInt() : (def))

#define JSON_ASINT64(j, x, def) \
    ((j).isObject() && (j).isMember(x) && (j)[x].isInt64() ? (j)[x].asInt64() : (def))

#define JSON_ASUINT(j, x, def) \
    ((j).isObject() && (j).isMember(x) && (j)[x].isUInt() ? (j)[x].asUInt() : (def))

#define JSON_ASUINT64(j, x, def) \
    ((j).isObject() && (j).isMember(x) && (j)[x].isUInt64() ? (j)[x].asUInt64() : (def))

#define JSON_ISARRAY(j, key) \
    ((j).isObject() && (j).isMember(key) && (j)[(key)].isArray())

#define JSON_ASCSTRING_D(j,def) \
    (j.isString()?j.asCString():def)

#define JSON_ASSTRING_D(j,def) \
    (j.isString()?j.asString():def)

#define JSON_ASBOOL_D(j,def) \
    (j.isBool()?j.asBool():def)

#define JSON_ASDOUBLE_D(j,def)  \
    (j.isDouble()?j.asDouble():def)

#define JSON_ASFLOAT_D(j,def) \
    (j.isFloat()?j.asFloat():def)

#define JSON_ASINT_D(j,def) \
    (j.isInt()?j.asInt():def)

#define JSON_ASINT64_D(j,def) \
    (j.isInt64()?j.asInt64():def)

#define JSON_ASUINT_D(j,def) \
    (j.isUInt()?j.asUInt():def)

#define JSON_ASUINT64_D(j,def) \
    (j.isUInt64()?j.asUInt64():def)

#define JSON_ISARRAY_D(j)  \
    (j.isArray())

#define JSON_ARRAY_ASSTRING(j, i, def) \
    ((j).isArray() && ((i) < (j).size()) && (j)[(i)].isString() ? (j)[(i)].asString() : (def))

#define JSON_ARRAY_ASBOOL(j, i, def) \
    ((j).isArray() && ((i) < (j).size()) && (j)[(i)].isBool() ? (j)[(i)].asBool() : (def))

#define JSON_ARRAY_ASDOUBLE(j, i, def) \
    ((j).isArray() && ((i) < (j).size()) && (j)[(i)].isDouble() ? (j)[(i)].asDouble() : (def))

#define JSON_ARRAY_ASFLOAT(j, i, def) \
    ((j).isArray() && ((i) < (j).size()) && (j)[(i)].isFloat() ? (j)[(i)].asFloat() : (def))

#define JSON_ARRAY_ASINT(j, i, def) \
    ((j).isArray() && ((i) < (j).size()) && (j)[(i)].isInt() ? (j)[(i)].asInt() : (def))

#define JSON_ARRAY_ASINT64(j, i, def) \
    ((j).isArray() && ((i) < (j).size()) && (j)[(i)].isInt64() ? (j)[(i)].asInt64() : (def))

#define JSON_ARRAY_ASUINT(j, i, def) \
    ((j).isArray() && ((i) < (j).size()) && (j)[(i)].isUInt() ? (j)[(i)].asUInt() : (def))

#define JSON_ARRAY_ASUINT64(j, i, def) \
    ((j).isArray() && ((i) < (j).size()) && (j)[(i)].isUInt64() ? (j)[(i)].asUInt64() : (def))

namespace Mantids { namespace Helpers {

std::string jsonToString(const json &value);
std::list<std::string> jsonToStringList(const json &value, const std::string & sub="");

// deprecated JSONReader replacement:
class JSONReader2 {
public:
    JSONReader2();
    ~JSONReader2();

    bool parse(const std::string& document, Json::Value& root);
    std::string getFormattedErrorMessages();

private:
    Json::CharReader * reader;
    std::string errors;
};

}};

#endif // JSON_H

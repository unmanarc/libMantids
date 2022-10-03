#ifndef JSON_H
#define JSON_H

#include <list>
#include <boost/json/src.hpp>

typedef boost::json::value json;

#define JSON_ASCSTRING(j,x,def) (j[x].is_string()?j[x].as_string().c_str():def)
#define JSON_ASSTRING(j,x,def) (j[x].is_string()?j[x].as_string():def)
#define JSON_ASBOOL(j,x,def) (j[x].is_bool()?j[x].as_bool():def)
#define JSON_ASDOUBLE(j,x,def) (j[x].is_double()?j[x].as_double():def)
/*#define JSON_ASFLOAT(j,x,def) (j[x].is_double()?j[x].asFloat():def)*/
/*#define JSON_ASINT(j,x,def) (j[x].is_int()?j[x].as_int():def)*/
#define JSON_ASINT64(j,x,def) (j[x].is_int64()?j[x].as_int64():def)
/*#define JSON_ASUINT(j,x,def) (j[x].is_uint()?j[x].as_int():def)*/
#define JSON_ASUINT64(j,x,def) (j[x].is_uint64()?j[x].as_uint64():def)
#define JSON_ISARRAY(j,x) (j.as_object().if_contains(x) && j[x].is_array())

/*
// jsoncpp macros:

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

}};*/

#endif // JSON_H

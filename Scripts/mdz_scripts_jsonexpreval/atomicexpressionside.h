#ifndef ATOMICEXPRESSIONSIDE_H
#define ATOMICEXPRESSIONSIDE_H

#include <mdz_hlp_functions/json.h>
#include <set>
#include <string>
#include <vector>

#ifdef USE_STD_REGEX
#include <regex>
#else
#include <boost/regex.hpp>
#endif

namespace Mantids { namespace Scripts { namespace Expressions {

class AtomicExpressionSide
{
public:

    enum eExpressionSideMode
    {
        EXPR_MODE_NUMERIC,
        EXPR_MODE_STATIC_STRING,
        EXPR_MODE_JSONPATH,
        EXPR_MODE_NULL,
        EXPR_MODE_UNDEFINED
    };


    AtomicExpressionSide(std::vector<std::string> * staticTexts);
    ~AtomicExpressionSide();

    bool calcMode();
    std::string getExpr() const;
    void setExpr(const std::string &value);

    std::set<std::string> resolve(const json & v, bool resolveRegex, bool ignoreCase);

#ifdef USE_STD_REGEX
    std::regex *getRegexp() const;
    void setRegexp(std::regex *value);
#else
    boost::regex *getRegexp() const;
    void setRegexp(boost::regex *value);
#endif


    eExpressionSideMode getMode() const;

private:
    std::set<std::string> recompileRegex(const std::string & r, bool ignoreCase);

#ifdef USE_STD_REGEX
    std::regex * regexp;
#else
    boost::regex * regexp;
#endif
    std::vector<std::string> * staticTexts;
    uint32_t staticIndex;
    std::string expr;
    eExpressionSideMode mode;
};
}}}
#endif // ATOMICEXPRESSIONSIDE_H

#ifndef RESOURCESFILTER_H
#define RESOURCESFILTER_H


#ifdef USE_STD_REGEX
#include <regex>
#else
#include <boost/regex.hpp>
#endif


#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include <string>
#include <list>
#include <mdz_auth/session.h>

namespace Mantids { namespace RPC { namespace Web {

class ResourcesFilter
{
public:
    ResourcesFilter();


    struct sFilterEvaluation
    {
        sFilterEvaluation()
        {
            accept = true;
        }
        bool accept;
        std::string redirectLocation;
    };

    enum eFilterActions
    {
        RFILTER_ACCEPT=0,
        RFILTER_DENY=1,
        RFILTER_REDIRECT=2
    };

    struct sFilter
    {
        sFilter( const std::list<std::string> & _regexs,
                 const std::string & _redirectLocation,
                 const std::list<std::string> & _reqAttrib,
                 const std::list<std::string> & _rejAttrib,
                 const eFilterActions & _action
                 )
        {
            for ( const auto &i : _regexs )
            {
                this->sRegexs.push_back(i);
#ifdef USE_STD_REGEX
                this->regexs.push_back( std::regex(i.c_str(),std::regex_constants::extended ));
#else
                this->regexs.push_back( boost::regex(i.c_str(),boost::regex::extended ));
#endif
            }
            this->redirectLocation = _redirectLocation;
            this->action = _action;
            this->reqAttrib = _reqAttrib;
            this->rejAttrib = _rejAttrib;
        }

        std::string listToString(const std::list<std::string> & list)const
        {
            std::string x;
            for (auto & i : list)
                x+="\"" + i + "\",";
            return x;
        }

        std::string actionToString(eFilterActions _action)const
        {
            switch (_action)
            {
            case     RFILTER_ACCEPT:
                return "ACCEPT";
            case     RFILTER_DENY:
                return "DENY";
            case     RFILTER_REDIRECT:
                return "REDIRECT";

            }
        }

        std::string toString() const
        {
            return    "Filter:\n"
                        "-------\n"
                        "Redirect Location: " + redirectLocation + "\n"
                        "reqAttrib: " + listToString(reqAttrib) + "\n"
                        "rejAttrib: " + listToString(rejAttrib) + "\n"
                        "Regexs: " + listToString(sRegexs) + "\n"
                        "Action: " + actionToString(action);
        }

#ifdef USE_STD_REGEX
        std::list<std::regex> regexs;
#else
        std::list<boost::regex> regexs;
#endif
        std::string redirectLocation;
        std::list<std::string> reqAttrib, rejAttrib, sRegexs;
        eFilterActions action;
    };

    bool loadFile(const std::string & filePath);
    void addFilter(const sFilter & filter);

    sFilterEvaluation evaluateAction(const std::string & uri, Mantids::Authentication::Session *hSession, Authentication::Manager *authorizer);
private:

    std::list<sFilter> filters;
};

}}}

#endif // RESOURCESFILTER_H

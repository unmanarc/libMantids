#include "resourcesfilter.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#ifdef USE_STD_REGEX
#include <regex>
#else
#include <boost/regex.hpp>
#endif


using namespace boost;
using namespace Mantids::RPC::Web;

ResourcesFilter::ResourcesFilter()
{
}

bool ResourcesFilter::loadFile(const std::string &filePath)
{
    // Create a root ptree
    property_tree::ptree root;

    // TODO:
    //    try {
    // Load the info file in this ptree
    property_tree::read_json(filePath, root);
    /*  }
    catch (property_tree::info_parser_error x)
    {
        return false;
    }*/

    /* try
    {*/
    for (const auto & i : root)
    {
        std::list<std::string> regexs;
        auto pRegexs = i.second.get_child_optional("uriRegex");
        if (pRegexs)
        {
            for (const auto & i : pRegexs.get())
            {
                regexs.push_back(i.second.get_value<std::string>());
            }
        }

        std::list<std::string> requiredAppAtrribs;
        auto pRequiredAppAttribs = i.second.get_child_optional("requiredAppAtrribs");
        if (pRequiredAppAttribs)
        {
            for (const auto & i : pRequiredAppAttribs.get())
            {
                requiredAppAtrribs.push_back(i.second.get_value<std::string>());
            }
        }

        std::list<std::string> rejectedAppAtrribs;
        auto pRejectedAppAttribs =  i.second.get_child_optional("rejectedAppAtrribs");
        if (pRejectedAppAttribs)
        {
            for (const auto & i : pRejectedAppAttribs.get())
            {
                rejectedAppAtrribs.push_back(i.second.get_value<std::string>());
            }
        }

        eFilterActions fAction;

        // Action is mandatory:
        std::string sAction = boost::to_upper_copy(i.second.get<std::string>("action"));
        if ( sAction == "REDIRECT" ) fAction = RFILTER_REDIRECT;
        if ( sAction == "DENY" ) fAction = RFILTER_DENY;
        if ( sAction == "ACCEPT" ) fAction = RFILTER_ACCEPT;


        std::string sRedirectLocation = i.second.get_optional<std::string>("redirectLocation")?i.second.get<std::string>("redirectLocation"):"";

        addFilter(sFilter( regexs,
                           sRedirectLocation,
                           requiredAppAtrribs,
                           rejectedAppAtrribs,
                           fAction
                           ));
    }
    /* }
    catch(property_tree::ptree_bad_path)
    {
        return false;
    }
    Explode and explain...
*/

    return true;
}

void Mantids::RPC::Web::ResourcesFilter::addFilter(const sFilter &filter)
{
    filters.push_back(filter);
}

Mantids::RPC::Web::ResourcesFilter::sFilterEvaluation ResourcesFilter::evaluateAction(const std::string &uri, Mantids::Authentication::Session * hSession, Mantids::Authentication::Manager * authorizer)
{
    sFilterEvaluation evalRet;

    int ruleId=0;
    for (const auto & filter : filters)
    {
        ruleId++;
        bool allAttribsDone=true;

        /*printf("Evaluating rule %d for %s\n%s",  ruleId,uri.c_str(), filter.toString().c_str());
        fflush(stdout);*/

        for (const auto & attrib : filter.reqAttrib)
        {
            if (!hSession || !authorizer) // Any attribute without the session is marked as false
            {
                allAttribsDone = false;
                break;
            }
            else if (attrib == "loggedin")
            {
                if (hSession->getAuthUser() == "")
                {
                    allAttribsDone = false;
                    break;
                }
            }
            else if (!authorizer->accountValidateAttribute(hSession->getAuthUser(),{hSession->getAppName(), attrib}))
            {
                allAttribsDone = false;
                break;
            }
        }
        for (const auto & attrib : filter.rejAttrib)
        {
            if (hSession && authorizer)
            {
                if (attrib == "loggedin")
                {
                    if (hSession->getAuthUser() != "")
                    {
                        allAttribsDone = false;
                        break;
                    }
                }
                else if (authorizer->accountValidateAttribute(hSession->getAuthUser(),{hSession->getAppName(), attrib}))
                {
                    allAttribsDone = false;
                    break;
                }
            }
        }

        if (!allAttribsDone)
        {
          /*  printf("\nRule %d does not match attributes.\n", ruleId);
            fflush(stdout);
*/
            continue; // Rule does not match
        }

       /* printf("\nRule %d match attributes, checking regex.\n", ruleId);
        fflush(stdout);
*/
#ifdef USE_STD_REGEX
        std::smatch what;  // std::smatch para std::regex
#else
        boost::cmatch what;  // boost::cmatch para boost::regex
#endif
        for (const auto & i : filter.regexs)
        {
#ifdef USE_STD_REGEX
            if (std::regex_match(uri, what, i))// std::regex_match
#else
            if (boost::regex_match(uri.c_str(), what, i)) // boost::regex_match
#endif
            {
                // printf("Rule %d match regex on URI %s, returning action.\n", ruleId, uri.c_str());
                // fflush(stdout);

                switch (  filter.action  )
                {
                case RFILTER_ACCEPT:
                    evalRet.accept = true;
                    break;
                case RFILTER_REDIRECT:
                    evalRet.accept = true;
                    evalRet.redirectLocation = filter.redirectLocation;
                    break;
                case RFILTER_DENY:
                default:
                    evalRet.accept = false;
                    break;
                }
                return evalRet;
            }
            else
            {
                // printf("Rule %d does not match regex on URI %s.\n", ruleId, uri.c_str());
                // fflush(stdout);
            }
        }
    }

    evalRet.accept = true;
    return evalRet;
}

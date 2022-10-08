#include "domains.h"

using namespace Mantids3::Authentication;
using namespace Mantids3;

Domains::Domains()
{

}

bool Domains::addDomain(const std::string &domainName, Manager *auth)
{
    return domainMap.addElement(domainName,auth);
}

Manager *Domains::openDomain(const std::string &domainName)
{
    Manager * i = (Manager *)domainMap.openElement(domainName);;
    if (i) i->checkConnection();
    return i;
}

bool Domains::releaseDomain(const std::string &domainName)
{
    return domainMap.releaseElement(domainName);
}

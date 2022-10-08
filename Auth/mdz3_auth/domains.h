#ifndef IAUTH_DOMAINS_H
#define IAUTH_DOMAINS_H

#include <mdz3_threads/map.h>
#include "manager.h"

namespace Mantids3 { namespace Authentication {

class Domains
{
public:
    Domains();

    bool addDomain( const std::string & domainName, Manager * auth);
    Manager * openDomain(const std::string & domainName);
    bool releaseDomain(const std::string & domainName);

private:
    Threads::Safe::Map<std::string> domainMap;
};

}}

#endif // IAUTH_DOMAINS_H

#ifndef IAUTH_VALIDATION_ACCOUNT_H
#define IAUTH_VALIDATION_ACCOUNT_H

#include <string>
#include "ds_auth_mode.h"
#include "ds_auth_reason.h"
#include "ds_auth_function.h"
#include "ds_auth_secret.h"

#include <cx2_thr_safecontainers/map_element.h>

namespace CX2 { namespace Authentication {

class AccountSecret_Validation : public CX2::Threads::Safe::Map_Element
{
public:
    AccountSecret_Validation();
    virtual ~AccountSecret_Validation();

    static Secret genSecret(const std::string & passwordInput, const Function & passFunction, bool forceExpiration = false, const time_t &expirationDate = std::numeric_limits<time_t>::max(), uint32_t _2faSteps = 0);

    virtual std::string accountConfirmationToken(const std::string & accountName)=0;
    virtual Secret_PublicData accountSecretPublicData(const std::string & accountName, bool * found, uint32_t passIndex=0)=0;
    virtual Reason authenticate(const std::string & accountName, const std::string & password, uint32_t passIndex = 0, Mode authMode = MODE_PLAIN, const std::string & cramSalt = "")=0;

    virtual bool accountValidateAttribute(const std::string & accountName, const std::string & attribName)=0;

protected:
    Reason validateSecret(const Secret & storedSecret, const std::string & passwordInput, const std::string &cramSalt, Mode authMode);

private:


    Reason validateCRAM(const std::string & passwordFromDB, const std::string & passwordInput, const std::string &cramSalt);
    Reason validateGAuth(const std::string & seed, const std::string & token);
};

}}

#endif // IAUTH_VALIDATION_ACCOUNT_H
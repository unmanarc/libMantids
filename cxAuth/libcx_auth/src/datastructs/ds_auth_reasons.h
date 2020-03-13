#ifndef AUTH_REASONS_H
#define AUTH_REASONS_H

enum AuthReason
{
    AUTH_REASON_AUTHENTICATED=0,      // AUTHENTICATED!
    AUTH_REASON_INTERNAL_ERROR=500,   // INTERNAL ERROR (OTHER)
    AUTH_REASON_NOT_IMPLEMENTED=501,  // AUTHENTICATION NOT IMPLEMENTED YET :(
    AUTH_REASON_DUPLICATED_SESSION=502,  // DUPLICATED SESSION ID
    AUTH_REASON_EXPIRED_PASSWORD=100, // VALIDATE, HOWEVER MUST CHANGE PASSWORD NOW!
    AUTH_REASON_EXPIRED_ACCOUNT=102,  // ACCOUNT EXPIRED. NOT USABLE
    AUTH_REASON_DISABLED_ACCOUNT=103, // ACCOUNT DISABLED BY ADMIN.
    AUTH_REASON_UNCONFIRMED_ACCOUNT=104,      // ACCOUNT NOT CONFIRMED YET.
    AUTH_REASON_BAD_ACCOUNT=105,      // INVALID OR NON-EXISTENT ACCOUNT
    AUTH_REASON_BAD_PASSWORD=106,      // AUTHENTICATION FAILED.
    AUTH_REASON_INVALID_AUTHENTICATOR=995,
    AUTH_REASON_SESSIONLIMITS_EXCEEDED=996,
    AUTH_REASON_ANSWER_TIMEDOUT=997,
    AUTH_REASON_EXPIRED=998,
    AUTH_REASON_UNAUTHENTICATED=999
};

static const char * cAUTH_REASON_AUTHENTICATED="Authenticated.";
static const char * cAUTH_REASON_INTERNAL_ERROR="Authentication Internal Error.";
static const char * cAUTH_REASON_NOT_IMPLEMENTED="Authentication not implemented yet.";
static const char * cAUTH_REASON_EXPIRED_PASSWORD="Password expired.";
static const char * cAUTH_REASON_EXPIRED_ACCOUNT = "Account expired.";
static const char * cAUTH_REASON_DISABLED_ACCOUNT = "Account disabled.";
static const char * cAUTH_REASON_UNCONFIRMED_ACCOUNT = "Account unconfirmed.";
static const char * cAUTH_REASON_BAD_ACCOUNT = "Bad Account.";
static const char * cAUTH_REASON_BAD_PASSWORD = "Bad password.";
static const char * cAUTH_REASON_EXPIRED = "Expired authentication.";
static const char * cAUTH_REASON_UNAUTHENTICATED = "Not authenticated yet.";
static const char * cAUTH_REASON_ANSWER_TIMEDOUT = "Answer timed out.";
static const char * cAUTH_REASON_DUPLICATED_SESSION = "Session ID Duplicated Error";
static const char * cAUTH_REASON_INVALID_AUTHENTICATOR = "Invalid or undefined authenticator";
static const char * cAUTH_REASON_SESSIONLIMITS_EXCEEDED = "Sessions limits exceeded";
static const char * cNULL = "";

static const char * getAuthReasonText(const AuthReason & reason)
{
    switch(reason)
    {
    case AUTH_REASON_SESSIONLIMITS_EXCEEDED: return cAUTH_REASON_SESSIONLIMITS_EXCEEDED;
    case AUTH_REASON_INVALID_AUTHENTICATOR: return cAUTH_REASON_INVALID_AUTHENTICATOR;
    case AUTH_REASON_DUPLICATED_SESSION: return cAUTH_REASON_DUPLICATED_SESSION;
    case AUTH_REASON_AUTHENTICATED: return cAUTH_REASON_AUTHENTICATED;
    case AUTH_REASON_INTERNAL_ERROR: return cAUTH_REASON_INTERNAL_ERROR;
    case AUTH_REASON_NOT_IMPLEMENTED: return cAUTH_REASON_NOT_IMPLEMENTED;
    case AUTH_REASON_EXPIRED_PASSWORD: return cAUTH_REASON_EXPIRED_PASSWORD;
    case AUTH_REASON_EXPIRED_ACCOUNT: return cAUTH_REASON_EXPIRED_ACCOUNT;
    case AUTH_REASON_DISABLED_ACCOUNT: return cAUTH_REASON_DISABLED_ACCOUNT;
    case AUTH_REASON_UNCONFIRMED_ACCOUNT: return cAUTH_REASON_UNCONFIRMED_ACCOUNT;
    case AUTH_REASON_BAD_ACCOUNT: return cAUTH_REASON_BAD_ACCOUNT;
    case AUTH_REASON_BAD_PASSWORD: return cAUTH_REASON_BAD_PASSWORD;
    case AUTH_REASON_ANSWER_TIMEDOUT: return cAUTH_REASON_ANSWER_TIMEDOUT;
    case AUTH_REASON_EXPIRED: return cAUTH_REASON_EXPIRED;
    case AUTH_REASON_UNAUTHENTICATED: return cAUTH_REASON_UNAUTHENTICATED;
    }
    return cNULL;
}

#endif // AUTH_REASONS_H

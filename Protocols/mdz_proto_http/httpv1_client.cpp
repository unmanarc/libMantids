#include "httpv1_client.h"

#include <boost/algorithm/string.hpp>
#include <mdz_hlp_functions/encoders.h>
#include <string>

using namespace boost;
using namespace boost::algorithm;
using namespace Mantids::Protocols::HTTP;
using namespace Mantids;

HTTPv1_Client::HTTPv1_Client(Memory::Streams::StreamableObject *sobject)
    : HTTPv1_Base(true, sobject)
{
    currentParser = (Memory::Streams::SubParser *) (&serverResponse.status);
    clientRequest.requestLine.getHTTPVersion()->setVersionMajor(1);
    clientRequest.requestLine.getHTTPVersion()->setVersionMinor(0);

    clientRequest.requestLine.setRequestMethod("GET");
    clientRequest.userAgent = std::string("libMantids/") + std::to_string(HTTP_PRODUCT_VERSION_MAJOR) + std::string(".") + std::to_string(HTTP_PRODUCT_VERSION_MINOR);
}

bool HTTPv1_Client::initProtocol()
{
    Memory::Streams::StreamableObject::Status wrStat;

    if (!clientRequest.requestLine.stream(wrStat))
        return false;
    if (!streamClientHeaders(wrStat))
        return false;
    if (!clientRequest.content.stream(wrStat))
        return false;

    // Succesfully initialized...
    return true;
}

bool HTTPv1_Client::changeToNextParser()
{
    if (currentParser == &serverResponse.status)
        currentParser = &serverResponse.headers;
    else if (currentParser == &serverResponse.headers)
    {
        // Process incomming server headers here:
        /////////////////////////////////////////////////////////////////////////

        // Parse Cache-Control
        serverResponse.cacheControl.fromString(serverResponse.headers.getOptionRawStringByName("Cache-Control"));

        // Security:
        // Parse Xframeopts...
        serverResponse.security.XFrameOpts.fromValue(serverResponse.headers.getOptionRawStringByName("X-Frame-Options"));
        // Parse XSS Protection.
        serverResponse.security.XSSProtection.fromValue(serverResponse.headers.getOptionRawStringByName("X-XSS-Protection"));
        // Parse HSTS Configuration.
        serverResponse.security.HSTS.fromValue(serverResponse.headers.getOptionRawStringByName("Strict-Transport-Security"));
        // Content No Sniff
        serverResponse.security.bNoSniffContentType = iequals(serverResponse.headers.getOptionRawStringByName("X-Content-Type-Options"), "nosniff");

        // TODO: validate/check if using HSTS with SSL
        // TODO: get the preload list.
        std::string authHeader = serverResponse.headers.getOptionRawStringByName("WWW-Authenticate");
        if (!authHeader.empty())
        {
            // TODO: charset...
            // TODO: non-basic auth.

            // 1. Trim leading and trailing spaces
            boost::trim(authHeader);

            // 2. Check if it starts with "Basic" (case-insensitive)
            if (boost::istarts_with(authHeader, "Basic"))
            {
                // Remove "Basic" from the beginning
                authHeader = authHeader.substr(5); // "Basic" is 5 characters long

                // 3. Trim any leading spaces after "Basic"
                boost::trim_left(authHeader);

                // 4. Check if it starts with "Realm=" (case-insensitive)
                if (boost::istarts_with(authHeader, "Realm="))
                {
                    // Remove "Realm=" from the beginning
                    authHeader = authHeader.substr(6); // "Realm=" is 6 characters long

                    // 5. Trim any leading spaces after "Realm="
                    boost::trim_left(authHeader);

                    // 6. Check if the string starts and ends with quotes
                    if (authHeader.front() == '"' && authHeader.back() == '"')
                    {
                        // TODO: url enconded
                        // Extract the value inside the quotes
                        serverResponse.sWWWAuthenticateRealm = authHeader.substr(1, authHeader.length() - 2);
                    }
                    else
                    {
                        serverResponse.sWWWAuthenticateRealm = authHeader;
                    }
                }
            }
        }

        // Parse content-type...
        serverContentType = serverResponse.headers.getOptionRawStringByName("Content-Type");

        // Parse server cookies...
        parseHeaders2ServerCookies();
        // Parse the transmition mode requested and act according it.
        currentParser = parseHeaders2TransmitionMode();
    }
    else // END.
        currentParser = nullptr;
    return true;
}

void HTTPv1_Client::parseHeaders2ServerCookies()
{
    std::list<MIME::MIME_HeaderOption *> setCookies = serverResponse.headers.getOptionsByName("");
    for (MIME::MIME_HeaderOption *serverCookie : setCookies)
        serverResponse.cookies.parseCookie(serverCookie->getOrigValue());
}

Memory::Streams::SubParser *HTTPv1_Client::parseHeaders2TransmitionMode()
{
    serverResponse.content.setTransmitionMode(Common::Content::TRANSMIT_MODE_CONNECTION_CLOSE);
    // Set Content Data Reception Mode.
    if (serverResponse.headers.exist("Content-Length"))
    {
        uint64_t len = serverResponse.headers.getOptionAsUINT64("Content-Length");
        serverResponse.content.setTransmitionMode(Common::Content::TRANSMIT_MODE_CONTENT_LENGTH);

        // Error setting up that size or no data... (don't continue)
        if (!len || !serverResponse.content.setContentLenSize(len))
            return nullptr;
    }
    else if (icontains(serverResponse.headers.getOptionValueStringByName("Transfer-Encoding"), "CHUNKED"))
        serverResponse.content.setTransmitionMode(Common::Content::TRANSMIT_MODE_CHUNKS);

    return &serverResponse.content;
}

bool HTTPv1_Client::streamClientHeaders(Memory::Streams::StreamableObject::Status &wrStat)
{
    // Act as a server. Send data from here.
    uint64_t strsize;

    // Can't use chunked mode on client.
    if ((strsize = clientRequest.content.getStreamSize()) == std::numeric_limits<uint64_t>::max())
        return false;
    else
    {
        clientRequest.headers.remove("Connetion");
        clientRequest.headers.replace("Content-Length", std::to_string(strsize));
    }

    // Put client cookies:
    clientCookies.putOnHeaders(&clientRequest.headers);

    // Put basic authentication on headers:
    if (clientRequest.basicAuth.bEnabled)
    {
        clientRequest.headers.replace("Authentication", "Basic " + Helpers::Encoders::toBase64(clientRequest.basicAuth.user + ":" + clientRequest.basicAuth.pass));
    }

    clientRequest.headers.replace("User-Agent", clientRequest.userAgent);

    // Put Virtual Host And Port (if exist)
    if (!clientRequest.virtualHost.empty())
        clientRequest.headers.replace("Host", clientRequest.virtualHost + (clientRequest.virtualPort == 80 ? "" : ":" + std::to_string(clientRequest.virtualPort)));
    // Stream it..
    return clientRequest.headers.stream(wrStat);
}

std::string HTTPv1_Client::getServerContentType() const
{
    return serverContentType;
}

void HTTPv1_Client::setClientRequest(const std::string &hostName, const std::string &uriPath)
{
    if (!hostName.empty())
        clientRequest.requestLine.getHTTPVersion()->upgradeMinorVersion(1);
    clientRequest.requestLine.setRequestURI(uriPath);
    clientRequest.virtualHost = hostName;
}

HTTPv1_Client::PostMIMERequest HTTPv1_Client::prepareRequestAsPostMIME(const std::string &hostName, const std::string &uriPath)
{
    HTTPv1_Client::PostMIMERequest req;

    setClientRequest(hostName, uriPath);

    clientRequest.requestLine.setRequestMethod("POST");
    clientRequest.content.setContainerType(Common::Content::CONTENT_TYPE_MIME);
    req.urlVars = (Common::URLVars *) clientRequest.requestLine.urlVars();
    req.postVars = clientRequest.content.getMultiPartVars();

    return req;
}

HTTPv1_Client::PostURLRequest HTTPv1_Client::prepareRequestAsPostURL(const std::string &hostName, const std::string &uriPath)
{
    HTTPv1_Client::PostURLRequest req;

    setClientRequest(hostName, uriPath);
    clientRequest.requestLine.setRequestMethod("POST");
    clientRequest.content.setContainerType(Common::Content::CONTENT_TYPE_URL);
    req.urlVars = (Common::URLVars *) clientRequest.requestLine.urlVars();
    req.postVars = clientRequest.content.getUrlPostVars();

    return req;
}

std::string HTTPv1_Client::getResponseHeader(const std::string &headerName)
{
    return serverResponse.headers.getOptionValueStringByName(headerName);
}

void HTTPv1_Client::setReferer(const std::string &refererURL)
{
    clientRequest.requestLine.getHTTPVersion()->upgradeMinorVersion(1);
    clientRequest.headers.replace("Referer", refererURL);
}

void HTTPv1_Client::addURLVar(const std::string &varName, const std::string &varValue)
{
    ((Common::URLVars *) clientRequest.requestLine.urlVars())->addVar(varName, new Memory::Containers::B_Chunks(varValue));
}

void HTTPv1_Client::addCookie(const std::string &cookieName, const std::string &cookieVal)
{
    clientCookies.addCookieVal(cookieName, cookieVal);
}

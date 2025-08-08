#pragma once

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Data/Session.h>
#include <Poco/AutoPtr.h>
#include <string>

// Forward declaration for DatabaseService to be used by NameRequestHandler
class DatabaseService;

class NameRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;
private:
    std::string parseFirstNameFromXML(const std::string& xml);
    std::string makeSoapResponse(const std::string& name);
    void sendSoapFault(Poco::Net::HTTPServerResponse& response,
                       Poco::Net::HTTPResponse::HTTPStatus status,
                       const std::string& faultCode,
                       const std::string& faultString);
};

class NameRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;
};

// New: Declaration for the DatabaseService class
class DatabaseService {
public:
    DatabaseService();
    
    bool connect();
    std::string getFullName(const std::string& firstName);
    void disconnect();

private:
    Poco::AutoPtr<Poco::Data::Session> _session;
};
#pragma once

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <string>

class NameRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;
private:
    std::string processNameFromXML(const std::string& xml);
    std::string makeSoapResponse(const std::string& name);
    // NEW: Declaration for the sendSoapFault method
    void sendSoapFault(Poco::Net::HTTPServerResponse& response, 
                       Poco::Net::HTTPResponse::HTTPStatus status, 
                       const std::string& faultCode, 
                       const std::string& faultString);
};

class NameRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;
};
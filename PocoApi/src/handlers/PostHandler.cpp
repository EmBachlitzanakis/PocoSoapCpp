#include "PostHandler.hpp"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include <iostream>

void PostHandler::handleRequest(Poco::Net::HTTPServerRequest& request, 
                              Poco::Net::HTTPServerResponse& response) {
    try {
        // Set response type
        response.setContentType("application/json");

        // Parse request body
        Poco::JSON::Parser parser;
        auto result = parser.parse(request.stream());
        auto jsonObj = result.extract<Poco::JSON::Object::Ptr>();

        // Create response
        Poco::JSON::Object responseObj;
        responseObj.set("status", "success");
        responseObj.set("message", "Data received successfully");
        
        // Echo back the received data
        responseObj.set("received_data", jsonObj);

        // Send response
        std::ostream& out = response.send();
        responseObj.stringify(out);
        out.flush();

    } catch (const std::exception& ex) {
        // Handle errors
        response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("application/json");
        
        Poco::JSON::Object errorObj;
        errorObj.set("status", "error");
        errorObj.set("message", ex.what());
        
        std::ostream& out = response.send();
        errorObj.stringify(out);
        out.flush();
    }
}

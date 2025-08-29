#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Util/ServerApplication.h"
#include "handlers/PostHandler.hpp"
#include <iostream>

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override {
        if (request.getMethod() == "POST" && request.getURI() == "/api/data") {
            return new PostHandler;
        }
        return nullptr;
    }
};

class WebServerApp : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>&) override {
        try {
            // Create server socket
            Poco::Net::ServerSocket socket(8080);
            
            // Configure server parameters
            Poco::Net::HTTPServerParams* params = new Poco::Net::HTTPServerParams;
            params->setMaxQueued(100);
            params->setMaxThreads(16);
            
            // Create and start server
            Poco::Net::HTTPServer server(
                new RequestHandlerFactory, 
                socket, 
                params
            );
            
            server.start();
            std::cout << "Server started on port 8080" << std::endl;
            
            // Wait for CTRL-C or kill
            waitForTerminationRequest();
            
            // Stop server
            server.stop();
            return Application::EXIT_OK;
            
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            return Application::EXIT_SOFTWARE;
        }
    }
};

POCO_SERVER_MAIN(WebServerApp)

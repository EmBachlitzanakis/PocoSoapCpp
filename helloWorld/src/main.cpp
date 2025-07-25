#include "NameService.hpp"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <iostream>

int main() {
    try {
        // Create a server socket
        Poco::Net::ServerSocket socket(8080);
        
        // Create HTTP server parameters
        Poco::Net::HTTPServerParams* params = new Poco::Net::HTTPServerParams;
    
        
        // Create the HTTP server
        Poco::Net::HTTPServer server(new NameRequestHandlerFactory, socket, params);
        
        // Start the server
        server.start();
        std::cout << "SOAP Server started on port 8080" << std::endl;
        std::cout << "Press Enter to stop the server..." << std::endl;
        
        std::cin.get();
        
        // Stop the server
        server.stop();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

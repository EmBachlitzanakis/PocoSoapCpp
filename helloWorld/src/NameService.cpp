#include "NameService.hpp"
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/XML/XMLWriter.h>
#include <sstream>
using namespace std;
using namespace Poco;

void NameRequestHandler::handleRequest(Net::HTTPServerRequest& request, Net::HTTPServerResponse& response) {
    if (request.getMethod() != "POST") {
        response.setStatusAndReason(Net::HTTPResponse::HTTP_BAD_REQUEST, "Only POST method is supported");
        response.send();
        return;
    }

    // Read the request body
    string requestBody;
    istream& i = request.stream();
    char c;
    while (i.get(c)) {
        requestBody += c;
    }

    // Process the XML and get the name
   string name = processNameFromXML(requestBody);
   string responseXml = makeSoapResponse(name);

    response.setContentType("application/soap+xml");
    response.setContentLength(responseXml.length());
    
    ostream& out = response.send();
    out << responseXml;
}

string NameRequestHandler::processNameFromXML(const string& xml) {
    try {
        XML::DOMParser parser;
        AutoPtr<XML::Document> doc = parser.parseString(xml);
        
        // Find the Name element
        AutoPtr<XML::NodeList> nameNodes = doc->getElementsByTagName("Name");
        if (nameNodes->length() > 0) {
            XML::Node* nameNode = nameNodes->item(0);
            if (nameNode && nameNode->firstChild()) {
                return nameNode->firstChild()->nodeValue() + " Osbourne ";
            }
        }
    } catch (const exception& e) {
        return "Error processing name: " + string(e.what());
    }
    
    return "Name not found in request";
}

Net::HTTPRequestHandler* NameRequestHandlerFactory::createRequestHandler(
    const Net::HTTPServerRequest& request) {
    return new NameRequestHandler;
}
string NameRequestHandler::makeSoapResponse(const string& name) {
    std::ostringstream oss;
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        << "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
        << "<soap:Body>"
        << "<GetNameResponse>"
        << "<Name>" << name << "</Name>"
        << "</GetNameResponse>"
        << "</soap:Body>"
        << "</soap:Envelope>";
    return oss.str();
}
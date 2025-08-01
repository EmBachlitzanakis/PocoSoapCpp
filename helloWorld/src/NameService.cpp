#include "NameService.hpp"
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/XML/XMLWriter.h> // Potentially for future use, but not strictly needed for basic escaping
#include <Poco/Net/NetException.h> // For network related exceptions
#include <sstream>

using namespace std;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::XML;

// --- Constants for common strings ---
const string METHOD_NOT_ALLOWED_MSG = "Only POST method is supported";
const string CONTENT_TYPE_SOAP_XML = "application/soap+xml";
const string NAME_NOT_FOUND_MSG = "Name not found in request";
const string ERROR_PROCESSING_NAME_MSG = "Error processing name: ";

// --- Helper function for XML escaping (simple, for illustration) ---
// For more robust escaping, consider using a dedicated XML library's utility or Poco's XMLWriter
string escapeXml(const string& data) {
    string buffer;
    buffer.reserve(data.size());
    for (char ch : data) {
        switch (ch) {
            case '&':  buffer.append("&amp;");   break;
            case '<':  buffer.append("&lt;");    break;
            case '>':  buffer.append("&gt;");    break;
            case '\"': buffer.append("&quot;");  break;
            case '\'': buffer.append("&apos;");  break;
            default:   buffer.append(1, ch);    break;
        }
    }
    return buffer;
}

// --- NameRequestHandler implementation ---
void NameRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    if (request.getMethod() != HTTPRequest::HTTP_POST) {
        sendSoapFault(response, HTTPResponse::HTTP_METHOD_NOT_ALLOWED, "Client.InvalidMethod", METHOD_NOT_ALLOWED_MSG);
        return;
    }

    string requestBody;
    try {
        // More efficient way to read the request body
        request.stream() >> requestBody;
        if (requestBody.empty()) {
            sendSoapFault(response, HTTPResponse::HTTP_BAD_REQUEST, "Client.EmptyRequest", "Request body is empty.");
            return;
        }
    } catch (const NetException& e) {
        sendSoapFault(response, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "Server.ReadError", "Failed to read request body: " + string(e.what()));
        return;
    } catch (const exception& e) {
        sendSoapFault(response, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "Server.ReadError", "An unexpected error occurred while reading request body: " + string(e.what()));
        return;
    }

    string name;
    try {
        name = processNameFromXML(requestBody);
    } catch (const XML::XMLException& e) {
        sendSoapFault(response, HTTPResponse::HTTP_BAD_REQUEST, "Client.InvalidXML", "Invalid XML format: " + string(e.what()));
        return;
    } catch (const exception& e) {
        sendSoapFault(response, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "Server.ProcessingError", ERROR_PROCESSING_NAME_MSG + string(e.what()));
        return;
    }

    if (name.empty()) { // Indicates "Name not found" or other specific parsing issue
        sendSoapFault(response, HTTPResponse::HTTP_BAD_REQUEST, "Client.NameNotFound", NAME_NOT_FOUND_MSG);
        return;
    }

    string responseXml = makeSoapResponse(name);

    response.setStatus(HTTPResponse::HTTP_OK);
    response.setContentType(CONTENT_TYPE_SOAP_XML);
    response.setContentLength(responseXml.length());
    
    ostream& out = response.send();
    out << responseXml;
}

string NameRequestHandler::processNameFromXML(const string& xml) {
    XML::DOMParser parser;
    AutoPtr<XML::Document> doc = parser.parseString(xml);
    
    AutoPtr<XML::NodeList> nameNodes = doc->getElementsByTagName("Name");
    if (nameNodes->length() > 0) {
        XML::Node* nameNode = nameNodes->item(0);
        if (nameNode && nameNode->firstChild()) {
            // Apply the fixed suffix and escape the XML
            return escapeXml(nameNode->firstChild()->nodeValue()) + " Osbourne ";
        }
    }
    return ""; // Return empty string to signify name not found
}

string NameRequestHandler::makeSoapResponse(const string& name) {
    ostringstream oss;
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        << "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
        << "<soap:Body>"
        << "<GetNameResponse>"
        << "<Name>" << name << "</Name>" // 'name' is already escaped
        << "</GetNameResponse>"
        << "</soap:Body>"
        << "</soap:Envelope>";
    return oss.str();
}

void NameRequestHandler::sendSoapFault(HTTPServerResponse& response, HTTPResponse::HTTPStatus status, 
                                       const string& faultCode, const string& faultString) {
    ostringstream oss;
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        << "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
        << "<soap:Body>"
        << "<soap:Fault>"
        << "<faultcode>" << faultCode << "</faultcode>"
        << "<faultstring>" << escapeXml(faultString) << "</faultstring>" // Escape fault string
        << "</soap:Fault>"
        << "</soap:Body>"
        << "</soap:Envelope>";

    string faultXml = oss.str();
    response.setStatusAndReason(status, faultString); // Set HTTP status and reason
    response.setContentType(CONTENT_TYPE_SOAP_XML);
    response.setContentLength(faultXml.length());
    
    ostream& out = response.send();
    out << faultXml;
}

// --- NameRequestHandlerFactory implementation ---
HTTPRequestHandler* NameRequestHandlerFactory::createRequestHandler(
    const HTTPServerRequest& request) {
    return new NameRequestHandler;
}
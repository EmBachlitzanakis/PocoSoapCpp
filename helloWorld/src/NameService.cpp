Based on the provided `NameService.hpp` and `NameService.cpp` code, you want to integrate a database connection and `SELECT` query into your SOAP service. Here's a revised `NameService.cpp` that incorporates a new database-related class, `DatabaseService`, and uses it to fetch a full name based on a given first name from the SOAP request.

-----

### `NameService.cpp` (Revised)

This code introduces a `DatabaseService` class to encapsulate the database logic, keeping your `NameRequestHandler` clean. The `NameRequestHandler::processNameFromXML` method will now use this service to query the database.

```cpp
#include "NameService.hpp"
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/XML/XMLWriter.h>
#include <Poco/Net/NetException.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/ODBC/Connector.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/DataException.h>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::XML;
using namespace Poco::Data;

// --- DatabaseService Class ---
// Encapsulates all database-related logic to keep the HTTP handler clean.
class DatabaseService {
public:
    DatabaseService() {
        Poco::Data::ODBC::Connector::registerConnector();
    }
    
    // Connects to the database
    bool connect() {
        try {
            const string connectionString =
                "DRIVER={ODBC Driver 17 for SQL Server};"
                "SERVER=PR-BACHLITZANA;"
                "DATABASE=FIDUCIAM_PROD;"
                "UID=db2admin;"
                "PWD=db2admin1;";
            
            _session = new Poco::Data::Session("ODBC", connectionString);
            return true;
        } catch (...) {
            cerr << "Database connection error: \n" ;
            return false;
        }
    }
    
    // Fetches the full name for a given first name
    string getFullName(const string& firstName) {
        if (!_session) {
            throw runtime_error("Database session is not connected.");
        }
        
        try {
            string fullName;
            string userLName;
            
            Statement select(*_session);
            select << "SELECT USER_LNAME FROM [dbo].[USER] WHERE USER_FNAME = ?",
                Keywords::into(userLName),
                Keywords::use(firstName),
                Keywords::limit(1); // Ensure only one result is returned

            select.execute();
            
            RecordSet rs(select);
            if (!rs.empty()) {
                fullName = firstName + " " + userLName;
                return fullName;
            } else {
                return ""; // Not found
            }
        } catch (const DataException& e) {
            cerr << "Query execution error: " << e.displayText() << endl;
            throw; // Re-throw to be caught by the main handler
        }
    }
    
    // Disconnects from the database
    void disconnect() {
        _session = nullptr;
    }

private:
    AutoPtr<Poco::Data::Session> _session;
};

// --- Constants for common strings ---
const string METHOD_NOT_ALLOWED_MSG = "Only POST method is supported";
const string CONTENT_TYPE_SOAP_XML = "application/soap+xml";
const string NAME_NOT_FOUND_MSG = "Name not found in request";
const string ERROR_PROCESSING_NAME_MSG = "Error processing name: ";
const string DB_CONNECTION_FAILED_MSG = "Failed to connect to the database.";
const string DB_QUERY_FAILED_MSG = "Database query failed.";

// --- Helper function for XML escaping (simple, for illustration) ---
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
    
    string firstName;
    try {
        firstName = parseFirstNameFromXML(requestBody);
    } catch (const XML::XMLException& e) {
        sendSoapFault(response, HTTPResponse::HTTP_BAD_REQUEST, "Client.InvalidXML", "Invalid XML format: " + string(e.what()));
        return;
    } catch (const exception& e) {
        sendSoapFault(response, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "Server.ProcessingError", ERROR_PROCESSING_NAME_MSG + string(e.what()));
        return;
    }

    if (firstName.empty()) {
        sendSoapFault(response, HTTPResponse::HTTP_BAD_REQUEST, "Client.NameNotFound", NAME_NOT_FOUND_MSG);
        return;
    }

    string fullName;
    DatabaseService dbService;
    if (!dbService.connect()) {
        sendSoapFault(response, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "Server.DatabaseError", DB_CONNECTION_FAILED_MSG);
        return;
    }

    try {
        fullName = dbService.getFullName(firstName);
    } catch (const exception& e) {
        dbService.disconnect();
        sendSoapFault(response, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "Server.DatabaseError", DB_QUERY_FAILED_MSG);
        return;
    }
    dbService.disconnect();

    if (fullName.empty()) {
        sendSoapFault(response, HTTPResponse::HTTP_NOT_FOUND, "Client.NameNotFoundInDB", "The name '" + firstName + "' was not found in the database.");
        return;
    }
    
    string responseXml = makeSoapResponse(escapeXml(fullName));
    
    response.setStatus(HTTPResponse::HTTP_OK);
    response.setContentType(CONTENT_TYPE_SOAP_XML);
    response.setContentLength(responseXml.length());
    
    ostream& out = response.send();
    out << responseXml;
}

string NameRequestHandler::parseFirstNameFromXML(const string& xml) {
    XML::DOMParser parser;
    AutoPtr<XML::Document> doc = parser.parseString(xml);
    
    AutoPtr<XML::NodeList> nameNodes = doc->getElementsByTagName("Name");
    if (nameNodes->length() > 0) {
        XML::Node* nameNode = nameNodes->item(0);
        if (nameNode && nameNode->firstChild()) {
            return nameNode->firstChild()->nodeValue();
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
        << "<Name>" << name << "</Name>"
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
        << "<faultstring>" << escapeXml(faultString) << "</faultstring>"
        << "</soap:Fault>"
        << "</soap:Body>"
        << "</soap:Envelope>";

    string faultXml = oss.str();
    response.setStatusAndReason(status, faultString);
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
```
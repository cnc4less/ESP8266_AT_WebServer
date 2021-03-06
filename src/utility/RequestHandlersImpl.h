/****************************************************************************************************************************
 * RequestHandlerImpl.h - Dead simple web-server.
 * For ESP8266 AT-command running shields
 *
 * ESP8266_AT_WebServer is a library for the ESP8266 AT-command shields to run WebServer
 *
 * Forked and modified from Arduino ESP8266_AT library
 * Built by Khoi Hoang https://github.com/khoih-prog/ESP8266_AT_WebServer
 * Licensed under MIT license
 * Version: 1.0.0
 *
 *
 * Version Modified By   Date      Comments
 * ------- -----------  ---------- -----------
 *  1.0.0   K Hoang      12/02/2020 Initial coding for Arduino Mega, Teensy, etc
 *****************************************************************************************************************************/

#ifndef RequestHandlerImpl_h
#define RequestHandlerImpl_h

#include "RequestHandler.h"

class FunctionRequestHandler : public RequestHandler {
public:
    FunctionRequestHandler(ESP8266_AT_WebServer::THandlerFunction fn, ESP8266_AT_WebServer::THandlerFunction ufn, const String &uri, HTTPMethod method)
    : _fn(fn)
    , _ufn(ufn)
    , _uri(uri)
    , _method(method)
    {
    }

    bool canHandle(HTTPMethod requestMethod, String requestUri) override  {
        if (_method != HTTP_ANY && _method != requestMethod)
            return false;

        if (requestUri == _uri)
            return true;

        if (_uri.endsWith("/*"))
        {
            String _uristart = _uri;
            _uristart.replace("/*","");
            if (requestUri.startsWith(_uristart))
                return true;
        }

        return false;
    }

    bool canUpload(String requestUri) override  {
        if (!_ufn || !canHandle(HTTP_POST, requestUri))
            return false;

        return true;
    }

    bool handle(ESP8266_AT_WebServer& server, HTTPMethod requestMethod, String requestUri) override {
        if (!canHandle(requestMethod, requestUri))
            return false;

        _fn();
        return true;
    }

    void upload(ESP8266_AT_WebServer& server, String requestUri, HTTPUpload& upload) override {
        if (canUpload(requestUri))
            _ufn();
    }

protected:
    ESP8266_AT_WebServer::THandlerFunction _fn;
    ESP8266_AT_WebServer::THandlerFunction _ufn;
    String _uri;
    HTTPMethod _method;
};

class StaticRequestHandler : public RequestHandler {
public:

    bool canHandle(HTTPMethod requestMethod, String requestUri) override  {
        if (requestMethod != HTTP_GET)
            return false;

        if ((_isFile && requestUri != _uri) || !requestUri.startsWith(_uri))
            return false;

        return true;
    }

    static String getContentType(const String& path) {
        if (path.endsWith(".html")) return "text/html";
        else if (path.endsWith(".htm")) return "text/html";
        else if (path.endsWith(".css")) return "text/css";
        else if (path.endsWith(".txt")) return "text/plain";
        else if (path.endsWith(".js")) return "application/javascript";
        else if (path.endsWith(".png")) return "image/png";
        else if (path.endsWith(".gif")) return "image/gif";
        else if (path.endsWith(".jpg")) return "image/jpeg";
        else if (path.endsWith(".ico")) return "image/x-icon";
        else if (path.endsWith(".svg")) return "image/svg+xml";
        else if (path.endsWith(".ttf")) return "application/x-font-ttf";
        else if (path.endsWith(".otf")) return "application/x-font-opentype";
        else if (path.endsWith(".woff")) return "application/font-woff";
        else if (path.endsWith(".woff2")) return "application/font-woff2";
        else if (path.endsWith(".eot")) return "application/vnd.ms-fontobject";
        else if (path.endsWith(".sfnt")) return "application/font-sfnt";
        else if (path.endsWith(".xml")) return "text/xml";
        else if (path.endsWith(".pdf")) return "application/pdf";
        else if (path.endsWith(".zip")) return "application/zip";
        else if(path.endsWith(".gz")) return "application/x-gzip";
        else if (path.endsWith(".appcache")) return "text/cache-manifest";
        return "application/octet-stream";
    }

protected:
    String _uri;
    String _path;
    String _cache_header;
    bool _isFile;
    size_t _baseUriLength;
};


#endif //RequestHandlerImpl_h

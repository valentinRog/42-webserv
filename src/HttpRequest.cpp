#include "HttpRequest.hpp"

HttpRequest::HttpRequest() {
    _validMethods.push_back("GET");
    _validMethods.push_back("POST");
    _validMethods.push_back("DELETE");
}

void    HttpRequest::readRequest(std::string & request) {
    _request = request;
    std::stringstream ss(request.c_str());
    ss >> _method;
    ss >> _path;
    ss >> _version;

    std::cout << "Method:" << _method << std::endl;
    std::cout << "path:" << _path << std::endl;
    std::cout << "version:" << _version << std::endl;
}

std::string HttpRequest::getMethod() { return (_method); }

std::string HttpRequest::getPath() { return (_path); }

std::string HttpRequest::getVersion() { return (_version); }
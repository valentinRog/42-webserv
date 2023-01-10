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

    // verifMethod();
    // verifPath(); 
    // verifVersion();

    std::cout << "Method:" << _method << std::endl;
    std::cout << "path:" << _path << std::endl;
    std::cout << "version:" << _version << std::endl;
}

// void    HttpRequest::verifMethod() {
//     for (std::vector<std::string>::iterator i = _validMethods.begin(); i != _validMethods.end(); i++) {
//         if (*i == _method)
//             return ;
//     }
//     std::cout << "thow exeption method not found invalid method" << std::endl;
// }

// void    HttpRequest::verifPath() {
//     size_t pos;

//     if ((pos = _path.find("?")) != std::string::npos) {
//         _query = _path.substr(pos + 1);
//         _path = _path.substr(0, _path.size() - _query.size() - 1); 
//     }
// }

// void    HttpRequest::verifVersion() {   
//     if (_version.size() > 8)
//         std::cout << "Http error" << std::endl;
//     else if (_version.substr(0, 5) != "HTTP/") {
//         std::cout << "Http error" << std::endl;
//     }
//     else if (_version.substr(5, 3) != "1.1") {
//         std::cout << "Http error" << std::endl;
//     }
// }

std::string HttpRequest::getMethod() { return (_method); }

std::string HttpRequest::getPath() { return (_path); }

std::string HttpRequest::getVersion() { return (_version); }
#include "HttpRequest.hpp"

HttpRequest::HttpRequest() {
    _validMethods.push_back("GET");
    _validMethods.push_back("POST");
    _validMethods.push_back("DELETE");
}

void    HttpRequest::readRequest(std::string & request) {
    std::string line = request.substr(0, request.find("\n"));

    _method = line.substr(0, line.find(" "));
    std::cout << _method << std::endl; 
    line.erase(0, line.find(" ") + 1);
    verifMethod();

    _path = line.substr(0, line.find(" "));
    std::cout << _path << std::endl;
    line.erase(0, line.find(" ") + 1);
    verifPath();

    _version = line;
    std::cout << _version << std::endl;
    verifVersion();
}

void    HttpRequest::verifMethod() {
    for (std::vector<std::string>::iterator i = _validMethods.begin(); i != _validMethods.end(); i++) {
        if (*i == _method)
            return ;
    }
    std::cout << "thow exeption method not found pour plus tard erreur 400" << std::endl;
}

void    HttpRequest::verifPath() {
    std::cout << "thow exeption path ou file not good pour plus tard erreur 400" << std::endl;
}

void    HttpRequest::verifVersion() {   
    if (_version.substr(0, 5) != "HTTP/") {
        std::cout << "thow exeption miss HTTP not good pour plus tard erreur 400" << std::endl;
    }
    else if (_version.substr(5, 3) != "1.1") {
        std::cout << "thow exeption miss HTTP not good pour plus tard erreur 400" << std::endl;
    }
}
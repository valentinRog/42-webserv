#include "HttpResponse.hpp"
#include <cstdlib>

HttpResponse::HttpResponse() {
    _responseStatus.insert(std::pair<int, std::string>(200, "OK"));
    _responseStatus.insert(std::pair<int, std::string>(201, "Created"));
    _responseStatus.insert(std::pair<int, std::string>(400, "Bad Request"));
    _responseStatus.insert(std::pair<int, std::string>(404, "Not Found"));
    _responseStatus.insert(std::pair<int, std::string>(405, "Method Not Allowed"));
    _responseStatus.insert(std::pair<int, std::string>(505, "Version Not Supported"));
}

void    HttpResponse::response(HttpRequest httpRequest, int clientFd) {
    _clientFd = clientFd;
    _rootPath = std::getenv("PWD"); //PAS C++98 et need to changer en fonction du serv loc
    _defaultPathError = "html/error400.html";
    
    // std::string root_path = __FILE__;
    // size_t pos = root_path.find_last_of("/\\");
    // if (pos != std::string::npos) {
    //     root_path = root_path.substr(0, pos);
    // }

    _method = httpRequest.getMethod();
    _path = httpRequest.getPath();
    _version = httpRequest.getVersion();
    std::cout << _path << std::endl;
    if (_method == "GET")
        getMethod();
    else if (_method == "POST") {}
    else if (_method == "DELETE") {}
}

void    HttpResponse::getMethod() {
    std::cout << "Get Method" << std::endl;
    //default loc; si vide;
    try {
        std::ifstream fd(_rootPath + _path); //PQQQQ
        if (!fd)
            sendResponse(404, _defaultPathError);
        std::string page((std::istreambuf_iterator<char>(fd)), std::istreambuf_iterator<char>());
        sendResponse(200, page);
    }
    catch (const std::ios_base::failure & fail) {
        std::cerr << fail.what() << std::endl; //a enlever
        sendResponse(400, _defaultPathError);
    }
}

void HttpResponse::sendResponse(int nb, std::string page) {
    std::string res = _version + " " + std::to_string(nb)+ " " +_responseStatus.find(nb)->second + "\r\n";
    res = res + "Content-type: text/html" + "\r\n\r\n";//a verif le type de docu + pt rajouter content-length
    res = res + page + "\r\n";
    send(_clientFd, res.c_str(), res.size(), 0);
}
#pragma once

#include "common.h"
#include "ServerConf.hpp"
#include "VirtualHostMapper.hpp"

class HttpResponse {
        std::map<int, std::string> _responseStatus; 

        //request
        int _clientFd;
        std::string _methodRequest;
        std::string _version;
        std::string _rootPath;

        //serv
        std::string _path;
        std::string _root;
        std::vector<std::string> _allowedMethod;
        std::string _defaultPathError;
        std::string _index;
        bool        _dirListing;
        std::string _redir;

        TestServ _serv;
        ServerConf _conf;
    public:
        HttpResponse();
        void    setInformation(HttpRequest httpRequest, int clientFd, TestServ serv);
        void    response(HttpRequest httpRequest, int clientFd, TestServ serv);
        int     verifLocation(std::string path, std::vector<Location *> locs);
        
        void    toRedir();
        void    toDirListing();
        void    getMethod();
        void    postMethod();
        void    deleteMethod();
        
        std::string getContentType(std::string path);
        void    sendResponse(int nb, std::string page);

};
#pragma once

#include "common.h"

class HttpRequest {
        std::vector<std::string> _validMethods;

        std::string _request;
        std::string _method;
        std::string _path;
        std::string _version;
        std::string _query;
        //std::map<std::string, std::string> _query;
    public:
        HttpRequest();
        std::string copy_until(const std::string & str, char c);
        void readRequest(std::string & request);
        void verifMethod();
        void verifPath();
        void verifVersion();
};
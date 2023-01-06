#pragma once

#include "common.h"

class HttpRequest {
        std::vector<std::string> _validMethods;

        std::string _method;
        std::string _path;
        std::string _version;
    public:
        HttpRequest();
        std::string copy_until(const std::string & str, char c);
        void readRequest(std::string & request);
        void verifMethod();
        void verifPath();
        void verifVersion();
};
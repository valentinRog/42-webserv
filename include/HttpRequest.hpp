#pragma once

#include "VirtualHostMapper.hpp"
#include "common.h"

class HttpRequest {
    std::vector< std::string > _validMethods;

    std::string _request;
    std::string _method;
    std::string _path;
    std::string _version;
    std::string _query;
    //std::map<std::string, std::string> _query;

public:
    HttpRequest( const std::string &s, const VirtualHostMapper &vhm );
    std::string copy_until( const std::string &str, char c );

    std::string getMethod();
    std::string getPath();
    std::string getVersion();

private:
    void _readRequest( const std::string &request );
};
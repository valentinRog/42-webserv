#include "HttpRequest.hpp"

HttpRequest::HttpRequest( const std::string &s, const VirtualHostMapper &vhm ) {
    _validMethods.push_back( "GET" );
    _validMethods.push_back( "POST" );
    _validMethods.push_back( "DELETE" );
    _readRequest( s );
}

std::string HttpRequest::getMethod() { return ( _method ); }

std::string HttpRequest::getPath() { return ( _path ); }

std::string HttpRequest::getVersion() { return ( _version ); }

void HttpRequest::_readRequest( const std::string &request ) {
    _request = request;
    std::stringstream ss( request.c_str() );
    ss >> _method;
    ss >> _path;
    ss >> _version;

    std::cout << "Method:" << _method << std::endl;
    std::cout << "path:" << _path << std::endl;
    std::cout << "version:" << _version << std::endl;
}

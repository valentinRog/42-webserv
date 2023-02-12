#pragma once

#include "HTTP.hpp"
#include "ServerConf.hpp"

/* ----------------------------- RequestHandler ----------------------------- */

class RequestHandler {
    Ptr::Shared< HTTP::Request > _request;
    Ptr::Shared< ServerConf >    _conf;
    const ServerConf::Route *    _route;
    std::string                  _path;

    struct CGI {
        enum e_env_key {
            PATH_INFO,
            REQUEST_METHOD,
            CONTENT_TYPE,
            CONTENT_LENGTH,
            QUERY_STRING,
            HTTP_COOKIE,
            HTTP_REFERER,
            HTTP_USER_AGENT,
            REMOTE_ADDR,
            REMOTE_HOST,
            REMOTE_PORT,
            SERVER_NAME,
            SERVER_SOFTWARE,
            SERVER_PROTOCOL,
            SERVER_PORT,
            SCRIPT_FILENAME,
            SCRIPT_NAME,
            REDIRECT_STATUS
        };
        static const std::string &env_key_to_string( e_env_key key );

        struct Env : public std::map< e_env_key, std::string > {
            char **     c_arr() const;
            static void clear_c_env( char **envp );
        };
    };

public:
    RequestHandler( Ptr::Shared< HTTP::Request > request,
                    Ptr::Shared< ServerConf >    conf );

    HTTP::Response make_response();

private:
    HTTP::Response _redir();
    HTTP::Response _autoindex();
    HTTP::Response _get();
    HTTP::Response _post();
    HTTP::Response _delete();
    HTTP::Response _cgi( const std::string &bin_path );

    const std::string &_content_type( const std::string &path ) const;
};

/* -------------------------------------------------------------------------- */

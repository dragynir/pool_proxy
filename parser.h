#pragma once

#include <unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<strings.h>
#include<string.h>


#include<iostream>


#include<errno.h>
#include<assert.h>

struct http_info{

	char * protocol;
	char * url;
	char * host;
	char * resource;
};



class HttpParser{

public:


	static int parse_client_request(char *  request, int request_length, char ** url, char ** host, char ** resource);
	
private:


	static int parse_start_line(char * start_line, int line_length, http_info * info);

	static int parse_url(char* parsing_str, http_info * info);

};

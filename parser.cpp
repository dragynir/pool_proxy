#include"parser.h"



int HttpParser::parse_start_line(char * start_line, int line_length, http_info * info){


	char * cursor = start_line;

	char * method_end = strchr(cursor, ' ');
	*method_end = '\0';

    if (strcmp(cursor, "GET") != 0) {
        fprintf(stderr, "Unsupported method! %s\n", cursor);
        return -1;
    }

    *method_end = ' ';

    cursor = method_end;
    cursor++;


	char * url_end = strchr(cursor, ' ');
	int url_length = url_end - cursor;


	char * url = (char *)malloc(url_length + 1);


	if(NULL == url){
		perror("malloc");
		return -1;
	}

	bcopy(cursor, url, url_length);
	url[url_length] = '\0';
	info->url = url;

	*url_end = ' ';

	cursor = url_end;
	cursor++;
	return 0;
}




int HttpParser::parse_url(char* parsing_str, http_info * info) {
    if (8 > strlen(parsing_str)) {
        fprintf(stderr, "URL format: http://<host_name>[:port]/[page]>\n");
        return -1;
    }

    int url_length = strlen(parsing_str);




    char * found = strstr(parsing_str, "//");
    char * host_start = parsing_str;
    char * host_end = NULL;

    if(NULL != found){

    	if(url_length - 1 == parsing_str - found){
    		printf("%s\n", "Invalid url");
    		return -1;
    	}
    	host_start = found + 2;
    }

    found = strchr(host_start, ':');



    char * resource_start = NULL;
    if(NULL == found){
    	host_end = strchr(host_start + 1, '/');
    	if(NULL == host_end){
    		printf("%s\n", "Invalid url: host");
    		return -1;
    	}
    	resource_start = host_end;
    }else{
    	host_end = found;
    	// ресурс начинается со /
    	resource_start = strchr(found, '/');
    	if(NULL == resource_start){
    		printf("%s\n", "Invalid url: resource");
    		return -1;
    	}
    }

    char keep_char = *host_end;
    *host_end = '\0';
    int host_length = strlen(host_start);
    info->host = (char *)malloc(host_length + 1);


    if(NULL == info->host){
    	perror("malloc: info.host");
    	return -1;
    }

    bcopy(host_start, info->host, host_length + 1);
    *host_end = keep_char;

    int resource_length = strlen(resource_start);
    info->resource = (char *)malloc(resource_length + 1);


    if(NULL == info->resource){
    	perror("malloc: info.resource");
    	free(info->host);
    	return -1;
    }

    bcopy(resource_start, info->resource, resource_length + 1);

    return 0;
}





int HttpParser::parse_client_request(
	char *  request, int request_length, char ** url, char ** host, char ** resource){

	char * start_line_end = strchr(request, '\r');
	*start_line_end = '\0';


	http_info info;

	int res = 0;
	res = parse_start_line(request, request  - start_line_end, &info);


	if(res < 0){
		return -1;
	}


	*start_line_end = '\r';

	*url = info.url;


	res = parse_url(info.url, &info);
	
	if(0 == res){
		//printf("Res: ++%s++\n", info.resource);
		//printf("Host: ++%s++\n", info.host);
		*resource = info.resource;
		*url = info.url;
		*host = info.host;
		return 0;
	}

	
	
	

	char * cursor = start_line_end;
	*cursor = '\r';
	cursor+=2;
	// next line


	while(1){
		
		char * host_line_end = strchr(cursor, '\r');

		if(NULL == host_line_end){
			free(info.url);
			return -1;
		}

		*host_line_end = '\0';

		char * has_host = strstr(cursor, "Host:");

		if(NULL != has_host){
			cursor = strchr(cursor, ' ');

			if(NULL == cursor){
				free(info.url);
				return -1;
			}


			cursor++;

			int host_length = host_line_end - cursor;

			char * host = (char *)malloc(host_length);


			if(NULL == host){
				perror("malloc");
				free(info.url);
				return -1;
			}


			bcopy(cursor, host, host_length);
			info.host = host;
			break;
		}

		cursor = host_line_end;
		*cursor = '\r';
		cursor+=2;
	}

	*url = NULL;
	*resource = info.url;
	*host = info.host;
	return 0;
}

#include"proxy.h"


ProxyWorker::ProxyWorker(SafeCacheMap * cache, std::queue<int> * connections,
    pthread_mutex_t * connections_mutex, pthread_cond_t * connections_cond){
	

    this->cache = cache;
    this->connections = connections;
    this->connections_mutex = connections_mutex;
    this->connections_cond = connections_cond;


	this->is_alive = true;
}



ProxyWorker::~ProxyWorker(){
    close_all_sessions();
}



int ProxyWorker::create_new_session(){

    int client_socket = this->connections->front();
    this->connections->pop();



    if(client_socket < 0){
        perror("Invalid connection fd");
        return -1;
    }



    /*if(fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1){
        perror("fcntl on create: ");
        return -1;
    }*/


    Session * session = NULL;
    try
    {
        session = new Session(client_socket, this->cache);
    }
    catch (std::bad_alloc& ba)
    {
        std::cout << "bad_alloc caught: " << ba.what() << '\n';
        return -1;
    }
    
    this->sessions.push_back(session);

    return 0;
}


void ProxyWorker::start(){

	int poll_wait_infinite = 200;//wait for 200 milliseconds

	pollfd * fds_array = NULL;

	

	while(this->is_alive){

		int result = 0;



        pthread_mutex_lock(this->connections_mutex);


        if(this->connections->empty()){

            if(this->sessions.empty()){




                while(this->connections->empty()){   
                    std::cout << "Wait for connections: " << "\n";
                    errno = pthread_cond_wait(this->connections_cond, this->connections_mutex);

                    if(0 != errno){
                        perror("pthread_cond_wait");
                        pthread_mutex_unlock(this->connections_mutex);
                        break;
                    }
                }



                result = create_new_session();
                if(result < 0){
                    std::cout << "Can't create new session" << "\n";
                }


            }

        }else{

            result = create_new_session();
            std::cout << "Create new session from queue" << "\n";
            if(result < 0){
                std::cout << "Can't create new session" << "\n";
            }
        }



        pthread_mutex_unlock(this->connections_mutex);





		result = update_sessions();

		if(result < 0){
			printf("%s\n", "update sessions error...");
			break;
		}

		fds_array = &this->fdset[0];

		assert(NULL != fds_array);


		int fdset_size = this->fdset.size();	
		int poll_result = poll(fds_array, fdset_size, poll_wait_infinite);


		if(poll_result < 0){
			perror("poll");
			break;
		}

		int sessions_count = this->sessions.size();
		
		for(int i = 0; i < sessions_count; ++i){
			result = serve_session(this->sessions[i], &fds_array[i * 2]);
			if(result < 0){
				assert(NULL != this->sessions[i]);
				delete this->sessions[i];
				this->sessions.erase(this->sessions.begin() + i);
				this->fdset.erase(this->fdset.begin() +  (i * 2));
				this->fdset.erase(this->fdset.begin() +  (i * 2));
				--i;
				--sessions_count;
			}
		}

	}

	close_all_sessions();

}





int ProxyWorker::serve_session(Session * session, pollfd * fds){
	
	assert(NULL != session);
	assert(NULL != fds);


	session->getState();


	int res = 0;

    switch (session->getState()) {

        case RECEIVE_CLIENT_REQUEST:
            
            if((POLLHUP | POLLERR) & fds[0].revents){
            	fprintf(stderr, "Client closed!\n");
            	session->close_sockets();
            	return -1;
            }

            if(POLLIN & fds[0].revents){
            	res = session->read_client_request();
            	if(res < 0){
            		session->close_sockets();
            	}
            	return res;
        	}
            break;
        case SEND_REQUEST:

            
            if ((POLLHUP | POLLERR) & fds[1].revents) {
                fprintf(stderr, "Remote server closed!\n");
                session->try_erase_cache();
                session->close_sockets();
                return -1;
            }

            if (POLLOUT & fds[1].revents) {
            	res = session->send_request();
            	if(res < 0){
            		session->close_sockets();
            	}

            	return res;
            }
            break;

        case MANAGE_RESPONSE:

            if ((fds[0].revents & (POLLHUP | POLLERR)) || (fds[1].revents & (POLLHUP | POLLERR))) {
                session->try_erase_cache();
            	session->close_sockets();
                std::cout << "Close connections for:===========================" << "\n";
                fprintf(stderr, "Connection closed POLLHUP!\n");
                return -1;
            }

            if ((fds[1].revents & POLLIN) || (session->is_sending() && (fds[0].revents & POLLOUT))) {
            	int res = session->manage_response(fds[1].revents & POLLIN, fds[0].revents & POLLOUT);
            	if(res < 0){
            		session->close_sockets();
            	}
                return res;
            }

            

            break;



        case USE_CACHE:
            if (fds[0].revents & POLLHUP) {
                fprintf(stderr, "Connection closed!\n");
                session->close_sockets();
                return -1;
            }
            if(fds[0].revents & POLLOUT){

            	res = session->use_cache();
            	if(res < 0){
            		session->close_sockets();
            	}
            	return res;

            }

            break;
        default:
            printf("%s\n", "Invalid session state");
        	return -1;

	}



	return 0;
}




int ProxyWorker::update_sessions(){
	int sessions_count = this->sessions.size();
	this->fdset.reserve(sessions_count * 2);
	int fds_count = this->fdset.size() / 2;



    pollfd new_pollfd;
    
    for (int i = 0; i < sessions_count; ++i) {

    	
    	int client_i = i * 2;
    	int server_i = client_i + 1;



        if(fds_count < i + 1){
       		this->fdset.push_back(new_pollfd);
       		this->fdset.push_back(new_pollfd);
        }


        this->fdset[client_i].fd = this->sessions[i]->client_socket;
        this->fdset[server_i].fd = this->sessions[i]->remote_socket;


         this->fdset[client_i].revents = 0;
         this->fdset[server_i].revents = 0;

        switch (sessions[i]->getState()) {
            case RECEIVE_CLIENT_REQUEST:
                this->fdset[client_i].events = POLLIN;
                this->fdset[server_i].events = 0;
                break;
            case SEND_REQUEST:
                this->fdset[client_i].events = 0;
                this->fdset[server_i].events = POLLOUT;
                break;

            case MANAGE_RESPONSE:
                this->fdset[client_i].events = POLLOUT;
                this->fdset[server_i].events = POLLIN;
                break;
            case USE_CACHE:
                this->fdset[client_i].events = POLLOUT;
                this->fdset[server_i].events = 0;
                break;
            default:
            	printf("%s\n", "Invalid session state");
            	return -1;
        }
    }

    return 0;
}






void ProxyWorker::close_all_sessions(){
	printf("%s\n", "All sessions closed");
	for(size_t i = 0; i < this->sessions.size(); ++i){
		this->sessions[i]->close_sockets();
		assert(NULL != this->sessions[i]);
		delete this->sessions[i];
	}
}

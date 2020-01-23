




#pragma once



#include"session.h"


#include<string>
#include<vector>
#include<map>
#include<poll.h>


#include<queue>


class ProxyWorker{


public:

	ProxyWorker(SafeCacheMap * cache, std::queue<int> * connections, pthread_mutex_t * connections_mutex, pthread_cond_t * connections_cond);

	~ProxyWorker();


	void start();

	void stop();

private:

	int serve_session(Session * session, pollfd * fds);

	int create_new_session();

	int update_sessions();

	void close_all_sessions();

	bool is_alive;
	SafeCacheMap * cache;
	std::vector<pollfd> fdset;
	std::vector<Session *> sessions;


	std::queue<int> * connections;
	pthread_mutex_t * connections_mutex;
	pthread_cond_t * connections_cond;

};

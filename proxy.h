#pragma once



#include"proxy_worker.h"


#include<string>
#include<vector>
#include<map>
#include<poll.h>





struct worker_attr{
	ProxyWorker * worker;
};



class Proxy{


public:

	Proxy(int listener, int pool_size);

	~Proxy();

	void start();

private:


	int accept_connection();

	std::vector<pthread_t> worker_threads;
	pthread_attr_t pthread_detach_attr;
	std::vector<ProxyWorker *> workers;


	std::queue<int> connections;
	pthread_cond_t connections_cond; 
    pthread_mutex_t connections_mutex;


	int listener;
	int pool_size;
	SafeCacheMap cache;
};

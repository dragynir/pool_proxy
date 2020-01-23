#include"proxy.h"


Proxy::Proxy(int listener, int pool_size){
	this->listener = listener;
    this->pool_size = pool_size;
    this->worker_threads.reserve(pool_size);
    this->workers.reserve(pool_size);
}





void * start_worker(void * worker_params){
    assert(NULL != worker_params);


    worker_attr * attr = (worker_attr *) worker_params;

    assert(NULL != attr->worker);

    ProxyWorker * worker = attr->worker;

    free(attr);

    worker->start();

    return NULL;
}



int Proxy::accept_connection(){
    std::cout << "Start accepting" << "\n";
    int client_socket = accept(this->listener, NULL, NULL);
    


    if(client_socket < 0){
        perror("accept");
        return -1;
    }


    if(fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1){
        perror("fcntl accept:");
        return -1;
    }

    

    errno = pthread_mutex_lock(&this->connections_mutex);



    if(0 != errno){
        perror("pthread_mutex_lock");
    }

    std::cout << "Push new connection" << "\n";
    this->connections.push(client_socket);

    
    pthread_cond_broadcast(&this->connections_cond);



    errno = pthread_mutex_unlock(&this->connections_mutex);

    return 0;
}   


void Proxy::start(){



    errno = pthread_mutex_init(&this->connections_mutex, NULL);

    if(0 != errno){
        perror("pthread_mutex_init");
        return;
    }


    errno = pthread_cond_init(&this->connections_cond, NULL);



    if(0 != errno){
        perror("pthread_cond_init");
        return;
    }




    pthread_t thread_id;
    int successed_startup_count = 0; 
    worker_attr * worker_params;

    for(int i = 0; i < pool_size; ++i){


        //reserve up
        //this->worker_threads[i] = thread_id;



        ProxyWorker * proxy_worker = NULL;

        try
        {

            proxy_worker = new ProxyWorker(&this->cache,
                &this->connections, &this->connections_mutex, &this->connections_cond);

        }
        catch (std::bad_alloc& ba)
        {
            std::cout << "bad_alloc caught: " << ba.what() << '\n';
            break;
        }

        worker_params = (worker_attr *)malloc(sizeof(worker_attr));


        if(NULL == worker_params){
            perror("malloc");
            delete proxy_worker;  
            break;
        }

        worker_params->worker = proxy_worker;


        errno = pthread_create(&this->worker_threads[i], NULL, start_worker, worker_params);

        if(0 != errno){
            free(worker_params);
            delete proxy_worker;
            perror("pthread_create");
            break;
        }

        std::cout << "Create: " << i << "\n";

        this->workers[i] = proxy_worker;

        successed_startup_count++;


    }


    int result = 0;

    while(1){
        result = accept_connection();
        if(result < 0){
            break;
        }
    }


    int bad_join = 0;
    for(int i = 0; i < successed_startup_count; ++i){

        errno = pthread_join(this->worker_threads[i], NULL);

        if(0 != errno){
            perror("pthread_join");
            bad_join = 1;
        }
    }



    if(0 != bad_join){
        return;
    }else{

        // delete resources
    }


}




Proxy::~Proxy(){

    close(this->listener);

    pthread_mutex_destroy(&this->connections_mutex);
    pthread_cond_destroy(&this->connections_cond);

}

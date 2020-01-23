
#include"cache.h"



CacheRecord::CacheRecord(bool local){
	this->data = NULL;
	this->local = local;
	this->size = 0;
	this->full = false;
	this->out_of_date = false;
	this->capacity = 0;
	this->links_count = 0;

	errno = pthread_rwlock_init(&rwlock, NULL);
	if(0 != errno){
		throw InitRwlockException();
	}
}


CacheRecord::~CacheRecord(){
	free(this->data);
	errno = pthread_rwlock_destroy(&rwlock);
	if(0 != errno){
		perror("pthread_rwlock_destroy");
	}
}

//обработать exception в session
int CacheRecord::add_data(char * add_data, size_t add_size){


	write_lock();

	if(0 == add_size){
		return 0;
	}

	size_t new_size = add_size + this->size;
	if(new_size > this->capacity){

		/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! factor * 2

		if(NULL == this->data){
			this->data = (char *)malloc(new_size * 2);
		}else{
			this->data = (char *)realloc(this->data, new_size * 2);
		}

		if(NULL == this->data){
			exit(EXIT_FAILURE);
		}
		

		if(NULL == this->data){
			perror("realloc");
			return -1;
		}
		this->capacity = new_size * 2;
	}



	bcopy(add_data, this->data + this->size, add_size);
	assert(new_size >= this->size);
	this->size = new_size;


	unlock();

	return 0;
}







void CacheRecord::read_lock(){
	errno = pthread_rwlock_rdlock(&this->rwlock);
	if(0 != errno){
		perror("pthread_rwlock_rdlock");
		throw RwlockException();
	}
}


void CacheRecord::write_lock(){
	errno = pthread_rwlock_wrlock(&this->rwlock);
	if(0 != errno){
		perror("pthread_rwlock_rdlock");
		throw RwlockException();
	}
}


void CacheRecord::unlock(){
	errno = pthread_rwlock_unlock(&this->rwlock); 
	if(0 != errno){
		perror("pthread_rwlock_wrlock");
		throw RwlockException();
	}
}

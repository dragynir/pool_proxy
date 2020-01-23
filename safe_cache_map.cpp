
#include"safe_cache_map.h"


SafeCacheMap::SafeCacheMap(){
	errno = pthread_mutex_init(&this->mutex, NULL);

	if(0 != errno){
		perror("pthread_mutex_init");
		throw MutexInitException();
	}
}


SafeCacheMap::~SafeCacheMap(){
	errno = pthread_mutex_destroy(&this->mutex);
	if(0 != errno){
		perror("pthread_mutex_destroy");
	}
}



std::map<std::string, CacheRecord *>::iterator SafeCacheMap::find(std::string& key){
	return this->cache.find(key);
}


void SafeCacheMap::insert(std::string& key, CacheRecord * record){
	this->cache.insert(std::pair<std::string, CacheRecord *>(key, record));
}


void SafeCacheMap::erase(std::map<std::string, CacheRecord *>::iterator it){
	this->cache.erase(it);
}



void SafeCacheMap::lock(){
	errno = pthread_mutex_lock(&this->mutex);
	if(0 != errno){
		perror("pthread_mutex_lock");
		throw MutexError();
	}
}

void SafeCacheMap::unlock(){
	errno = pthread_mutex_unlock(&this->mutex);
	if(0 != errno){
		perror("pthread_mutex_lock");
		throw MutexError();
	}
}

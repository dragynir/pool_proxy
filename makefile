all: proxy.o main.o cache.o session.o parser.o safe_cache_map.o proxy_worker.o
	g++ -g proxy.o main.o session.o cache.o parser.o safe_cache_map.o proxy_worker.o -o s -pthread


main.o: main.cpp
	g++ -g -Wall -c  main.cpp


proxy_worker.o: proxy_worker.cpp
	g++ -g -Wall -c proxy_worker.cpp

proxy.o: proxy.cpp
	g++ -g -Wall -c proxy.cpp

session.o: session.cpp
	g++ -g -Wall -c session.cpp

cache.o: cache.cpp
	g++ -g -Wall -c cache.cpp

parser.o: parser.cpp
	g++ -g -Wall -c parser.cpp

safe_cache_map.o: safe_cache_map.cpp
	g++ -g -Wall -c safe_cache_map.cpp

clean:
	rm proxy.o main.o cache.o session.o parser.o safe_cache_map.o proxy_worker.o s
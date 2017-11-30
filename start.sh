g++ -Wall -g -I ./ -c ACstates.h http.h Hash.h
g++ -Wall -g -I ./ *.o crawler.cpp -o ./crawler -levent -lpthread
.PHONY:all
all: server client

server: dict_server.cc
	g++ -o $@ $^ -std=c++11 -I../../libs/muduo/include -L../../libs/muduo/lib -lmuduo_net -lmuduo_base -lpthread
client: dict_client.cc
	g++ -o $@ $^ -std=c++11 -I../../libs/muduo/include -L../../libs/muduo/lib -lmuduo_net -lmuduo_base -lpthread
.PHONY:clean
clean:
	rm -f server client
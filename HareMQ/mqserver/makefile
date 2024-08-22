CFLAG= -I../libs/muduo/include
LFLAG= -L../libs/muduo/lib -lgtest -lprotobuf -lsqlite3 -lpthread -lmuduo_net -lmuduo_base -lz
server: server.cc ../mqcommon/*.cc ../libs/muduo/include/muduo/protoc/codec.cc
	g++ -g -std=c++11 $(CFLAG) $^ -o $@  $(LFLAG)
.PHONY:clean
clean:
	rm -f server;
	rm -rf data;
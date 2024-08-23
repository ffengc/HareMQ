CFLAG= -I../tools/muduo/include
LFLAG= -L../tools/muduo/lib -lgtest -lprotobuf -lsqlite3 -lpthread -lmuduo_net -lmuduo_base -lz
server: server.cc ../mqcommon/*.cc ../tools/muduo/include/muduo/protoc/codec.cc
	g++ -g -std=c++11 $(CFLAG) $^ -o $@  $(LFLAG)
.PHONY:clean
clean:
	rm -f server;
	rm -rf data/ log/;
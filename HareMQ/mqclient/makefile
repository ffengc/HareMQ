CFLAG= -I../libs/muduo/include
LFLAG= -L../libs/muduo/lib -lgtest -lprotobuf -lsqlite3 -lpthread -lmuduo_net -lmuduo_base -lz
.PHONY:all
all: publish_client consume_client client
publish_client: publish_client.cc ../mqcommon/*.cc ../libs/muduo/include/muduo/protoc/codec.cc
	g++ -g -std=c++11 $(CFLAG) $^ -o $@  $(LFLAG)
consume_client: consume_client.cc ../mqcommon/*.cc ../libs/muduo/include/muduo/protoc/codec.cc
	g++ -g -std=c++11 $(CFLAG) $^ -o $@  $(LFLAG)
client: client.cc ../mqcommon/*.cc ../libs/muduo/include/muduo/protoc/codec.cc
	g++ -g -std=c++11 $(CFLAG) $^ -o $@  $(LFLAG)
.PHONY:clean
clean:
	rm -f publish_client consume_client client
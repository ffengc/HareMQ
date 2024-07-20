.PHONY:all
all: server client

# server: dict_server.cc
# 	g++ -o $@ $^ -std=c++11 -I../../libs/muduo/include -L../../libs/muduo/lib -lmuduo_net -lmuduo_base -lpthread
# client: dict_client.cc
# 	g++ -o $@ $^ -std=c++11 -I../../libs/muduo/include -L../../libs/muduo/lib -lmuduo_net -lmuduo_base -lpthread
# .PHONY:clean
# clean:
# 	rm -f server client

server: proto_server.cc request.pb.cc /home/parallels/Project/HareMQ/HareMQ/libs/muduo/include/muduo/protoc/codec.cc
	g++ -o $@ $^ -std=c++11 -I../../libs/muduo/include -L../../libs/muduo/lib -lmuduo_net -lmuduo_base -lpthread -lprotobuf -lz
client: proto_client.cc request.pb.cc /home/parallels/Project/HareMQ/HareMQ/libs/muduo/include/muduo/protoc/codec.cc
	g++ -o $@ $^ -std=c++11 -I../../libs/muduo/include -L../../libs/muduo/lib -lmuduo_net -lmuduo_base -lpthread -lprotobuf -lz
.PHONY:clean
clean:
	rm -f server client
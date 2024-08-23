test: test_logger.cc ../mqcommon/*.cc
	g++ -o $@ $^ -std=c++11 -g -lgtest -lprotobuf -lsqlite3 -I../libs/muduo/include
.PHONY:clean
clean:
	rm -f test
test: map_helper_test.cc ../mqcommon/*.cc
	g++ -o $@ $^ -std=c++11 -g -lgtest -lprotobuf -lsqlite3
.PHONY:clean
clean:
	rm -f test
.PHONY:all
all: async promise packaged_task

async:async.cc
	g++ -o $@ $^ -std=c++11
promise: promise.cc
	g++ -o $@ $^ -std=c++11
packaged_task: packaged_task.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f async promise packaged_task
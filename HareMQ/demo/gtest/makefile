.PHONY:all
all: test_assert test_global test_suit

test_assert:test_ASSERT.cc
	g++ -o $@ $^ -std=c++11 -lgtest
test_global:test_GLOBAL.cc
	g++ -o $@ $^ -std=c++11 -lgtest
test_suit:test_SUIT.cc
	g++ -o $@ $^ -std=c++11 -lgtest
.PHONY:clean
clean:
	rm -f test_assert test_global test_suit
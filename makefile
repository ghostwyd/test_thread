CXX = g++
CFLAGS = -Wall -g -O0 -pthread -Wno-deprecated

all:test_thread test_epoll

test_thread:test_thread.o
	 	$(CXX) $(CFLAGS) -o test_thread test_thread.o
test_epoll:test_epoll.o
	 	$(CXX) $(CFLAGS) -o $@ $^ 

%.o: %.cpp
		$(CXX)  $(CFLAGS) -c -o $@ $^ 

.PHONY:clean
clean:
		rm -rf test_thread test_thread.o

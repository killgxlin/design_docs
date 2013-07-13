#include <sys/epoll.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>

struct mydata {
    int fd;
};

void init_socket(int socket_) {
    int opts;
    opts = fcntl(socket_, F_GETFL);
    if (opts < 0) {
	perror("fcntl, F_GETFL");
	exit(-1);
    }

    opts |= O_NONBLOCK;
    if (fcntl(socket_, F_SETFL, opts) < 0) {
	perror("fcntl, F_SETFL");
	exit(-1);
    }
}

void deal_event(int epfd_, int ctrl_, int fd_, uint32_t events_) {
    epoll_event event;
    event.events = ctrl_;
    event.data.ptr = malloc(sizeof(mydata));
    mydata* data = (mydata*)(event.data.ptr);
    data->fd = fd_;
    if (epoll_ctl(epfd_, ctrl_, fd_, &event) < 0) {
	perror("epoll ctl");
	exit(-1);
    }
}

int main() {
    int max_num = 10000;
    int epfd = epoll_create(max_num);
    if (epfd < 0) {
	perror("create epfd");
	exit(-1);
    }

    int acceptor = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptor < 0) {
	perror("create acceptor");
	exit(-1);
    }
    
    init_socket(acceptor);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(addr.sin_zero), 8);
    if (bind(acceptor, (sockaddr*)&addr, sizeof(sockaddr)) < 0) {
	perror("bind");
	exit(-1);
    }
    if (listen(acceptor, 10) < 0) {
	perror("listen");
	exit(-1);
    }

    deal_event(epfd, EPOLL_CTL_ADD, acceptor, EPOLLIN|EPOLLET);

    epoll_event events[max_num];

    while (true) {
	int num = epoll_wait(epfd, events, max_num, -1);
	for (int i=0; i<num; ++i) {
	    epoll_event* ev = events+i;
	    mydata* md = (mydata*)ev->data.ptr;

            if (md->fd == acceptor) {
		sockaddr_in client_addr;
		socklen_t sinsize = sizeof(client_addr);
	    	int newfd = accept(acceptor, (sockaddr*)&client_addr, &sinsize);
	    	if (newfd < 0) {
	    	    perror("accept newfd");
	    	    exit(-1);
	    	}

	    	init_socket(newfd);
	    	deal_event(epfd, EPOLL_CTL_ADD, newfd, EPOLLIN|EPOLLET);
            } else {
		if (ev->events | EPOLLIN) {
		}
		if (ev->events | EPOLLOUT) {
		}
	    }
	}
    }
    
    close(acceptor);
    close(epfd);
}

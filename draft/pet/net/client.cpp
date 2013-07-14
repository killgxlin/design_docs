#include "mydata.h"
struct transport_t {
    int epfd;
    int fd;
    bool connected;
    bool closed;

    sockaddr_in addr;
};

transport_t* deal_event(int epfd_, int ctrl_, int fd_, int events_, transport_t* data_) {
    
}

int main() {
    int epfd = epoll_create(100);
    if (epfd < 0) {
	perror("epoll create");
	return;
    }
    for (int i=0; i<1; ++i) {
        int conn = socket(AF_INET, SOCK_STREAM, 0);
        if (conn < 0) {
    	    perror("socket");
    	    break;
        }
        set_nonblock(conn);
        set_nodelay(conn);
    
        auto trans = deal_event(epfd, EPOLL_CTL_ADD, conn, EPOLLOUT|EPOLLET, new transport_t);
        trans->addr.sin_family = AF_INET;
        trans->addr.sin_port = htons(9999);
        if (inet_aton("127.0.0.1", &(trans->addr.sin_addr)) < 0) {
    	    perror("inet aton");
    	    break;
        }
        if (connect(conn, (sockaddr*)&(trans->addr), sizeof(trans->addr)) < 0) {
    	    if (errno != INPROGRESS) {
    	        perror("connect");
    	        break;
    	    }
        }
    }
    epoll_event events[100];
    
    while (true) {
	int num = epoll_wait(epfd, events, 100, -1);

	for (int i=0; i<num; ++i) {
	    auto ev = events+i;    
	    auto trans = ev->data.ptr;
	    
	    if (ev->events & (EPOLLERR | EPOLLHUP)) {
		trans->close();
	    } else {
		if (ev->events & EPOLLIN) {
		}
		if (ev->events & EPOLLOUT) {
		    if (!trans->connected) {
			connect(trans->fd, (sockaddr*)&(trans->addr), sizeof(trans->addr));
			if (errno == EISCONN) {
			    printf("connected");
			    trans->connected = true;
			} else {
			    perror("connect");
			}
		    } else {
			trans->
		    }
		}
	    }

	    if (trans->closed.load()) {
		shutdown(trans->fd, SHUT_RDWR);
		close(trans->fd);
		delete trans;
	    }
	}
    }

    close(epfd);
}

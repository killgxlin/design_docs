#include "dealsocket.h"
#include "mydata.h"

mydata_t* deal_event(int epfd_, int ctrl_, int fd_, uint32_t events_, mydata_t* md_) {
    epoll_event event;
    event.events = events_;
    event.data.ptr = md_;
    md_->epfd = epfd_;
    md_->fd = fd_;
    if (epoll_ctl(epfd_, ctrl_, fd_, &event) < 0) {
	perror("epoll ctl");
	exit(-1);
    }

    return md_;
}


bool network_t::init(int32_t max_conn_) {
    max_conn = max_conn_;   
    epfd = epoll_create(max_conn);
    if (epfd < 0) {
	perror("create epfd");
	return false;
    }

    events = (epoll_event*)malloc(sizeof(epoll_event) * max_conn);
    return true;
}

void network_t::destroy() {
    close(epfd);
    free(events);
}

void network_t::stop_accept() {
    close(acceptor);
}

bool network_t::start_accept(
    const uint16_t port_, 
    handler_t logon_, 
    handler_t logoff_) {

    logon = logon_;
    logoff = logoff_;

    acceptor = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptor < 0) {
	perror("create acceptor");
	return false;
    }
 
    set_nonblock(acceptor);
    set_reuseaddr(acceptor);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(addr.sin_zero), 8);
    if (bind(acceptor, (sockaddr*)&addr, sizeof(sockaddr)) < 0) {
	perror("bind");
	return false;
    }
    if (listen(acceptor, 10) < 0) {
	perror("listen");
	return false;
    }

    deal_event(epfd, EPOLL_CTL_ADD, acceptor, EPOLLIN|EPOLLET, new mydata_t);

    return true;
}

void network_t::update() {
    int num = epoll_wait(epfd, events, max_conn, -1);
	
    #pragma omp parallel for
    for (int i=0; i<num; ++i) {
        epoll_event* ev = events+i;
        mydata_t* md = (mydata_t*)ev->data.ptr;

        if (md->fd == acceptor) {
       	    sockaddr_in client_addr;
	    socklen_t sinsize = sizeof(client_addr);

	    int newfd = accept(acceptor, (sockaddr*)&client_addr, &sinsize);
	    while (newfd >= 0) {
		set_nonblock(newfd);
		set_linger(newfd, 0);
		set_nodelay(newfd);
		auto md = deal_event(epfd, EPOLL_CTL_ADD, newfd, EPOLLIN|EPOLLET, new mydata_t);
		
		logon(md);

	        newfd = accept(acceptor, (sockaddr*)&client_addr, &sinsize);
	    }

	    if (errno != EWOULDBLOCK && errno != EAGAIN) {
	       perror("accept newfd");
	       continue;
	    }
        } else {
	    if (ev->events & (EPOLLERR | EPOLLHUP)) {
	        md->close();
	    } else {
	        if (ev->events & EPOLLIN) {
	            md->deal_read();
		}
		if (ev->events & EPOLLOUT) {
		    md->deal_write();
		}
            }

	    if (md->closed.load()) {
		logoff(md);

		shutdown(md->fd, SHUT_RDWR);
		close(md->fd);
		delete md;
	    }
	}
    }
	
}


#include <set>

std::set<mydata_t*> conns;

int main() {

    network_t net;
    net.init(10000);
    net.start_accept(9999, 
	[](mydata_t* login_){
	    conns.insert(login_);
	    printf("conns %d\n", conns.size());
	    pmsg_t msg = std::make_shared<msg_t>(14);
	    for (int i=0; i<14; ++i) {
		*(msg->data()+i) = '0' + i;
	    }
	}, 
	[](mydata_t* logout_){
	    conns.erase(logout_);
	    printf("conns %d\n", conns.size());
	}
    );

    while (true) {
	net.update();

	for(auto itr = conns.begin();
	    itr != conns.end();
	    ++itr) {
	    
	    auto md = *itr;
	    auto msg = md->recv();
	    while (msg) {
		md->send(msg);
		msg = md->recv();
	    }
	}

    }

    net.stop_accept();
    net.destroy();
}

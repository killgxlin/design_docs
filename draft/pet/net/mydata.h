#ifndef MYDATA_H
#define MYDATA_H

#include <sys/epoll.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <memory>
#include <list>
#include <functional>
#include <stdint.h>
#include <errno.h>
#include <boost/atomic.hpp>
#include <boost/lockfree/spsc_queue.hpp>

using std::list;
using std::shared_ptr;
using std::vector;
using boost::atomic;

typedef vector<uint8_t> msg_t;
typedef shared_ptr<msg_t> pmsg_t;
typedef boost::lockfree::spsc_queue<pmsg_t, boost::lockfree::capacity<1024> > msg_queue_t;

struct mydata_t {
    int epfd;
    int fd;
    atomic<bool> active_send;
    atomic<bool> closed;
    
    pmsg_t recving;
    pmsg_t sending;

    int recv_pos;
    int send_pos;

    msg_queue_t send_queue;
    msg_queue_t recv_queue;

    mydata_t() {
	epfd = -1;
	fd = -1;
	closed = false;
	active_send = false;

	recving = NULL;
	sending = NULL;

	recv_pos = 0;
	send_pos = 0;

    }
    void deal_read();
    void deal_write();

    bool send(pmsg_t msg_);
    pmsg_t recv();

    void close();
};

extern mydata_t* deal_event(int epfd_, int ctrl_, int fd_, uint32_t events_, mydata_t* md_);
extern void set_nodelay(int socket_); 
extern void set_linger(int socket_, int linger_ = 0);
extern void set_reuseaddr(int socket_); 
extern void set_nonblock(int socket_); 
 
typedef std::function<void(mydata_t*)> handler_t;
struct network_t {
    bool init(int32_t max_conn_);
    void destroy();

    void update();

    void stop_accept();
    bool start_accept(
	const uint16_t port_, 
	handler_t logon_, 
	handler_t logoff_
    );

    int epfd;
    int acceptor;
    epoll_event* events;
    int32_t max_conn;
    handler_t logon;
    handler_t logoff;
};
#endif

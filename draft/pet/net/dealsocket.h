#ifndef DEALSOCKET_H
#define DEALSOCKET_H

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

extern bool set_nonblock(int socket_);
extern bool set_nodelay(int socket_);
extern bool set_linger(int socket_, int seconds_);
extern bool set_reuseaddr(int socket_);

#endif

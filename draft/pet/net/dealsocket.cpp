#include "dealsocket.h"

bool set_nodelay(int socket_) {
    int opt = 1;
    if (setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(opt)) < 0) {
	perror("set_tcpnodelay");
	return false;
    }

    return true;
}

bool set_linger(int socket_, int linger_) {
    linger opt;
    opt.l_onoff = linger_>0 ? 1 : 0;
    opt.l_linger = linger_>0 ? linger_ : 0; 
    if (setsockopt(socket_, SOL_SOCKET, SO_LINGER, (char*)&opt, sizeof(opt)) < 0) {
	perror("set_linger");
	return false;
    }

    return true;
}

bool set_reuseaddr(int socket_) {
    int opt = 1;
    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
	perror("set_reuseaddr");
	return false;
    }

    return true;
}

bool set_nonblock(int socket_) {
    int opts;
    opts = fcntl(socket_, F_GETFL);
    if (opts < 0) {
	perror("fcntl, F_GETFL");
	return false;
    }

    opts |= O_NONBLOCK;
    if (fcntl(socket_, F_SETFL, opts) < 0) {
	perror("fcntl, F_SETFL");
	return false;
    }

    return true;
}

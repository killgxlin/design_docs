#include "dealsocket.h"
#include <netinet/in.h>
#include <arpa/inet.h>

struct trans_t {
    bool init(const char* ip_, const uint16_t port_) {
	fd = -1;
	connecting = false;
	connected = false;

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_);
	if (inet_aton(ip_, &addr.sin_addr) < 0) {
	    perror("inet_aton");
	    return false;
	}

	return true;
    }

    void destroy() {
	disconnect();
    }
    void update() {
	fd_set wfds;
	fd_set rfds;
	timeval tv;

	FD_ZERO(&wfds);
	FD_ZERO(&rfds);
	FD_SET(fd, &wfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 300*1000;

	int valid = select(fd+1, &rfds, &wfds, NULL, &tv);
	if (valid < 0) {
	    perror("select");
	    disconnect();
	    return;
	}
	if (valid == 0) {
	    return;
	}

	if (FD_ISSET(fd, &wfds)) {
	    if (connecting) {
		int error;
		socklen_t len = sizeof(error);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
		    perror("get opt");
		    disconnect();
		    return;
		}
		connected = true;
		connecting = false;
	    }
	    if (connected) {
		// write some data
	    }
	}
	if (FD_ISSET(fd, &rfds)) {
	    if (connected) {
		// read some data
	    }
	}
    }

    bool try_connect() {
	if (fd >= 0 || connecting || connected) {
	    return false;
	}
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
	    perror("socket");
	    return false;
	}
	set_nonblock(fd);
	set_nodelay(fd);

	if (connect(fd, (sockaddr*)&addr, sizeof(addr)) == 0) {
	    connected = true;
	    connecting = false;

	    return true;
	}
	if (errno == EINPROGRESS) {
	    connected = false;
	    connecting = true;

	    return true;
	} else if (errno == EISCONN) {
	    connected = true;
	    connecting = false;

	    return true;
	} else if (errno == EALREADY) {
	    connected = false;
	    connecting = true;

	    return true;
	} else {
	    connected = false;
	    connecting = false;

	    close(fd);
	    fd = -1;
	    
	    return false;
	}
    }
    bool is_connecting() { return connecting; }
    bool is_connected() { return connected; }
    void disconnect() {
	if (fd >= 0) {
	    shutdown(fd, SHUT_RDWR);
	    close(fd);
	}
	connected = false;
	connecting = false;
    }

    bool send(pmsg_t msg_);
    pmsg_t recv();

    sockaddr_in address;
    bool connecting;
    bool connected;
    int fd;

};

struct circ_buff {
    uint32_t write_alloc(uint32_t len_);
    uint32_t write_avalible();
    uint32_t write(const char* buffer_, uint32_t len_);
    
    uint32_t read_avaliable();
    uint32_t read(char* buffer_, uint32_t len_);

};

int main() {
    trans_t trans;
    if (trans.init("127.0.0.1", 9999)) {
	perror("init error");
	return 1;
    }
    
    do {
	if (!trans.is_connecting())
	    if (!trans.try_connect()) {
		perror("try error");
		return 1;
	    }
    } while (!trans.is_connected());

    int stage = 0;
    pmsg_t msg_gen = NULL
    while (trans.is_connected()) {
	switch(stage) {
	    case 0:
		static const uint32_t max = 1024*512;
		static const uint32_t min = 1;
		uint32_t size = rand() % (max - min) + min;
		msg_gen = std::make_shared<msg_t>(size + 4);
		*(uint32_t*)msg_gen ->data() = size;
		for (int i=0; i<size; ++i)
		    msg_gen->data()[4+i] = i;
		if (!trans.send(msg_gen)) {
		    trans.disconnect();
		    perror("send false");
		    break;
		}
		stage = 1;
		break;
	    case 1:
		pmsg_t msg = trans.recv();
		if (msg) {
		    uint32_t recv_len = *(uint32_t*)msg->data();
		    uint32_t hold_len = *(uint32_t*)msg_gen->data();
		    if (recv_len != hold_len) {
			trans.disconnect();
			perror("len error");
			break;
		    }
		    auto recv = msg->data()+4;
		    auto hold = msg_gen->data()+4;
		    uint32_t i=0;
		    for (; i<hold_len; ++i) {
			if [recv[i] != hold[i]) {
			    trans.disconnect();
			    perror("len error");
			    break;
			}
		    }
		    if (i != hold_len)
			break;

		    stage = 0;
		}
		break;
	    default:
		perror("error stage");
		break;
	}
    }

    trans.destroy();
}

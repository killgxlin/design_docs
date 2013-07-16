#include "dealsocket.h"
#include "mydata.h"

void mydata_t::close() {
    closed.store(true);
}

bool mydata_t::send(pmsg_t msg_) {
    if (closed.load())
	return false;

    if (!send_queue.push(msg_))
	return false;

    bool f = false;
    if (active_send.compare_exchange_strong(f, true)) {
	deal_event(epfd, EPOLL_CTL_MOD, fd, EPOLLIN|EPOLLOUT|EPOLLET, this);
    }

    return true;
}

pmsg_t mydata_t::recv() {
    if (closed.load())
	return false;

    pmsg_t msg = NULL;
    recv_queue.pop(msg);

    return msg;
}

void mydata_t::deal_write() {
    if (closed.load())
	return;

    if (sending == NULL) {
	if (!send_queue.pop(sending)) {
	    deal_event(epfd, EPOLL_CTL_MOD, fd, EPOLLIN|EPOLLET, this);
	    active_send.store(false);
	    return;
	}
	send_pos = 0;
    }
    if (send_pos < sending->size()) {
	int need = sending->size() - send_pos;
	int len = write(fd, sending->data()+send_pos, need);
	if (len > 0) {
	    need -= len;
	    send_pos += len;
	    if (need > 0) {
		return;
	    } else if (need == 0) {
		sending.reset();
		deal_write();
		return;
	    } else {
		close();
		return;
	    }
	} else if (len == 0) {
	    close();
	    return;
	} else {
	    if (errno != EAGAIN && errno != EWOULDBLOCK) {
		close();
	    }
	    return;
	}
    }
}
void mydata_t::deal_read() {
    if (closed.load())
	return;

    if (recving == NULL) {
	recving = std::make_shared<msg_t>(4);
	recv_pos = 0;
    }
    if (recv_pos < 4) {
	int need = 4 - recv_pos;
	int len = read(fd, recving->data()+recv_pos, need);
	if (len > 0) {
	    need -= len;
	    recv_pos += len;
	    if (need > 0) {
		return;
	    } else if (need == 0) {
		uint32_t msg_len = *(uint32_t*)recving->data();
		msg_len = 10;
		if (msg_len >= 0xffff) {
		    close();
		    return;
		}
		recving->resize(msg_len + 4);
	    } else {
		close();
		return;
	    }
	} else if (len == 0) {
	    close();
	    return;
	} else {
	    if (errno != EAGAIN && errno != EWOULDBLOCK) {
		close();
	    }
	    return;
	}
    }
    if (recv_pos >= 4) {
	int need = recving->size() - recv_pos;
	int len = read(fd, recving->data()+recv_pos, need);
	if (len > 0) {
	    need -= len;
	    recv_pos += len;
	    if (need > 0) {
		return;
	    } else if (need == 0) {
		if (!recv_queue.push(recving)) {
		    close();
		    return;
		}
		recving.reset();
		deal_read();
	    } else {
		close();
		return;
	    }
	} else if (len == 0) {
	    close();
	    return;
	} else {
	    if (errno != EAGAIN && errno != EWOULDBLOCK) {
		close();
	    }
	    return;
	}
    }
}

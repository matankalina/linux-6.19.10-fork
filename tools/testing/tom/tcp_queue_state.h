#ifndef TCP_QUEUE_STATE_H
#define TCP_QUEUE_STATE_H

#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#ifndef TCP_QUEUE_STATE
#define TCP_QUEUE_STATE 100
#endif

struct tcp_queue_state_snapshot {
	uint64_t unacked_time_ns, unacked_integral, unacked_total;
	int64_t  unacked_size;
	uint64_t unread_time_ns, unread_integral, unread_total;
	int64_t  unread_size;
	uint64_t ackdelay_time_ns, ackdelay_integral, ackdelay_total;
	int64_t  ackdelay_size;
};

struct queue_avgs {
	double avg_size;
	double throughput;
	double latency_ns;
};

static inline int get_queue_state(int sockfd, struct tcp_queue_state_snapshot *snap)
{
	socklen_t len = sizeof(*snap);
	memset(snap, 0, sizeof(*snap));
	return getsockopt(sockfd, IPPROTO_TCP, TCP_QUEUE_STATE, snap, &len);
}

static inline struct queue_avgs get_avgs(
	uint64_t prev_integral, uint64_t prev_total, uint64_t prev_time_ns,
	uint64_t now_integral,  uint64_t now_total,  uint64_t now_time_ns)
{
	struct queue_avgs a = {0, 0, 0};
	double dt = (double)(now_time_ns - prev_time_ns);
	if (dt <= 0)
		return a;
	a.avg_size   = (double)(now_integral - prev_integral) / dt;
	a.throughput = (double)(now_total - prev_total) / dt;
	if (a.throughput > 0)
		a.latency_ns = a.avg_size / a.throughput;
	return a;
}

#endif

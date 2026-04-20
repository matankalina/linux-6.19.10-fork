# TCP Receive Logging Format

This document defines the logging format used by the TCP receive instrumentation.

The purpose of the logging system is to record how the TCP stack processes each received payload segment and how the internal buffering state changes as a result.

The logs make it possible to reconstruct the behavior of the TCP receive pipeline during execution.

---

# Logging Structure

Each event in the receive pipeline generates a structured log entry.

The structure of the log entry is shown below.

```
[TCP_RX_TRACE]
event=<event_name>
sk_pointer=<socket_pointer>
skb_sequence_start=<sequence_number>
skb_sequence_end=<sequence_number>
skb_rcv_payload=<payload_bytes>
readable_queue_bytes_before=<value>
readable_queue_bytes_after=<value>
ofo_queue_bytes_before=<value>
ofo_queue_bytes_after=<value>
total_unread_bytes_before=<value>
total_unread_bytes_after=<value>
note=<optional_description>
```

Each field describes either the received segment or the state of the TCP buffering structures before and after the event.

---

# Field Definitions

## event

The name of the receive event that occurred.

Examples include

• rx_inorder  
• inorder_queue  
• inorder_coalesce  
• ofo_insert  
• ofo_coalesce  
• ofo_overlap_drop  
• ofo_promote_queue  

The event identifies how the TCP stack handled the segment.

---

## sk_pointer

Kernel pointer to the struct sock representing the TCP connection.

This value allows grouping log entries belonging to the same connection.

---

## skb_sequence_start

Sequence number of the first byte carried by the received segment.

This value corresponds to

```
TCP_SKB_CB(skb)->seq
```

---

## skb_sequence_end

Sequence number immediately after the last byte carried by the segment.

This value corresponds to

```
TCP_SKB_CB(skb)->end_seq
```

---

## skb_rcv_payload

Number of payload bytes contained in the received segment.

This value is computed as

```
skb_sequence_end - skb_sequence_start
```

It represents the total payload that arrived in the segment before duplicate filtering.

---

## readable_queue_bytes_before

Total payload bytes stored in the readable receive queue before the event.

Readable queue

```
sk->sk_receive_queue
```

---

## readable_queue_bytes_after

Total payload bytes stored in the readable receive queue after processing the event.

This value reflects how the readable buffer changed.

---

## ofo_queue_bytes_before

Total payload bytes stored in the out-of-order queue before the event.

Out-of-order queue

```
tp->out_of_order_queue
```

---

## ofo_queue_bytes_after

Total payload bytes stored in the out-of-order queue after the event.

This value reflects insertions, merges, or removals inside the OFO tree.

---

## total_unread_bytes_before

Total unread payload stored inside TCP before the event.

This value is defined as

```
total_unread_bytes =
    readable_queue_bytes +
    ofo_queue_bytes
```

---

## total_unread_bytes_after

Total unread payload stored inside TCP after the event.

This value should equal the combined payload stored in the readable and OFO queues.

---

## note

Optional descriptive text used to provide additional context.

Typical values include

• coalesce  
• duplicate  
• partial_overlap  
• replacement  
• promotion  

---

# Example Log Entry

Below is an example of a log entry generated when a segment is appended to the readable queue.

```
[TCP_RX_TRACE]
event=inorder_queue
sk_pointer=ffff888103c2a000
skb_sequence_start=1040
skb_sequence_end=1140
skb_rcv_payload=100
readable_queue_bytes_before=300
readable_queue_bytes_after=400
ofo_queue_bytes_before=0
ofo_queue_bytes_after=0
total_unread_bytes_before=300
total_unread_bytes_after=400
note=new_readable_segment
```

Interpretation of the example

A TCP segment carrying 100 payload bytes was received.

The segment was in-order relative to the current receive window.

The payload was appended to the readable receive queue.

The total unread buffered payload increased from 300 bytes to 400 bytes.
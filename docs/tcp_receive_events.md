
Content:

```markdown
# TCP Receive Events

This document defines the events logged by the receive-side instrumentation.

Each event represents how the TCP stack handled a received payload segment.

---

# Receive Classification Events

These occur inside `tcp_data_queue()`.

## rx_inorder

Condition:

The segment begins exactly at `rcv_nxt`.

Meaning:

The segment is the next expected byte in the TCP stream.

Effect:

The segment is accepted immediately and processed by `tcp_queue_rcv()`.

State Change:

Payload bytes become readable.

---

## rx_old_data_drop

Condition:

end_seq <= rcv_nxt

Meaning:

The segment contains only bytes that were already received earlier.

Typical causes:

- retransmission
- duplicate segment
- delayed packet

Effect:

Segment is dropped.

State Change:

No change to unread buffered bytes.

---

## rx_partial_overlap

Condition:

seq < rcv_nxt < end_seq

Meaning:

Part of the segment overlaps previously received data while the remainder is new.

Effect:

Duplicate bytes are ignored.

Only the new suffix is queued.

State Change:

Readable bytes increase only by the new suffix length.

---

## rx_ofo_dispatch

Condition:

seq > rcv_nxt

Meaning:

The segment arrived before missing earlier bytes.

Effect:

Segment is passed to `tcp_data_queue_ofo()`.

State Change:

Segment stored in the Out-of-order queue.

---

# In-order Buffering Events

These occur inside `tcp_queue_rcv()`.

## inorder_queue

Condition:

The received skb cannot be merged with the last skb in the receive queue.

Effect:

A new skb is appended to `sk_receive_queue`.

State Change:

Readable bytes increase.

---

## inorder_coalesce

Condition:

`tcp_try_coalesce()` succeeds.

Meaning:

Payload is merged into the previous skb.

Effect:

Queue length increases without creating a new skb.

State Change:

Readable bytes increase.

---

# Out-of-order Queue Events

These occur inside `tcp_data_queue_ofo()`.

## ofo_insert

Condition:

The segment does not overlap with existing OFO entries.

Effect:

Segment inserted as a new node in the OFO RB-tree.

State Change:

OFO bytes increase.

---

## ofo_coalesce

Condition:

Segment is adjacent to an existing OFO skb.

Effect:

Payload merged with that skb.

State Change:

OFO bytes increase.

---

## ofo_overlap_drop

Condition:

Segment is fully covered by existing OFO data.

Effect:

Segment discarded.

State Change:

None.

---

## ofo_replace

Condition:

New segment covers a larger range than an existing OFO entry.

Effect:

Existing node replaced and neighbors may be removed.

State Change:

May increase or remain constant depending on overlap.

---

# OFO Promotion Events

These occur inside `tcp_ofo_queue()`.

## ofo_promote_queue

Condition:

OFO segment becomes contiguous with `rcv_nxt`.

Effect:

Segment moved into the readable queue.

State Change:

Readable bytes increase.

OFO bytes decrease.

Total unread bytes remain unchanged.

---

## ofo_promote_coalesce

Condition:

Promoted OFO segment can be merged with the receive queue tail.

Effect:

Payload merges into the existing readable skb.

State Change:

Readable bytes increase.

OFO bytes decrease.

Total unread bytes remain unchanged.
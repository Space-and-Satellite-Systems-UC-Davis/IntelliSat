/******************************************************************************
* File:             pcp.c
*
* Author:           Eric Xu  
* Created:          2025-01-26 20:54
* Description:      See pcp.h
*****************************************************************************/

#include "UART/pcp.h"
#include <stdio.h>

//// Forward declarations not part of the packet interface
static void update_rx(PCPDevice* dev);
static void set_rx_waiting(PCPDevice* dev);

// Constants
const uint8_t PACKET_START = '{';
const uint8_t PACKET_END = '}';
const uint8_t ACK_START = '<';
const uint8_t ACK_END = '>';
const uint8_t ESCAPE = '\\';

/**
 * Return the distance from `before` to `after`
 */
size_t distance(SeqNum before, SeqNum after) {
    if (after > before)
        return after - before;
    else
        return 255 - before + after;
}

static void append(PCPBuf* buf, uint8_t* data, int nbytes) {
    memcpy(buf->data, data, nbytes);
    buf->len += nbytes;
}

int make_pcpdev(PCPDevice* out, USART_TypeDef *bus) {
    /* TODO: timeout is made slow for testing <02-02-25, Eric Xu> */
    return make_pcpdev_advanced(out, bus, 1000, 128 - PCP_HEAD_NBYTES,
                              128 - PCP_HEAD_NBYTES, 5);
}

int make_pcpdev_advanced(PCPDevice* out,
         USART_TypeDef *bus,
         int timeout_ms,
         int outgoing_payload_maxbytes,
         int incoming_payload_maxbytes,
         int window_size) {
    // Window size < max(sequence number) / 2
    if (window_size > 128) {
        return -E_INVALID;
    }
    PCPBuf* tx_bufs = calloc(window_size, sizeof(PCPBuf));
    for (int i = 0; i < window_size; ++i) {
        tx_bufs[i].data = calloc(2 * (outgoing_payload_maxbytes + PCP_HEAD_NBYTES),
                                 sizeof(uint8_t));
        tx_bufs[i].len = 0;
    }
    PCPBuf* rx_bufs = calloc(RX_BUFSIZ, sizeof(PCPBuf));
    for (int i = 0; i < RX_BUFSIZ; ++i) {
        rx_bufs[i].data = calloc(2 * incoming_payload_maxbytes, sizeof(uint8_t));
        rx_bufs[i].len = 0;
    }
    PCPDevice dev = (PCPDevice){
        .bus = bus,
        .timeout_ms = timeout_ms,
        .outgoing_payload_maxbytes = outgoing_payload_maxbytes,
        .incoming_payload_maxbytes = incoming_payload_maxbytes,
        .window_size = window_size,
        .tx_old_seq = 0,
        .curr_window_sz = 0,
        .last_tx_time = 0,
        .tx_bufs = tx_bufs,
        .rx_bufs = rx_bufs,
        .rx_head = 0,
        .rx_tail = 0,
        .rx_full = false,
        .rx_tail_seq = 0,
        .rx_curr_seq = 0,
        .rx_readnbytes = 0,
        .rx_escaping = false,
        .rx_received = 0,
    };
    memcpy(out, &dev, sizeof(PCPDevice));
    return 0;
}

void del_pcpdev_members(PCPDevice *dev) {
    for (int i = 0; i < dev->window_size; ++i) {
        free(dev->tx_bufs[i].data);
    }
    free(dev->tx_bufs);
    for (int i = 0; i < RX_BUFSIZ; ++i) {
        free(dev->rx_bufs[i].data);
    }
    free(dev->rx_bufs);
}

int pcp_transmit(PCPDevice *dev, uint8_t *payload, int nbytes) {
    // PACKET_START SEQ_NUM <payload> PACKET_END
    if (dev->curr_window_sz >= dev->window_size)
        return -E_OVERFLOW;
    SeqNum seq = dev->tx_old_seq + dev->curr_window_sz;
    PCPBuf* tx_buf = dev->tx_bufs + (seq % dev->window_size);

    tx_buf->data[tx_buf->len++] = PACKET_START;
    tx_buf->data[tx_buf->len++] = seq;
    for (int i = 0; i < nbytes; i++) {
        if (payload[i] == PACKET_END || payload[i] == ESCAPE)
            tx_buf->data[tx_buf->len++] = ESCAPE;
        tx_buf->data[tx_buf->len++] = payload[i];
    }
    tx_buf->data[tx_buf->len++] = PACKET_END;

    dev->last_tx_time = getSysTime();
    dev->curr_window_sz++;
    usart_transmitBytes(dev->bus, tx_buf->data, tx_buf->len);
    return true;
}

void pcp_retransmit(PCPDevice* dev) {
    update_rx(dev);
    if (dev->curr_window_sz > 0
            && getSysTime() - dev->last_tx_time > (uint64_t)dev->timeout_ms) {
        dev->last_tx_time = getSysTime();
        PCPBuf* tx_buf = dev->tx_bufs + (dev->tx_old_seq % dev->window_size);
        usart_transmitBytes(dev->bus,
                       tx_buf->data,
                       tx_buf->len);
    }
}

int pcp_receive(PCPDevice* dev, uint8_t* buf) {
    update_rx(dev);
    if (!dev->rx_full && dev->rx_head == dev->rx_tail)
        return -1;
    PCPBuf rx_buf = dev->rx_bufs[dev->rx_head];
    memcpy(buf, rx_buf.data, rx_buf.len);
    int ret = rx_buf.len;
    rx_buf.len = 0;
    dev->rx_head = (dev->rx_head + 1) % RX_BUFSIZ;
    dev->rx_full = false;
    return ret;
}

/**
 * Return true if packet with `seq` is received as indicated by `rx_received`.
 * False otherwise. Assume `seq` is within window range.
 */
static bool received(PCPDevice* dev, SeqNum seq) {
    return dev->rx_received << distance(dev->rx_tail_seq, seq) & 1;
}

/**
 * Set bit for packet with `seq`. Assume `seq` is within window range.
 */
static void set_received(PCPDevice* dev, SeqNum seq, bool val) {
    int mask = 1 << distance(dev->rx_tail_seq, seq);
    if (val)
        dev->rx_received |= mask;
    else
        dev->rx_received = ~(~dev->rx_received | mask);
}

static void acknowledge(PCPDevice *dev, SeqNum seq) {
    uint8_t buf[3] = {ACK_START, seq, ACK_END};
    usart_transmitBytes(dev->bus, buf, 3);
}

/**
 * Flush `dev->bus`'s receive buffer into `dev`. This also transmits and
 * receives acknowledgements.
 */
static void update_rx(PCPDevice* dev) {
    if (dev->rx_full)
        return;

    // A bit less than 100 ms per character received
    uint64_t timeout = getSysTime() + 500;
    uint8_t read_buf;
    while (true) {
        if (getSysTime() > timeout || !usart_receiveBufferNotEmpty(dev->bus)) {
            break;
        }

        if (usart_receiveBytes(dev->bus, &read_buf, 1) == 0) continue;

        // New packet, identify type
        if (dev->rx_readnbytes == 0) {
            if (read_buf == PACKET_START) {
                dev->rx_readnbytes++;
            }
            if (read_buf == ACK_START) {
                dev->rx_readnbytes++;
                dev->rx_acknowledging = true;
            }
            continue;
        }
        // read_buf is the sequence number
        if (dev->rx_readnbytes == 1) {
            if (dev->rx_acknowledging ||
                distance(read_buf, dev->rx_tail_seq) <= dev->window_size) {
                dev->rx_readnbytes++;
                dev->rx_curr_seq = read_buf;
            } else {
                set_rx_waiting(dev);
            }
            continue;
        }
        // Packet is acknowledgement
        if (dev->rx_acknowledging) {
            if (read_buf != ACK_END) {
                // ignore malformed acknowledgement
            } else if (dev->rx_curr_seq == dev->tx_old_seq) {
                dev->tx_old_seq += 1 + dev->rx_num_acknowledging;
                dev->curr_window_sz -= 1 + dev->rx_num_acknowledging;
                dev->rx_num_acknowledging = 0;
            } else if (distance(dev->tx_old_seq, dev->rx_curr_seq) <
                       dev->curr_window_sz) {
                dev->rx_num_acknowledging++;
            }
            set_rx_waiting(dev);
            continue;
        }
        // Packet is transmission
        const int offset =
            (distance(dev->rx_tail_seq, dev->rx_curr_seq) + dev->rx_tail) %
            RX_BUFSIZ;
        PCPBuf* write_buf = dev->rx_bufs + offset;
        // read_buf is escaped
        if (dev->rx_escaping) {
            if (read_buf != ESCAPE && read_buf != PACKET_END)
                dev->rx_readnbytes = 0;
            else
                append(write_buf, &read_buf, 1);
            continue;
        // Next read is escaping
        } else if (read_buf == ESCAPE) {
            dev->rx_escaping = true;
            continue;
        }
        if (read_buf == PACKET_END) {
            set_received(dev, dev->rx_curr_seq, true);
            if (dev->rx_curr_seq == dev->rx_tail_seq) {
                while (dev->rx_received & 1) {
                    dev->rx_tail_seq++;
                    dev->rx_received >>= 1;
                    dev->rx_tail++;
                    dev->rx_tail = (dev->rx_tail + 1) % RX_BUFSIZ;
                }
            }
            acknowledge(dev, dev->rx_tail_seq);
            set_rx_waiting(dev);
        }
        append(write_buf, &read_buf, 1);
	}
}

/**
* Set RX state to listening for new packets.
*/
static void set_rx_waiting(PCPDevice* dev) {
    dev->rx_acknowledging = false;
    dev->rx_readnbytes = 0;
}

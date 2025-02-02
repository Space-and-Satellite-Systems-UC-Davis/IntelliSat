/******************************************************************************
* File:             pcp.c
*
* Author:           Eric Xu  
* Created:          2025-01-26 20:54
* Description:      See pcp.h
*****************************************************************************/

#include "UART/pcp.h"

//// Forward declarations not part of the packet interface
static void update_rx(PCPDevice* dev);
static void transmit_bytes(USART_TypeDef *bus, uint8_t message[], int nbytes);

// Constants
const uint8_t PACKET_START = '{';
const uint8_t PACKET_END = '}';
const uint8_t ACK_START = '<';
const uint8_t ACK_END = '>';
const uint8_t ESCAPE = '\\';

/**
 * Type for sequence number. Use `distance()` for subtraction and `+` for
 * addition.
 */
typedef uint8_t SeqNum;

/**
 * Return the distance from `before` to `after`
 */
int distance(SeqNum before, SeqNum after) {
    if (after > before)
        return after - before;
    else
        return 255 - before + after;
}

typedef struct {
    uint8_t* data;
    size_t len;
} PCPBuf;

static void append(PCPBuf* buf, uint8_t* data, int nbytes) {
    memcpy(buf->data, data, nbytes);
    buf->len += nbytes;
}

/**
 * Contains the configurations and state of a PCP device, used by PCP functions.
 *
 * Use `pcp.h` functions to create, delete, and interact with this object
 * instead of directly manipulating it.
 *
 * Peripheral control protocol (PCP) is a transmission protocol built above UART
 * that allows the sending and receiving of packets.
 */
struct PCPDevice {
    // Device configurations
    /** An initialized bus */
    USART_TypeDef *const bus;
    /** Milliseconds without acknowledgement until retransmission */
    const int timeout_ms;
    /** Must be at least 1 */
    const int outgoing_payload_maxbytes;
    /** Must be at least 1 */
    const int incoming_payload_maxbytes;
    /** Maximum number of packets being transmitted or received concurrently */
    const size_t window_size;

    // Device state

    // TX
    /** The sequence number of the oldest unacknowledged packet */
    SeqNum tx_old_seq;
    /** Number of packets in transmission */
    size_t curr_window_sz;
    /** The system time of the last transmission */
    uint64_t last_tx_time;
    /**
     * `window_size` number of transmission buffers, indexable by sequence
     * number % window size. Each buffer stores full packets.
     */
    PCPBuf* tx_bufs;

    // RX
    /**
     * `RX_BUFSIZ` number of receive buffers, circular queue with [rx_head] and
     * [rx_tail]. Each buffer stores payload of received messages.
     */
    PCPBuf* rx_bufs;
    int rx_head;
    /** Exclusive */
    int rx_tail;
    bool rx_full;
    /** Sequence number of packet rx_bufs[rx_tail] */
    SeqNum rx_tail_seq;
    /** Sequence number of the current packet being read */
    SeqNum rx_curr_seq;
    /**
     * The number of bytes read of the current packet; Used for processing
     * header.
     */
    int rx_readnbytes;
    /** True if the next character is escaped */
    bool rx_escaping;
    /**
     * Bitmap indicating whether a packet has been received. Used by received().
     * The least significant bit indicates status of `rx_tail_seq`.
     */
    int rx_received;
};

/**
 * Creates a `PCPDevice`. See documentation on `struct PCPDevice`.
 *
 * @returns 0 if successful, -E_INVALID if parameters are invalid
 */
int make_pcpdev(PCPDevice* out,
         USART_TypeDef *bus) {
  return make_pcpdev_advanced(out, bus, 200, 128 - PCP_HEAD_NBYTES,
                              128 - PCP_HEAD_NBYTES, 5);
}

/**
 * Creates a `PCPDevice`. See documentation on `struct PCPDevice`.
 *
 * @returns 0 if successful, -E_INVALID if parameters are invalid
 */
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

/**
 * Transmit a message. Return 0 if packet is queued for transmission,
 * -E_OVERFLOW if otherwise.
 *
 * @param dev
 * @param payload
 * @param nbytes
 */
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
    transmit_bytes(dev->bus, tx_buf->data, tx_buf->len);
    return true;
}

/**
 * Retransmit requests that timed out.
 */
void pcp_retransmit(PCPDevice* dev) {
    update_rx(dev);
    if (dev->curr_window_sz > 0
            && getSysTime() - dev->last_tx_time > dev->timeout_ms) {
        dev->last_tx_time = getSysTime();
        transmit_bytes(dev->bus,
                       dev->tx_bufs[dev->tx_old_seq].data,
                       dev->tx_bufs[dev->tx_old_seq].len);
    }
}

/**
 * Read a response into `buf`. Assume that `buf` is large enough
 * (PAYLOAD_MAXBYTES). Returns size of recvonse if successfully read, -1 if
 * otherwise.
 */
int pcp_receive(PCPDevice* dev, uint8_t* buf) {
    update_rx(dev);
    if (!dev->rx_full && dev->rx_head == dev->rx_tail)
        return -1;
    PCPBuf rx_buf = dev->rx_bufs[dev->rx_head];
    memcpy(buf, rx_buf.data, rx_buf.len);
    int ret = rx_buf.len;
    rx_buf.len = 0;
    dev->rx_head++;
    dev->rx_full = false;
    return ret;
}

/**
 * Return true if packet with `seq` is received as indicated by `rx_received`.
 * False otherwise. Assume `seq` is within window range.
 */
bool received(PCPDevice* dev, SeqNum seq) {
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
    transmit_bytes(dev->bus, buf, 3);
}

/**
 * Flush `dev->bus`'s receive buffer into `dev`. This also transmits and
 * receives acknowledgements.
 */
static void update_rx(PCPDevice* dev) {
    if (dev->rx_full)
        return;

    uint64_t timeout = getSysTime() + 10;
    uint8_t read_buf;
    while (true) {
        if (getSysTime() > timeout) {
            break;
        }

        int read_nbytes = usart_recieveBytes(dev->bus, &read_buf, 1);

        // Nothing
        if (read_nbytes == 0)
            continue;
        // New packet
        if (dev->rx_readnbytes == 0) {
            if (read_buf == PACKET_START)
                dev->rx_readnbytes++;
            continue;
        }
        // read_buf is the sequence number
        if (dev->rx_readnbytes == 1) {
            if (distance(read_buf, dev->rx_tail_seq) > dev->window_size)
                dev->rx_readnbytes = 0;
            else {
                dev->rx_readnbytes++;
                dev->rx_curr_seq = read_buf;
            }
            continue;
        }

        const int offset = distance(dev->rx_tail_seq, dev->rx_curr_seq);
        PCPBuf* write_buf = dev->rx_bufs + dev->rx_tail + offset;
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
                }
            }
            acknowledge(dev, dev->rx_tail_seq);
        }

        append(write_buf, &read_buf, 1);
	}
}

/**
 * Transmit `nbytes` from `message` to MGT. References usart_transmitBytes
 */
static void transmit_bytes(USART_TypeDef *bus, uint8_t message[], int nbytes) {
	// Enable UART3 and Transmitter
	bus->CR1 |= USART_CR1_UE | USART_CR1_TE;

	// Transfer each character one at a time
	for (int i = 0; i < nbytes; i++){
		// wait until Data register is empty
		while (!(bus->ISR & USART_ISR_TXE));
		// Place the character in the Data Register
		bus->TDR = message[i];
	}

	// Wait for the Transfer to be completed by monitoring the TC flag
	while(!(bus->ISR & USART_ISR_TC));
}

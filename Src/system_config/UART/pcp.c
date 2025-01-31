/******************************************************************************
* File:             pcp.c
*
* Author:           Eric Xu  
* Created:          2025-01-26 20:54
* Description:      See pcp.h
*****************************************************************************/

#include "UART/pcp.h"

//// Forward declarations not part of the packet interface
void update_rx(PCPDevice* dev);
void update_recv(PCPDevice* dev);
void transmit_bytes(USART_TypeDef *bus, uint8_t message[], int nbytes);

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
typedef struct {
    uint8_t* data;
    size_t len;
} PCPBuf;

void append(PCPBuf* buf, uint8_t* data, int nbytes) {
    memcpy(buf->data, data, nbytes);
    buf->len += nbytes;
}

struct PCPDevice {
    // Configuration
    /** An initialized bus */
    USART_TypeDef *const bus;
    /** Milliseconds without acknowledgement until retransmission */
    const int timeout_ms;
    /** Must be at least 1 */
    const int outgoing_payload_maxbytes;
    /** Must be at least 1 */
    const int incoming_payload_maxbytes;
    /** Maximum number of packets being transmitted or received concurrently */
    const int window_size;

    // State

    // TX
    /** The sequence number of the oldest unacknowledged packet */
    SeqNum tx_old_seq;
    /** Number of packets in transmission */
    size_t curr_window_sz;
    /** The system time of the last transmission */
    uint64_t last_tx_time;
    /** Transmission buffers, indexable by sequence number */
    PCPBuf* tx_bufs;

    // RX
    /** Receive circular buffer */
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
    //bool req_sent;
    //uint8_t *req_buf;
    //int req_nbytes;
    //uint8_t *recv_buf;
    //int rx_readnbytes;
    //bool received_recv;
};

/**
 * Creates a `PCPDevice`. See documentation on `struct PCPDevice`.
 *
 * @returns 0 if successful, -E_INVALID if parameters are invalid
 */
int make_pcpdevice(PCPDevice* out,
                   USART_TypeDef *bus,
                   int timeout_ms,
                   int outgoing_payload_maxbytes,
                   int incoming_payload_maxbytes,
                   int window_size) {
    // Window size < max(sequence number) / 2
    if (window_size > 128) {
        return -E_INVALID;
    }
    PCPDevice dev = (PCPDevice){
        .bus = bus,
        .timeout_ms = timeout_ms,
        .outgoing_payload_maxbytes = outgoing_payload_maxbytes,
        .incoming_payload_maxbytes = incoming_payload_maxbytes,
        .window_size = window_size,

        .tx_new_seq = 0,
        //.last_req_time = 0,
        //.req_sent = false,
        //.req_buf = calloc(2 * outgoing_payload_maxbytes, sizeof(uint8_t)),
        //.req_nbytes = 0,
        //.recv_buf = calloc(2 * incoming_payload_maxbytes, sizeof(uint8_t)),
        //.rx_readnbytes = 0,
        //.escaping = false,
        //.received_recv = false,
    };
    memcpy(out, &dev, sizeof(PCPDevice));
    return 0;
}

void delete(PCPDevice device) {
}

/**
 * Return the distance from `before` to `after`
 */
int distance(SeqNum before, SeqNum after) {
    if (after > before)
        return after - before;
    else
        return 255 - before + after;
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
bool set_received(PCPDevice* dev, SeqNum seq, bool val) {
    int mask = 1 << distance(dev->rx_tail_seq, seq);
    if (val)
        dev->rx_received |= mask;
    else
        dev->rx_received = ~(~dev->rx_received | mask);
}

/**
 * Transmit a message. Return 0 if packet is queued for transmission,
 * -E_OVERFLOW if otherwise.
 *
 * @param dev
 * @param payload
 * @param nbytes
 */
int transmit(PCPDevice *dev, uint8_t *payload, int nbytes) {
    // PACKET_START SEQ_NUM <payload> PACKET_END
    if (dev->curr_window_sz >= dev->window_size)
        return -E_OVERFLOW;
    SeqNum seq = dev->tx_old_seq + dev->curr_window_sz;
    PCPBuf* tx_buf = dev->tx_bufs + seq;

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
void retransmit(PCPDevice* dev) {
    update_rx(dev);
    if (dev->curr_window_sz >= 0
            && getSysTime() - dev->last_tx_time > dev->timeout_ms) {
        dev->last_tx_time = getSysTime();
        transmit_bytes(dev->bus,
                       dev->tx_bufs[dev->tx_old_seq].data,
                       dev->tx_bufs[dev->tx_old_seq].len);
    }
}

void check_ack(PCPDevice* dev) {
    if (dev->curr_window_sz <= 0)
        return;
}

void acknowledge(PCPDevice *dev, SeqNum seq) {
    uint8_t buf[3] = {ACK_START, seq, ACK_END};
    transmit_bytes(dev->bus, buf, 3);
}

/**
 * Flush `dev->bus`'s receive buffer into `dev`. This also transmits and
 * receives acknowledgements.
 */
void update_rx(PCPDevice* dev) {
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
 * Read a response into `buf`. Assume that `buf` is large enough
 * (PAYLOAD_MAXBYTES). Returns size of recvonse if successfully read, -1 if
 * otherwise.
 */
int get_recvonse(PCPDevice* dev, uint8_t* buf) {
    update_recv(dev);
    if (!dev->received_recv)
        return -1;

    dev->received_recv = false;
    dev->tx_new_seq = !dev->tx_new_seq;
    dev->req_sent = false;
    dev->req_nbytes = 0;
    dev->rx_readnbytes = 0;

    // only copy payload
    memcpy(buf, dev->recv_buf + 2, dev->rx_readnbytes - 3);
    return dev->rx_readnbytes;
}

/**
 * Buffers recvonse without reading the recvonse.
 */
void update_recv(PCPDevice* dev) {
    if (dev->received_recv)
        return;

    uint8_t buf;
	uint64_t init_time = getSysTime();
	while (true) {
        if (getSysTime() - init_time > 10) {
            break;
        }
        int read_nbytes = usart_recieveBytes(dev->bus, &buf, 1);

        // Not reading if
        if (read_nbytes == 0    // nothing is read
                || (dev->rx_readnbytes == 0 && buf != PACKET_START) // no start delim
                || (dev->req_nbytes == 0)    // no request sent
            )
            continue;

        if (dev->rx_readnbytes == 1 && buf != dev->tx_new_seq) {
            dev->rx_readnbytes = 0;
            continue;
        }

        // Handle escaped characters
        if (dev->escaping) {
            if (buf != ESCAPE && buf != PACKET_END)
                dev->rx_readnbytes = 0;
            else
                dev->recv_buf[dev->rx_readnbytes++] = buf;
            continue;
        } else if (buf == ESCAPE) {
            dev->escaping = true;
            continue;
        }

        dev->recv_buf[dev->rx_readnbytes++] = buf;

        if (buf == PACKET_END)
            dev->received_recv = true;
	}
}

/**
 * Transmit `nbytes` from `message` to MGT. References usart_transmitBytes
 */
void transmit_bytes(USART_TypeDef *bus, uint8_t message[], int nbytes) {
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

/******************************************************************************
* File:             pcp.c
*
* Author:           Eric Xu  
* Created:          2025-01-26 20:54
* Description:      See pcp.h
*****************************************************************************/


#include "UART/pcp.h"

//// Forward declarations not part of the packet interface
void update_recv(PCPDevice* dev);
void transmit_bytes(USART_TypeDef *bus, uint8_t message[], int nbytes);

// Constants
const uint8_t PACKET_START = '{';
const uint8_t PACKET_END = '}';
const uint8_t ESCAPE = '\\';

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
    /** Maximum number of packets concurrently in transit */
    const int window_size;

    // State
    /** The sequence number of the oldest unacknowledged packet */
    uint8_t tx_old_seq;
    /** Number of packets in transmission */
    int curr_window_sz;
    /** The system time of the last transmission */
    uint64_t last_tx_time;
    /** Transmission buffers, indexable by sequence number */
    uint8_t** tx_bufs;
    int* tx_buf_lens;
    //bool req_sent;
    //uint8_t *req_buf;
    //int req_nbytes;
    //uint8_t *recv_buf;
    //int recv_nbytes;
    ///** True if the next character is escaped. */
    //bool escaping;
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
        //.recv_nbytes = 0,
        //.escaping = false,
        //.received_recv = false,
    };
    memcpy(out, &dev, sizeof(PCPDevice));
    return 0;
}

void delete(PCPDevice device) {
}

/**
 * Transmit a message. Return 0 if packet is queued for transmission,
 * -E_OVERFLOW if otherwise.
 *
 * @param dev
 * @param payload
 * @param nbytes
 */
int transmit(PCPDevice *dev, uint8_t payload[], int nbytes) {
    // PACKET_START SEQ_NUM <payload> PACKET_END
    if (dev->curr_window_sz >= dev->window_size)
        return -E_OVERFLOW;
    int seq = dev->tx_old_seq + dev->curr_window_sz;
    uint8_t* tx_buf = dev->tx_bufs[seq];
    int* tx_buf_len = dev->tx_buf_lens + seq;

    tx_buf[*tx_buf_len++] = PACKET_START;
    tx_buf[*tx_buf_len++] = seq;
    for (int i = 0; i < nbytes; i++) {
        if (payload[i] == PACKET_END || payload[i] == ESCAPE)
            tx_buf[*tx_buf_len++] = ESCAPE;
        tx_buf[*tx_buf_len++] = payload[i];
    }
    tx_buf[*tx_buf_len++] = PACKET_END;

    dev->last_tx_time = getSysTime();
    dev->curr_window_sz++;
    transmit_bytes(dev->bus, tx_buf, *tx_buf_len);
    return true;
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
    dev->recv_nbytes = 0;

    // only copy payload
    memcpy(buf, dev->recv_buf + 2, dev->recv_nbytes - 3);
    return dev->recv_nbytes;
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
                || (dev->recv_nbytes == 0 && buf != PACKET_START) // no start delim
                || (dev->req_nbytes == 0)    // no request sent
            )
            continue;

        if (dev->recv_nbytes == 1 && buf != dev->tx_new_seq) {
            dev->recv_nbytes = 0;
            continue;
        }

        // Handle escaped characters
        if (dev->escaping) {
            if (buf != ESCAPE && buf != PACKET_END)
                dev->recv_nbytes = 0;
            else
                dev->recv_buf[dev->recv_nbytes++] = buf;
            continue;
        } else if (buf == ESCAPE) {
            dev->escaping = true;
            continue;
        }

        dev->recv_buf[dev->recv_nbytes++] = buf;

        if (buf == PACKET_END)
            dev->received_recv = true;
	}
}

/**
 * Retransmit requests that timed out.
 */
void retransmit(PCPDevice* dev) {
    update_recv(dev);
    if (dev->req_nbytes != 0
            && !dev->received_recv
            && getSysTime() - dev->last_tx_time > dev->timeout_ms) {
        dev->last_tx_time = getSysTime();
        transmit_bytes(dev->bus, dev->req_buf, dev->req_nbytes);
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

/******************************************************************************
* File:             pcp.h
*
* Author:           Eric Xu  
* Created:          2025-01-26 20:54
* Description:      Peripheral control protocol (PCP) is a transmission protocol
*                   built above UART that allows the sending and receiving of
*                   packets. Features include
*                   - Buffered send and receive
*                   - Acknowledgement and retransmission
*
*****************************************************************************/


#ifndef SYSTEM_CONFIG_UART_PCP_H_
#define SYSTEM_CONFIG_UART_PCP_H_

#include <globals.h>
#include <stm32l476xx.h>
#include <UART/uart.h>
#include <stdarg.h>
#include <Timers/timers.h>
#include <stdlib.h>

#define E_LOST 1
#define E_INVALID 2
#define E_OVERFLOW 3

// PACKET_START, SEQ_NUM, PACKET_END
#define PCP_HEAD_NBYTES 3
// Number of packets in rx buffer
#define RX_BUFSIZ 32

/**
 * Type for sequence number. Use `distance()` for subtraction and `+` for
 * addition.
 */
typedef uint8_t SeqNum;

typedef struct {
    uint8_t* data;
    size_t len;
} PCPBuf;

/**
 * Contains the configurations and state of a PCP device, used by PCP functions.
 *
 * Use `pcp.h` functions to create, delete, and interact with this object
 * instead of directly manipulating it.
 *
 * Peripheral control protocol (PCP) is a transmission protocol built above UART
 * that allows the sending and receiving of packets.
 */
typedef struct PCPDevice {
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
} PCPDevice;

/**
 * Creates a `PCPDevice`. See documentation on `struct PCPDevice`.
 *
 * @returns 0 if successful, -E_INVALID if parameters are invalid
 */
int make_pcpdev(PCPDevice* out, USART_TypeDef *bus);

/**
 * Creates a `PCPDevice` with more finetuned configurations. For general
 * purpose, it is recommended to use [make_pcpdev] instead.
 *
 * @returns 0 if successful, -E_INVALID if parameters are invalid
 */
int make_pcpdev_advanced(PCPDevice* out,
         USART_TypeDef *bus,
         int timeout_ms,
         int outgoing_payload_maxbytes,
         int incoming_payload_maxbytes,
         int window_size);

void del_pcpdev_members(PCPDevice *dev);

/**
 * Transmit a message. Return 0 if packet is queued for transmission,
 * -E_OVERFLOW if otherwise.
 *
 * @param dev
 * @param payload
 * @param nbytes
 */
int pcp_transmit(PCPDevice *dev, uint8_t *payload, int nbytes);

/**
 * Read a response into `buf`. Assume that `buf` is large enough
 * (PAYLOAD_MAXBYTES). Returns size of recvonse if successfully read, -1 if
 * otherwise.
 */
int pcp_receive(PCPDevice* dev, uint8_t* buf);

/**
 * Retransmit requests that timed out.
 */
void pcp_retransmit(PCPDevice* dev);

#endif /* SYSTEM_CONFIG_UART_PCP_H_ */

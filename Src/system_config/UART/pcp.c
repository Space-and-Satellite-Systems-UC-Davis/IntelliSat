/******************************************************************************
* File:             mgt.c
*
* Author:           Eric Xu  
* Created:          11/03/24 
* Description:      Communicate to MGT
*****************************************************************************/


#include "UART/pcp.h"

//// Forward declarations not part of the packet interface
//void updateResp();
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

    // State
    bool seq_num;
    uint64_t last_req_time;
    bool req_sent;
    uint8_t *req_buf;
    int req_nbytes;
    uint8_t *resp_buf;
    int resp_nbytes;
    /** True if the next character is escaped. */
    bool escaping;
    bool received_resp;
};

/**
 * Creates a PCPDevice. See documentation on that.
 */
PCPDevice make_pcpdevice(USART_TypeDef *bus,
                       int timeout_ms,
                       int outgoing_payload_maxbytes,
                       int incoming_payload_maxbytes) {
    return (PCPDevice){
        .bus = bus,
        .timeout_ms = timeout_ms,
        .outgoing_payload_maxbytes = outgoing_payload_maxbytes,
        .incoming_payload_maxbytes = incoming_payload_maxbytes,

        .seq_num = 0,
        .last_req_time = 0,
        .req_sent = false,
        .req_buf = calloc(2 * outgoing_payload_maxbytes, sizeof(uint8_t)),
        .req_nbytes = 0,
        .resp_buf = calloc(2 * incoming_payload_maxbytes, sizeof(uint8_t)),
        .resp_nbytes = 0,
        .escaping = false,
        .received_resp = false,
    };
}

void delete(PCPDevice device) {
    free(device.req_buf);
    free(device.resp_buf);
}

/**
 * Send a request. Return `true` if packet is queued, `false` if otherwise.
 * Request cannot be sent if a previous response is not received via
 * `getResponse()`
 *
 * @param payload
 * @param nbytes
 * @return queued
 */
bool send(PCPDevice *dev, uint8_t payload[], int nbytes) {
    if (dev->req_nbytes != 0)
        return false;
    // PACKET_START SEQ_NUM <payload> PACKET_END
    dev->req_buf[dev->req_nbytes++] = PACKET_START;
    dev->req_buf[dev->req_nbytes++] = dev->seq_num;
    for (int i = 0; i < nbytes; i++) {
        if (payload[i] == PACKET_END || payload[i] == ESCAPE)
            dev->req_buf[dev->req_nbytes++] = ESCAPE;
        dev->req_buf[dev->req_nbytes++] = payload[i];
    }
    dev->req_buf[dev->req_nbytes++] = PACKET_END;

    dev->last_req_time = getSysTime();
    transmit_bytes(dev->bus, dev->req_buf, dev->req_nbytes);
    return true;
}

/**
 * Read a response into `buf`. Assume that `buf` is large enough
 * (PAYLOAD_MAXBYTES). Returns size of response if successfully read, -1 if
 * otherwise.
 */
int get_response(PCPDevice* dev, uint8_t* buf) {
    update_resp();
    if (!dev->received_resp)
        return -1;

    dev->received_resp = false;
    dev->seq_num = !dev->seq_num;
    dev->req_sent = false;
    dev->req_nbytes = 0;
    dev->resp_nbytes = 0;

    // only copy payload
    memcpy(buf, dev->resp_buf + 2, dev->resp_nbytes - 3);
    return dev->resp_nbytes;
}

/**
 * Buffers response without reading the response.
 */
void update_resp(PCPDevice* dev) {
    if (dev->received_resp)
        return;

    uint8_t buf;
	uint64_t initTime  = getSysTime();
	while (true) {
        if (getSysTime() - initTime > 10) {
            break;
        }
        int read_nbytes = usart_recieveBytes(dev->bus, &buf, 1);

        // Not reading if
        if (read_nbytes == 0    // nothing is read
                || (dev->resp_nbytes == 0 && buf != PACKET_START) // no start delim
                || (dev->req_nbytes == 0)    // no request sent
            )
            continue;

        if (dev->resp_nbytes == 1 && buf != dev->seq_num) {
            dev->resp_nbytes = 0;
            continue;
        }

        // Handle escaped characters
        if (dev->escaping) {
            if (buf != ESCAPE && buf != PACKET_END)
                dev->resp_nbytes = 0;
            else
                dev->resp_buf[dev->resp_nbytes++] = buf;
            continue;
        } else if (buf == ESCAPE) {
            dev->escaping = true;
            continue;
        }

        dev->resp_buf[dev->resp_nbytes++] = buf;

        if (buf == PACKET_END)
            dev->received_resp = true;
	}
}

/**
 * Retransmit requests that timed out.
 */
void retransmit(PCPDevice* dev) {
    update_resp(dev);
    if (dev->req_nbytes != 0
            && !dev->received_resp
            && getSysTime() - dev->last_req_time > dev->timeout_ms) {
        dev->last_req_time = getSysTime();
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

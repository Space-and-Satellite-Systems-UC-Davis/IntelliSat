/******************************************************************************
* File:             mgt.c
*
* Author:           Eric Xu  
* Created:          11/03/24 
* Description:      Communicate to MGT
*****************************************************************************/


#include "UART/pcp.h"

//// Forward declarations not part of the packet interface
//void mgt_updateResp();
//void mgt_transmitBytes(uint8_t message[], int nbytes);

// Constants
const uint8_t PACKET_START = '{';
const uint8_t PACKET_END = '}';
const uint8_t ESCAPE = '\\';

typedef struct {
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
} PCPDevice;

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

///**
// * Initialize MGT
// */
//void mgt_init() {
//	usart_init(MGT_USART, MGT_BAUDRATE);
//}
//
///**
// * Send a request. Return `true` if packet is queued, `false` if otherwise.
// * Request cannot be sent if a previous response is not received via
// * `mgt_getResponse()`
// *
// * @param payload
// * @param nbytes
// * @return queued
// */
//bool mgt_request(uint8_t payload[], int nbytes) {
//    if (req_nbytes != 0)
//        return false;
//    // PACKET_START SEQ_NUM <payload> PACKET_END
//    req_buf[req_nbytes++] = PACKET_START;
//    req_buf[req_nbytes++] = seq_num;
//    for (int i = 0; i < nbytes; i++) {
//        if (payload[i] == PACKET_END || payload[i] == ESCAPE)
//            req_buf[req_nbytes++] = ESCAPE;
//        req_buf[req_nbytes++] = payload[i];
//    }
//    req_buf[req_nbytes++] = PACKET_END;
//
//    last_req_time = getSysTime();
//    mgt_transmitBytes(req_buf, req_nbytes);
//    return true;
//}
//
///**
// * Read a response into `buf`. Assume that `buf` is large enough
// * (PAYLOAD_MAXBYTES). Returns size of response if successfully read, -1 if
// * otherwise.
// */
//int mgt_getResponse(uint8_t* buf) {
//    mgt_updateResp();
//    if (!received_resp)
//        return -1;
//
//    received_resp = false;
//    seq_num = !seq_num;
//    req_sent = false;
//    req_nbytes = 0;
//    resp_nbytes = 0;
//
//    // only copy payload
//    memcpy(buf, resp_buf + 2, resp_nbytes - 3);
//    return resp_nbytes;
//}
//
///**
// * Retransmit requests that timed out.
// */
//void mgt_retransmit() {
//    mgt_updateResp();
//    if (req_nbytes != 0
//            && !received_resp
//            && getSysTime() - last_req_time > MGT_ACKNOWLEDGEMENT_TIMEOUT) {
//        last_req_time = getSysTime();
//        mgt_transmitBytes(req_buf, req_nbytes);
//    }
//}
//
///**
// * Buffers response without reading the response.
// */
//void mgt_updateResp() {
//    if (received_resp)
//        return;
//
//    uint8_t buf;
//	uint64_t initTime  = getSysTime();
//	while (true) {
//        if (getSysTime() - initTime > 10) {
//            break;
//        }
//        int read_nbytes = usart_recieveBytes(MGT_USART, &buf, 1);
//
//        // Not reading if
//        if (read_nbytes == 0    // nothing is read
//                || (resp_nbytes == 0 && buf != PACKET_START) // no start delim
//                || (req_nbytes == 0)    // no request sent
//            )
//            continue;
//
//        if (resp_nbytes == 1 && buf != seq_num) {
//            resp_nbytes = 0;
//            continue;
//        }
//
//        // Handle escaped characters
//        if (escaping) {
//            if (buf != ESCAPE && buf != PACKET_END)
//                resp_nbytes = 0;
//            else
//                resp_buf[resp_nbytes++] = buf;
//            continue;
//        } else if (buf == ESCAPE) {
//            escaping = true;
//            continue;
//        }
//
//        resp_buf[resp_nbytes++] = buf;
//
//        if (buf == PACKET_END)
//            received_resp = true;
//	}
//}
//
///**
// * Transmit `nbytes` from `message` to MGT. References usart_transmitBytes
// */
//void mgt_transmitBytes(uint8_t message[], int nbytes) {
//	// Enable UART3 and Transmitter
//	MGT_USART->CR1 |= USART_CR1_UE | USART_CR1_TE;
//
//	// Transfer each character one at a time
//	for (int i = 0; i < nbytes; i++){
//		// wait until Data register is empty
//		while (!(MGT_USART->ISR & USART_ISR_TXE));
//		// Place the character in the Data Register
//		MGT_USART->TDR = message[i];
//	}
//
//	// Wait for the Transfer to be completed by monitoring the TC flag
//	while(!(MGT_USART->ISR & USART_ISR_TC));
//}

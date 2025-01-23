/******************************************************************************
* File:             mgt.c
*
* Author:           Eric Xu  
* Created:          11/03/24 
* Description:      Communicate to MGT
*****************************************************************************/


#include "MGT/mgt.h"

// Forward declarations not part of the packet interface
void mgt_updateResp();
void mgt_transmitBytes(uint8_t message[], int nbytes);

const uint8_t PACKET_START = '{';
const uint8_t PACKET_END = '}';
const uint8_t ESCAPE = '\\';
//enum mgt_op {
//    SET_PERCENT = 'S',
//    GET_CURRENT = 'C',
//    ACKNOWLEDGE = 'A',
//    SHUTDOWN = 'D',
//    STOP_TIMER = 'T',
//};
//typedef enum mgt_op mgt_op;

// Sequence number of the packet in transit
bool seq_num = 0;
uint64_t req_time = 0;
bool req_sent = false;
uint8_t req_buf[2*PACKET_MAXBYTES]; // extra memory for escape
int req_nbytes = 0;
uint8_t resp_buf[PACKET_MAXBYTES];
int resp_nbytes = 0;
// Next character will be escaped
bool escaping = false;
bool received_resp = false;

/**
 * Initialize mgt
 */
void mgt_init() {
	usart_init(MGT_USART, MGT_BAUDRATE);
}

/**
 * Send a request. Return `true` if packet is queued, `false` if otherwise.
 * Request cannot be sent if a previous response is not received via
 * `mgt_getResponse()`
 *
 * @param payload
 * @param nbytes
 * @return queued
 */
bool mgt_request(uint8_t payload[], int nbytes) {
    if (req_nbytes != 0)
        return false;
    // PACKET_START SEQ_NUM <payload> PACKET_END
    req_buf[req_nbytes++] = PACKET_START;
    req_buf[req_nbytes++] = seq_num;
    for (int i = 0; i < nbytes; i++) {
        if (payload[i] == PACKET_END || payload[i] == ESCAPE)
            req_buf[req_nbytes++] = ESCAPE;
        req_buf[req_nbytes++] = payload[i];
    }
    req_buf[req_nbytes++] = PACKET_END;

    req_time = getSysTime();
    mgt_transmitBytes(req_buf, req_nbytes);
    return true;
}

/**
 * Read a response into `buf`. Assume that `buf` is large enough
 * (PAYLOAD_MAXBYTES). Returns size of response if successfully read, -1 if
 * otherwise.
 */
int mgt_getResponse(uint8_t* buf) {
    mgt_updateResp();
    if (!received_resp)
        return -1;

    received_resp = false;
    seq_num = !seq_num;
    req_sent = false;
    req_nbytes = 0;
    resp_nbytes = 0;

    memcpy(buf, resp_buf, resp_nbytes);
    return resp_nbytes;
}

/**
 * Retransmit requests that timed out.
 */
void mgt_retransmit() {
    mgt_updateResp();
    if (req_nbytes == 0) {
        return;
    }
    if (!received_resp && getSysTime() - req_time > MGT_ACKNOWLEDGEMENT_TIMEOUT) {
        req_time = getSysTime();
        mgt_transmitBytes(req_buf, req_nbytes);
    }
}

/**
 * Buffers response without reading the response.
 */
void mgt_updateResp() {
    if (received_resp)
        return;

    uint8_t buf;
	uint64_t initTime  = getSysTime();
	while (true) {
        if (getSysTime() - initTime > 10) {
            break;
        }
        int read_nbytes = usart_recieveBytes(MGT_USART, &buf, 1);

        // Not reading if
        if (read_nbytes == 0    // nothing is read
            || (resp_nbytes == 0 && buf != PACKET_START)) // hasn't started
            continue;

        if (resp_nbytes == 1 && buf != seq_num) {
            resp_nbytes = 0;
            continue;
        }

        // Handle escaped characters
        if (escaping) {
            if (buf != ESCAPE && buf != PACKET_END)
                resp_nbytes = 0;
            else
                resp_buf[resp_nbytes++] = buf;
            continue;
        } else if (buf == ESCAPE) {
            escaping = true;
            continue;
        }

        resp_buf[resp_nbytes++] = buf;

        if (buf == PACKET_END)
            received_resp = true;
	}
}

/**
 * Transmit `nbytes` from `message` to MGT. References usart_transmitBytes
 */
void mgt_transmitBytes(uint8_t message[], int nbytes) {
	// Enable UART3 and Transmitter
	MGT_USART->CR1 |= USART_CR1_UE | USART_CR1_TE;

	// Transfer each character one at a time
	for (int i = 0; i < nbytes; i++){
		// wait until Data register is empty
		while (!(MGT_USART->ISR & USART_ISR_TXE));
		// Place the character in the Data Register
		MGT_USART->TDR = message[i];
	}

	// Wait for the Transfer to be completed by monitoring the TC flag
	while(!(MGT_USART->ISR & USART_ISR_TC));
}

///**
// * Set percent to Coil X PWM 0/1
// *
// * @param coilNumber - A number between 0 and 2 (inclusive)
// * @param pwm - 0 or 1
// * @param percentage - A number between 0 and 100 (inclusive)
// *
// * @return bytes sent if message is acknowledged, -E_MGT_LOST if message is sent but not
// * acknowledged, -E_MGT_INVALID if message is invalid and not sent
// */
//int mgt_setCoilPercent(uint8_t coilNumber, uint8_t pwm, uint8_t percentage) {
//    uint8_t payload[3];
//    payload[0] = coilNumber;
//    payload[1] = pwm;
//    payload[2] = percentage;
//    return mgt_transmitPacket(SET_PERCENT, payload, 3);
//}
//
//int mgt_requestCoilCurrent(float* out) {
//    int ret = mgt_transmitPacket(GET_CURRENT, NULL, 0);
//    if (ret < 0)
//        return ret;
//}

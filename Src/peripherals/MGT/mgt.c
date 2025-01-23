/******************************************************************************
* File:             mgt.c
*
* Author:           Eric Xu  
* Created:          11/03/24 
* Description:      Based on ethanese's radio.c
*****************************************************************************/


#include "MGT/mgt.h"

// Forward declarations not part of the packet interface
void mgt_updateResp();
void mgt_transmitBytes(uint8_t message[], int nbytes);

const uint8_t PACKET_START = '{';
const uint8_t PACKET_END = '}';
const uint8_t ESCAPE = '\\';
enum mgt_op {
    SET_PERCENT = 'S',
    GET_CURRENT = 'C',
    ACKNOWLEDGE = 'A',
    SHUTDOWN = 'D',
    STOP_TIMER = 'T',
};
typedef enum mgt_op mgt_op;

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
 * Retransmit request if timed out.
 */
void mgt_retransmit() {
    mgt_updateResp();
    if (req_nbytes == 0) {
        return;
    }
    if (!received_resp && getSysTime() - req_time > TIMEOUT) {
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
// * TODO
// * Pauses the system to wait for acknowledgement byte from other device
// *
// * @return false if time out, true if didn't received acknowledgement
// */
//bool mgt_waitForAcknowledgement(uint64_t initialTime) {
//	while (mgt_readOneByte() != 'A') {
//		if (getSysTime() - initialTime > MGT_ACKNOWLEDGEMENT_TIMEOUT) {
//			return false;
//		}
//	}
//	return true;
//}
//
///**
// * Send a packet to MGT and wait for acknowledgement
// *
// * @param op
// * @param payload
// * @param payload_nbytes
// *
// * @return bytes sent if message is acknowledged, -E_MGT_LOST if message is sent but not
// * acknowledged, -E_MGT_INVALID if message is invalid and not sent
// */
//int mgt_transmitPacket(mgt_op op, uint8_t payload[], int payload_nbytes) {
//    // Check for delimiters in payload
//    for (int i = 0; i < payload_nbytes; i++) {
//        if (payload[i] == '{' || payload[i] == '}')
//            return -E_MGT_INVALID;
//    }
//
//    uint8_t message[packet_maxnbytes];
//    int i = 0;
//    message[i++] = PACKET_START;
//    message[i++] = op;
//    memcpy(message + i, payload, payload_nbytes);
//    i += payload_nbytes;
//    message[i++] = PACKET_END;
//	bool acknowledged = mgt_waitForAcknowledgement(getSysTime());
//    return acknowledged? i : -E_MGT_LOST;
//}
//
///**
// * Receive a packet from MGT and put it into `op` and `payload`. If the packet
// * payload overflows `payload_maxsz`, then flush the rest of the packet and
// * return -E_MGT_OVERFLOW
// */
//int mgt_receivePacket(mgt_op* op, uint8_t* payload, int payload_maxsz) {
//    int received_nbytes = 0;
//    char buf;
//    while (received_nbytes < payload_maxsz) {
//        int bytes_read = usart_recieveBytes(MGT_USART, &buf, 1);
//    }
//    //if (usart_recieveBufferNotEmpty(MGT_USART))
//    //    led_d2(true);
//    //else
//    //    led_d2(false);
//}
//
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
//
///**
// * Send a request to transfer data to the ground station
// *
// * @param numBytesToSend: The number of bytes to send to the ground station
// * @param numTimesToSend: Number of times to resend the data to ground station
// *
// * @return true if it succeeded and false if it failed
// */
////bool mgt_transferToGroundStationRequest(uint8_t numBytesToSend, int numTimesToSend) {
////	mgt_sendByte('t');
////	mgt_sendByte(numBytesToSend);
////	mgt_sendByte(numTimesToSend);
////
////	return mgt_waitForAcknowledgement(getSysTime());
////}
//
///**
// * Gets the current state of the mgt as r = RX_ACTIVE, t = TX_ACTIVE, o = OFF
// * If timesout will return -1 as an error
// *
// * @return the state of the mgt as a character
// */
////char mgt_receiveStateRequest(void){
////	mgt_sendByte('R');
////
////	char byteRead = -1;
////
////	uint64_t initRequestTime = getSysTime();
////	while (byteRead != 'r' && byteRead != 't' && byteRead != 'o') {
////
////		if (getSysTime() - initRequestTime > MGT_ACKNOWLEDGEMENT_TIMEOUT) {
////			return -1;
////		}
////
////		byteRead = mgt_readOneByte();
////	}
////
////	mgt_sendByte('A');
////
////	return byteRead;
////}
//
///**
// * Send a stream of bytes
// *
// * @param numberOfBytes: Number of bytes that is being sent in the buffer
// * @param buffer: the data to send
// *
// */
////void mgt_sendByteStream(int numberOfBytes, uint8_t buffer[]) {
////	uint64_t startingStreamTime = getSysTime();
////	for (int i = 0; i < numberOfBytes; i++) {
////			//If byte per second past threshold then halt byte sending
////			if (i / ((startingStreamTime - getSysTime()) / 1000) > BYTE_PER_SECOND_LIMIT) {
////				i--;
////			} else {
////				mgt_sendByte(buffer[i]);
////			}
////	}
////}
//
//
///**
// * Sends a stream of bytes
// *
// * @param numberOfBytes: Number of bytes to send
// * @param buffer: The buffer with the bytes to send
// *
// * @return False if sending failed and true otherwise
// */
////bool mgt_sendByteStreamToMemRequest(int numberOfBytes, uint8_t buffer[]) {
////	uint64_t startingStreamTime = getSysTime();
////	if (!mgt_sendTransferToMemRequest(numberOfBytes)) {
////		return false;
////	}
////	mgt_sendByteStream(numberOfBytes, buffer);
////	return true;
////}
////
////void mgt_sendState(enum MgtState state) {
////	if (getSysTime() - lastStateChange > MGT_MAX_STATE_CHANGE_TIME_LIMIT) {
////		switch (state) {
////			case MGT_OFF:
////				mgt_sendByte('o');
////				break;
////			case MGT_TX_ACTIVE:
////				mgt_sendByte('t');
////				break;
////			case MGT_RX_ACTIVE:
////				mgt_sendByte('r');
////				break;
////		}
////		lastStateChange = getSysTime();
////	}
////
////}
//
//
///**
// * Reads bytes from the mgt into a buffer
// *
// * @param buffer: the buffer that will be filled by the mgt's data
// * @param receive_buffer_size: the number of bytes the buffer will take
// */
//int mgt_readBytes(uint8_t buffer[], int receive_buffer_size) {
//	uint16_t amount_to_recieve = receive_buffer_size;
//	uint16_t amount_read = 0;
//
//	uint64_t initTime  = getSysTime();
//	while (amount_read < amount_to_recieve) {
//
//			if (getSysTime() - initTime > 10) {
//                break;
//			}
//
//			amount_read += usart_recieveBytes(MGT_USART,
//					buffer,
//					(receive_buffer_size - amount_read));
//			buffer += amount_read;	// pointer arithmetic :)
//
//            // usart_recieverTimedOut not implemented
//			//if (usart_recieverTimedOut(MGT_USART)) {
//			//	break;
//			//}
//	}
//    // Not sure what this line does
//	//amount_read = usart_recieveBytes(MGT_USART, buffer, receive_buffer_size);
//    return amount_read;
//}
//
///**
// * Reads one byte from the buffer and returns it
// */
////int mgt_readOneByte() {
////	uint8_t buffer[1];
////	mgt_readBytes(buffer, 1);
////	return buffer[0];
////
////}

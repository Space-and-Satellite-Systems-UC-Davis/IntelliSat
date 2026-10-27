#include "UART/crc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static USART_TypeDef bus;
static uint8_t transmitted[512], input[512];
static size_t transmitted_size, input_size, input_pos;
static unsigned transmissions, receive_acks, failures;
static bool receiving;
static uint8_t reply = 'A';
static bool timeout;
static unsigned ack_after_transmission, fail_on_transmission;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static void reset(void) {
    transmitted_size = input_size = input_pos = 0;
    transmissions = receive_acks = 0;
    receiving = false;
    reply = 'A';
    timeout = false;
    ack_after_transmission = 1;
    fail_on_transmission = 0;
    memset(transmitted, 0, sizeof transmitted);
    memset(input, 0, sizeof input);
}

/* Only the UART hardware seam is replaced. CRC/framing code is the real source. */
void usart_transmitBytes(USART_TypeDef *dev, uint8_t *bytes, int count) {
    assert(dev == &bus);
    if (receiving) {
        assert(count == 1 && bytes[0] == 'A');
        receive_acks++;
    } else {
        assert(count >= 0 && (size_t)count <= sizeof transmitted);
        memcpy(transmitted, bytes, (size_t)count);
        transmitted_size = (size_t)count;
        transmissions++;
    }
}
int usart_receiveBytes(USART_TypeDef *dev, uint8_t *out, uint16_t size) {
    assert(dev == &bus);
    if (size == 0) return 0;
    if (!receiving) {
        if (timeout || transmissions < ack_after_transmission) return 0;
        out[0] = transmissions == fail_on_transmission ? 'N' : reply;
        return 1;
    }
    size_t count = input_size - input_pos;
    if (count > size) count = size;
    memcpy(out, input + input_pos, count);
    input_pos += count;
    return (int)count;
}
bool packet_available(void) { return input_pos < input_size; }

static void receive_input(const uint8_t *bytes, size_t size) {
    assert(size <= sizeof input);
    memcpy(input, bytes, size);
    input_size = size;
    input_pos = 0;
    receive_acks = 0;
    receiving = true;
}

static void literal_roundtrip(uint8_t *payload, size_t size,
                              const uint8_t *wire, size_t wire_size) {
    reset();
    CHECK(crc_transmit(&bus, payload, (int)size));
    CHECK(transmissions == 1);
    CHECK(transmitted_size == wire_size);
    CHECK(memcmp(transmitted, wire, wire_size) == 0);

    receive_input(wire, wire_size);
    uint8_t out[MAX_PAYLOAD_BYTES] = {0};
    CHECK(crc_read(&bus, out) == (int)size);
    CHECK(memcmp(out, payload, size) == 0);
    CHECK(input_pos == input_size);
    CHECK(receive_acks == 1);
}

static void send_success_requires_an_ack(void) {
    uint8_t payload[120] = {0};

    reset();
    timeout = true;
    CHECK(!crc_transmit(&bus, payload, 3));
    CHECK(transmissions == 5); /* Existing no-response retry budget. */

    reset();
    ack_after_transmission = 2;
    CHECK(crc_transmit(&bus, payload, 3));
    CHECK(transmissions == 2);

    reset();
    reply = 'N';
    CHECK(!crc_chunked_transmit(&bus, payload, sizeof payload, 50));
    CHECK(transmissions == 1);

    reset();
    fail_on_transmission = 2;
    CHECK(!crc_chunked_transmit(&bus, payload, sizeof payload, 50));
    CHECK(transmissions == 2); /* Do not send chunk 2 after chunk 1 fails. */

    reset();
    fail_on_transmission = 2;
    CHECK(!crc_chunked_transmit(&bus, payload, 51, 50));
    CHECK(transmissions == 2); /* Failure in the final short chunk. */

    reset();
    CHECK(crc_chunked_transmit(&bus, payload, sizeof payload, 50));
    CHECK(transmissions == 3);
}

static void binary_roundtrips(void) {
    for (unsigned value = 0; value < 256; value++) {
        uint8_t payload[] = {0, (uint8_t)value, 'U'};
        reset();
        CHECK(crc_transmit(&bus, payload, sizeof payload));
        receive_input(transmitted, transmitted_size);
        uint8_t out[MAX_PAYLOAD_BYTES] = {0};
        CHECK(crc_read(&bus, out) == (int)sizeof payload);
        CHECK(memcmp(out, payload, sizeof payload) == 0);
        CHECK(receive_acks == 1);
    }
    uint8_t chunk[51] = {0};
    for (size_t i = 1; i < sizeof chunk; i++) chunk[i] = i % 2 ? ';' : '\\';
    reset();
    CHECK(crc_transmit(&bus, chunk, sizeof chunk));
    receive_input(transmitted, transmitted_size);
    uint8_t out[MAX_PAYLOAD_BYTES] = {0};
    CHECK(crc_read(&bus, out) == (int)sizeof chunk);
    CHECK(memcmp(out, chunk, sizeof chunk) == 0);
    CHECK(input_pos == input_size && receive_acks == 1);
}

static void expect_rejected(const uint8_t *wire, size_t size) {
    reset();
    receive_input(wire, size);
    uint8_t out[MAX_PAYLOAD_BYTES];
    memset(out, 0xa5, sizeof out);
    CHECK(crc_read(&bus, out) == -1);
    CHECK(receive_acks == 0);
    CHECK(out[0] == 0xa5);
}
static void oversized_frame_is_rejected(void) {
    uint8_t wire[MAX_MESSAGE_BYTES + 1];
    memset(wire, 'X', sizeof wire);
    wire[sizeof wire - 1] = ';';
    expect_rejected(wire, sizeof wire);
    CHECK(input_pos <= MAX_MESSAGE_BYTES);
}
static void frame_at_receive_capacity_is_accepted(void) {
    uint8_t payload[MAX_MESSAGE_BYTES - 2];
    memset(payload, 'X', sizeof payload);
    reset();
    CHECK(crc_transmit(&bus, payload, sizeof payload));
    CHECK(transmitted_size == MAX_MESSAGE_BYTES);
    receive_input(transmitted, transmitted_size);
    uint8_t out[MAX_PAYLOAD_BYTES] = {0};
    CHECK(crc_read(&bus, out) == (int)sizeof payload);
    CHECK(memcmp(out, payload, sizeof payload) == 0 && receive_acks == 1);
}
static void incomplete_or_corrupt_frames_are_not_acked(void) {
    /* The first vector has a zero CRC remainder but lacks its terminator. */
    const uint8_t truncated[] = {'X', 'Y', 0};
    const uint8_t corrupt[] = {'X', 'Y', 1, ';'};
    const uint8_t lone_end[] = {';'};
    const uint8_t dangling_escape[] = {'X', '\\'};
    expect_rejected(truncated, sizeof truncated);
    expect_rejected(corrupt, sizeof corrupt);
    expect_rejected(lone_end, sizeof lone_end);
    expect_rejected(dangling_escape, sizeof dangling_escape);
    expect_rejected(truncated, 0);
    oversized_frame_is_rejected();
}
static void back_to_back_frames_remain_separate(void) {
    const uint8_t wire[] = {'X', '\\', ';', 'Y', 0x18, ';', 'X', 'Y', 0, ';'};
    reset();
    receive_input(wire, sizeof wire);
    uint8_t out[MAX_PAYLOAD_BYTES] = {0};
    CHECK(crc_read(&bus, out) == 3 && memcmp(out, "X;Y", 3) == 0);
    CHECK(input_pos == 6);
    CHECK(crc_read(&bus, out) == 2 && memcmp(out, "XY", 2) == 0);
    CHECK(input_pos == sizeof wire && receive_acks == 2);
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "oversize") == 0) {
        oversized_frame_is_rejected();
        return failures ? 1 : 0;
    }
    /* Fixed vectors pin the existing 0xD8 CRC and escape format. */
    uint8_t plain[] = {'X', 'Y'};
    const uint8_t plain_wire[] = {0x58, 0x59, 0x00, 0x3b};
    literal_roundtrip(plain, sizeof plain, plain_wire, sizeof plain_wire);
    uint8_t slash[] = {'X', '\\', 'Y'};
    const uint8_t slash_wire[] = {0x58, 0x5c, 0x5c, 0x59, 0x58, 0x3b};
    literal_roundtrip(slash, sizeof slash, slash_wire, sizeof slash_wire);
    uint8_t semicolon[] = {'X', ';', 'Y'};
    const uint8_t semicolon_wire[] = {0x58, 0x5c, 0x3b, 0x59, 0x18, 0x3b};
    literal_roundtrip(semicolon, sizeof semicolon, semicolon_wire, sizeof semicolon_wire);
    send_success_requires_an_ack();
    binary_roundtrips();
    frame_at_receive_capacity_is_accepted();
    incomplete_or_corrupt_frames_are_not_acked();
    back_to_back_frames_remain_separate();
    if (failures) {
        fprintf(stderr, "%u CRC checks failed\n", failures);
        return 1;
    }
    puts("CRC tests passed");
    return 0;
}

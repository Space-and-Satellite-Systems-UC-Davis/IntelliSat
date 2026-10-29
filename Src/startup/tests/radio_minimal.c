#include "Radio_intercom.h"
#include "Radio/log_record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

USART_TypeDef test_usart;
static unsigned checks, tx_calls, tx_fail_at, read_at, read_count;
static uint8_t sent[16][MAX_PAYLOAD_BYTES], incoming[256][MAX_PAYLOAD_BYTES];
static int sent_size[16], incoming_size[256];
#define CHECK(condition) do { ++checks; if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); exit(1); \
} } while (0)

bool usart_init(USART_TypeDef *bus, int baud) { (void)bus; (void)baud; return true; }
int usart_receiveBytes(USART_TypeDef *bus, uint8_t *p, uint16_t n) {
    (void)bus; (void)p; (void)n; return 0;
}
void printMsg(const char *format, ...) { (void)format; }
bool mgt_killall(void) { return false; }
void setCommunicatingRadio(bool value) { (void)value; }
log_record_idle fill_log_idle(void) { return (log_record_idle){0}; }

bool crc_transmit(USART_TypeDef *bus, uint8_t *p, int n) {
    CHECK(bus == USART1 && p && n > 0 && n <= (int)sizeof(sent[0]));
    ++tx_calls;
    if (tx_calls <= 16) {
        memcpy(sent[tx_calls - 1], p, (size_t)n);
        sent_size[tx_calls - 1] = n;
    }
    return tx_calls != tx_fail_at;
}
int crc_read(USART_TypeDef *bus, uint8_t *p) {
    CHECK(bus == USART1 && p && read_at < read_count);
    int n = incoming_size[read_at];
    if (n > 0 && n <= MAX_PAYLOAD_BYTES) memcpy(p, incoming[read_at], (size_t)n);
    ++read_at;
    return n;
}
bool crc_chunked_transmit(USART_TypeDef *bus, uint8_t *payload, int nbytes, int lchunks) {
    int nchunks = ((nbytes - 1) / lchunks) + 1;
    int bytes_copied = 0;
    uint8_t subchunk[MAX_PAYLOAD_BYTES];
    for (int i = 0; i < nchunks; ++i) {
        subchunk[0] = (uint8_t)i;
        int count = nbytes - bytes_copied;
        if (count > lchunks) count = lchunks;
        memcpy(subchunk + 1, payload + i * lchunks, (size_t)count);
        if (!crc_transmit(bus, subchunk, count + 1)) return false;
        bytes_copied += count;
    }
    return true;
}
/* Legacy payload-level fixture. tests/radio also links the real CRC source. */
int crc_chunked_read(USART_TypeDef *bus, uint8_t *buffer, int nchunks, size_t capacity) {
    if (!buffer || !capacity || nchunks <= 0 || nchunks > 256) return -1;
    uint8_t subchunk[MAX_PAYLOAD_BYTES];
    int read = 0;
    for (int i = 0; i < nchunks; ++i) {
        int size = crc_read(bus, subchunk);
        if (size <= 1 || subchunk[0] != i ||
            (size_t)(size - 1) > capacity - (size_t)read) return -1;
        memcpy(buffer + read, subchunk + 1, (size_t)(size - 1));
        read += size - 1;
    }
    return read;
}

static void reset(void) {
    tx_calls = tx_fail_at = read_at = read_count = 0;
    memset(sent, 0, sizeof(sent));
    memset(sent_size, 0, sizeof(sent_size));
    memset(incoming, 0, sizeof(incoming));
    memset(incoming_size, 0, sizeof(incoming_size));
    radio_init();
}
static void frame(unsigned at, const uint8_t *bytes, int size) {
    CHECK(at < 256 && size >= -1 && size <= MAX_PAYLOAD_BYTES);
    incoming_size[at] = size;
    if (size > 0) memcpy(incoming[at], bytes, (size_t)size);
    if (read_count <= at) read_count = at + 1;
}
static void response(const uint8_t *payload, size_t size, uint8_t datatype) {
    uint8_t header[4] = {'U', (uint8_t)((size - 1) / CHUNK_LENGTH + 1),
                         (uint8_t)((size - 1) % CHUNK_LENGTH + 1), datatype};
    frame(0, header, 4);
    for (size_t offset = 0, i = 1; offset < size; ++i) {
        uint8_t packet[CHUNK_LENGTH + 1];
        size_t count = size - offset;
        if (count > CHUNK_LENGTH) count = CHUNK_LENGTH;
        packet[0] = (uint8_t)(i - 1);
        memcpy(packet + 1, payload + offset, count);
        frame((unsigned)i, packet, (int)count + 1);
        offset += count;
    }
}

static void test_push(void) {
    const uint8_t payload[] = "123456789";
    reset();
    CHECK(radio_push((uint8_t *)payload, sizeof(payload) - 1));
    CHECK(tx_calls == 3 && sent_size[1] == 3 && sent[1][0] == 'D'
          && sent[1][1] == 1 && sent[1][2] == 9);
    CHECK(sent_size[2] == 10 && sent[2][0] == 0
          && !memcmp(sent[2] + 1, payload, 9));
    reset();
    uint8_t full[50];
    memset(full, 'X', sizeof full);
    CHECK(radio_push(full, sizeof full));
    CHECK(sent[1][1] == 1 && sent[1][2] == 50);
    CHECK(sent_size[2] == 51 && sent[2][0] == 0
          && !memcmp(sent[2] + 1, full, sizeof full));
    for (unsigned failed = 1; failed <= 3; ++failed) {
        reset();
        tx_fail_at = failed;
        CHECK(!radio_push((uint8_t *)payload, sizeof(payload) - 1));
    }
    reset();
    CHECK(!radio_push(NULL, 1) && !radio_push((uint8_t *)payload, 0));
    CHECK(!radio_push((uint8_t *)payload, (size_t)CHUNK_LENGTH * MAX_UINT8_T + 1));
    CHECK(tx_calls == 0);
}

static void test_force_pull(void) {
    const uint8_t payload[] = "123456789";
    struct { uint8_t before, data[MAX_PAYLOAD_BYTES], after; } out;
    memset(&out, 0, sizeof(out)); out.before = 0xa5; out.after = 0x5a;
    reset(); response(payload, 9, 'u');
    RadioPacket packet = radio_force_pull(out.data, sizeof out.data);
    CHECK(packet.size == 9 && packet.datatype == 'u' && !memcmp(out.data, payload, 9));
    CHECK(out.before == 0xa5 && out.after == 0x5a);

    reset();
    packet = radio_force_pull(NULL, sizeof out.data);
    CHECK(packet.size == (size_t)-1 && read_at == 0);
    reset(); frame(0, (uint8_t[]){'U', 1, 3, 'u'}, 3);
    packet = radio_force_pull(out.data, sizeof out.data); CHECK(packet.size == (size_t)-1 && read_at == 1);
    reset(); frame(0, (uint8_t[]){'U', 0, 3, 'u'}, 4);
    packet = radio_force_pull(out.data, sizeof out.data); CHECK(packet.size == (size_t)-1 && read_at == 1);
    reset(); frame(0, (uint8_t[]){'U', 1, 0, 'u'}, 4);
    packet = radio_force_pull(out.data, sizeof out.data); CHECK(packet.size == (size_t)-1 && read_at == 1);
    reset(); frame(0, (uint8_t[]){'U', 1, 51, 'u'}, 4);
    packet = radio_force_pull(out.data, sizeof out.data); CHECK(packet.size == (size_t)-1 && read_at == 1);
    reset(); frame(0, (uint8_t[]){'U', 32, 8, 'u'}, 4);
    packet = radio_force_pull(out.data, sizeof out.data); CHECK(packet.size == (size_t)-1 && read_at == 1);
    reset(); tx_fail_at = 2;
    packet = radio_force_pull(out.data, sizeof out.data); CHECK(packet.size == (size_t)-1 && read_at == 0);
    reset(); frame(0, (uint8_t[]){'U', 1, 3, 'u'}, -1);
    packet = radio_force_pull(out.data, sizeof out.data); CHECK(packet.size == (size_t)-1 && read_at == 1);

    reset(); response(payload, 9, 'u'); incoming[1][0] = 1;
    packet = radio_force_pull(out.data, sizeof out.data);
    CHECK(packet.size == 0 && out.before == 0xa5 && out.after == 0x5a);
    reset(); response(payload, 9, 'u'); incoming_size[1] = 11;
    packet = radio_force_pull(out.data, sizeof out.data);
    CHECK(packet.size == 0 && out.before == 0xa5 && out.after == 0x5a);
    reset(); response(payload, 9, 'u'); incoming_size[1] = 0;
    packet = radio_force_pull(out.data, sizeof out.data); CHECK(packet.size == 0);
    reset(); response(payload, 9, 'u'); incoming_size[1] = -1;
    packet = radio_force_pull(out.data, sizeof out.data); CHECK(packet.size == 0);
}

static void test_pull_and_state(void) {
    const size_t multi_size = MAX_PAYLOAD_BYTES + 1;
    struct { uint8_t before, data[MAX_PAYLOAD_BYTES + 1], after; } multi;
    for (size_t i = 0; i < multi_size; ++i) multi.data[i] = (uint8_t)(i * 13u + 7u);
    uint8_t expected[MAX_PAYLOAD_BYTES + 1];
    memcpy(expected, multi.data, multi_size);
    reset();
    for (size_t offset = 0, index = 0; offset < multi_size; ++index) {
        uint8_t wire[CHUNK_LENGTH + 1];
        size_t count = multi_size - offset;
        if (count > CHUNK_LENGTH) count = CHUNK_LENGTH;
        wire[0] = (uint8_t)index;
        memcpy(wire + 1, expected + offset, count);
        frame((unsigned)index, wire, (int)count + 1);
        offset += count;
    }
    multi.before = 0xa5; multi.after = 0x5a;
    RadioPacket long_packet = radio_pull(multi.data, multi_size, UploadData);
    CHECK(long_packet.size == multi_size && long_packet.datatype == UploadData);
    CHECK(!memcmp(multi.data, expected, multi_size));
    CHECK(multi.before == 0xa5 && multi.after == 0x5a);

    const uint8_t first[] = {0, 'a', 'b', 'c'};
    struct { uint8_t before, data[3], after; } out = {0xa5, {0}, 0x5a};
    reset(); frame(0, first, sizeof(first));
    RadioPacket packet = radio_pull(out.data, 3, UploadData);
    CHECK(packet.size == 3 && packet.datatype == UploadData && !memcmp(out.data, "abc", 3));
    CHECK(out.before == 0xa5 && out.after == 0x5a);
    reset(); frame(0, (uint8_t[]){0, 'a', 'b', 'c', 'x'}, 5);
    packet = radio_pull(out.data, 3, UploadData); CHECK(packet.size == 0);
    reset(); frame(0, (uint8_t[]){1, 'a', 'b', 'c'}, 4);
    packet = radio_pull(out.data, 3, UploadData); CHECK(packet.size == 0);
    reset(); frame(0, (uint8_t[]){0, 'a', 'b', 'c'}, -1);
    packet = radio_pull(out.data, 3, UploadData); CHECK(packet.size == 0);
    reset(); CHECK(radio_pull(out.data, 0, UploadData).size == 0);
    reset(); tx_fail_at = 1;
    CHECK(radio_get_state() == RadioStateUnknown);
    reset(); tx_fail_at = 2;
    CHECK(radio_get_state() == RadioStateUnknown);
    reset(); frame(0, (uint8_t[]){TXactive}, 1);
    CHECK(radio_get_state() == TXactive);
    reset(); frame(0, (uint8_t[]){TXactive}, -1);
    CHECK(radio_get_state() == RadioStateUnknown);
}

int main(void) {
    test_push(); test_force_pull(); test_pull_and_state();
    printf("radio minimal: %u checks passed\n", checks);
    return 0;
}

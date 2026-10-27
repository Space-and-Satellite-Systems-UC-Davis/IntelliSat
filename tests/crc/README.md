# CRC framing regression tests

Run `make -C tests/crc test` from the repository root. Requires Clang and Make;
AddressSanitizer and UndefinedBehaviorSanitizer are enabled. No hardware is used.

The suite compiles the repository's actual `Src/system_config/UART/crc.c` and
its actual CRC header. Only the UART read/write and packet-availability interface
is replaced by a byte-stream adapter. It does not mock CRC or duplicate its
implementation.

Coverage:
- Literal wire vectors pin the existing polynomial (0xD8), escape bytes, and
  terminator, including the previously rejected escaped-semicolon payload.
- All 256 possible byte values and a heavily escaped 50-byte data chunk round-trip.
- Back-to-back frames remain separate.
- Missing terminators, bad CRCs, dangling escapes, empty input, and over-capacity
  frames are rejected without ACK; the defined receive-capacity boundary works.
- A send requires a received ACK. No-response retries keep the existing
  five-attempt budget; a later ACK succeeds.
- Chunked sends fail when their first, middle, or final short chunk fails.

The host adapter returns immediately instead of modeling physical UART time.
These tests do not establish radio/MCU interoperability on the boards, RF
delivery, timeout duration, interrupt timing, or receiver FIFO capacity.
They also do not change or certify unrelated chunk-order/retry reconstruction.

Production scope: only the existing CRC reader's framing/bounds and send-result
handling are changed. The wire format, constants, public signatures, UART driver,
radio main loop, and retry counts are unchanged. Firmware flashing requires
separate approval.

An identical suite is kept in the MCU and radio repositories so each can be
tested independently without depending on the other's checkout.

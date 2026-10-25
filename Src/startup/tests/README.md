# Native host tests for the actual Startup and the SSP1 store

Two seams are tested, and nothing else:

1. `startup_store_load` / `startup_store_save` (`Src/startup/startup_store.h`)
   against a memory FRAM model — `test_startup_store.c`.
2. The real `Startup()` (`Src/startup/startup.h`) with `startup_store.c` linked,
   against modeled PDB, radio, TIM7 time and FRAM — `test_startup.c`.

`test_main_entry.c` compiles the exact `main()` body extracted from `Src/main.c`;
`test_init_entry.c` compiles the exact `init_init()` body extracted from
`Src/platform_init.h` against the real `power_distribution.c`.

## Run

```sh
make -C Src/startup/tests -f test.mk clean test
```

Fully isolated evidence build (recommended, keeps the source tree clean):

```sh
make -C Src/startup/tests -f test.mk BIN_DIR=<isolated-dir> clean test
```

Every binary is built with `-Wall -Wextra -Werror` and
`-fsanitize=address,undefined`, and run with `halt_on_error=1`.

## What the tests establish

`startup_store`:

- The CONTRACT-VECTOR.json golden slot and image (sequence 1, `INITIAL_CHECKS`,
  0 attempts, unarmed, remaining 1800000) are reproduced byte-exactly, and the
  next save publishes a byte-exact record in the inactive slot.
- One valid slot is used; two valid slots resolve to the higher sequence; equal
  sequences must be byte-identical; no valid slot, an ambiguous pair or any
  out-of-range field is an error.
- A cut at every byte of a save (0..21: invalidate, 19 payload bytes, commit)
  still loads as the complete old or the complete new record. A commit that
  landed but whose acknowledgement was lost is recovered as the new record.
- Read, write-enable, write-disable and verified-readback faults return an
  error, leave the active slot intact and allow a retry. Sequence exhaustion
  refuses to wrap.
- Boot counter (0..3), ADCS mask (4..7) and bytes 48..32767 are never written.

`Startup`:

- The 30 minute inhibit waits in full (one checkpoint per counted minute), the
  consumed attempt and the full retry interval are durable before ON, the burn
  is exactly 5000 ms, and `MAIN_OPS` is durable before the handoff.
- Resumed remaining time is waited in full; time spent off is never credited.
- Two-day intervals up to six attempts, weekly from the seventh, saturated at
  seven attempts with no catch-up bursts.
- A pending START wins over a pending retry; the first normal burn still
  precedes listening, so a START already queued for the first eligible boot does
  not skip it; a *retry* that receives START after the reservation is saved
  commands OFF and enters the handshake without ON.
- The ground ACK gates success, HELLO is retried on the response timeout, READY
  is best effort, and only the exact `START`/`ACK` tokens count.
- A reset mid-save never authorizes ON and retries the uncommitted decision; a
  reset after commit waits out the saved interval and never replays the burn.
- Storage faults command OFF and then reset, with no ON and no handoff, and a
  failed pre-Startup boot counter takes the same path without logging.
- A valid deliberately unarmed record immediately stays OFF/inhibited with
  one-second awake pacing: no countdown, Startup writes, deployment or scheduler
  handoff, including observation beyond the full 30-minute inhibit.
- The wait counts every interval exactly once even when polling costs real
  time, so a checkpoint's saved remaining always matches the real time that
  passed (no early expiry, no double subtraction); polling stays at least once
  a second while listening while the FRAM save cadence stays a minute.
- `main()` reaches the OFF/reset path with no logging call between the failed
  FRAM boot counter and the reset request.
- With the production default (`STARTUP_BATTERY_ELIGIBLE()` false, its own
  binary) no attempt is made and nothing is rewritten.

## Limits

`stubs/` replaces hardware and time signatures only. The modeled clock is a
boot-local monotonic counter that restarts at zero on every modeled boot, so no
RTC continuity, oscillator, wake, RF delivery, electrical cutoff, SPI timing or
physical actuation is exercised. `startup_fixtures.h` is a second, independent
implementation of the 20-byte record used to state expected bytes; the golden
literal in `test_startup_store.c` anchors it to the shared contract vector.
`radio_minimal.c` and `radio_stubs/` are older radio scaffolding that no test
in this directory references.

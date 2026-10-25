# Actual blocking Startup integration (#47)

## Current power policy (RTC sleep separated)

`Startup()` now waits **awake** during the 30-minute inhibit: `delay_ms()` advances the
monotonic timer, at most 60 seconds per measurement/checkpoint. It never arms RTC
wake or enters low-power run/WFI. The existing saved-progress and no-unknown-off-time
rules are unchanged. Both host models reject any call to `PWR_enterLPSleepMode()`;
the delay boundary models elapsed time, cross-day progress, torn clocks and resets.
The former RTC wake code, sleep tests and override are preserved unchanged in the
detached `.worktrees/rtc-sleep-preserved` archive. FreeRTOS (which starts only after
Startup returns) now uses the Cortex-M4 port's default SysTick-based tickless idle,
not the removed application RTC-backed override. No real current/board wake claim.
Historical sleep-specific traces mentioned below describe the pre-separation candidate.

## Run

```sh
make -C Src/startup/tests -f test.mk clean test
```

Canonical development evidence (fresh binaries, warning-clean ASan/UBSan):

```sh
make -C Src/startup/tests -f test.mk BIN_DIR=../../../build/startup-minimal-dev-37bhyt_o/integration/bin clean test
```

Five executables: unchanged 23 decision assertions; actual `Startup()` with modeled
Electrical eligibility; the same source with the production-default eligibility
macro (false); unchanged persistence foundation/domain matrix (46 groups, 108803
assertions). Both startup executables include the actual `Src/startup/startup.h` exactly
once and link real decisions **and real persistence**. There is no alternative
controller, production reset/escape hook, callback context or private-state getter.
Production constants are not shortened.

## Boundaries, properties and evidence

`stubs/` substitutes hardware/time signatures only. Independently encoded v3 FRAM
fixtures replace the original BKP fixtures. FRAM bytes 0–7 and all modeled legacy
BKP registers must remain unchanged. RTC milliseconds, awake monotonic time, sleep,
physical-media bytes, observed PDB commands and local Radio requests are separate.
`RadioPacket` retains size/datatype semantics and its 254-byte capacity. Complete
positive bounded packets must exactly equal START or ACK, without newline/NUL,
case folding or substring matching. Those tokens are not authentication/replay
protection. Radio push followed by one-chunk downlink models only local request
acceptance, never RF or ground delivery. PR14 head 57746b620848bbcbdc7bc8730a2323ba46e82ccd
has unresolved U/push/chunk/RX compatibility; host tests do not repair/validate it.

The normal tracer checks full 30-minute inhibit, verified reservation, immediate
permit-to-ON, 5000ms awake burn, OFF before storage/Radio, exact START/HELLO/ACK/READY,
durable completion and normal return. Correct old characterization properties
(order, no early wait completion, remaining-five-minutes recovery, saved consumed
reservation never replayed, unusable receives rejected, pending uplink wins, later
uplink recovery) are migrated, not retained against a second obsolete controller.

Additional cases cover early/subsecond-rounded sleep; 60-second checkpoints and
due boundaries; 25 saved + 5 new minutes without retained clock; revision/source/
backward/invalid clock changes and torn entry sampling; low energy/default false
without reservation; seven total fast reservations followed by weekly indefinitely
(including a ninth reservation); no catch-up; strict/wrong-step/duplicate packets;
failed HELLO paced at 60 seconds; local downlink failure; lost ACK timeout/re-HELLO;
both READY queue and transfer failures still complete after ACK; reset at reserved,
ON, OFF, retirement and completion; mirrored repair; FRAM read/write/WRDI/no-op/
readback faults and inhibited/invalid media. Completion reboots initialize then
return without a fresh uplink even with default battery eligibility false.

Host reset injection abandons only a host invocation using setjmp/longjmp and
retains durable media. Fatal void Startup halts are observed in bounded child
processes (OFF and no ON/mission return), not by adding a production escape.
Real PDB initializer is additionally included under symbol renaming and tested
against a register/line model to verify latch clearing before output mode.

The entry split follows the production boot chain, `init_init()` then
`init_platform()` then `Startup()`:

- `test_main_entry.c` compiles the exact `main()` body extracted from
  `Src/main.c` by `extract_main.py`. Three scenarios assert that `main()` only
  hands off to the init chain (nothing precedes `init_init()`, `init_platform()`
  receives `run_scheduler = true`), that a failed FRAM boot counter idles in
  `__WFI` instead of starting up, and that no PDB/GPIO/RCC entry point is
  reachable from `main()` at all (they are poisoned).
- `test_init_entry.c` compiles the exact `init_init()` body extracted from
  `Src/platform_init.h` against the real `power_distribution.c`. Four scenarios
  require GPIOA/GPIOC clocks plus a separate readback before any GPIO access,
  latch-before-output-mode clearing, exactly two OFF commands, and no ON; they
  then run every fallible step in order and stall at boot evidence, core clocks
  or the startup clock with the latch already commanded OFF.
- `test_startup.c` models the boot chain by replaying the `init_init()` PDB prefix
  before each `Startup()` invocation, so `Startup()`'s `radio_init()` still fails
  if the actuator is left commanded ON, and `Startup()` itself must not re-init
  the PDB.

No real RTC, peripheral work, electrical behavior or full target initialization is
executed by these models.
Fresh red/green, matrix, preservation and new frozen candidate evidence for #61
live under `integration/early-off/`; earlier integration/recovery evidence is intact.

Incremental red/green logs, exact candidate, manifests, diffs and global/index/
ref/SPI preservation are under `build/startup-minimal-dev-37bhyt_o/integration/`.
The untouched 46-group module matrix and five parent probes remain required.
Earlier characterization/module evidence below is historical, not fresh cutover
acceptance. Tickets 02/03/04 remain claimed awaiting independent parent review.
No modeled physics, electrical cutoff/energy qualification, hardware actuation,
RF transmission, target build/link/ELF or flight readiness is claimed. Exact target
toolchain and operator-authorized full-timing flatsat remain separate gates.

## Bounded RTC contract recovery after timeout

The host model formerly declared `rtc_confirm_wait_clock` incorrectly as void/no-args;
that earlier green result did not establish production API compatibility. That
function is not part of this boundary. The real private interface is the Startup
clock module in `Src/startup/startup_clock.h`, whose confirmation entry is
`bool startup_clock_confirm_wait(uint32_t source, uint32_t revision)`, and the real
RTC primitives it calls (`rtc_config`, `rtc_setCalendar`, `rtc_setTime`,
`rtc_getTime`, `rtc_getCalendar`) are declared in the protected production RTC
header. Canonical test.mk extracts both sets of declarations into
`$(BIN_DIR)/startup_clock_contract.h` and preincludes it for `test_startup`,
`test_startup_default_battery`, `test_startup_clock` and
`test_startup_clock_integration`, so a permissive stub cannot hide interface drift.
Startup checks false
before an awake wait **or** inhibit completion, discards its cursor, reopens NO_OLD and
paces failed confirmation with an awake one-second delay. Focused cases inject
false confirmation, revision change during confirmation, a due wait, and repeated
rejection: no unconfirmed awake wait/ON/return and no per-loop retained-credit reset.
The host-only reboot helper is source-named `model_reboot` to avoid libc collision.
This checks the declared boundary, not real RTC hardware/marker physics or a
full target compile. Fresh red/green, canonical matrix, probes and a new frozen
candidate live under `integration/recovery/`; earlier candidates remain intact.

## Startup persistence foundation (ticket 02 substep #46)

The test executable `test_persistence` tests the plain-C persistence module
`Src/startup/startup_persistence.c/.h` implementing the accepted record contract v2.1:
- 32,768-byte memory FRAM model preserving bytes 0–7 (boot and ADCS sentinels).
- Independent literal PREPARED, INHIBIT and progress wire fixtures (Python zlib CRCs;
  PREPARED CRC `0xf87ebba0`). No private production codec or test hook is called.
- All 7 legal wire phases (PREPARED, INHIBIT, ELIGIBLE, RESERVED, RETRY_WAIT,
  RETIRED, COMPLETE) and legal sequence-adjacent predecessor/successor pairs.
- Unmirrored critical state recovery (`ST_RECOVERABLE`) and verified mirroring (`st_repair`).
- Verified write stages: invalidation -> payload -> commit -> full readback.
- The FRAM boundary enforces WREN; every WREN attempt requires an attempted WRDI cleanup.
- 1,046 exhaustive written-byte cut positions, including zero/all bytes: start 201,
  repair 101, ordinary alternating checkpoint 69, ordinary finish 269, pending-entry
  barrier 69, pending-entry finish 337 (entry barrier + current progress + both
  critical replicas, 336 written bytes). The prior 640 ordinary-path positions are
  retained separately from the 406 new pending-entry positions.
  Two additional targeted cuts preserve a sole valid progress
  survivor in either slot. Partial media survives hardware-fault reset and public
  `st_load`; recovery is mirrored/recoverable, conservative and never early.
- 249 independently injected per-call hardware faults (the prior 145 ordinary-path
  cases plus 104 pending-entry cases): every read (including stage,
  full-record and final-snapshot readback), write, WREN and WRDI; successful no-op
  and corrupt writes. A harmless idempotent no-op may succeed only when the final
  snapshot equals the verified normal result. The old seven write-call cuts remain
  separate; they were not byte cuts.
- Initial inhibit lifecycle:
  - Inhibit start (`st_start_inhibit`) with CAS conflict detection.
  - Wait open (`st_wait_open`) with entry accounting: first boot (`ST_NO_OLD_TIME_CREDIT`)
    and warm restart (`ST_BOOT_CONTINUITY_PROVEN`).
  - Time measurement (`st_wait_measure`) and progress checkpoints (`st_checkpoint`).
  - Lost clock retention: 25 verified saved minutes + 5 new minutes qualifies.
  - Discontinuity detection: backward time or source/revision change rejected with
    `ST_REANCHOR_REQUIRED` and cursor invalidated.
  - Exact subsecond boundary: 1,799,999 ms returns `ST_NOT_DUE` without critical writes;
    1,800,000 ms qualifies and commits INHIBIT -> ELIGIBLE.
- All APIs compare all 320 live bytes; decoded authorization/control/progress/slot/
  sequence metadata cannot authorize, shorten a wait or overwrite its sole survivor.
- Invalid entry revokes the old cursor; source/revision/backward/invalid clocks,
  reference-only checkpoints, reentry/rebinding, progress exhaustion, wrong waits,
  semantic-invalid progress, incompatible pairs and wrapped phase sequences are covered.
- Contract §4's first no-credit checkpoint is an enforced entry barrier: saved E and
  entry anchor A persist even when called at A+60s. The cursor keeps measured time
  and its current last sample independently; the next checkpoint stores current
  elapsed/reference. Due finish verifies both checkpoints before critical transition.
  Reset after the barrier credits from A once with proven continuity, otherwise only
  E. Same-sample barriers manufacture no elapsed; exhaustion between the barrier
  and current checkpoint cannot wrap or authorize a partial finish.
  This corrects a P2 paper-contract mismatch, not a demonstrated premature-ON bug.
- Counts are separate: 23 original decision assertions, 30 characterization assertions
  across seven characterization scenarios, and 32 foundation scenario functions with
  23,509 evaluated assertions (including matrix iterations). These are **not 54 tests**.
  Foundation byte-cut and hardware-fault counts above are separate coverage dimensions,
  not additional top-level test functions.

Red/green logs, unchanged parent public probes, preservation reports and candidate
hashes are under `build/startup-minimal-dev-37bhyt_o/foundation/`: prior `codex-*`
artifacts remain frozen; the narrow entry-barrier correction is under `reanchor/`
with red/green logs and a candidate diff against `corrected-review/`. The wrap
regression was already green on entry because the archived partial fix was live;
its historical red is `parent-contract-red.json`, not a manufactured guard removal.
Coverage additions that already pass are reported as such. Module-only acceptance
remains pending parent/reviewer approval; ticket 02 remains claimed. Completed
Standards review was not rerun.

## Deployment-domain public operation coverage (#52)

`test_persistence.c` now exercises all five approved domain APIs through eight
separate operation scenarios, with a shared table-driven executor. Setup uses
independent contract fixtures and real prior public calls. Hardware counters reset
**at operation entry**; measured byte totals exclude setup and recovery.

| Operation | Written bytes | Cuts 0..full | Full+1 success | R/W/WREN/WRDI failures | No-op/corrupt writes | Total faults |
|---|---:|---:|---:|---|---|---:|
| A reserve ELIGIBLE | 200 | 201 | 1 | 10/6/6/6 | 6/6 | 40 |
| B reserve due RETRY_WAIT, ordinary cursor | 268 | 269 | 1 | 17/9/9/9 | 9/9 | 62 |
| C reserve due RETRY_WAIT, pending no-old barrier | 336 | 337 | 1 | 23/12/12/12 | 12/12 | 83 |
| D after OFF | 200 | 201 | 1 | 10/6/6/6 | 6/6 | 40 |
| E retire ELIGIBLE | 200 | 201 | 1 | 10/6/6/6 | 6/6 | 40 |
| F retire RETRY_WAIT | 200 | 201 | 1 | 10/6/6/6 | 6/6 | 40 |
| G complete RETIRED | 200 | 201 | 1 | 10/6/6/6 | 6/6 | 40 |
| H take fresh permit (read only) | 0 | 0 | 0 | 1/0/0/0 | 0/0 | 1 |

New totals: **1,611 byte cuts, seven outside-cut successes, 346 per-call faults**.
The full-byte endpoint is still an I/O error in this model: the last byte persists,
but power is lost before acknowledgement. WREN enforcement, partial-byte writes,
verified readbacks and attempted WRDI cleanup remain enforced. Successful no-op
writes are accepted only if all 320 resulting bytes exactly match the independent
contract oracle. Every normal operation checks exact critical/progress wire slots.

Fault cases check no permit on error (including post-mirror read failure), old
permit revocation, strongest valid predecessor/successor recovery, exact selected
progress observations, read-only public load, byte-identical repair and no ON
reissue. B/C may retain a due checkpoint with the old RETRY_WAIT; saved elapsed is
recoverable, but the failed operation never grants ON. D retains the consumed
reservation and full old-wait checkpoint without reusing its credit. Recovery of
RESERVED consumes the same identity/count once, creates a new post-OFF wait,
ignores old progress, and requires the new wait's early/exact deadline before a
fresh unique reservation. An additional **16 acknowledged single-replica loss**
cases verify no rollback of mirrored reservations, retirement or completion.
All cases preserve boot/ADCS sentinels 0–7.

A ninth new scenario saves one verified day of a two-day cooldown, reboots without
clock continuity, waits precisely the remaining day, then reserves. An uncertain
reservation reload is consumed once; its fresh wait ignores the old due progress.
Nine reservations have distinct IDs 3..19; count saturates at seven, with two later
weekly reservations (eight and nine), including 1 ms early/exact checks.

Frozen #52 matrix totals: 43 persistence scenario functions / 106,793 assertions: 34 existing
functions / 28,895 assertions plus nine new functions / 77,898 assertions. Existing
foundation 1,046 byte cuts / 249 hardware faults and the earlier four-operation
808 cuts are **not** new matrix coverage. Original decisions 23 and Startup
characterization 30 assertions remain separate. New coverage is green on the
already-present module: no historical test-first claim or production correction.
The first matrix run exposed a test-oracle assumption (a newly created wait 4 has
zero progress, not saved wait-4 progress); its correction did not change production.

Evidence, entry source copies, preservation audit, fresh command logs, frozen
candidate/SHA256 and diffs versus accepted foundation and permit-fixed base live
under `build/startup-minimal-dev-37bhyt_o/domain/matrix/`. No frozen prior archive or
parent probe is modified. Independent review remains required.

## Intended final-snapshot readback correction (#53)

The new public regressions substitute **only the final successful 320-byte read**
with a different, independently load-validated CRC-valid/mirrored image. Earlier
per-stage reads remain faithful, and persistent media contains the intended result.
Variants return the predecessor/historical image, another legal reservation, or
unchanged critical records with unexpected progress bytes. Both separate and
`expected == out` destinations are exercised. This is **adversarial detector
coverage beyond the faithful-readback hardware assumption**, not an ordinary power
cut, demonstrated physical reproduction or evidence of physical ON.

Actual red, before production edits: all 78 substituted reads returned `ST_OK`,
including 18 unexpected reserve permit mints; domain and foundation focused
regressions each exited 134 on their final rejection assertion. The five affected
implementation paths were `commit_control` (A–G), `st_start_inhibit`, `st_repair`,
`st_checkpoint` (ordinary/entry), and `st_finish_inhibit` (ordinary/pending).
Supervisor explicitly authorized the same correction on those foundation paths.

Each path now constructs its expected wire poststate from validated current raw
bytes and **only** the intended destination record(s): both critical replicas for
transitions, the other critical replica for repair, or the selected inactive
progress slot for checkpoint. After forced checkpoints, finish/reserve preserve
the updated progress bytes. Final full-snapshot readback must match all 320 bytes
before decoding, cursor rebinding or permit minting; mismatch returns `ST_IO` and
invalidates affected cursor authority. No control predicate, write phase, public
API, record contract or driver changed; no native-structure/padding comparison.

Green: all 78 adversarial cases return `ST_IO`, expose no unexpected decoded output
or fresh permit, reject original/copied permits, invalidate affected cursors, and
recover the actual intended media through public load. Thirteen additional normal
output-alias cases pass. Three new scenario functions add 2,010 assertions;
fresh totals are **46 persistence groups / 108,803 assertions**. The previous
43 groups / 106,793 assertions and all byte-cut/per-call matrix counts are retained
unchanged and are not new #53 fault coverage. All five unchanged parent probes exit
0. Warning-clean ASan/UBSan host evidence is not target or hardware qualification.

Red/green command logs, entry copies, exact preservation audit, corrected frozen
candidate/SHA256 and diff versus the immutable #52 matrix candidate are under
`build/startup-minimal-dev-37bhyt_o/domain/readback/`. The entire earlier matrix
directory remains untouched. #49 is **not complete until independent review**;
no Startup integration or hardware action was performed.

## Limits and future work (integration tickets 03/04 remain unresolved)

The original absolute-RTC 1800-second inhibit and fourteen-day timeout are kept.
The first ON must not be early; the existing inter-state delay makes the observed
first ON 1801 seconds after start. This suite does not require an exact 1800-second
ON or replace the original timeout with the future two-day policy.

A nonempty, non-error packet is only "usable" under the original implementation,
not evidence of protocol-validated ground reception. Successful `radio_push`
is modeled, not proved to be RF transmission. The diagnostics trace includes the
original extra ON, but does not assert unbounded ON or its retry policy as desired
future behavior. The persistence module is **not integrated into Startup**. Reservation/ON
permits, after-OFF, retirement and completion mutation APIs exist and have the
public-domain coverage above; no stubs were introduced. Bounded OFF, send-failure
handling and integration of
FRAM authorization/time recovery and the approved retry policy belong to later
slices. Target compiler/link/ELF and hardware qualification were not run here;
these host results are not flight qualification.

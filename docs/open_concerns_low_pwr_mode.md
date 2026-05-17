# Open Concerns — Low Power Mode (RTOS_low_pwr_mode)

These concerns were identified during design of the tickless idle hook and battery
low-power mode but are **out of scope** for this branch. They are either pre-existing
issues, accepted tradeoffs, or improvements that require separate work.

---

## T3 — ISR Timing During WFI

**What:** When the tickless idle hook puts the MCU into WFI, the CPU is in LP Run
mode (HCLK = 1.25 MHz). If a non-RTC interrupt fires and wakes the MCU, the ISR
executes at 1.25 MHz. Any ISR that uses TIM7-based delays (`delay_ms`,
`wait_with_timeout`) will run 64× slower than intended.

**Impact:** Incorrect timing in ISR if it relies on `getSysTime()` or `delay_ms()`.

**Current status:** Existing ISRs (UART, TIM3, TIM7, SPI) do not use TIM7-based
delays in their handlers. Constraint must be maintained as new ISRs are added.

**Suggested fix:** Document as a hard constraint in ISR authoring guidelines:
ISR handlers must not call `delay_ms()`, `wait_with_timeout()`, or `getSysTime()`
as these are uncalibrated during LP Run.

---

## T4 — IWDG Maximum Sleep Cap

**What:** `watchdog_IWDGSleepMode()` extends the IWDG timeout to its hardware
maximum of 32768 ms (~32 s). If `xExpectedIdleTime` ever exceeds 32 s (i.e., all
tasks are blocked for longer than 32 s), the tickless idle hook would arm the RTC
for >32 s but the IWDG would reset the MCU at ~32 s.

**Impact:** Unexpected MCU reset if any `vTaskDelay` or blocked period exceeds ~32 s.

**Current status:** The battery monitor task polls every 30 s — within the 32 s cap
with ~2 s margin. All other tasks have shorter periods. Currently safe but fragile.

**Suggested fix:** Add a cap in `vPortSuppressTicksAndSleep`:
```c
if (seconds > (IWDG_MAX_TIMEOUT / 1000) - 1)
    seconds = (IWDG_MAX_TIMEOUT / 1000) - 1;
```
This prevents the hook from sleeping longer than the IWDG can tolerate.

---

## T7 — `wait_with_timeout` Effective Duration in `PWR_exitLPRunMode`

**What:** `PWR_exitLPRunMode()` calls `wait_with_timeout(is_REGLPF_set, 1000)` while
HCLK is still at 1.25 MHz. TIM7 runs 64× slower at this clock speed, making the
effective timeout ~64 s instead of 1 s.

**Impact:** If `REGLPF` ever fails to clear (hardware fault), the busy-wait hangs
for ~64 s instead of aborting after 1 s.

**Current status:** Accepted tradeoff. Under normal operation `REGLPF` clears in
microseconds, so the inflated timeout is harmless. Only matters during a hardware
fault scenario.

**Suggested fix:** Poll `REGLPF` with a raw cycle counter rather than `getSysTime()`
inside `PWR_exitLPRunMode`, so the timeout is clock-speed independent.

---

## B0 — Battery Monitor: Event-Driven Alternative to Polling

**What:** The battery monitor task currently polls `pwrmon_getBusVoltage()` every
30 seconds via `vTaskDelay`. The INA226 has a hardware ALERT pin that can be
configured to assert when bus voltage crosses a programmable threshold, which
would allow interrupt-driven entry/exit of low-power mode instead of periodic
polling.

**Impact:** Polling introduces up to 30 s of latency between a voltage drop and
the system's response. An interrupt-driven approach would respond within one
INA226 conversion cycle (~1 ms at the current configuration).

**Current status:** The ALERT pin is not wired or configured. `pwrmon_setMode`
and the config register in `INA226.c` do not set the ALERT limit or mask
registers. Polling is the only available mechanism.

**Suggested fix:**
1. Wire the INA226 ALERT pin to a free GPIO with EXTI interrupt capability.
2. Configure the INA226 alert limit register (`0x07`) and alert mask register
   (`0x06`) via `softi2c_writeReg16` to assert ALERT when bus voltage falls
   below the entry threshold.
3. In the EXTI ISR, notify `lowPwr_task` via `xTaskNotifyFromISR`.
4. Replace `vTaskDelay` in `lowPwr_task` with `xTaskNotifyWait` — the task
   blocks until ALERT fires, then checks voltage and transitions accordingly.

---

## B5 — softi2c Slowdown in LP Run

**What:** `softi2c_delay()` calls `nop(30)` — a hardcoded 30-iteration NOP loop.
At 1.25 MHz HCLK, each NOP takes ~800 ns, making each delay call ~24 µs. I2C SCL
drops to ~10 kHz during LP Run.

**Impact:** `pwrmon_getBusVoltage()` in the battery monitor task takes ~4 ms per
call instead of ~62 µs at 80 MHz. Negligible on a 30 s polling loop.

**Current status:** Accepted. I2C slaves (INA226) tolerate any SCL frequency below
their maximum. No data corruption risk.

**Suggested fix (optional):** Scale `nop()` count by `core_Hz / 80000000` to
maintain consistent I2C timing across clock speeds.

---

## A1 — `vi_start_control_transaction` / `vi_end_control_transaction` Unimplemented

**What:** These functions are declared in `virtual_intellisat.h` and documented to
prevent scheduler preemption during a control loop iteration (sensor read → compute
→ actuate). They have no implementation in `virtual_intellisat.c`.

**Impact:**
1. Linker error if ADCS code calls them (undefined reference).
2. ADCS control loops currently have no preemption protection. The scheduler can
   switch tasks between an IMU read and a magnetorquer command, leaving the control
   algorithm with stale sensor data.

**Current status:** Unimplemented. ADCS integration is incomplete enough that no
linker error surfaces yet.

**Suggested fix:** Implement in `virtual_intellisat.c`:
```c
void vi_start_control_transaction() { vTaskSuspendAll(); }
void vi_end_control_transaction()   { xTaskResumeAll();  }
```
Note: `vi_delay_ms` (→ `vTaskDelay`) must not be called inside a transaction, as
`vTaskDelay` does not block while the scheduler is suspended. This is safe by
convention since `vi_delay_ms` belongs between loop iterations, not inside them.

---

## A2 — `vTaskDelay` Inside `vTaskSuspendAll` Convention

**What:** Once A1 is implemented (`vi_start_control_transaction` → `vTaskSuspendAll`),
calling `vi_delay_ms` (→ `vTaskDelay`) inside a control transaction will silently
not block — FreeRTOS defers the delay until `xTaskResumeAll()`.

**Impact:** If ADCS code ever calls `vi_delay_ms` inside a transaction, the delay
is a no-op, potentially causing a busy loop.

**Current status:** By design, `vi_delay_ms` is called between loop iterations.
Not a current bug but a latent trap.

**Suggested fix:** Document as a constraint in `virtual_intellisat.h`:
> `vi_delay_ms` must not be called between `vi_start_control_transaction` and
> `vi_end_control_transaction`.

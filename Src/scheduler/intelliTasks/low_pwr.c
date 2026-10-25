/*
 * No application tickless override. The Cortex-M4 FreeRTOS port supplies its
 * weak vPortSuppressTicksAndSleep(), using SysTick and shallow WFI while the
 * scheduler runs. The previous RTC-backed override is preserved in the
 * detached .worktrees/rtc-sleep-preserved worktree and must not run in flight.
 */

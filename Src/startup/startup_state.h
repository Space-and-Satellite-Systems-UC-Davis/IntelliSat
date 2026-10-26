#ifndef STARTUP_STATE_H
#define STARTUP_STATE_H

/*
 * The one boot state machine, kept at its original numeric values: a persisted
 * SSP1 record stores one of these values verbatim, so the durable resume point
 * is never a second, parallel phase enum.
 */
typedef enum {
    INITIAL_CHECKS = 0,     /* 30 minute inhibit before the first attempt */
    WAIT_POWER_ON = 1,      /* inhibit/cooldown finished, attempt may proceed */
    WAITING_FOR_UPLINK = 2, /* post-attempt cooldown, listening for START */
    SAT_TO_GROUND_1 = 3,    /* handshake: satellite -> ground HELLO */
    GROUND_TO_SAT_1 = 4,    /* handshake: waiting for the ground ACK */
    SAT_TO_GROUND_2 = 5,    /* handshake: satellite READY */
    MAIN_OPS = 6,           /* handshake complete, hand off to the scheduler */
    DIAGNOSTICS_MODE = 7    /* never persisted; continues as a retry wait */
} MCUState;

#endif /* STARTUP_STATE_H */

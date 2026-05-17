SOURCES := \
    ADCS.c \
    $(wildcard adcs_math/*.c) \
    control/bdot/bdot_control.c \
    control/detumble/detumble.c \
    control/detumble/detumble_util.c \
    $(wildcard control/experiment/*.c) \
    control/PID/PID.c \
    control/ramp/ramp.c \
    determination/TRIAD/triad.c \
    determination/determination.c \
    determination/mag_lookup/mag_lookup.c \
    determination/novasc3.1/eph_manager.c \
    determination/novasc3.1/nutation.c \
    determination/novasc3.1/novas.c \
    determination/novasc3.1/novascon.c \
    determination/novasc3.1/readeph0.c \
    determination/novasc3.1/solsys1.c \
    determination/pos_lookup/ECEF_to_geodetic.c \
    determination/pos_lookup/pos_lookup.c \
    determination/pos_lookup/sgp4/src/c/SGP4.c \
    determination/pos_lookup/sgp4/src/c/TLE.c \
    determination/sun_lookup/spa.c \
    determination/sun_lookup/sun_lookup.c

OBJECTS := $(SOURCES:.c=.o)


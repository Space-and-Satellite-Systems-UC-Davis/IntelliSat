CC := gcc
CFLAGS := -I. -lm -Wswitch -c

# Source files
SOURCES := determination/determination.c determination/mag_lookup/mag_lookup.c determination/TRIAD/triad.c \
           determination/sun_lookup/sun_lookup.c adcs_math/vector.c adcs_math/matrix.c adcs_math/calibration.c adcs_math/sensors.c\
           determination/sun_lookup/spa.c determination/pos_lookup/pos_lookup.c \
           determination/pos_lookup/sgp4/src/c/SGP4.c determination/pos_lookup/sgp4/src/c/TLE.c \
           determination/novasc3.1/novas.c determination/novasc3.1/novascon.c \
           determination/novasc3.1/solsys1.c determination/novasc3.1/eph_manager.c \
           determination/novasc3.1/readeph0.c determination/pos_lookup/ECEF_to_geodetic.c \
           determination/novasc3.1/nutation.c control/detumble/detumble.c control/detumble/detumble_util.c control/bdot/bdot_control.c \
		   control/ramp/ramp.c control/experiment/ramp_experiment.c ADCS.c control/experiment/PID_experiment.c \
           control/experiment/determination_experiment.c

# Target executable name
TARGET := libADCS.a

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS)
	ar rcs $@ *.o
	rm *.o

run:
	./$(TARGET)

clean:
	rm -f $(TARGET) 
	rm -f *.o


.PHONY: all clean run

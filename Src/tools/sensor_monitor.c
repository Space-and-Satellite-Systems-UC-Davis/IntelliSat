/*
 * sensor_monitor.c
 *
 * Prints IMU, magnetometer and sun sensor readings on the debug console
 * (LPUART1 on OPR2/3) as CSV, so flatsat data can be logged from a serial
 * terminal.
 *
 * Console keys:
 *   a  all channels              s      only the selected channel
 *   i  IMU0 and IMU1             + / -  select next / previous channel
 *   m  MAG0 and MAG1             l      list channels
 *   d  sun sensor diodes         h      help
 *   p  panel power and temps     space  pause / resume
 * SW1 does the same as '+'.
 */

#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>
#include <IMU/ASM330LHH.h>
#include <MAG/QMC5883L.h>
#include <SunSensors/sun_sensors.h>
#include "sensor_monitor.h"

// At 9600 baud a line with every channel (~400 characters) takes about 0.4 s
#define ALL_PERIOD_MS		1000
#define GROUP_PERIOD_MS		250
#define SINGLE_PERIOD_MS	100

typedef enum { SM_IMU0, SM_IMU1, SM_MAG0, SM_MAG1, SM_DIODE, SM_PANEL } SM_Source;

typedef enum {
	SM_ACEL_X, SM_ACEL_Y, SM_ACEL_Z, SM_GYRO_X, SM_GYRO_Y, SM_GYRO_Z, SM_IMU_TEMP,
	SM_MAG_X, SM_MAG_Y, SM_MAG_Z, SM_MAG_TEMP,
	SM_DIODE0, SM_DIODE1,
	SM_BUS_VOLTAGE, SM_CURRENT, SM_TEMP0, SM_TEMP1
} SM_Item;

typedef struct {
	const char *name;
	const char *unit;
	SM_Source source;
	SM_Item item;
	PANELS panel;	// only used by SM_DIODE and SM_PANEL
	int decimals;
} SM_Channel;

typedef enum { SHOW_ALL, SHOW_IMU, SHOW_MAG, SHOW_DIODE, SHOW_PANEL, SHOW_SELECTED } SM_Show;

static const SM_Channel channels[] = {
	{"imu0_ax", "g", SM_IMU0, SM_ACEL_X, PANEL0, 4},
	{"imu0_ay", "g", SM_IMU0, SM_ACEL_Y, PANEL0, 4},
	{"imu0_az", "g", SM_IMU0, SM_ACEL_Z, PANEL0, 4},
	{"imu0_gx", "dps", SM_IMU0, SM_GYRO_X, PANEL0, 3},
	{"imu0_gy", "dps", SM_IMU0, SM_GYRO_Y, PANEL0, 3},
	{"imu0_gz", "dps", SM_IMU0, SM_GYRO_Z, PANEL0, 3},
	{"imu0_temp", "C", SM_IMU0, SM_IMU_TEMP, PANEL0, 2},
	{"imu1_ax", "g", SM_IMU1, SM_ACEL_X, PANEL0, 4},
	{"imu1_ay", "g", SM_IMU1, SM_ACEL_Y, PANEL0, 4},
	{"imu1_az", "g", SM_IMU1, SM_ACEL_Z, PANEL0, 4},
	{"imu1_gx", "dps", SM_IMU1, SM_GYRO_X, PANEL0, 3},
	{"imu1_gy", "dps", SM_IMU1, SM_GYRO_Y, PANEL0, 3},
	{"imu1_gz", "dps", SM_IMU1, SM_GYRO_Z, PANEL0, 3},
	{"imu1_temp", "C", SM_IMU1, SM_IMU_TEMP, PANEL0, 2},
	{"mag0_x", "raw", SM_MAG0, SM_MAG_X, PANEL0, 0},
	{"mag0_y", "raw", SM_MAG0, SM_MAG_Y, PANEL0, 0},
	{"mag0_z", "raw", SM_MAG0, SM_MAG_Z, PANEL0, 0},
	{"mag0_temp", "raw", SM_MAG0, SM_MAG_TEMP, PANEL0, 0},
	{"mag1_x", "raw", SM_MAG1, SM_MAG_X, PANEL0, 0},
	{"mag1_y", "raw", SM_MAG1, SM_MAG_Y, PANEL0, 0},
	{"mag1_z", "raw", SM_MAG1, SM_MAG_Z, PANEL0, 0},
	{"mag1_temp", "raw", SM_MAG1, SM_MAG_TEMP, PANEL0, 0},
	{"pan0_d0", "V", SM_DIODE, SM_DIODE0, PANEL0, 3},
	{"pan0_d1", "V", SM_DIODE, SM_DIODE1, PANEL0, 3},
	{"pan1_d0", "V", SM_DIODE, SM_DIODE0, PANEL1, 3},
	{"pan1_d1", "V", SM_DIODE, SM_DIODE1, PANEL1, 3},
	{"pan2_d0", "V", SM_DIODE, SM_DIODE0, PANEL2, 3},
	{"pan2_d1", "V", SM_DIODE, SM_DIODE1, PANEL2, 3},
	{"pan3_d0", "V", SM_DIODE, SM_DIODE0, PANEL3, 3},
	{"pan3_d1", "V", SM_DIODE, SM_DIODE1, PANEL3, 3},
	{"pan4_d0", "V", SM_DIODE, SM_DIODE0, PANEL4, 3},
	{"pan4_d1", "V", SM_DIODE, SM_DIODE1, PANEL4, 3},
	{"pan5_d0", "V", SM_DIODE, SM_DIODE0, PANEL5, 3},
	{"pan5_d1", "V", SM_DIODE, SM_DIODE1, PANEL5, 3},
	// Only panels 0-3 have the INA226 and TMP275s; panels 4 and 5 have no I2C pins set
	{"pan0_bus", "V", SM_PANEL, SM_BUS_VOLTAGE, PANEL0, 3},
	{"pan0_i", "A", SM_PANEL, SM_CURRENT, PANEL0, 4},
	{"pan0_t0", "C", SM_PANEL, SM_TEMP0, PANEL0, 2},
	{"pan0_t1", "C", SM_PANEL, SM_TEMP1, PANEL0, 2},
	{"pan1_bus", "V", SM_PANEL, SM_BUS_VOLTAGE, PANEL1, 3},
	{"pan1_i", "A", SM_PANEL, SM_CURRENT, PANEL1, 4},
	{"pan1_t0", "C", SM_PANEL, SM_TEMP0, PANEL1, 2},
	{"pan1_t1", "C", SM_PANEL, SM_TEMP1, PANEL1, 2},
	{"pan2_bus", "V", SM_PANEL, SM_BUS_VOLTAGE, PANEL2, 3},
	{"pan2_i", "A", SM_PANEL, SM_CURRENT, PANEL2, 4},
	{"pan2_t0", "C", SM_PANEL, SM_TEMP0, PANEL2, 2},
	{"pan2_t1", "C", SM_PANEL, SM_TEMP1, PANEL2, 2},
	{"pan3_bus", "V", SM_PANEL, SM_BUS_VOLTAGE, PANEL3, 3},
	{"pan3_i", "A", SM_PANEL, SM_CURRENT, PANEL3, 4},
	{"pan3_t0", "C", SM_PANEL, SM_TEMP0, PANEL3, 2},
	{"pan3_t1", "C", SM_PANEL, SM_TEMP1, PANEL3, 2},
};

#define NUM_CHANNELS	((int)(sizeof(channels) / sizeof(channels[0])))

static SM_Show show = SHOW_ALL;
static int selected = 0;
static bool paused = false;
static bool needHeader = true;
static uint64_t lastSample = 0;

static float readChannel(const SM_Channel *ch) {
	switch (ch->source) {
		case SM_IMU0:
		case SM_IMU1:
			set_IMU(ch->source == SM_IMU0 ? IMU0 : IMU1);
			switch (ch->item) {
				case SM_ACEL_X: return imu_readAcel_X();
				case SM_ACEL_Y: return imu_readAcel_Y();
				case SM_ACEL_Z: return imu_readAcel_Z();
				case SM_GYRO_X: return imu_readGyro_X();
				case SM_GYRO_Y: return imu_readGyro_Y();
				case SM_GYRO_Z: return imu_readGyro_Z();
				default:        return imu_readTemp();
			}
		case SM_MAG0:
		case SM_MAG1:
			mag_select(ch->source == SM_MAG0 ? MAG0 : MAG1);
			switch (ch->item) {
				case SM_MAG_X: return mag_read_X();
				case SM_MAG_Y: return mag_read_Y();
				case SM_MAG_Z: return mag_read_Z();
				default:       return mag_readTemp();
			}
		case SM_DIODE:
			return sun_sensors_readVoltage(ch->panel, ch->item == SM_DIODE0 ? DIODE0 : DIODE1);
		case SM_PANEL:
			switch (ch->item) {
				case SM_BUS_VOLTAGE: return sun_sensors_readBusVoltage(ch->panel);
				case SM_CURRENT:     return sun_sensors_readCurrent(ch->panel);
				case SM_TEMP0:       return sun_sensors_readTemp(ch->panel, TEMP0);
				default:             return sun_sensors_readTemp(ch->panel, TEMP1);
			}
	}
	return 0;
}

static bool isShown(int i) {
	switch (show) {
		case SHOW_ALL:      return true;
		case SHOW_SELECTED: return i == selected;
		case SHOW_IMU:      return channels[i].source == SM_IMU0 || channels[i].source == SM_IMU1;
		case SHOW_MAG:      return channels[i].source == SM_MAG0 || channels[i].source == SM_MAG1;
		case SHOW_DIODE:    return channels[i].source == SM_DIODE;
		case SHOW_PANEL:    return channels[i].source == SM_PANEL;
	}
	return false;
}

static const char* showName() {
	switch (show) {
		case SHOW_IMU:      return "imu";
		case SHOW_MAG:      return "mag";
		case SHOW_DIODE:    return "diodes";
		case SHOW_PANEL:    return "panels";
		case SHOW_SELECTED: return channels[selected].name;
		default:            return "all";
	}
}

static uint64_t period() {
	if (show == SHOW_ALL) return ALL_PERIOD_MS;
	if (show == SHOW_SELECTED) return SINGLE_PERIOD_MS;
	return GROUP_PERIOD_MS;
}

static void printHelp() {
	printMsg("# keys: a all, i imu, m mag, d diodes, p panels, s selected channel\r\n");
	printMsg("#       +/- next/previous channel (also SW1), l list, space pause, h help\r\n");
}

static void printList() {
	for (int i = 0; i < NUM_CHANNELS; i++) {
		printMsg("# %d %s[%s]%s\r\n", i, channels[i].name, channels[i].unit, i == selected ? " <" : "");
	}
}

static void printHeader() {
	printMsg("# %s, every %d ms\r\n", showName(), (int)period());
	printMsg("t_ms");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (isShown(i)) {
			printMsg(",%s[%s]", channels[i].name, channels[i].unit);
		}
	}
	printMsg("\r\n");
}

static void printSample(uint64_t now) {
	printMsg("%lu", (unsigned long)now);
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (isShown(i)) {
			printMsg(",%.*f", channels[i].decimals, readChannel(&channels[i]));
		}
	}
	printMsg("\r\n");
}

static void setShow(SM_Show newShow) {
	show = newShow;
	paused = false;
	needHeader = true;
}

static void selectChannel(int step) {
	selected = (selected + step + NUM_CHANNELS) % NUM_CHANNELS;
	setShow(SHOW_SELECTED);
}

static void handleKey(char key) {
	switch (key) {
		case 'a': setShow(SHOW_ALL); break;
		case 'i': setShow(SHOW_IMU); break;
		case 'm': setShow(SHOW_MAG); break;
		case 'd': setShow(SHOW_DIODE); break;
		case 'p': setShow(SHOW_PANEL); break;
		case 's': setShow(SHOW_SELECTED); break;
		case '+':
		case '=': selectChannel(1); break;
		case '-': selectChannel(-1); break;
		case 'l': printList(); break;
		case 'h':
		case '?': printHelp(); break;
		case ' ':
			paused = !paused;
			printMsg(paused ? "# paused\r\n" : "# resumed\r\n");
			break;
		default: break;	// ignore CR, LF and other keys
	}
}

void sensor_monitor_init() {
	show = SHOW_ALL;
	selected = 0;
	paused = false;
	needHeader = true;
	printHelp();
}

void sensor_monitor_poll() {
	char key;
	while (scanChar(&key)) {
		handleKey(key);
	}
	if (get_buttonStatus_SW1()) {
		selectChannel(1);
	}
	if (paused) {
		return;
	}

	uint64_t now = getSysTime();
	if (needHeader) {
		printHeader();
		needHeader = false;
		lastSample = now - period();	// print the first line right away
	}
	if (now - lastSample >= period()) {
		lastSample = now;
		printSample(now);
	}
}

void testFunction_Sensor_Monitor() {
	sensor_monitor_init();
	while (1) {
		sensor_monitor_poll();
	}
}

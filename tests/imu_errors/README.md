# IMU read error tests

Run on Linux with a C compiler:

```sh
bash tests/imu_errors/run.sh
```

The tests compile the production IMU reader and ADCS call path. A fake SPI
transport injects failures in the command, low byte, or high byte on both IMUs.
They check CS release, invalid readings, WHO_AM_I, and the existing three-attempt
ADCS retry limit. Valid zero and signed readings are also covered.

SPI and calibration are test doubles; this does not test recovery of a physical
bus. The separate `tests/spi_timeout` suite exercises the compiled ARM SPI driver.

# SPI timeout regression tests

Build the production SPI driver and `is_time_out()` with Arm GNU Toolchain,
then run the Thumb code in Unicorn with controlled SPI registers and a clock:

```sh
python -m pip install unicorn==2.1.4 pyelftools==0.32
bash tests/spi_timeout/build.sh
python tests/spi_timeout/test_spi.py build/spi-timeout/spi.elf
```

Set `CC` if `arm-none-eabi-gcc` is not on PATH. These tests cover transfer
results and buffer access under injected stalls; they do not model bus timing
or replace board tests. Production sources are compiled without test hooks.

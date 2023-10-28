#include "w25q128jv.h"

// code implmenentation of the functions from the header...
// Implement things like:
//   - FLASH Config (calls qspi_config() as well)
//   - A generic FLASH Read & Write  (never write more than 1 page)
//   - Ways to erase the page being written to
//   - Functions to change status registers: Write enable, Write disable, Quad enable
// When configuring registers, poll the registers afterwards. When erasing, poll the BUSY flag afterwards

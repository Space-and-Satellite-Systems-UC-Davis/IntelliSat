#include "../../globals.h"
#include "w25q128jv.h"

void branch_main() {
  // start FLASH here
  while (1) {
    if (button1) {
      button1 = false;
      // test code here
    }
  }
}

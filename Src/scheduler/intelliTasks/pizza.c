#include <print_scan.h>
#include <stdbool.h>


bool pizza_time() {
    printMsg("pizza time?\n\r");
    return true;
}

void config_pizza() {
    printMsg("pizza config\n\r");
}

void pizza() {
    while (1) {
        printMsg("pizza run\n\r");
    }
}

void clean_pizza() {
    printMsg("pizza cleanup\n\r");
}

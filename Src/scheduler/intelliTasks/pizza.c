#include <stdbool.h>

bool pizza_time() {
    printMsg("pizza time?\n");
    return true;
}

void config_pizza() {
    printMsg("pizza config\n");
}

void pizza() {
    while (1) {
        printMsg("pizza run\n");
    }
}

void clean_pizza() {
    printMsg("pizza cleanup\n");
}
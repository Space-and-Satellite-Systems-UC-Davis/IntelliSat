#include "platform_init.h"
#include <print_scan.h>

/*
 * For now this won't be implemented on the main branch
 * Therefore, the main branch cannot be built.
 * Each development / feature branch has it's own implementation
 * which is used for testing specific features.
 */
void branch_main();

int main() {
	init_platform();
	branch_main();
}

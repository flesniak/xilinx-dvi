#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PRINT(x) printf("%-42s = 0x%08x %010u\n", #x, x, x)
#define PRINTL(x) printf("%-42s = 0x%08llx %010llu\n", #x, x, x)

#define DVI_BASE_ADDRESS 0x80000000U
#define DVI_CONTROL_REGS_SIZE 32U
#define DVI_VMEM_ADDRESS DVI_BASE_ADDRESS+DVI_CONTROL_REGS_SIZE
#define DVI_VMEM_SIZE 0x400000U

int main() {
PRINT(DVI_BASE_ADDRESS);
PRINT(DVI_VMEM_ADDRESS);
PRINT(DVI_VMEM_SIZE);

PRINT(UINT_MAX);
PRINT(UINT_MAX-1);
PRINT(UINT_MAX-DVI_VMEM_SIZE);
PRINT(UINT_MAX-DVI_VMEM_ADDRESS);
PRINT(UINT_MAX-DVI_VMEM_SIZE-DVI_VMEM_ADDRESS);
PRINT(UINT_MAX-(DVI_VMEM_ADDRESS+DVI_VMEM_SIZE));

uint64_t a = UINT_MAX;
uint64_t b = DVI_VMEM_ADDRESS;
uint64_t c = DVI_VMEM_SIZE;
uint64_t d = a-b-c-1;
PRINTL(d);

return 0;
}

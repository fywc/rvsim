#include <stdio.h>

#include "rvsim.h"

int main(int argc, char *argv[]) 
{
    assert(argc > 1);

    machine_t m = {0};
    machine_load_program(&m, argv[1]);

    printf("entry: 0x%08lx\n", m.mmu.entry);
    return 0;
}

#include <stdio.h>

#include "rvsim.h"

int main(int argc, char *argv[]) 
{
    assert(argc > 1);

    machine_t m = {0};
    machine_load_program(&m, argv[1]);
    machine_setup(&m, argc, argv);

    printf("entry: 0x%08lx\n", m.mmu.entry);
    printf("host_alloc: 0x%08lx\n", m.mmu.host_alloc);

    enum exit_reason_t exit_reason;
    while (true) {
        exit_reason = machine_step(&m);
        assert(exit_reason == ecall);

        u64 syscall = machine_get_gp_reg(&m, a7);
        fatalf("syscall %lu", syscall);
        u64 ret = do_syscall(&m, syscall);
        machine_set_gp_reg(&m, a0, ret);
    }
    return 0;
}

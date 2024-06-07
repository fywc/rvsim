#include "rvsim.h"
#include <fcntl.h>

void machine_load_program(machine_t *m, char *prog)
{
    int fd = open(prog, O_RDONLY);
    if (fd < 0) {
        fatal(strerror(errno));
    }

    mmu_load_elf(&m->mmu, fd);
    close(fd);

    m->state.pc = (u64)m->mmu.entry;
    return ;
}

enum exit_reason_t machine_step(machine_t *m)
{
    while (true) {
        exec_block_interp(&m->state);

        if (m->state.exit_reason == indirect_branch || m->state.exit_reason == direct_branch) {
            continue;
        }

        break;
    }
    assert(m->state.exit_reason == ecall);
    return ecall;

}

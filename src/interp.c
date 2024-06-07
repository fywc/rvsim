#include "rvsim.h"

typedef void (func_t)(state_t *, insn_t *);

static void func_empty(state_t *state, insn_t *insn) {}

// lb -> load byte
static void func_lb(state_t *state, insn_t *insn)
{

}


static func_t *funcs[] = {
    func_lb,
    func_empty,
};

void exec_block_interp(state_t *state)
{
    static insn_t insn = {0};
    while (true) {
        u32 inst_data = *(u32 *)TO_HOST(state->pc);
        insn_decode(&insn, inst_data);

        funcs[insn.type](state, &insn);
        state->gp_regs[zero] = 0;
        
        if (insn.cont)
            break;

        state->pc += insn.rvc ? 2 : 4;
    }
}

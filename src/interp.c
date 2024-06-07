#include "rvsim.h"

typedef void (func_t)(state_t *, insn_t *);

static void func_empty(state_t *state, insn_t *insn) {}

// lb -> load byte
static void func_lb(state_t *state, insn_t *insn)
{
    u64 addr = state->gp_regs[insn->rs1] + (i64)insn->imm;
    state->gp_regs[insn->rd] = *(i8 *)TO_HOST(addr);
}

static void func_lh(state_t *state, insn_t *insn)
{
    u64 addr = state->gp_regs[insn->rs1] + (i64)insn->imm;
    state->gp_regs[insn->rd] = *(i16 *)TO_HOST(addr);
}

static void func_lw(state_t *state, insn_t *insn)
{
    u64 addr = state->gp_regs[insn->rs1] + (i64)insn->imm;
    state->gp_regs[insn->rd] = *(i32 *)TO_HOST(addr);
}

static void func_ld(state_t *state, insn_t *insn)
{
    u64 addr = state->gp_regs[insn->rs1] + (i64)insn->imm;
    state->gp_regs[insn->rd] = *(i64 *)TO_HOST(addr);
}

static void func_lbu(state_t *state, insn_t *insn)
{
    u64 addr = state->gp_regs[insn->rs1] + (i64)insn->imm;
    state->gp_regs[insn->rd] = *(u8 *)TO_HOST(addr);
}

static void func_lhu(state_t *state, insn_t *insn)
{
    u64 addr = state->gp_regs[insn->rs1] + (i64)insn->imm;
    state->gp_regs[insn->rd] = *(u16 *)TO_HOST(addr);
}

static void func_lwu(state_t *state, insn_t *insn)
{
    u64 addr = state->gp_regs[insn->rs1] + (i64)insn->imm;
    state->gp_regs[insn->rd] = *(u32 *)TO_HOST(addr);
}

static func_t *funcs[] = {
    func_lb,
    func_lh,
    func_lw,
    func_ld,
    func_lbu,
    func_lhu,
    func_lwu,
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

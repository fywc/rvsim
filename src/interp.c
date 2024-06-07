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

static void func_addi(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = rs1 + imm;
}

static void func_slli(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = rs1 << (imm & 0x3ff);
}

static void func_slti(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = rs1 < (i64)imm ? 1 : 0;
}

static void func_sltiu(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = rs1 < (u64)imm ? 1 : 0;
}

static void func_xori(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = rs1 ^ imm;
}

static void func_srli(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = rs1 >> (imm & 0x3f);
}

static void func_srai(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = (i64)rs1 >> (imm & 0x3f);
}

static void func_ori(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = rs1 | (u64)imm;
    // rs1 is u64, so we need (u64) to convert i64 -> u64
}

static void func_andi(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = rs1 & (u64)imm;
}

static void func_addiw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = (i64)(i32)rs1 + imm;
}

static void func_slliw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = (i64)(i32)(rs1 << (imm & 0x1f));
}

static void func_srliw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = (i64)(i32)((u32)rs1 >> (imm & 0x1f));
}

static void func_sraiw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    i64 imm = (i64)insn->imm;
    state->gp_regs[insn->rd] = (i64)((i32)rs1 >> (imm & 0x1f));
}

static void func_auipc(state_t *state, insn_t *insn)
{
    u64 val = state->pc + (i64)insn->imm;
    state->gp_regs[insn->rd] = val;
}

static func_t *funcs[] = {
    func_lb,
    func_lh,
    func_lw,
    func_ld,
    func_lbu,
    func_lhu,
    func_lwu,
    func_addi,
    func_slli,
    func_slti,
    func_sltiu,
    func_xori,
    func_srli,
    func_srai,
    func_ori,
    func_andi,
    func_addiw,
    func_slliw,
    func_srliw,
    func_sraiw,
    func_auipc,
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

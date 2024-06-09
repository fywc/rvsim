#include "rvsim.h"
#include "interp_util.h"

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

static void func_sb(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    *(u8 *)TO_HOST(rs1 + insn->imm) = (u8)rs2;
}

static void func_sh(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    *(u16 *)TO_HOST(rs1 + insn->imm) = (u16)rs2;
}

static void func_sw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    *(u32 *)TO_HOST(rs1 + insn->imm) = (u32)rs2;
}

static void func_sd(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    *(u64 *)TO_HOST(rs1 + insn->imm) = (u8)rs2;
}

static void func_add(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rs1 + rs2;
}

static void func_sll(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rs1 << (rs2 & 0x3f);
}

static void func_slt(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (i64)rs1 < (i64)rs2 ? 1 : 0;
}

static void func_sltu(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (u64)rs1 < (u64)rs2 ? 1 : 0;
}

static void func_xor(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rs1 ^ rs2;
}

static void func_srl(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rs1 >> (rs2 & 0x3f);
}

static void func_or(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rs1 | rs2;
}

static void func_and(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rs1 & rs2;
}

static void func_mul(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rs1 * rs2;
}

static void func_mulh(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = mulh(rs1, rs2);
}

static void func_mulhu(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = mulhu(rs1, rs2);
}

static void func_mulhsu(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = mulhsu(rs1, rs2);
}

static void func_div(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = div1(rs1, rs2);
}
  
static void func_divu(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = divu(rs1, rs2);
}
  
static void func_rem(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rem(rs1, rs2);
}
  
static void func_remu(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = remu(rs1, rs2);
}
  
static void func_sub(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rs1 - rs2;
}
  
static void func_sra(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (i64)rs1 >> (rs2 & 0x3f);
}
  
static void func_lui(state_t *state, insn_t *insn)
{
    state->gp_regs[insn->rd] = (i64)insn->imm;
}
  
static void func_addw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (i64)(i32)(rs1 + rs2);
}
  
static void func_sllw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (i64)(i32)(rs1 << (rs2 & 0x3f));
}
  
static void func_srlw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (i64)(i32)(rs1 >> (rs2 & 0x3f));
}
  
static void func_mulw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (i64)(i32)(rs1 * rs2);
}
  
static void func_divw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (rs2 == 0) ? UINT64_MAX : (i32)(((i64)(i32)rs1 / (i64)(i32)rs2)); 
}
  
static void func_divuw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (rs2 == 0) ? UINT64_MAX : (i32)((u32)rs1 / (u32)rs2); 
}
  
static void func_remw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rs2 == 0 ? (i64)(i32)rs1 : (i64)(i32)((i64)(i32)rs1 % (i64)(i32)rs2);
}
  
static void func_remuw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = rs2 == 0 ? (i64)(i32)(u32)rs1 : (i64)(i32)((u32)rs1 % (u32)rs2);
}
  
static void func_subw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (i64)(i32)(rs1 - rs2);
}

static void func_sraw(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    state->gp_regs[insn->rd] = (i64)(i32)((i32)rs1 >> (rs2 & 0x1f));
}

static void func_beq(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    u64 target_addr = state->pc + (i64)insn->imm;
    if ((u64)rs1 == (u64)rs2) {
        state->reenter_pc = state->pc = target_addr;
        state->exit_reason = direct_branch;
        insn->cont = true;
    }
}

static void func_bne(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    u64 target_addr = state->pc + (i64)insn->imm;
    if ((u64)rs1 != (u64)rs2) {
        state->reenter_pc = state->pc = target_addr;
        state->exit_reason = direct_branch;
        insn->cont = true;
    }
}

static void func_blt(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    u64 target_addr = state->pc + (i64)insn->imm;
    if ((i64)rs1 < (i64)rs2) {
        state->reenter_pc = state->pc = target_addr;
        state->exit_reason = direct_branch;
        insn->cont = true;
    }
}

static void func_bge(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    u64 target_addr = state->pc + (i64)insn->imm;
    if ((i64)rs1 >= (i64)rs2) {
        state->reenter_pc = state->pc = target_addr;
        state->exit_reason = direct_branch;
        insn->cont = true;
    }
}

static void func_bltu(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    u64 target_addr = state->pc + (i64)insn->imm;
    if ((u64)rs1 < (u64)rs2) {
        state->reenter_pc = state->pc = target_addr;
        state->exit_reason = direct_branch;
        insn->cont = true;
    }
}

static void func_bgeu(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    u64 rs2 = state->gp_regs[insn->rs2];
    u64 target_addr = state->pc + (i64)insn->imm;
    if ((i64)rs1 >= (i64)rs2) {
        state->reenter_pc = state->pc = target_addr;
        state->exit_reason = direct_branch;
        insn->cont = true;
    }
}

static void func_jalr(state_t *state, insn_t *insn)
{
    u64 rs1 = state->gp_regs[insn->rs1];
    state->gp_regs[insn->rd] = state->pc + (insn->rvc ? 2 : 4);
    state->exit_reason = indirect_branch;
    state->reenter_pc = (rs1 + (i64)insn->imm) & ~(u64)1;
}

static void func_jal(state_t *state, insn_t *insn)
{
    state->gp_regs[insn->rd] = state->pc + (insn->rvc ? 2 : 4);
    state->exit_reason = direct_branch;
    state->reenter_pc = state->pc = state->pc + (i64)insn->imm;
}

static void func_ecall(state_t *state, insn_t *insn)
{
    state->exit_reason = ecall;
    state->reenter_pc = state->pc + 4;
}

#define FUNC()                         \
    switch (insn->csr) {               \
    case fflags:                       \
    case frm:                          \
    case fcsr:                         \
        break;                         \
    default: fatal("unsupported csr"); \
    }                                  \
    state->gp_regs[insn->rd] = 0;      \

static void func_csrrw(state_t *state, insn_t *insn) { FUNC(); }
static void func_csrrs(state_t *state, insn_t *insn) { FUNC(); }
static void func_csrrc(state_t *state, insn_t *insn) { FUNC(); }
static void func_csrrwi(state_t *state, insn_t *insn) { FUNC(); }
static void func_csrrsi(state_t *state, insn_t *insn) { FUNC(); }
static void func_csrrci(state_t *state, insn_t *insn) { FUNC(); }

#undef FUNC

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
    func_sb,
    func_sh,
    func_sw,
    func_sd,
    func_add,
    func_sll,
    func_slt,
    func_sltu,
    func_xor,
    func_srl,
    func_or,
    func_and,
    func_mul,
    func_mulh,
    func_mulhu,
    func_mulhsu,
    func_div,
    func_divu,
    func_rem,
    func_remu,
    func_sub,
    func_sra,
    func_lui,
    func_addw,
    func_sllw,
    func_srlw,
    func_mulw,
    func_divw,
    func_divuw,
    func_remw,
    func_remuw,
    func_subw,
    func_sraw,
    func_beq,
    func_bne,
    func_blt,
    func_bge,
    func_bltu,
    func_bgeu,
    func_jalr,
    func_jal,
    func_ecall,
    func_csrrc,
    func_csrrci,
    func_csrrs,
    func_csrrsi,
    func_csrrw,
    func_csrrwi,
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

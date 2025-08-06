#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string>

namespace VmStr {

#define VMSTR_INLINE __forceinline

#define BYTECODE_MAX_SIZE 1000
#define STACK_MAX_SIZE 200

namespace Util {
constexpr int digit_to_int(char c) { return c - '0'; }

inline void _memcpy(volatile void *dst, const volatile void *src,
                    std::size_t size) {
  auto *d = static_cast<volatile uint8_t *>(dst);
  auto *s = static_cast<const volatile uint8_t *>(src);
  for (std::size_t i = 0; i < size; ++i)
    d[i] = s[i];
}

constexpr uint32_t rand_int(uint32_t seed) {
  uint32_t val = seed;
  val = (1103515245u * val + 12345u) & 0x7FFFFFFFu;
  return val;
}
} // namespace Util

namespace Global {

constexpr char _time[] = __TIME__;

constexpr int seed =
    Util::digit_to_int(_time[7]) + Util::digit_to_int(_time[6]) * 10 +
    Util::digit_to_int(_time[4]) * 60 + Util::digit_to_int(_time[3]) * 600 +
    Util::digit_to_int(_time[1]) * 3600 + Util::digit_to_int(_time[0]) * 36000;

consteval std::array<uint8_t, BYTECODE_MAX_SIZE> ET1(uint8_t key) {
  std::array<uint8_t, BYTECODE_MAX_SIZE> ret{};
  for (size_t i = 0; i < BYTECODE_MAX_SIZE - 1; ++i) {
    uint8_t val = Util::rand_int(Global::seed + i) % 255;
    ret[i] = val ^ key;
  }
  return ret;
}

consteval std::array<uint8_t, BYTECODE_MAX_SIZE> ET2() {
  std::array<uint8_t, BYTECODE_MAX_SIZE> ret{};
  for (size_t i = 0; i < BYTECODE_MAX_SIZE - 1; ++i) {
    uint8_t val = Util::rand_int(Global::seed + i) % 255;
    ret[i] = val;
  }
  return ret;
}

constexpr uint8_t GK = Util::rand_int(Global::seed + 0xdeadbeef);

static std::array<uint8_t, BYTECODE_MAX_SIZE> _ET1 = ET1(GK);
constexpr std::array<uint8_t, BYTECODE_MAX_SIZE> _ET2 = ET2();

std::array<uint8_t, BYTECODE_MAX_SIZE> _ET3{};

} // namespace Global

namespace Expression {

#define DEFINE_EXPR(name, body)                                                \
  VMSTR_INLINE constexpr uint8_t cexpr_##name(uint8_t n,                       \
                                              [[maybe_unused]] uint8_t d) body \
                                                                               \
      VMSTR_INLINE uint8_t                                                     \
      name(uint8_t n, [[maybe_unused]] uint8_t d) body

DEFINE_EXPR(_xor, { return n ^ d; })
DEFINE_EXPR(_not, { return ~n; })

DEFINE_EXPR(rotl, {
  constexpr uint8_t INT_BITS = std::numeric_limits<uint8_t>::digits;
  d %= INT_BITS;
  return (n << d) | (n >> (INT_BITS - d));
})

DEFINE_EXPR(rotr, {
  constexpr uint8_t INT_BITS = std::numeric_limits<uint8_t>::digits;
  d %= INT_BITS;
  return (n >> d) | (n << (INT_BITS - d));
})

VMSTR_INLINE uint8_t alt_xor(uint8_t a, uint8_t b) {
  return ((((((~a | ~b) ^ -~a) + (2 * ((~a | ~b) & -~a))) & b) +
           ((((~a | ~b) ^ -~a) + (2 * ((~a | ~b) & -~a))) | b)) -
          ((((~a & ~b) & b) + ((~a & ~b) | b)) - ~a));
}

VMSTR_INLINE uint8_t alt_not(uint8_t a, uint8_t b) {
  return (((~~a | ((((~a | b) - ~a) + ((a & ~b) + b)) -
                   (((~a | b) - ~a) + ((a & ~b) + b)))) -
           ~~a) +
          (~a | ((((~a | b) - ~a) + ((a & ~b) + b)) -
                 (((~a | b) - ~a) + ((a & ~b) + b)))));
}

} // namespace Expression

namespace Vm {

enum OP : uint8_t {
  PUSH,
  XOR,
  NOT,
  ROTR,
  ROTL,

  ALT_XOR,
  ALT_NOT,

  END,

  TERMINATOR,
};

#define INSERT(val)                                                            \
  ret[vip] = val;                                                              \
  vip++;

#define K table[vip]

template <size_t N> consteval auto gen_bytecode(const char (&str)[N]) {
  std::array<uint8_t, BYTECODE_MAX_SIZE> ret{};
  auto table = Global::_ET2;

  int vip = 0;
  for (size_t i = 0; i < N - 1; ++i) {
    OP op = static_cast<OP>(Util::rand_int(Global::seed + i) %
                            (static_cast<uint8_t>(OP::END)));
    if (op == OP::XOR) {
      uint8_t key = Util::rand_int(Global::seed + i) % 255;
      auto val = Expression::cexpr__xor(str[i], key);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::PUSH);
      INSERT(static_cast<char>(key) ^ K);
      INSERT(OP::XOR);
      continue;
    } else if (op == OP::NOT) {
      auto val = Expression::cexpr__not(str[i], 0);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::NOT);
      continue;
    } else if (op == OP::ROTR) {
      uint8_t key = Util::rand_int(Global::seed + i) % 255;
      auto val = Expression::cexpr_rotr(str[i], key);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::PUSH);
      INSERT(static_cast<char>(key) ^ K);
      INSERT(OP::ROTL);
      continue;
    } else if (op == OP::ROTL) {
      uint8_t key = Util::rand_int(Global::seed + i) % 255;
      auto val = Expression::cexpr_rotl(str[i], key);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::PUSH);
      INSERT(static_cast<char>(key) ^ K);
      INSERT(OP::ROTR);
      continue;
    } else if (op == OP::ALT_XOR) {
      uint8_t key = Util::rand_int(Global::seed + i) % 255;
      auto val = Expression::cexpr__xor(str[i], key);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::PUSH);
      INSERT(static_cast<char>(key) ^ K);
      INSERT(OP::ALT_XOR);
      continue;
    } else if (op == OP::ALT_NOT) {
      auto val = Expression::cexpr__not(str[i], 0);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::ALT_NOT);
      continue;
    }

    INSERT(OP::PUSH);
    INSERT(str[i] ^ K);
  }

  ret[vip] = OP::TERMINATOR;

  return ret;
}

#define CHAIN(offset) run<vip + offset, bytecode>(vsp, stack)

template <size_t vip, std::array bytecode>
VMSTR_INLINE void run(volatile size_t &vsp, volatile uint8_t *stack) {
  volatile auto opaque_var =
      Expression::alt_xor(Global::_ET1[vip + 1], Global::_ET1[vip + 1]);

  constexpr OP op = static_cast<OP>(bytecode[vip]);
  if constexpr (op == OP::PUSH) {
    Global::_ET3[vip + 1] = Global::_ET1[vip + 1];
    Global::_ET3[vip + 1] =
        Expression::alt_xor(Global::_ET3[vip + 1], Global::GK);
    volatile auto _key = Global::_ET3[vip + 1];
    uint8_t imm = bytecode[vip + 1] ^ _key;
    Util::_memcpy(stack + vsp, &imm, sizeof(imm));
    vsp += sizeof(imm) + opaque_var;
    CHAIN(2);
  } else if constexpr (op == OP::XOR) {
    volatile auto imm = stack[vsp - 2];
    volatile auto key = stack[vsp - 1];
    vsp -= sizeof(imm) + sizeof(key);
    uint8_t xor_val = Expression::_xor(imm, (key + opaque_var));
    Util::_memcpy(stack + vsp, &xor_val, sizeof(xor_val));
    vsp += sizeof(xor_val) + opaque_var;
    CHAIN(1);
  } else if constexpr (op == OP::NOT) {
    volatile auto imm = stack[vsp - 1];
    vsp -= sizeof(imm);
    volatile uint8_t not_val = Expression::_not(imm + opaque_var, 0);
    Util::_memcpy(stack + vsp, &not_val, sizeof(not_val));
    vsp += sizeof(not_val) + opaque_var;
    CHAIN(1);
  } else if constexpr (op == OP::ROTR) {
    volatile auto imm = stack[vsp - 2];
    volatile auto key = stack[vsp - 1];
    vsp -= sizeof(imm) + sizeof(key);
    volatile uint8_t rotr_val = Expression::rotr(imm, key + opaque_var);
    Util::_memcpy(stack + vsp, &rotr_val, sizeof(rotr_val));
    vsp += sizeof(rotr_val) + opaque_var;
    CHAIN(1);
  } else if constexpr (op == OP::ROTL) {
    volatile auto imm = stack[vsp - 2];
    volatile auto key = stack[vsp - 1];
    vsp -= sizeof(imm) + sizeof(key);
    volatile uint8_t rotl_val = Expression::rotl(imm, key + opaque_var);
    Util::_memcpy(stack + vsp, &rotl_val, sizeof(rotl_val));
    vsp += sizeof(rotl_val) + opaque_var;
    CHAIN(1);
  } else if constexpr (op == OP::ALT_XOR) {
    volatile auto imm = stack[vsp - 2];
    volatile auto key = stack[vsp - 1];
    vsp -= sizeof(imm) + sizeof(key);
    volatile uint8_t mbaxor_val = Expression::alt_xor(imm, key + opaque_var);
    Util::_memcpy(stack + vsp, &mbaxor_val, sizeof(mbaxor_val));
    vsp += sizeof(mbaxor_val) + opaque_var;
    CHAIN(1);
  } else if constexpr (op == OP::ALT_NOT) {
    volatile auto imm = stack[vsp - 1];
    vsp -= sizeof(imm);
    volatile uint8_t not_val = Expression::alt_not(imm + opaque_var, imm);
    Util::_memcpy(stack + vsp, &not_val, sizeof(not_val));
    vsp += sizeof(not_val) + opaque_var;
    CHAIN(1);
  } else if constexpr (op == OP::TERMINATOR) {
    uint8_t terminator = 0x0;
    Util::_memcpy(stack + vsp, &terminator, sizeof(terminator));
    return;
  } else {
    assert(false && "Invalid opcode");
  }
}

template <std::array bytecode> VMSTR_INLINE std::string exec_str() {
  volatile size_t vsp = 0;
  volatile uint8_t stack[STACK_MAX_SIZE];
  run<0, bytecode>(vsp, stack);

  uint8_t const *t = const_cast<uint8_t *>(stack);
  const char *t2 = reinterpret_cast<const char *>(t);
  auto str = std::string(t2);

  return str;
}

template <std::array bytecode> VMSTR_INLINE const char *exec_cstr() {
  volatile size_t vsp = 0;
  static volatile uint8_t stack[STACK_MAX_SIZE];
  run<0, bytecode>(vsp, stack);

  uint8_t const *t = const_cast<uint8_t *>(stack);
  const char *t2 = reinterpret_cast<const char *>(t);

  return t2;
}

} // namespace Vm
} // namespace VmStr

#define vm_str(str) VmStr::Vm::exec_str<VmStr::Vm::gen_bytecode(str)>()
#define vm_cstr(str) VmStr::Vm::exec_cstr<VmStr::Vm::gen_bytecode(str)>()

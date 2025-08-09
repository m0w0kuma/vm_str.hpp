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

template <typename T>
inline void _memcpy(volatile void *dst, const volatile void *src,
                    std::size_t size) {
  auto *d = static_cast<volatile T *>(dst);
  auto *s = static_cast<const volatile T *>(src);
  for (std::size_t i = 0; i < size / sizeof(T); ++i)
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
  template <typename T>                                                        \
  constexpr T cexpr_##name(T n, [[maybe_unused]] T d) body                     \
                                                                               \
      template <typename T>                                                    \
      VMSTR_INLINE T name(T n, [[maybe_unused]] T d) body

DEFINE_EXPR(_xor, { return n ^ d; })
DEFINE_EXPR(_not, { return ~n; })

DEFINE_EXPR(rotl, {
  constexpr T INT_BITS = std::numeric_limits<T>::digits;
  d %= INT_BITS;
  return (n << d) | (n >> (INT_BITS - d));
})

DEFINE_EXPR(rotr, {
  constexpr T INT_BITS = std::numeric_limits<T>::digits;
  d %= INT_BITS;
  return (n >> d) | (n << (INT_BITS - d));
})

template <typename T> VMSTR_INLINE T alt_xor(T a, T b) {
  return ((((((~a | ~b) ^ -~a) + (2 * ((~a | ~b) & -~a))) & b) +
           ((((~a | ~b) ^ -~a) + (2 * ((~a | ~b) & -~a))) | b)) -
          ((((~a & ~b) & b) + ((~a & ~b) | b)) - ~a));
}

template <typename T> VMSTR_INLINE T alt_not(T a, T b) {
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
  ret[vip] = static_cast<T>(val);                                              \
  vip++;

#define K static_cast<T>(table[vip])

template <typename ST, typename T, size_t N>
constexpr auto gen_bytecode(const ST (&str)[N]) {
  std::array<T, BYTECODE_MAX_SIZE> ret{};
  auto table = Global::_ET2;

  int vip = 0;
  for (size_t i = 0; i < N - 1; ++i) {
    OP op = static_cast<OP>(Util::rand_int(Global::seed + i) %
                            (static_cast<uint8_t>(OP::END)));
    if (op == OP::XOR) {
      T key = static_cast<T>(Util::rand_int(Global::seed + i) %
                             std::numeric_limits<T>::max());
      T val = Expression::cexpr__xor<T>(str[i], key);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::PUSH);
      INSERT(static_cast<T>(key) ^ K);
      INSERT(OP::XOR);
      continue;
    } else if (op == OP::NOT) {
      T val = Expression::cexpr__not<T>(str[i], 0);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::NOT);
      continue;
    } else if (op == OP::ROTR) {
      T key = static_cast<T>(Util::rand_int(Global::seed + i) %
                             std::numeric_limits<T>::max());
      T val = Expression::cexpr_rotr<T>(str[i], key);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::PUSH);
      INSERT(static_cast<T>(key) ^ K);
      INSERT(OP::ROTL);
      continue;
    } else if (op == OP::ROTL) {
      T key = static_cast<T>(Util::rand_int(Global::seed + i) %
                             std::numeric_limits<T>::max());
      T val = Expression::cexpr_rotl<T>(str[i], key);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::PUSH);
      INSERT(static_cast<T>(key) ^ K);
      INSERT(OP::ROTR);
      continue;
    } else if (op == OP::ALT_XOR) {
      T key = static_cast<T>(Util::rand_int(Global::seed + i) %
                             std::numeric_limits<T>::max());
      T val = Expression::cexpr__xor<T>(str[i], key);
      INSERT(OP::PUSH);
      INSERT(val ^ K);
      INSERT(OP::PUSH);
      INSERT(static_cast<T>(key) ^ K);
      INSERT(OP::ALT_XOR);
      continue;
    } else if (op == OP::ALT_NOT) {
      auto val = Expression::cexpr__not<T>(str[i], 0);
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

#define CHAIN(offset) run<T, vip + offset, bytecode>(vsp, stack)
#define VSP(x) x + opaque_var

template <typename T, size_t vip, std::array bytecode>
VMSTR_INLINE void run(volatile size_t &vsp, volatile T *stack) {
  volatile uint8_t opaque_var = Expression::alt_xor<uint8_t>(
      Global::_ET1[vip + 1], Global::_ET1[vip + 1]);
  constexpr OP op = static_cast<OP>(bytecode[vip]);
  if constexpr (op == OP::PUSH) {
    Global::_ET3[vip + 1] = Global::_ET1[vip + 1];
    Global::_ET3[vip + 1] =
        Expression::alt_xor<uint8_t>(Global::_ET3[vip + 1], Global::GK);
    volatile uint8_t _key = Global::_ET3[vip + 1];
    T imm = bytecode[vip + 1] ^ static_cast<T>(_key);
    Util::_memcpy<T>(stack + vsp, &imm, sizeof(T));
    vsp += VSP(1);
    CHAIN(2);
  } else if constexpr (op == OP::XOR) {
    volatile T imm = stack[vsp - 2];
    volatile T key = stack[vsp - 1];
    vsp -= VSP(2);
    T xor_val = Expression::_xor<T>(imm, (key + opaque_var));
    Util::_memcpy<T>(stack + vsp, &xor_val, sizeof(T));
    vsp += VSP(1);
    CHAIN(1);
  } else if constexpr (op == OP::NOT) {
    volatile T imm = stack[vsp - 1];
    vsp -= VSP(1);
    volatile T not_val = Expression::_not<T>(imm + opaque_var, 0);
    Util::_memcpy<T>(stack + vsp, &not_val, sizeof(T));
    vsp += VSP(1);
    CHAIN(1);
  } else if constexpr (op == OP::ROTR) {
    volatile T imm = stack[vsp - 2];
    volatile T key = stack[vsp - 1];
    vsp -= VSP(2);
    volatile T rotr_val = Expression::rotr<T>(imm, key + opaque_var);
    Util::_memcpy<T>(stack + vsp, &rotr_val, sizeof(T));
    vsp += VSP(1);
    CHAIN(1);
  } else if constexpr (op == OP::ROTL) {
    volatile T imm = stack[vsp - 2];
    volatile T key = stack[vsp - 1];
    vsp -= VSP(2);
    volatile T rotl_val = Expression::rotl<T>(imm, key + opaque_var);
    Util::_memcpy<T>(stack + vsp, &rotl_val, sizeof(T));
    vsp += VSP(1);
    CHAIN(1);
  } else if constexpr (op == OP::ALT_XOR) {
    volatile T imm = stack[vsp - 2];
    volatile T key = stack[vsp - 1];
    vsp -= VSP(2);
    volatile T mbaxor_val = Expression::alt_xor<T>(imm, key + opaque_var);
    Util::_memcpy<T>(stack + vsp, &mbaxor_val, sizeof(T));
    vsp += VSP(1);
    CHAIN(1);
  } else if constexpr (op == OP::ALT_NOT) {
    volatile T imm = stack[vsp - 1];
    vsp -= VSP(1);
    volatile T not_val = Expression::alt_not<T>(imm + opaque_var, imm);
    Util::_memcpy<T>(stack + vsp, &not_val, sizeof(T));
    vsp += VSP(1);
    CHAIN(1);
  } else if constexpr (op == OP::TERMINATOR) {
    T terminator = 0x0;
    Util::_memcpy<T>(stack + vsp, &terminator, sizeof(T));
    return;
  } else {
    assert(false && "Invalid opcode");
  }
}

template <std::array bytecode> VMSTR_INLINE std::string exec_str() {
  volatile size_t vsp = 0;
  volatile uint8_t stack[STACK_MAX_SIZE];
  run<uint8_t, 0, bytecode>(vsp, stack);

  uint8_t const *t = const_cast<uint8_t *>(stack);
  const char *t2 = reinterpret_cast<const char *>(t);
  auto str = std::string(t2);

  return str;
}

template <std::array bytecode> VMSTR_INLINE const char *exec_cstr() {
  volatile size_t vsp = 0;
  static volatile uint8_t stack[STACK_MAX_SIZE];
  run<uint8_t, 0, bytecode>(vsp, stack);

  uint8_t const *t = const_cast<uint8_t *>(stack);
  const char *t2 = reinterpret_cast<const char *>(t);

  return t2;
}

template <std::array bytecode> VMSTR_INLINE std::wstring exec_wstr() {
  volatile size_t vsp = 0;
  volatile uint16_t stack[STACK_MAX_SIZE];
  run<uint16_t, 0, bytecode>(vsp, stack);

  uint16_t const *t = const_cast<uint16_t *>(stack);
  const wchar_t *t2 = reinterpret_cast<const wchar_t *>(t);
  auto str = std::wstring(t2);

  return str;
}

template <std::array bytecode> VMSTR_INLINE const wchar_t *exec_cwstr() {
  volatile size_t vsp = 0;
  static volatile uint16_t stack[STACK_MAX_SIZE];
  run<uint16_t, 0, bytecode>(vsp, stack);

  uint16_t const *t = const_cast<uint16_t *>(stack);
  const wchar_t *t2 = reinterpret_cast<const wchar_t *>(t);

  return t2;
}

} // namespace Vm
} // namespace VmStr

#define VM_STR(str)                                                            \
  VmStr::Vm::exec_str<VmStr::Vm::gen_bytecode<char, uint8_t>(str)>()
#define VM_CSTR(str)                                                           \
  VmStr::Vm::exec_cstr<VmStr::Vm::gen_bytecode<char, uint8_t>(str)>()

#define VM_W_STR(wstr)                                                         \
  VmStr::Vm::exec_wstr<VmStr::Vm::gen_bytecode<wchar_t, uint16_t>(wstr)>()
#define VM_W_CSTR(wstr)                                                        \
  VmStr::Vm::exec_cwstr<VmStr::Vm::gen_bytecode<wchar_t, uint16_t>(wstr)>()

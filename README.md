# vm_str.hpp
vm_str.hpp is a header only C++20 compile time string obfuscator.

# About
vm_str.hpp is a header-only string obfuscation library, heavily inspired by xorstr. Unlike xorstr, it generates a different encryption scheme for every build. This is achieved using a virtual machine (VM)-like approach with a predefined set of operations.

At compile time (constexpr), the library generates bytecode representing the obfuscation schema. Then, at runtime, a stack-based VM interprets this bytecode to reconstruct the original string.

The string is constructed on the stack at runtime and does not appear anywhere in the executable prior to execution. 

<img width="1365" height="589" alt="Captura de tela 2025-08-06 190129" src="https://github.com/user-attachments/assets/dc40ab72-ae15-4fa4-9b52-424a35104c51" />

*How the string construction appears on IDA decompiler.*

# Use
- `VM_CSTR(...)` to get a pointer to a c-like string.
- `VM_STR(...)` to get a std c++ string.

- `VM_W_CSTR(...)` to get a pointer to a c-like wide string.
- `VM_W_STR(...)` to get a std c++ wide string.

```cpp
#include "vm.hpp"

int main() {
   const char *c_like_string = VM_CSTR("Hello, ");
   std::string cpp_std_string = VM_STR("World!");
   std::cout << c_like_string << cpp_std_string << std::endl;

   const wchar_t *cw_like_string = VM_W_CSTR(L"Hello, ");
   std::wstring cpp_std_wstring = VM_W_STR(L"World!");
   std::wcout << cw_like_string << cpp_std_wstring << std::endl;
}
```

# Features
- Generates different obfuscation schema for every build, making general deobfuscators like [that](https://github.com/yubie-re/ida-jm-xorstr-decrypt-plugin) harder to develop.
- String is constructed on the stack at runtime and does not appear in the `.data` section.
- The string's runtime decryption is purposefully convoluted, making static analysis harder.

# Data types supported
- [x] char*
- [x] std::string
- [x] wchar_t*
- [x] std::wstring

See [Limitations](#Limitations)

# Supported compilers
- [x] msvc

See [Limitations](#Limitations)

# Limitations
- We currently support UTF-8 and UTF-16 strings; other encodings are not supported.  
- No compiler other than MSVC will be supported.
- Builds with C++ standards earlier than C++20 will fail.
- Build time is highly affected by `vm_str.hpp` since it makes extensive use of `constexpr` evaluations to generate the bytecode. Based on anecdotal data, even a single character can increase build time by ~1 second. Runtime performance does not seem to be significantly affected, though.

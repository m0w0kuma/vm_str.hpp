# vm_str.hpp
vm_str.hpp is a header only C++20 compile time string obfuscator.

# About
vm_str.hpp is a header-only string obfuscation library, heavily inspired by xorstr. Unlike xorstr, it generates a different encryption scheme for every build. This is achieved using a virtual machine (VM)-like approach with a predefined set of operations.

At compile time (constexpr), the library generates bytecode representing the obfuscation schema. Then, at runtime, a stack-based VM interprets this bytecode to reconstruct the original string.

The string is constructed on the stack at runtime and does not appear anywhere in the executable prior to execution. 
# Use
- `vm_cstr(...)` to get a pointer to a c-like string.
- `vm_str(...)` to get a std c++ string.

```cpp
#include "vm.hpp"

int main() {
    const char *c_like_string = vm_cstr("Hello, ");
    std::string cpp_std_string = vm_str("World!");
    std::cout << c_like_string << cpp_std_string << std::endl;
}
```

# Features
- Generates different obfuscation schema for every build, making general deobfuscators like [that](https://github.com/yubie-re/ida-jm-xorstr-decrypt-plugin) harder to develop.
- String is constructed on the stack at runtime and does not appear in the `.data` section.
- The string's runtime decryption is purposefully convoluted, making static analysis harder.

# Data types supported
- [x] char*
- [x] std::string
- [ ] wchar_t*
- [ ] std::wstring

See [Limitations](#Limitations)

# Compilers supported
- [x] msvc

See [Limitations](#Limitations)

# Limitations
- Right now this only supports UTF-8 strings; non-UTF-8 (`wchar_t*`, `std::wstring`) may get support in the future.  
- No compiler other than MSVC will be supported.
- Builds with C++ standards earlier than C++20 will fail.
- Build time is highly affected by `vm_str.hpp` since it makes extensive use of `constexpr` evaluations to generate the bytecode. Based on anecdotal data, even a single character can increase build time by ~1 second. Runtime performance does not seem to be significantly affected, though.

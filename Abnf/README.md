# ABNF (Augmented Backus-Naur Form) Library

A high-performance, memory-safe C++ implementation of RFC2234 ABNF grammar notation, designed for use in Finite State Machine (FSM) implementations.

## Features

- **Complete RFC2234 Support**: All core rules (ALPHA, DIGIT, HEXDIG, etc.)
- **Multiple Construction Methods**: 
  - Single characters ('C' or %x43)
  - Character ranges ('A'-'Z' or %x41-5A)
  - Character lists
  - Composite rules
- **High Performance**: O(1) character matching using bitsets
- **Memory Safe**: Modern C++17 with RAII and no raw pointers
- **Type Safe**: Strong typing with comprehensive error handling
- **Set Operations**: Union, intersection, and complement operations
- **Builder Pattern**: For complex rule construction

## Quick Start

### Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
./abnf_example
```

### Basic Usage

```cpp
#include "abnf.hpp"
using namespace fsm;

// Single character matching
ABNF rule_a('A');
bool matches = rule_a.matches('A');  // true

// Core rules
ABNF digit = ABNF::digit();
bool is_digit = digit.matches('5');  // true

// Ranges
ABNF upper_alpha('A', 'Z');
bool is_upper = upper_alpha.matches('M');  // true

// Composite rules
ABNF alphanumeric = {ABNF::digit(), ABNF::alpha()};
bool is_alnum = alphanumeric.matches('7');  // true

// Set operations
ABNF not_digit = ~ABNF::digit();
bool excluded = not_digit.matches('A');  // true
```

## RFC2234 Core Rules

All core rules from RFC2234 are supported:

| Rule | Description | Example |
|------|-------------|---------|
| ALPHA | A-Z / a-z | 'A', 'z' |
| BIT | "0" / "1" | '0', '1' |
| CHAR | %x01-7F | Any 7-bit ASCII (excluding NUL) |
| CR | %x0D | Carriage return |
| CRLF | CR LF | Internet standard newline |
| CTL | %x00-1F / %x7F | Control characters |
| DIGIT | %x30-39 | '0'-'9' |
| DQUOTE | %x22 | Double quote (") |
| HEXDIG | DIGIT / "A"-"F" | '0'-'9', 'A'-'F', 'a'-'f' |
| HTAB | %x09 | Horizontal tab |
| LF | %x0A | Line feed |
| LWSP | WSP / CRLF WSP | Linear white space |
| OCTET | %x00-FF | Any 8-bit value |
| SP | %x20 | Space |
| VCHAR | %x21-7E | Visible characters |
| WSP | SP / HTAB | White space |

## Performance

- **Time Complexity**: O(1) for character matching
- **Space Complexity**: 32 bytes per ABNF object (256-bit bitset)
- **Cache Friendly**: Compact representation fits in L1 cache

## Security & Safety

- Modern C++17 with RAII
- No raw pointers or manual memory management
- Bounds checking on all operations
- Exception safety guarantees
- Compiler warnings as errors
- Stack protection enabled

## Design Rationale

### Bitset Approach

Uses `std::bitset<256>` for character sets, providing:
- O(1) lookup time
- Minimal memory footprint (32 bytes)
- Hardware-optimized bit operations
- Type-safe interface

### Builder Pattern

For complex rules requiring multiple operations:

```cpp
ABNF custom = ABNF::Builder()
    .addCoreRule(ABNF::CoreRule::DIGIT)
    .addChar('-')
    .addRange('A', 'Z')
    .build();
```

## Future: FSM Integration

This ABNF class is designed as the foundation for a Finite State Machine implementation, where:
- ABNF rules define valid transitions
- Character matching determines state changes
- Composite rules model complex state behaviors

## Testing

To build with tests:

```bash
cmake -DBUILD_TESTS=ON ..
cmake --build .
ctest
```

## License

[Your License Here]

## Authors

Created for high-performance protocol parsing and FSM implementations.
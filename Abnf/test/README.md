# ABNF Test Suite

Comprehensive test suite for the ABNF (Augmented Backus-Naur Form) implementation using Google Test.

## Test Coverage

### 1. RFC2234 Core Rules Tests (`RFC2234CoreRulesTest`)
Validates all 16 core rules from RFC2234:
- **ALPHA**: Uppercase and lowercase letters
- **BIT**: Binary digits (0, 1)
- **CHAR**: 7-bit ASCII characters (excluding NUL)
- **CR**: Carriage return
- **LF**: Line feed
- **CRLF**: Internet standard newline
- **CTL**: Control characters
- **DIGIT**: Decimal digits (0-9)
- **DQUOTE**: Double quote
- **HEXDIG**: Hexadecimal digits (0-9, A-F, a-f)
- **HTAB**: Horizontal tab
- **LWSP**: Linear white space
- **OCTET**: All 256 byte values
- **SP**: Space
- **VCHAR**: Visible characters
- **WSP**: White space (space and tab)

### 2. Constructor Tests (`ABNFConstructorTest`)
- Default constructor
- Single character constructor
- Single byte constructor
- Character range constructor
- Byte range constructor
- Core rule constructor
- Character list constructor
- Byte list constructor
- Rule list constructor (union)
- Copy/move constructors and assignment operators
- Invalid range error handling

### 3. Matching Operations Tests (`ABNFMatchingTest`)
- Character matching
- Byte value matching
- Different integral type handling (char, unsigned char, signed char, uint8_t, int8_t)
- Exclusion testing
- Operator() overload

### 4. Set Operations Tests (`ABNFSetOperationsTest`)
- Union operations (`unionWith`, `|`)
- Intersection operations (`intersectWith`, `&`)
- Complement operations (`complement`, `~`)
- Complex combinations
- Edge cases (self-union, self-intersection, complement properties)

### 5. Utility Methods Tests (`ABNFUtilityTest`)
- `isEmpty()` checks
- `count()` verification
- `toString()` descriptions

### 6. Builder Pattern Tests (`ABNFBuilderTest`)
- Empty builds
- Adding individual characters
- Adding ranges
- Adding core rules
- Adding existing rules
- Complex combinations
- Method chaining
- Error handling

### 7. Edge Cases Tests (`ABNFEdgeCasesTest`)
- NUL character handling
- Full byte range (0x00-0xFF)
- Single character ranges
- Extended ASCII (0x80-0xFF)
- Set operation edge cases

### 8. Real-World Use Cases (`ABNFUseCasesTest`)
- URI scheme characters (RFC 3986)
- Base64 character set
- Email local part characters
- IP address digits and dots
- Programming language identifiers

### 9. Performance Tests (`ABNFPerformanceTest`)
- Basic constant-time matching verification
- Set operation completion checks

## Building and Running Tests

### Build with tests (default):
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run tests:
```bash
# Using CTest
ctest --output-on-failure

# Or directly
./abnf_tests

# With verbose output
./abnf_tests --gtest_color=yes

# Run specific test suite
./abnf_tests --gtest_filter=RFC2234CoreRulesTest.*

# Run specific test
./abnf_tests --gtest_filter=RFC2234CoreRulesTest.ALPHA_MatchesUppercaseLetters
```

### Build without tests:
```bash
cmake -DBUILD_TESTS=OFF ..
cmake --build .
```

## Test Statistics

- **Total Test Suites**: 9
- **Total Test Cases**: 100+
- **Core Rules Coverage**: 16/16 (100%)
- **Constructor Coverage**: All variants tested
- **Set Operations**: All combinations tested
- **Edge Cases**: Comprehensive boundary testing

## Test Output Example

```
[==========] Running 100+ tests from 9 test suites.
[----------] Global test environment set-up.
[----------] 16 tests from RFC2234CoreRulesTest
[ RUN      ] RFC2234CoreRulesTest.ALPHA_MatchesUppercaseLetters
[       OK ] RFC2234CoreRulesTest.ALPHA_MatchesUppercaseLetters (0 ms)
...
[==========] 100+ tests from 9 test suites ran. (XX ms total)
[  PASSED  ] 100+ tests.
```

## Adding New Tests

To add new tests, follow this pattern:

```cpp
TEST_F(TestSuiteName, TestName) {
    // Arrange
    ABNF rule = /* create rule */;
    
    // Act
    bool result = rule.matches('X');
    
    // Assert
    EXPECT_TRUE(result);
}
```

### Test Naming Convention
- Test suite names end with `Test`
- Test names use `PascalCase_WithUnderscores`
- Names should be descriptive of what is being tested

### Assertion Guidelines
- Use `EXPECT_*` for non-fatal assertions
- Use `ASSERT_*` for fatal assertions (test stops on failure)
- Always include descriptive messages for complex assertions
- Test both positive and negative cases

## Continuous Integration

These tests can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions
- name: Build and Test
  run: |
    mkdir build && cd build
    cmake ..
    cmake --build .
    ctest --output-on-failure
```

## Coverage Goals

- ✅ All RFC2234 core rules validated
- ✅ All constructors tested
- ✅ All public methods tested
- ✅ Error conditions tested
- ✅ Edge cases covered
- ✅ Real-world use cases demonstrated
- ✅ Performance characteristics verified

## Debugging Failed Tests

1. Run specific test with verbose output:
   ```bash
   ./abnf_tests --gtest_filter=*FailingTest* --gtest_color=yes
   ```

2. Enable detailed logging in test code if needed

3. Check test output for assertion messages

4. Use debugger:
   ```bash
   gdb ./abnf_tests
   (gdb) run --gtest_filter=*FailingTest*
   ```
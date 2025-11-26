#ifndef ABNF_HPP
#define ABNF_HPP

#include <bitset>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace fsm {

/**
 * @brief ABNF (Augmented Backus-Naur Form) class implementing RFC2234 grammar notation
 * 
 * This class provides a high-performance implementation of ABNF rules with support for:
 * - Single character matching
 * - Character ranges (e.g., %x41-5A for A-Z)
 * - Character sets and combinations
 * - Predefined core rules from RFC2234
 * - Composite rules combining multiple ABNF definitions
 */
class ABNF {
public:
    // Forward declarations
    class Builder;
    
    /**
     * @brief Enumeration of RFC2234 core rules
     */
    enum class CoreRule {
        ALPHA,      // A-Z / a-z
        BIT,        // "0" / "1"
        CHAR,       // %x01-7F (any 7-bit US-ASCII character, excluding NUL)
        CR,         // %x0D (carriage return)
        CRLF,       // CR LF (Internet standard newline)
        CTL,        // %x00-1F / %x7F (controls)
        DIGIT,      // %x30-39 (0-9)
        DQUOTE,     // %x22 (double quote)
        HEXDIG,     // DIGIT / "A" / "B" / "C" / "D" / "E" / "F"
        HTAB,       // %x09 (horizontal tab)
        LF,         // %x0A (line feed)
        LWSP,       // *(WSP / CRLF WSP) (linear white space)
        OCTET,      // %x00-FF (8 bits of data)
        SP,         // %x20 (space)
        VCHAR,      // %x21-7E (visible printing characters)
        WSP         // SP / HTAB (white space)
    };

    // ========================================================================
    // Constructors
    // ========================================================================

    /**
     * @brief Default constructor - creates an empty ABNF rule (matches nothing)
     */
    ABNF();

    /**
     * @brief Construct from a single character
     * @param ch The character to match
     */
    explicit ABNF(char ch);

    /**
     * @brief Construct from a single unsigned byte value
     * @param value The byte value to match (e.g., 0x43 for 'C')
     */
    explicit ABNF(uint8_t value);

    /**
     * @brief Construct from a character range (inclusive)
     * @param start Start of the range
     * @param end End of the range (inclusive)
     * @throws std::invalid_argument if start > end
     */
    ABNF(char start, char end);

    /**
     * @brief Construct from a byte value range (inclusive)
     * @param start Start of the range
     * @param end End of the range (inclusive)
     * @throws std::invalid_argument if start > end
     */
    ABNF(uint8_t start, uint8_t end);

    /**
     * @brief Construct from a predefined core rule
     * @param rule The RFC2234 core rule to use
     */
    explicit ABNF(CoreRule rule);

    /**
     * @brief Construct from multiple character values
     * @param chars Initializer list of characters
     */
    ABNF(std::initializer_list<char> chars);

    /**
     * @brief Construct from multiple byte values
     * @param values Initializer list of byte values
     */
    ABNF(std::initializer_list<uint8_t> values);

    /**
     * @brief Construct from a combination of ABNF rules (union)
     * @param rules Initializer list of ABNF rules to combine
     */
    ABNF(std::initializer_list<ABNF> rules);

    /**
     * @brief Copy constructor
     */
    ABNF(const ABNF& other);

    /**
     * @brief Move constructor
     */
    ABNF(ABNF&& other) noexcept;

    /**
     * @brief Copy assignment operator
     */
    ABNF& operator=(const ABNF& other);

    /**
     * @brief Move assignment operator
     */
    ABNF& operator=(ABNF&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~ABNF() = default;

    // ========================================================================
    // Matching Operations
    // ========================================================================

    /**
     * @brief Check if a character matches this ABNF rule
     * @param ch The character to test
     * @return true if the character matches, false otherwise
     */
    [[nodiscard]] bool matches(char ch) const noexcept;

    /**
     * @brief Check if a byte value matches this ABNF rule
     * @param value The byte value to test
     * @return true if the value matches, false otherwise
     */
    [[nodiscard]] bool matches(uint8_t value) const noexcept;

    /**
     * @brief Check if a character is explicitly excluded from this ABNF rule
     * @param ch The character to test
     * @return true if the character does not match, false if it matches
     */
    [[nodiscard]] bool excludes(char ch) const noexcept;

    /**
     * @brief Check if a byte value is explicitly excluded from this ABNF rule
     * @param value The byte value to test
     * @return true if the value does not match, false if it matches
     */
    [[nodiscard]] bool excludes(uint8_t value) const noexcept;

    /**
     * @brief Operator overload for match testing
     * @param ch The character to test
     * @return true if the character matches, false otherwise
     */
    [[nodiscard]] bool operator()(char ch) const noexcept;

    /**
     * @brief Operator overload for match testing
     * @param value The byte value to test
     * @return true if the value matches, false otherwise
     */
    [[nodiscard]] bool operator()(uint8_t value) const noexcept;

    // ========================================================================
    // Set Operations
    // ========================================================================

    /**
     * @brief Create a union of this ABNF rule with another
     * @param other The ABNF rule to union with
     * @return A new ABNF rule representing the union
     */
    [[nodiscard]] ABNF unionWith(const ABNF& other) const;

    /**
     * @brief Create an intersection of this ABNF rule with another
     * @param other The ABNF rule to intersect with
     * @return A new ABNF rule representing the intersection
     */
    [[nodiscard]] ABNF intersectWith(const ABNF& other) const;

    /**
     * @brief Create the complement of this ABNF rule
     * @return A new ABNF rule that matches everything this rule doesn't
     */
    [[nodiscard]] ABNF complement() const;

    /**
     * @brief Union operator
     */
    [[nodiscard]] ABNF operator|(const ABNF& other) const;

    /**
     * @brief Intersection operator
     */
    [[nodiscard]] ABNF operator&(const ABNF& other) const;

    /**
     * @brief Complement operator
     */
    [[nodiscard]] ABNF operator~() const;

    // ========================================================================
    // Utility Methods
    // ========================================================================

    /**
     * @brief Check if this rule matches nothing
     * @return true if the rule matches no characters, false otherwise
     */
    [[nodiscard]] bool isEmpty() const noexcept;

    /**
     * @brief Get the number of possible values this rule matches
     * @return Count of matching values (0-256)
     */
    [[nodiscard]] size_t count() const noexcept;

    /**
     * @brief Get a string representation of this ABNF rule
     * @return String description of the rule
     */
    [[nodiscard]] std::string toString() const;

    // ========================================================================
    // Factory Methods for Core Rules
    // ========================================================================

    static ABNF alpha();
    static ABNF bit();
    static ABNF charRule();
    static ABNF cr();
    static ABNF crlf();
    static ABNF ctl();
    static ABNF digit();
    static ABNF dquote();
    static ABNF hexdig();
    static ABNF htab();
    static ABNF lf();
    static ABNF lwsp();
    static ABNF octet();
    static ABNF sp();
    static ABNF vchar();
    static ABNF wsp();

private:
    // Use a bitset for O(1) lookup performance with minimal memory (32 bytes)
    std::bitset<256> char_set_;
    std::string description_; // For debugging and toString()

    /**
     * @brief Set a single bit in the character set
     */
    void setBit(uint8_t value) noexcept;

    /**
     * @brief Set a range of bits in the character set
     */
    void setRange(uint8_t start, uint8_t end);

    /**
     * @brief Clear all bits
     */
    void clear() noexcept;

    /**
     * @brief Initialize from a core rule
     */
    void initFromCoreRule(CoreRule rule);

    /**
     * @brief Generate description string for a core rule
     */
    static std::string coreRuleToString(CoreRule rule);
};

// ============================================================================
// Builder Pattern for Complex ABNF Rules
// ============================================================================

/**
 * @brief Builder class for constructing complex ABNF rules
 */
class ABNF::Builder {
public:
    Builder() = default;

    Builder& addChar(char ch);
    Builder& addChar(uint8_t value);
    Builder& addRange(char start, char end);
    Builder& addRange(uint8_t start, uint8_t end);
    Builder& addRule(const ABNF& rule);
    Builder& addCoreRule(CoreRule rule);
    
    [[nodiscard]] ABNF build() const;

private:
    ABNF abnf_;
};

} // namespace fsm

#endif // ABNF_HPP
#include <abnf/abnf.hpp>
#include <sstream>
#include <iomanip>

namespace fsm {

// ============================================================================
// Constructors
// ============================================================================

ABNF::ABNF() : char_set_(), description_("EMPTY") {}

ABNF::ABNF(char ch) : char_set_(), description_() {
    setBit(static_cast<uint8_t>(ch));
    std::ostringstream oss;
    if (ch >= 0x20 && ch <= 0x7E && ch != '\'') {
        oss << "'" << ch << "'";
    } else {
        oss << "%x" << std::hex << std::uppercase << std::setw(2) 
            << std::setfill('0') << static_cast<int>(static_cast<uint8_t>(ch));
    }
    description_ = oss.str();
}

ABNF::ABNF(uint8_t value) : char_set_(), description_() {
    setBit(value);
    std::ostringstream oss;
    oss << "%x" << std::hex << std::uppercase << std::setw(2) 
        << std::setfill('0') << static_cast<int>(value);
    description_ = oss.str();
}

ABNF::ABNF(char start, char end) : char_set_(), description_() {
    if (start > end) {
        throw std::invalid_argument("ABNF range: start must be <= end");
    }
    setRange(static_cast<uint8_t>(start), static_cast<uint8_t>(end));
    
    std::ostringstream oss;
    oss << "%x" << std::hex << std::uppercase << std::setw(2) 
        << std::setfill('0') << static_cast<int>(static_cast<uint8_t>(start))
        << "-" << std::setw(2) << std::setfill('0') 
        << static_cast<int>(static_cast<uint8_t>(end));
    description_ = oss.str();
}

ABNF::ABNF(uint8_t start, uint8_t end) : char_set_(), description_() {
    if (start > end) {
        throw std::invalid_argument("ABNF range: start must be <= end");
    }
    setRange(start, end);
    
    std::ostringstream oss;
    oss << "%x" << std::hex << std::uppercase << std::setw(2) 
        << std::setfill('0') << static_cast<int>(start)
        << "-" << std::setw(2) << std::setfill('0') << static_cast<int>(end);
    description_ = oss.str();
}

ABNF::ABNF(CoreRule rule) : char_set_(), description_() {
    initFromCoreRule(rule);
    description_ = coreRuleToString(rule);
}

ABNF::ABNF(std::initializer_list<char> chars) : char_set_(), description_() {
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (char ch : chars) {
        setBit(static_cast<uint8_t>(ch));
        if (!first) oss << ", ";
        if (ch >= 0x20 && ch <= 0x7E && ch != '\'') {
            oss << "'" << ch << "'";
        } else {
            oss << "%x" << std::hex << std::uppercase << std::setw(2) 
                << std::setfill('0') << static_cast<int>(static_cast<uint8_t>(ch));
        }
        first = false;
    }
    oss << "]";
    description_ = oss.str();
}

ABNF::ABNF(std::initializer_list<uint8_t> values) : char_set_(), description_() {
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (uint8_t value : values) {
        setBit(value);
        if (!first) oss << ", ";
        oss << "%x" << std::hex << std::uppercase << std::setw(2) 
            << std::setfill('0') << static_cast<int>(value);
        first = false;
    }
    oss << "]";
    description_ = oss.str();
}

ABNF::ABNF(std::initializer_list<ABNF> rules) : char_set_(), description_() {
    std::ostringstream oss;
    oss << "(";
    bool first = true;
    for (const auto& rule : rules) {
        char_set_ |= rule.char_set_;
        if (!first) oss << " / ";
        oss << rule.description_;
        first = false;
    }
    oss << ")";
    description_ = oss.str();
}

ABNF::ABNF(const ABNF& other) 
    : char_set_(other.char_set_), description_(other.description_) {}

ABNF::ABNF(ABNF&& other) noexcept 
    : char_set_(std::move(other.char_set_)), 
      description_(std::move(other.description_)) {}

ABNF& ABNF::operator=(const ABNF& other) {
    if (this != &other) {
        char_set_ = other.char_set_;
        description_ = other.description_;
    }
    return *this;
}

ABNF& ABNF::operator=(ABNF&& other) noexcept {
    if (this != &other) {
        char_set_ = std::move(other.char_set_);
        description_ = std::move(other.description_);
    }
    return *this;
}

// ============================================================================
// Matching Operations
// ============================================================================

bool ABNF::matches(char ch) const noexcept {
    return char_set_[static_cast<uint8_t>(ch)];
}

bool ABNF::matches(uint8_t value) const noexcept {
    return char_set_[value];
}

bool ABNF::excludes(char ch) const noexcept {
    return !matches(ch);
}

bool ABNF::excludes(uint8_t value) const noexcept {
    return !matches(value);
}

bool ABNF::operator()(char ch) const noexcept {
    return matches(ch);
}

bool ABNF::operator()(uint8_t value) const noexcept {
    return matches(value);
}

// ============================================================================
// Set Operations
// ============================================================================

ABNF ABNF::unionWith(const ABNF& other) const {
    ABNF result;
    result.char_set_ = char_set_ | other.char_set_;
    result.description_ = "(" + description_ + " / " + other.description_ + ")";
    return result;
}

ABNF ABNF::intersectWith(const ABNF& other) const {
    ABNF result;
    result.char_set_ = char_set_ & other.char_set_;
    result.description_ = "(" + description_ + " & " + other.description_ + ")";
    return result;
}

ABNF ABNF::complement() const {
    ABNF result;
    result.char_set_ = ~char_set_;
    result.description_ = "~(" + description_ + ")";
    return result;
}

ABNF ABNF::operator|(const ABNF& other) const {
    return unionWith(other);
}

ABNF ABNF::operator&(const ABNF& other) const {
    return intersectWith(other);
}

ABNF ABNF::operator~() const {
    return complement();
}

// ============================================================================
// Utility Methods
// ============================================================================

bool ABNF::isEmpty() const noexcept {
    return char_set_.none();
}

size_t ABNF::count() const noexcept {
    return char_set_.count();
}

std::string ABNF::toString() const {
    return description_;
}

// ============================================================================
// Factory Methods for Core Rules
// ============================================================================

ABNF ABNF::alpha() {
    return ABNF(CoreRule::ALPHA);
}

ABNF ABNF::bit() {
    return ABNF(CoreRule::BIT);
}

ABNF ABNF::charRule() {
    return ABNF(CoreRule::CHAR);
}

ABNF ABNF::cr() {
    return ABNF(CoreRule::CR);
}

ABNF ABNF::crlf() {
    return ABNF(CoreRule::CRLF);
}

ABNF ABNF::ctl() {
    return ABNF(CoreRule::CTL);
}

ABNF ABNF::digit() {
    return ABNF(CoreRule::DIGIT);
}

ABNF ABNF::dquote() {
    return ABNF(CoreRule::DQUOTE);
}

ABNF ABNF::hexdig() {
    return ABNF(CoreRule::HEXDIG);
}

ABNF ABNF::htab() {
    return ABNF(CoreRule::HTAB);
}

ABNF ABNF::lf() {
    return ABNF(CoreRule::LF);
}

ABNF ABNF::lwsp() {
    return ABNF(CoreRule::LWSP);
}

ABNF ABNF::octet() {
    return ABNF(CoreRule::OCTET);
}

ABNF ABNF::sp() {
    return ABNF(CoreRule::SP);
}

ABNF ABNF::vchar() {
    return ABNF(CoreRule::VCHAR);
}

ABNF ABNF::wsp() {
    return ABNF(CoreRule::WSP);
}

// ============================================================================
// Private Methods
// ============================================================================

void ABNF::setBit(uint8_t value) noexcept {
    char_set_.set(value);
}

void ABNF::setRange(uint8_t start, uint8_t end) {
    for (unsigned int i = start; i <= end; ++i) {
        char_set_.set(i);
    }
}

void ABNF::clear() noexcept {
    char_set_.reset();
}

void ABNF::initFromCoreRule(CoreRule rule) {
    switch (rule) {
        case CoreRule::ALPHA:
            // A-Z / a-z
            setRange(0x41, 0x5A);  // A-Z
            setRange(0x61, 0x7A);  // a-z
            break;
            
        case CoreRule::BIT:
            // "0" / "1"
            setBit(0x30);  // '0'
            setBit(0x31);  // '1'
            break;
            
        case CoreRule::CHAR:
            // %x01-7F (any 7-bit US-ASCII character, excluding NUL)
            setRange(0x01, 0x7F);
            break;
            
        case CoreRule::CR:
            // %x0D (carriage return)
            setBit(0x0D);
            break;
            
        case CoreRule::CRLF:
            // CR LF - Note: This is a sequence, but for single char matching
            // we include both CR and LF
            setBit(0x0D);  // CR
            setBit(0x0A);  // LF
            break;
            
        case CoreRule::CTL:
            // %x00-1F / %x7F (controls)
            setRange(0x00, 0x1F);
            setBit(0x7F);
            break;
            
        case CoreRule::DIGIT:
            // %x30-39 (0-9)
            setRange(0x30, 0x39);
            break;
            
        case CoreRule::DQUOTE:
            // %x22 (double quote)
            setBit(0x22);
            break;
            
        case CoreRule::HEXDIG:
            // DIGIT / "A" / "B" / "C" / "D" / "E" / "F"
            setRange(0x30, 0x39);  // 0-9
            setRange(0x41, 0x46);  // A-F
            setRange(0x61, 0x66);  // a-f (commonly accepted)
            break;
            
        case CoreRule::HTAB:
            // %x09 (horizontal tab)
            setBit(0x09);
            break;
            
        case CoreRule::LF:
            // %x0A (line feed)
            setBit(0x0A);
            break;
            
        case CoreRule::LWSP:
            // *(WSP / CRLF WSP) - linear white space
            // For single char matching: WSP
            setBit(0x20);  // SP
            setBit(0x09);  // HTAB
            break;
            
        case CoreRule::OCTET:
            // %x00-FF (8 bits of data)
            setRange(0x00, 0xFF);
            break;
            
        case CoreRule::SP:
            // %x20 (space)
            setBit(0x20);
            break;
            
        case CoreRule::VCHAR:
            // %x21-7E (visible printing characters)
            setRange(0x21, 0x7E);
            break;
            
        case CoreRule::WSP:
            // SP / HTAB (white space)
            setBit(0x20);  // SP
            setBit(0x09);  // HTAB
            break;
    }
}

std::string ABNF::coreRuleToString(CoreRule rule) {
    switch (rule) {
        case CoreRule::ALPHA:   return "ALPHA";
        case CoreRule::BIT:     return "BIT";
        case CoreRule::CHAR:    return "CHAR";
        case CoreRule::CR:      return "CR";
        case CoreRule::CRLF:    return "CRLF";
        case CoreRule::CTL:     return "CTL";
        case CoreRule::DIGIT:   return "DIGIT";
        case CoreRule::DQUOTE:  return "DQUOTE";
        case CoreRule::HEXDIG:  return "HEXDIG";
        case CoreRule::HTAB:    return "HTAB";
        case CoreRule::LF:      return "LF";
        case CoreRule::LWSP:    return "LWSP";
        case CoreRule::OCTET:   return "OCTET";
        case CoreRule::SP:      return "SP";
        case CoreRule::VCHAR:   return "VCHAR";
        case CoreRule::WSP:     return "WSP";
        default:                return "UNKNOWN";
    }
}

// ============================================================================
// Builder Implementation
// ============================================================================

ABNF::Builder& ABNF::Builder::addChar(char ch) {
    abnf_.setBit(static_cast<uint8_t>(ch));
    return *this;
}

ABNF::Builder& ABNF::Builder::addChar(uint8_t value) {
    abnf_.setBit(value);
    return *this;
}

ABNF::Builder& ABNF::Builder::addRange(char start, char end) {
    if (start > end) {
        throw std::invalid_argument("Builder::addRange: start must be <= end");
    }
    abnf_.setRange(static_cast<uint8_t>(start), static_cast<uint8_t>(end));
    return *this;
}

ABNF::Builder& ABNF::Builder::addRange(uint8_t start, uint8_t end) {
    if (start > end) {
        throw std::invalid_argument("Builder::addRange: start must be <= end");
    }
    abnf_.setRange(start, end);
    return *this;
}

ABNF::Builder& ABNF::Builder::addRule(const ABNF& rule) {
    abnf_.char_set_ |= rule.char_set_;
    return *this;
}

ABNF::Builder& ABNF::Builder::addCoreRule(CoreRule rule) {
    ABNF temp(rule);
    abnf_.char_set_ |= temp.char_set_;
    return *this;
}

ABNF ABNF::Builder::build() const {
    return abnf_;
}

} // namespace fsm
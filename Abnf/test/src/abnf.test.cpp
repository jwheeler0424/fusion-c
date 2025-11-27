#include <gtest/gtest.h>
#include <abnf/abnf.hpp>
#include <string>
#include <vector>

using namespace abnf;

// ============================================================================
// Test Fixture for ABNF Tests
// ============================================================================

class ABNFTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// ============================================================================
// RFC2234 Core Rules Tests
// ============================================================================

class RFC2234CoreRulesTest : public ABNFTest {};

TEST_F(RFC2234CoreRulesTest, ALPHA_MatchesUppercaseLetters) {
    ABNF alpha = ABNF::alpha();
    
    // Test all uppercase letters A-Z
    for (char ch = 'A'; ch <= 'Z'; ++ch) {
        EXPECT_TRUE(alpha.matches(ch)) 
            << "ALPHA should match '" << ch << "'";
    }
}

TEST_F(RFC2234CoreRulesTest, ALPHA_MatchesLowercaseLetters) {
    ABNF alpha = ABNF::alpha();
    
    // Test all lowercase letters a-z
    for (char ch = 'a'; ch <= 'z'; ++ch) {
        EXPECT_TRUE(alpha.matches(ch)) 
            << "ALPHA should match '" << ch << "'";
    }
}

TEST_F(RFC2234CoreRulesTest, ALPHA_DoesNotMatchDigits) {
    ABNF alpha = ABNF::alpha();
    
    for (char ch = '0'; ch <= '9'; ++ch) {
        EXPECT_FALSE(alpha.matches(ch)) 
            << "ALPHA should not match digit '" << ch << "'";
    }
}

TEST_F(RFC2234CoreRulesTest, ALPHA_DoesNotMatchSpecialCharacters) {
    ABNF alpha = ABNF::alpha();
    
    std::string special_chars = "!@#$%^&*()_+-=[]{}|;:,.<>?/~`\" \t\n\r";
    for (char ch : special_chars) {
        EXPECT_FALSE(alpha.matches(ch)) 
            << "ALPHA should not match special char '" << ch << "'";
    }
}

TEST_F(RFC2234CoreRulesTest, ALPHA_Count) {
    ABNF alpha = ABNF::alpha();
    EXPECT_EQ(52, alpha.count()) << "ALPHA should match exactly 52 characters (A-Z, a-z)";
}

TEST_F(RFC2234CoreRulesTest, BIT_MatchesZeroAndOne) {
    ABNF bit = ABNF::bit();
    
    EXPECT_TRUE(bit.matches('0'));
    EXPECT_TRUE(bit.matches('1'));
}

TEST_F(RFC2234CoreRulesTest, BIT_DoesNotMatchOtherDigits) {
    ABNF bit = ABNF::bit();
    
    for (char ch = '2'; ch <= '9'; ++ch) {
        EXPECT_FALSE(bit.matches(ch)) 
            << "BIT should not match '" << ch << "'";
    }
}

TEST_F(RFC2234CoreRulesTest, BIT_Count) {
    ABNF bit = ABNF::bit();
    EXPECT_EQ(2, bit.count()) << "BIT should match exactly 2 characters";
}

TEST_F(RFC2234CoreRulesTest, CHAR_Matches7BitASCII) {
    ABNF char_rule = ABNF::charRule();
    
    // Should match %x01-7F (excluding NUL)
    for (unsigned int i = 0x01; i <= 0x7F; ++i) {
        EXPECT_TRUE(char_rule.matches(static_cast<uint8_t>(i)))
            << "CHAR should match 0x" << std::hex << i;
    }
}

TEST_F(RFC2234CoreRulesTest, CHAR_DoesNotMatchNUL) {
    ABNF char_rule = ABNF::charRule();
    EXPECT_FALSE(char_rule.matches(0x00)) << "CHAR should not match NUL";
}

TEST_F(RFC2234CoreRulesTest, CHAR_DoesNotMatchExtendedASCII) {
    ABNF char_rule = ABNF::charRule();
    
    // Should not match %x80-FF
    for (unsigned int i = 0x80; i <= 0xFF; ++i) {
        EXPECT_FALSE(char_rule.matches(static_cast<uint8_t>(i)))
            << "CHAR should not match extended ASCII 0x" << std::hex << i;
    }
}

TEST_F(RFC2234CoreRulesTest, CHAR_Count) {
    ABNF char_rule = ABNF::charRule();
    EXPECT_EQ(127, char_rule.count()) << "CHAR should match 127 characters (0x01-0x7F)";
}

TEST_F(RFC2234CoreRulesTest, CR_MatchesCarriageReturn) {
    ABNF cr = ABNF::cr();
    
    EXPECT_TRUE(cr.matches('\r'));
    EXPECT_TRUE(cr.matches(0x0D));
    EXPECT_EQ(1, cr.count());
}

TEST_F(RFC2234CoreRulesTest, CR_DoesNotMatchOtherCharacters) {
    ABNF cr = ABNF::cr();
    
    EXPECT_FALSE(cr.matches('\n'));
    EXPECT_FALSE(cr.matches('\t'));
    EXPECT_FALSE(cr.matches(' '));
}

TEST_F(RFC2234CoreRulesTest, LF_MatchesLineFeed) {
    ABNF lf = ABNF::lf();
    
    EXPECT_TRUE(lf.matches('\n'));
    EXPECT_TRUE(lf.matches(0x0A));
    EXPECT_EQ(1, lf.count());
}

TEST_F(RFC2234CoreRulesTest, CRLF_MatchesBothCRAndLF) {
    ABNF crlf = ABNF::crlf();
    
    // Note: CRLF in our implementation matches both CR and LF individually
    // This is for character-level matching; sequence matching is abnf's job
    EXPECT_TRUE(crlf.matches('\r'));
    EXPECT_TRUE(crlf.matches('\n'));
    EXPECT_EQ(2, crlf.count());
}

TEST_F(RFC2234CoreRulesTest, CTL_MatchesControlCharacters) {
    ABNF ctl = ABNF::ctl();
    
    // Should match %x00-1F
    for (unsigned int i = 0x00; i <= 0x1F; ++i) {
        EXPECT_TRUE(ctl.matches(static_cast<uint8_t>(i)))
            << "CTL should match control char 0x" << std::hex << i;
    }
    
    // Should also match DEL (0x7F)
    EXPECT_TRUE(ctl.matches(0x7F)) << "CTL should match DEL (0x7F)";
}

TEST_F(RFC2234CoreRulesTest, CTL_DoesNotMatchPrintableCharacters) {
    ABNF ctl = ABNF::ctl();
    
    // Should not match %x20-7E (printable characters)
    for (unsigned int i = 0x20; i <= 0x7E; ++i) {
        EXPECT_FALSE(ctl.matches(static_cast<uint8_t>(i)))
            << "CTL should not match printable char 0x" << std::hex << i;
    }
}

TEST_F(RFC2234CoreRulesTest, CTL_Count) {
    ABNF ctl = ABNF::ctl();
    EXPECT_EQ(33, ctl.count()) << "CTL should match 33 characters (0x00-0x1F + 0x7F)";
}

TEST_F(RFC2234CoreRulesTest, DIGIT_MatchesAllDigits) {
    ABNF digit = ABNF::digit();
    
    for (char ch = '0'; ch <= '9'; ++ch) {
        EXPECT_TRUE(digit.matches(ch)) 
            << "DIGIT should match '" << ch << "'";
    }
}

TEST_F(RFC2234CoreRulesTest, DIGIT_DoesNotMatchLetters) {
    ABNF digit = ABNF::digit();
    
    EXPECT_FALSE(digit.matches('A'));
    EXPECT_FALSE(digit.matches('Z'));
    EXPECT_FALSE(digit.matches('a'));
    EXPECT_FALSE(digit.matches('z'));
}

TEST_F(RFC2234CoreRulesTest, DIGIT_Count) {
    ABNF digit = ABNF::digit();
    EXPECT_EQ(10, digit.count()) << "DIGIT should match exactly 10 characters";
}

TEST_F(RFC2234CoreRulesTest, DQUOTE_MatchesDoubleQuote) {
    ABNF dquote = ABNF::dquote();
    
    EXPECT_TRUE(dquote.matches('"'));
    EXPECT_TRUE(dquote.matches(0x22));
    EXPECT_EQ(1, dquote.count());
}

TEST_F(RFC2234CoreRulesTest, HEXDIG_MatchesHexDigits) {
    ABNF hexdig = ABNF::hexdig();
    
    // Test 0-9
    for (char ch = '0'; ch <= '9'; ++ch) {
        EXPECT_TRUE(hexdig.matches(ch)) 
            << "HEXDIG should match '" << ch << "'";
    }
    
    // Test A-F
    for (char ch = 'A'; ch <= 'F'; ++ch) {
        EXPECT_TRUE(hexdig.matches(ch)) 
            << "HEXDIG should match '" << ch << "'";
    }
    
    // Test a-f (commonly accepted)
    for (char ch = 'a'; ch <= 'f'; ++ch) {
        EXPECT_TRUE(hexdig.matches(ch)) 
            << "HEXDIG should match '" << ch << "'";
    }
}

TEST_F(RFC2234CoreRulesTest, HEXDIG_DoesNotMatchInvalidHex) {
    ABNF hexdig = ABNF::hexdig();
    
    EXPECT_FALSE(hexdig.matches('G'));
    EXPECT_FALSE(hexdig.matches('g'));
    EXPECT_FALSE(hexdig.matches('Z'));
    EXPECT_FALSE(hexdig.matches('z'));
}

TEST_F(RFC2234CoreRulesTest, HEXDIG_Count) {
    ABNF hexdig = ABNF::hexdig();
    EXPECT_EQ(22, hexdig.count()) << "HEXDIG should match 22 characters (0-9, A-F, a-f)";
}

TEST_F(RFC2234CoreRulesTest, HTAB_MatchesHorizontalTab) {
    ABNF htab = ABNF::htab();
    
    EXPECT_TRUE(htab.matches('\t'));
    EXPECT_TRUE(htab.matches(0x09));
    EXPECT_EQ(1, htab.count());
}

TEST_F(RFC2234CoreRulesTest, LWSP_MatchesWhitespace) {
    ABNF lwsp = ABNF::lwsp();
    
    // LWSP should match space and tab for single-character matching
    EXPECT_TRUE(lwsp.matches(' '));
    EXPECT_TRUE(lwsp.matches('\t'));
    EXPECT_EQ(2, lwsp.count());
}

TEST_F(RFC2234CoreRulesTest, OCTET_MatchesAllBytes) {
    ABNF octet = ABNF::octet();
    
    // Should match all 256 possible byte values
    for (unsigned int i = 0; i <= 255; ++i) {
        EXPECT_TRUE(octet.matches(static_cast<uint8_t>(i)))
            << "OCTET should match 0x" << std::hex << i;
    }
    
    EXPECT_EQ(256, octet.count());
}

TEST_F(RFC2234CoreRulesTest, SP_MatchesSpace) {
    ABNF sp = ABNF::sp();
    
    EXPECT_TRUE(sp.matches(' '));
    EXPECT_TRUE(sp.matches(0x20));
    EXPECT_EQ(1, sp.count());
}

TEST_F(RFC2234CoreRulesTest, SP_DoesNotMatchTab) {
    ABNF sp = ABNF::sp();
    EXPECT_FALSE(sp.matches('\t'));
}

TEST_F(RFC2234CoreRulesTest, VCHAR_MatchesVisibleCharacters) {
    ABNF vchar = ABNF::vchar();
    
    // Should match %x21-7E (visible printing characters)
    for (unsigned int i = 0x21; i <= 0x7E; ++i) {
        EXPECT_TRUE(vchar.matches(static_cast<uint8_t>(i)))
            << "VCHAR should match visible char 0x" << std::hex << i;
    }
}

TEST_F(RFC2234CoreRulesTest, VCHAR_DoesNotMatchControlOrSpace) {
    ABNF vchar = ABNF::vchar();
    
    EXPECT_FALSE(vchar.matches(' ')) << "VCHAR should not match space";
    EXPECT_FALSE(vchar.matches('\t')) << "VCHAR should not match tab";
    EXPECT_FALSE(vchar.matches('\r')) << "VCHAR should not match CR";
    EXPECT_FALSE(vchar.matches('\n')) << "VCHAR should not match LF";
    EXPECT_FALSE(vchar.matches(0x00)) << "VCHAR should not match NUL";
}

TEST_F(RFC2234CoreRulesTest, VCHAR_Count) {
    ABNF vchar = ABNF::vchar();
    EXPECT_EQ(94, vchar.count()) << "VCHAR should match 94 characters (0x21-0x7E)";
}

TEST_F(RFC2234CoreRulesTest, WSP_MatchesSpaceAndTab) {
    ABNF wsp = ABNF::wsp();
    
    EXPECT_TRUE(wsp.matches(' '));
    EXPECT_TRUE(wsp.matches('\t'));
    EXPECT_EQ(2, wsp.count());
}

TEST_F(RFC2234CoreRulesTest, WSP_DoesNotMatchOtherWhitespace) {
    ABNF wsp = ABNF::wsp();
    
    EXPECT_FALSE(wsp.matches('\r'));
    EXPECT_FALSE(wsp.matches('\n'));
}

// ============================================================================
// Constructor Tests
// ============================================================================

class ABNFConstructorTest : public ABNFTest {};

TEST_F(ABNFConstructorTest, DefaultConstructor_IsEmpty) {
    ABNF empty;
    
    EXPECT_TRUE(empty.isEmpty());
    EXPECT_EQ(0, empty.count());
}

TEST_F(ABNFConstructorTest, DefaultConstructor_MatchesNothing) {
    ABNF empty;
    
    for (unsigned int i = 0; i <= 255; ++i) {
        EXPECT_FALSE(empty.matches(static_cast<uint8_t>(i)))
            << "Empty ABNF should not match 0x" << std::hex << i;
    }
}

TEST_F(ABNFConstructorTest, SingleCharConstructor_MatchesExactChar) {
    ABNF rule('A');
    
    EXPECT_TRUE(rule.matches('A'));
    EXPECT_FALSE(rule.matches('B'));
    EXPECT_FALSE(rule.matches('a'));
    EXPECT_EQ(1, rule.count());
}

TEST_F(ABNFConstructorTest, SingleCharConstructor_SpecialCharacters) {
    ABNF question('?');
    ABNF exclaim('!');
    ABNF space(' ');
    
    EXPECT_TRUE(question.matches('?'));
    EXPECT_TRUE(exclaim.matches('!'));
    EXPECT_TRUE(space.matches(' '));
}

TEST_F(ABNFConstructorTest, SingleByteConstructor_MatchesExactByte) {
    ABNF rule(static_cast<uint8_t>(0x43)); // 'C'
    
    EXPECT_TRUE(rule.matches(0x43));
    EXPECT_TRUE(rule.matches('C'));
    EXPECT_FALSE(rule.matches(0x44));
    EXPECT_EQ(1, rule.count());
}

TEST_F(ABNFConstructorTest, CharRangeConstructor_MatchesRange) {
    ABNF range('A', 'Z');
    
    // Test boundaries
    EXPECT_TRUE(range.matches('A'));
    EXPECT_TRUE(range.matches('Z'));
    
    // Test middle values
    EXPECT_TRUE(range.matches('M'));
    EXPECT_TRUE(range.matches('G'));
    
    // Test outside range
    EXPECT_FALSE(range.matches('a'));
    EXPECT_FALSE(range.matches('0'));
    
    EXPECT_EQ(26, range.count());
}

TEST_F(ABNFConstructorTest, CharRangeConstructor_InvalidRange_Throws) {
    EXPECT_THROW({
        ABNF bad_range('Z', 'A');
    }, std::invalid_argument);
}

TEST_F(ABNFConstructorTest, ByteRangeConstructor_MatchesRange) {
    ABNF range(static_cast<uint8_t>(0x30), static_cast<uint8_t>(0x39)); // 0-9
    
    for (char ch = '0'; ch <= '9'; ++ch) {
        EXPECT_TRUE(range.matches(ch));
    }
    
    EXPECT_FALSE(range.matches('A'));
    EXPECT_EQ(10, range.count());
}

TEST_F(ABNFConstructorTest, ByteRangeConstructor_InvalidRange_Throws) {
    EXPECT_THROW({
        ABNF bad_range(static_cast<uint8_t>(0xFF), static_cast<uint8_t>(0x00));
    }, std::invalid_argument);
}

TEST_F(ABNFConstructorTest, CoreRuleConstructor) {
    ABNF digit(ABNF::CoreRule::DIGIT);
    
    EXPECT_TRUE(digit.matches('5'));
    EXPECT_FALSE(digit.matches('A'));
    EXPECT_EQ(10, digit.count());
}

TEST_F(ABNFConstructorTest, CharListConstructor) {
    ABNF punctuation = {'!', '?', '.', ','};
    
    EXPECT_TRUE(punctuation.matches('!'));
    EXPECT_TRUE(punctuation.matches('?'));
    EXPECT_TRUE(punctuation.matches('.'));
    EXPECT_TRUE(punctuation.matches(','));
    EXPECT_FALSE(punctuation.matches(';'));
    EXPECT_EQ(4, punctuation.count());
}

TEST_F(ABNFConstructorTest, ByteListConstructor) {
    ABNF bytes = {static_cast<uint8_t>(0x00), static_cast<uint8_t>(0xFF), 
                  static_cast<uint8_t>(0x7F)};
    
    EXPECT_TRUE(bytes.matches(0x00));
    EXPECT_TRUE(bytes.matches(0xFF));
    EXPECT_TRUE(bytes.matches(0x7F));
    EXPECT_FALSE(bytes.matches(0x01));
    EXPECT_EQ(3, bytes.count());
}

TEST_F(ABNFConstructorTest, RuleListConstructor_Union) {
    ABNF combo = {ABNF::digit(), ABNF('A'), ABNF('?')};
    
    // Should match all digits
    for (char ch = '0'; ch <= '9'; ++ch) {
        EXPECT_TRUE(combo.matches(ch));
    }
    
    EXPECT_TRUE(combo.matches('A'));
    EXPECT_TRUE(combo.matches('?'));
    EXPECT_FALSE(combo.matches('B'));
    EXPECT_EQ(12, combo.count()); // 10 digits + 'A' + '?'
}

TEST_F(ABNFConstructorTest, CopyConstructor) {
    ABNF original = ABNF::digit();
    ABNF copy(original);
    
    EXPECT_EQ(original.count(), copy.count());
    
    for (unsigned int i = 0; i <= 255; ++i) {
        EXPECT_EQ(original.matches(static_cast<uint8_t>(i)), 
                  copy.matches(static_cast<uint8_t>(i)));
    }
}

TEST_F(ABNFConstructorTest, MoveConstructor) {
    ABNF original = ABNF::digit();
    size_t original_count = original.count();
    
    ABNF moved(std::move(original));
    
    EXPECT_EQ(original_count, moved.count());
    EXPECT_TRUE(moved.matches('5'));
}

TEST_F(ABNFConstructorTest, CopyAssignment) {
    ABNF original = ABNF::digit();
    ABNF copy;
    
    copy = original;
    
    EXPECT_EQ(original.count(), copy.count());
    EXPECT_TRUE(copy.matches('5'));
}

TEST_F(ABNFConstructorTest, MoveAssignment) {
    ABNF original = ABNF::digit();
    size_t original_count = original.count();
    ABNF moved;
    
    moved = std::move(original);
    
    EXPECT_EQ(original_count, moved.count());
    EXPECT_TRUE(moved.matches('5'));
}

// ============================================================================
// Matching Operations Tests
// ============================================================================

class ABNFMatchingTest : public ABNFTest {};

TEST_F(ABNFMatchingTest, Matches_WithChar) {
    ABNF rule('X');
    
    EXPECT_TRUE(rule.matches('X'));
    EXPECT_FALSE(rule.matches('Y'));
}

TEST_F(ABNFMatchingTest, Matches_WithUint8) {
    ABNF rule(static_cast<uint8_t>(0x58)); // 'X'
    
    EXPECT_TRUE(rule.matches(static_cast<uint8_t>(0x58)));
    EXPECT_FALSE(rule.matches(static_cast<uint8_t>(0x59)));
}

TEST_F(ABNFMatchingTest, Matches_WithSignedChar) {
    ABNF rule('A');
    signed char ch = 'A';
    
    EXPECT_TRUE(rule.matches(ch));
}

TEST_F(ABNFMatchingTest, Matches_WithUnsignedChar) {
    ABNF rule('A');
    unsigned char ch = 'A';
    
    EXPECT_TRUE(rule.matches(ch));
}

TEST_F(ABNFMatchingTest, Excludes_OppositeOfMatches) {
    ABNF digit = ABNF::digit();
    
    EXPECT_FALSE(digit.excludes('5'));
    EXPECT_TRUE(digit.excludes('A'));
}

TEST_F(ABNFMatchingTest, OperatorCall_SameAsMatches) {
    ABNF digit = ABNF::digit();
    
    EXPECT_TRUE(digit('5'));
    EXPECT_FALSE(digit('A'));
}

TEST_F(ABNFMatchingTest, Matches_AllIntegralTypes) {
    ABNF rule('A');
    
    // char
    EXPECT_TRUE(rule.matches('A'));
    
    // unsigned char
    unsigned char uc = 'A';
    EXPECT_TRUE(rule.matches(uc));
    
    // signed char
    signed char sc = 'A';
    EXPECT_TRUE(rule.matches(sc));
    
    // uint8_t
    uint8_t u8 = 'A';
    EXPECT_TRUE(rule.matches(u8));
    
    // int8_t
    int8_t i8 = 'A';
    EXPECT_TRUE(rule.matches(i8));
}

// ============================================================================
// Set Operations Tests
// ============================================================================

class ABNFSetOperationsTest : public ABNFTest {};

TEST_F(ABNFSetOperationsTest, UnionWith_CombinesRules) {
    ABNF digits = ABNF::digit();
    ABNF letters = ABNF::alpha();
    
    ABNF alphanumeric = digits.unionWith(letters);
    
    EXPECT_TRUE(alphanumeric.matches('5'));
    EXPECT_TRUE(alphanumeric.matches('A'));
    EXPECT_TRUE(alphanumeric.matches('z'));
    EXPECT_FALSE(alphanumeric.matches('!'));
    
    EXPECT_EQ(62, alphanumeric.count()); // 10 digits + 52 letters
}

TEST_F(ABNFSetOperationsTest, UnionOperator_SameAsUnionWith) {
    ABNF digits = ABNF::digit();
    ABNF letters = ABNF::alpha();
    
    ABNF alphanumeric = digits | letters;
    
    EXPECT_EQ(62, alphanumeric.count());
    EXPECT_TRUE(alphanumeric.matches('5'));
    EXPECT_TRUE(alphanumeric.matches('A'));
}

TEST_F(ABNFSetOperationsTest, IntersectWith_FindsCommonElements) {
    ABNF digits = ABNF::digit();
    ABNF hexdig = ABNF::hexdig();
    
    ABNF intersection = digits.intersectWith(hexdig);
    
    // Should only match 0-9 (common between DIGIT and HEXDIG)
    for (char ch = '0'; ch <= '9'; ++ch) {
        EXPECT_TRUE(intersection.matches(ch));
    }
    
    EXPECT_FALSE(intersection.matches('A'));
    EXPECT_FALSE(intersection.matches('F'));
    EXPECT_EQ(10, intersection.count());
}

TEST_F(ABNFSetOperationsTest, IntersectOperator_SameAsIntersectWith) {
    ABNF digits = ABNF::digit();
    ABNF hexdig = ABNF::hexdig();
    
    ABNF intersection = digits & hexdig;
    
    EXPECT_EQ(10, intersection.count());
}

TEST_F(ABNFSetOperationsTest, IntersectWith_NoCommonElements) {
    ABNF digits = ABNF::digit();
    ABNF alpha = ABNF::alpha();
    
    ABNF intersection = digits.intersectWith(alpha);
    
    EXPECT_TRUE(intersection.isEmpty());
    EXPECT_EQ(0, intersection.count());
}

TEST_F(ABNFSetOperationsTest, Complement_InvertsRule) {
    ABNF digits = ABNF::digit();
    ABNF not_digits = digits.complement();
    
    // Not-digits should match everything except 0-9
    EXPECT_FALSE(not_digits.matches('5'));
    EXPECT_TRUE(not_digits.matches('A'));
    EXPECT_TRUE(not_digits.matches('!'));
    
    EXPECT_EQ(246, not_digits.count()); // 256 - 10
}

TEST_F(ABNFSetOperationsTest, ComplementOperator_SameAsComplement) {
    ABNF digits = ABNF::digit();
    ABNF not_digits = ~digits;
    
    EXPECT_EQ(246, not_digits.count());
    EXPECT_FALSE(not_digits.matches('5'));
    EXPECT_TRUE(not_digits.matches('A'));
}

TEST_F(ABNFSetOperationsTest, DoubleComplement_RestoresOriginal) {
    ABNF original = ABNF::digit();
    ABNF double_complement = ~~original;
    
    EXPECT_EQ(original.count(), double_complement.count());
    
    for (unsigned int i = 0; i <= 255; ++i) {
        EXPECT_EQ(original.matches(static_cast<uint8_t>(i)),
                  double_complement.matches(static_cast<uint8_t>(i)));
    }
}

TEST_F(ABNFSetOperationsTest, ComplexSetOperations) {
    // (DIGIT | ALPHA) & ~CTL
    ABNF digit = ABNF::digit();
    ABNF alpha = ABNF::alpha();
    ABNF ctl = ABNF::ctl();
    
    ABNF result = (digit | alpha) & ~ctl;
    
    // Should match alphanumeric but not control characters
    EXPECT_TRUE(result.matches('A'));
    EXPECT_TRUE(result.matches('5'));
    EXPECT_FALSE(result.matches('\t'));
    EXPECT_FALSE(result.matches(0x00));
}

// ============================================================================
// Utility Methods Tests
// ============================================================================

class ABNFUtilityTest : public ABNFTest {};

TEST_F(ABNFUtilityTest, IsEmpty_DefaultConstructor) {
    ABNF empty;
    EXPECT_TRUE(empty.isEmpty());
}

TEST_F(ABNFUtilityTest, IsEmpty_NonEmptyRule) {
    ABNF non_empty('A');
    EXPECT_FALSE(non_empty.isEmpty());
}

TEST_F(ABNFUtilityTest, IsEmpty_AfterIntersectionWithNoCommon) {
    ABNF digit = ABNF::digit();
    ABNF alpha = ABNF::alpha();
    ABNF empty = digit & alpha;
    
    EXPECT_TRUE(empty.isEmpty());
}

TEST_F(ABNFUtilityTest, Count_VariousRules) {
    EXPECT_EQ(0, ABNF().count());
    EXPECT_EQ(1, ABNF('A').count());
    EXPECT_EQ(10, ABNF::digit().count());
    EXPECT_EQ(52, ABNF::alpha().count());
    EXPECT_EQ(256, ABNF::octet().count());
}

TEST_F(ABNFUtilityTest, ToString_ReturnsDescription) {
    ABNF digit = ABNF::digit();
    std::string desc = digit.toString();
    
    EXPECT_FALSE(desc.empty());
    EXPECT_EQ("DIGIT", desc);
}

TEST_F(ABNFUtilityTest, ToString_SingleChar) {
    ABNF rule('A');
    EXPECT_EQ("'A'", rule.toString());
}

TEST_F(ABNFUtilityTest, ToString_CoreRules) {
    EXPECT_EQ("ALPHA", ABNF::alpha().toString());
    EXPECT_EQ("DIGIT", ABNF::digit().toString());
    EXPECT_EQ("HEXDIG", ABNF::hexdig().toString());
}

// ============================================================================
// Builder Pattern Tests
// ============================================================================

class ABNFBuilderTest : public ABNFTest {};

TEST_F(ABNFBuilderTest, Builder_EmptyBuild) {
    ABNF::Builder builder;
    ABNF result = builder.build();
    
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ABNFBuilderTest, Builder_AddChar) {
    ABNF::Builder builder;
    ABNF result = builder.addChar('A').addChar('B').build();
    
    EXPECT_TRUE(result.matches('A'));
    EXPECT_TRUE(result.matches('B'));
    EXPECT_FALSE(result.matches('C'));
    EXPECT_EQ(2, result.count());
}

TEST_F(ABNFBuilderTest, Builder_AddRange) {
    ABNF::Builder builder;
    ABNF result = builder.addRange('A', 'Z').build();
    
    EXPECT_TRUE(result.matches('A'));
    EXPECT_TRUE(result.matches('M'));
    EXPECT_TRUE(result.matches('Z'));
    EXPECT_FALSE(result.matches('a'));
    EXPECT_EQ(26, result.count());
}

TEST_F(ABNFBuilderTest, Builder_AddCoreRule) {
    ABNF::Builder builder;
    ABNF result = builder.addCoreRule(ABNF::CoreRule::DIGIT).build();
    
    EXPECT_TRUE(result.matches('5'));
    EXPECT_FALSE(result.matches('A'));
    EXPECT_EQ(10, result.count());
}

TEST_F(ABNFBuilderTest, Builder_AddRule) {
    ABNF digit = ABNF::digit();
    
    ABNF::Builder builder;
    ABNF result = builder.addRule(digit).addChar('-').build();
    
    EXPECT_TRUE(result.matches('5'));
    EXPECT_TRUE(result.matches('-'));
    EXPECT_FALSE(result.matches('A'));
    EXPECT_EQ(11, result.count());
}

TEST_F(ABNFBuilderTest, Builder_ComplexCombination) {
    ABNF::Builder builder;
    ABNF result = builder
        .addCoreRule(ABNF::CoreRule::DIGIT)
        .addRange('A', 'Z')
        .addRange('a', 'z')
        .addChar('-')
        .addChar('_')
        .build();
    
    // Should match alphanumeric plus hyphen and underscore
    EXPECT_TRUE(result.matches('5'));
    EXPECT_TRUE(result.matches('A'));
    EXPECT_TRUE(result.matches('z'));
    EXPECT_TRUE(result.matches('-'));
    EXPECT_TRUE(result.matches('_'));
    EXPECT_FALSE(result.matches('!'));
    
    EXPECT_EQ(64, result.count()); // 10 + 26 + 26 + 1 + 1
}

TEST_F(ABNFBuilderTest, Builder_InvalidRange_Throws) {
    ABNF::Builder builder;
    
    EXPECT_THROW({
        builder.addRange('Z', 'A');
    }, std::invalid_argument);
}

TEST_F(ABNFBuilderTest, Builder_ChainedCalls) {
    ABNF::Builder builder;
    
    // Test that chaining works properly
    auto& ref1 = builder.addChar('A');
    auto& ref2 = ref1.addChar('B');
    auto& ref3 = ref2.addRange('0', '9');
    
    ABNF result = ref3.build();
    
    EXPECT_TRUE(result.matches('A'));
    EXPECT_TRUE(result.matches('B'));
    EXPECT_TRUE(result.matches('5'));
}

// ============================================================================
// Edge Cases and Boundary Tests
// ============================================================================

class ABNFEdgeCasesTest : public ABNFTest {};

TEST_F(ABNFEdgeCasesTest, NullCharacter) {
    ABNF null_char(static_cast<uint8_t>(0x00));
    
    EXPECT_TRUE(null_char.matches(0x00));
    EXPECT_FALSE(null_char.matches(0x01));
}

TEST_F(ABNFEdgeCasesTest, FullByteRange) {
    ABNF full_range(static_cast<uint8_t>(0x00), static_cast<uint8_t>(0xFF));
    
    EXPECT_EQ(256, full_range.count());
    
    for (unsigned int i = 0; i <= 255; ++i) {
        EXPECT_TRUE(full_range.matches(static_cast<uint8_t>(i)));
    }
}

TEST_F(ABNFEdgeCasesTest, SingleCharRange) {
    ABNF single('A', 'A');
    
    EXPECT_TRUE(single.matches('A'));
    EXPECT_FALSE(single.matches('B'));
    EXPECT_EQ(1, single.count());
}

TEST_F(ABNFEdgeCasesTest, ExtendedASCII) {
    ABNF high_byte(static_cast<uint8_t>(0x80), static_cast<uint8_t>(0xFF));
    
    for (unsigned int i = 0x80; i <= 0xFF; ++i) {
        EXPECT_TRUE(high_byte.matches(static_cast<uint8_t>(i)));
    }
    
    EXPECT_FALSE(high_byte.matches(0x7F));
    EXPECT_EQ(128, high_byte.count());
}

TEST_F(ABNFEdgeCasesTest, UnionWithSelf) {
    ABNF digit = ABNF::digit();
    ABNF result = digit | digit;
    
    EXPECT_EQ(digit.count(), result.count());
}

TEST_F(ABNFEdgeCasesTest, IntersectWithSelf) {
    ABNF digit = ABNF::digit();
    ABNF result = digit & digit;
    
    EXPECT_EQ(digit.count(), result.count());
}

TEST_F(ABNFEdgeCasesTest, UnionWithComplement) {
    ABNF digit = ABNF::digit();
    ABNF all = digit | ~digit;
    
    EXPECT_EQ(256, all.count());
}

TEST_F(ABNFEdgeCasesTest, IntersectWithComplement) {
    ABNF digit = ABNF::digit();
    ABNF none = digit & ~digit;
    
    EXPECT_TRUE(none.isEmpty());
    EXPECT_EQ(0, none.count());
}

// ============================================================================
// Real-World Use Cases
// ============================================================================

class ABNFUseCasesTest : public ABNFTest {};

TEST_F(ABNFUseCasesTest, URISchemeCharacters) {
    // RFC 3986: scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    ABNF::Builder builder;
    ABNF scheme_chars = builder
        .addCoreRule(ABNF::CoreRule::ALPHA)
        .addCoreRule(ABNF::CoreRule::DIGIT)
        .addChar('+')
        .addChar('-')
        .addChar('.')
        .build();
    
    EXPECT_TRUE(scheme_chars.matches('h'));
    EXPECT_TRUE(scheme_chars.matches('t'));
    EXPECT_TRUE(scheme_chars.matches('p'));
    EXPECT_TRUE(scheme_chars.matches('s'));
    EXPECT_TRUE(scheme_chars.matches('+'));
    EXPECT_FALSE(scheme_chars.matches(':'));
    EXPECT_FALSE(scheme_chars.matches('/'));
}

TEST_F(ABNFUseCasesTest, Base64Characters) {
    // Base64: A-Z / a-z / 0-9 / "+" / "/"
    ABNF::Builder builder;
    ABNF base64 = builder
        .addRange('A', 'Z')
        .addRange('a', 'z')
        .addRange('0', '9')
        .addChar('+')
        .addChar('/')
        .build();
    
    EXPECT_EQ(64, base64.count());
    EXPECT_TRUE(base64.matches('A'));
    EXPECT_TRUE(base64.matches('z'));
    EXPECT_TRUE(base64.matches('0'));
    EXPECT_TRUE(base64.matches('+'));
    EXPECT_TRUE(base64.matches('/'));
    EXPECT_FALSE(base64.matches('='));
}

TEST_F(ABNFUseCasesTest, EmailLocalPartCharacters) {
    // Simplified: alphanumeric + . ! # $ % & ' * + - / = ? ^ _ ` { | } ~
    ABNF::Builder builder;
    ABNF email_local = builder
        .addCoreRule(ABNF::CoreRule::ALPHA)
        .addCoreRule(ABNF::CoreRule::DIGIT)
        .addChar('.')
        .addChar('!')
        .addChar('#')
        .addChar('$')
        .addChar('%')
        .addChar('&')
        .addChar('\'')
        .addChar('*')
        .addChar('+')
        .addChar('-')
        .addChar('/')
        .addChar('=')
        .addChar('?')
        .addChar('^')
        .addChar('_')
        .addChar('`')
        .addChar('{')
        .addChar('|')
        .addChar('}')
        .addChar('~')
        .build();
    
    EXPECT_TRUE(email_local.matches('u'));
    EXPECT_TRUE(email_local.matches('s'));
    EXPECT_TRUE(email_local.matches('e'));
    EXPECT_TRUE(email_local.matches('r'));
    EXPECT_TRUE(email_local.matches('.'));
    EXPECT_TRUE(email_local.matches('+'));
    EXPECT_FALSE(email_local.matches('@'));
}

TEST_F(ABNFUseCasesTest, IPAddressDigitsAndDots) {
    ABNF::Builder builder;
    ABNF ip_chars = builder
        .addCoreRule(ABNF::CoreRule::DIGIT)
        .addChar('.')
        .build();
    
    EXPECT_TRUE(ip_chars.matches('1'));
    EXPECT_TRUE(ip_chars.matches('9'));
    EXPECT_TRUE(ip_chars.matches('2'));
    EXPECT_TRUE(ip_chars.matches('.'));
    EXPECT_FALSE(ip_chars.matches(':'));
}

TEST_F(ABNFUseCasesTest, IdentifierCharacters) {
    // C-style identifier: (ALPHA / "_") *( ALPHA / DIGIT / "_" )
    ABNF::Builder builder;
    ABNF identifier = builder
        .addCoreRule(ABNF::CoreRule::ALPHA)
        .addCoreRule(ABNF::CoreRule::DIGIT)
        .addChar('_')
        .build();
    
    EXPECT_TRUE(identifier.matches('m'));
    EXPECT_TRUE(identifier.matches('y'));
    EXPECT_TRUE(identifier.matches('_'));
    EXPECT_TRUE(identifier.matches('v'));
    EXPECT_TRUE(identifier.matches('a'));
    EXPECT_TRUE(identifier.matches('r'));
    EXPECT_TRUE(identifier.matches('1'));
    EXPECT_FALSE(identifier.matches('-'));
    EXPECT_FALSE(identifier.matches(' '));
}

// ============================================================================
// Performance Tests (basic)
// ============================================================================

class ABNFPerformanceTest : public ABNFTest {};

TEST_F(ABNFPerformanceTest, MatchingIsConstantTime) {
    ABNF digit = ABNF::digit();
    
    // Matching should be O(1) regardless of the character
    // This is a basic sanity check, not a real performance test
    for (int i = 0; i < 1000; ++i) {
        EXPECT_TRUE(digit.matches('5'));
        EXPECT_FALSE(digit.matches('A'));
    }
}

TEST_F(ABNFPerformanceTest, SetOperationsComplete) {
    ABNF digit = ABNF::digit();
    ABNF alpha = ABNF::alpha();
    
    // Ensure set operations complete without hanging
    for (int i = 0; i < 100; ++i) {
        ABNF result = digit | alpha;
        EXPECT_EQ(62, result.count());
    }
}
// #include <gtest/gtest.h>
// #include <fsm/simd_utils.hpp>
// #include <abnf/abnf_simd.hpp>

// using namespace fsm::simd;
// using namespace abnf;

// class SIMDTest : public ::testing::Test {
// protected:
//     void SetUp() override {
//         const auto& cpu = CPUInfo::instance();
//         std::cout << "\nCPU Features: ";
//         if (cpu.hasAVX512F()) std::cout << "AVX512F ";
//         if (cpu.hasAVX2()) std::cout << "AVX2 ";
//         if (cpu.hasAVX()) std::cout << "AVX ";
//         if (cpu.hasSSE42()) std::cout << "SSE4.2 ";
//         if (cpu.hasSSE2()) std::cout << "SSE2 ";
//         std::cout << "\n";
//     }
// };

// // ============================================================================
// // CPU Detection Tests
// // ============================================================================

// TEST_F(SIMDTest, CPUDetection) {
//     const auto& cpu = CPUInfo::instance();
    
// #ifdef __x86_64__
//     EXPECT_TRUE(cpu.hasSSE2());
// #endif
    
//     if (cpu.hasAVX2()) {
//         EXPECT_TRUE(cpu.hasAVX());
//         EXPECT_TRUE(cpu.hasSSE2());
//     }
// }

// // ============================================================================
// // Character Range Finding Tests
// // ============================================================================

// TEST_F(SIMDTest, FindDigit_Scalar) {
//     std::string data = "abcdefg123xyz";
//     size_t pos = charset::findFirstInRange_scalar(data. data(), data.size(), '0', '9');
//     EXPECT_EQ(7, pos);
// }

// #ifdef __SSE2__
// TEST_F(SIMDTest, FindDigit_SSE2) {
//     std::string data = "abcdefghijklmnop123xyz";
//     size_t pos = charset::findFirstInRange_sse2(data.data(), data.size(), '0', '9');
//     EXPECT_EQ(16, pos);
// }
// #endif

// #ifdef __AVX2__
// TEST_F(SIMDTest, FindDigit_AVX2) {
//     std::string data = "abcdefghijklmnopqrstuvwxyzABCDEF123xyz";
//     size_t pos = charset::findFirstInRange_avx2(data.data(), data.size(), '0', '9');
//     EXPECT_EQ(32, pos);
// }
// #endif

// TEST_F(SIMDTest, FindDigit_Auto) {
//     std::string data = "xyz123abc";
//     size_t pos = charset::findFirstInRange(data.data(), data.size(), '0', '9');
//     EXPECT_EQ(3, pos);
// }

// TEST_F(SIMDTest, FindDigit_NotFound) {
//     std::string data = "abcdefghijklmnopqrstuvwxyz";
//     size_t pos = charset::findFirstInRange(data.data(), data.size(), '0', '9');
//     EXPECT_EQ(data.size(), pos);
// }

// // ============================================================================
// // String Matching Tests
// // ============================================================================

// TEST_F(SIMDTest, StartsWith_Scalar_Match) {
//     EXPECT_TRUE(string_match::startsWith_scalar("http://example.com", "http://"));
//     EXPECT_FALSE(string_match::startsWith_scalar("https://example.com", "http://"));
// }

// #ifdef __SSE2__
// TEST_F(SIMDTest, StartsWith_SSE2_Match) {
//     EXPECT_TRUE(string_match::startsWith_sse2("http://example. com", "http://"));
//     EXPECT_FALSE(string_match::startsWith_sse2("https://example.com", "http://"));
// }
// #endif

// #ifdef __AVX2__
// TEST_F(SIMDTest, StartsWith_AVX2_Match) {
//     std::string long_literal = "this_is_a_very_long_literal_string_for_testing";
//     std::string match = long_literal + "_with_more";
//     std::string no_match = "different_string";
    
//     EXPECT_TRUE(string_match::startsWith_avx2(match, long_literal));
//     EXPECT_FALSE(string_match::startsWith_avx2(no_match, long_literal));
// }
// #endif

// TEST_F(SIMDTest, StartsWith_Auto) {
//     EXPECT_TRUE(string_match::startsWith("GET /index.html", "GET "));
//     EXPECT_FALSE(string_match::startsWith("POST /data", "GET "));
// }

// // ============================================================================
// // ABNF SIMD Matcher Tests
// // ============================================================================

// TEST_F(SIMDTest, ABNFMatcher_StartsWith) {
//     EXPECT_TRUE(SIMDMatcher::startsWith("http://example.com", "http://"));
//     EXPECT_FALSE(SIMDMatcher::startsWith("ftp://example.com", "http://"));
// }

// // ============================================================================
// // Edge Cases
// // ============================================================================

// TEST_F(SIMDTest, EmptyString) {
//     std::string empty;
//     size_t pos = charset::findFirstInRange(empty.data(), empty.size(), '0', '9');
//     EXPECT_EQ(0, pos);
// }

// TEST_F(SIMDTest, SingleCharacter) {
//     std::string single = "5";
//     size_t pos = charset::findFirstInRange(single.data(), single.size(), '0', '9');
//     EXPECT_EQ(0, pos);
// }

// TEST_F(SIMDTest, LongString_AllMatch) {
//     std::string digits(1000, '5');
//     size_t pos = charset::findFirstInRange(digits.data(), digits.size(), '0', '9');
//     EXPECT_EQ(0, pos);
// }

// TEST_F(SIMDTest, LongString_NoMatch) {
//     std::string letters(1000, 'a');
//     size_t pos = charset::findFirstInRange(letters. data(), letters.size(), '0', '9');
//     EXPECT_EQ(1000, pos);
// }

// TEST_F(SIMDTest, LongString_MatchAtEnd) {
//     std::string data(999, 'a');
//     data += '5';
//     size_t pos = charset::findFirstInRange(data.data(), data.size(), '0', '9');
//     EXPECT_EQ(999, pos);
// }
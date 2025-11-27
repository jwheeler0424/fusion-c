#include <gtest/gtest.h>
#include <fsm/fsm.hpp>
#include <abnf/abnf.hpp>

using namespace fsm;
using namespace abnf;

class FsmTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Common setup
    }

    void TearDown() override
    {
        // Cleanup
    }
};

// ============================================================================
// Basic Construction Tests
// ============================================================================

TEST_F(FsmTest, ConstructEmptyFSM)
{
    FSM fsm;
    EXPECT_EQ(0, fsm.getStateCount());
    EXPECT_EQ(0, fsm.getTransitionCount());
}

TEST_F(FsmTest, ConstructNamedFSM)
{
    FSM fsm("test_fsm");
    EXPECT_EQ("test_fsm", fsm.getName());
}

TEST_F(FsmTest, BuilderBasicConstruction)
{
    auto fsm = FSM::Builder("simple")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    EXPECT_EQ(2, fsm->getStateCount());
    EXPECT_EQ(1, fsm->getTransitionCount());
}

// ============================================================================
// State Management Tests
// ============================================================================

TEST_F(FsmTest, AddState)
{
    auto fsm = FSM::Builder("test")
                   .addState("S1")
                   .addState("S2", "Description")
                   .addState("S3", StateType::ACCEPT)
                   .setStartState("S1")
                   .addAcceptState("S3")
                   .build();

    EXPECT_EQ(3, fsm->getStateCount());
}

TEST_F(FsmTest, SetStartState)
{
    auto fsm = FSM::Builder("test")
                   .addState("START", StateType::START)
                   .addState("OTHER")
                   .setStartState("START")
                   .addAcceptState("OTHER")
                   .build();

    EXPECT_TRUE(fsm->getStartState().isValid());
}

TEST_F(FsmTest, AddAcceptState)
{
    auto fsm = FSM::Builder("test")
                   .addState("START", StateType::START)
                   .addState("ACCEPT")
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .build();

    EXPECT_EQ(1, fsm->getAcceptStates().size());
}

TEST_F(FsmTest, MultipleAcceptStates)
{
    auto fsm = FSM::Builder("test")
                   .addState("START", StateType::START)
                   .addState("ACCEPT1", StateType::ACCEPT)
                   .addState("ACCEPT2", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT1")
                   .addAcceptState("ACCEPT2")
                   .build();

    EXPECT_EQ(2, fsm->getAcceptStates().size());
}

// ============================================================================
// ABNF Transition Tests
// ============================================================================

TEST_F(FsmTest, DigitTransition)
{
    auto fsm = FSM::Builder("digit")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    EXPECT_TRUE(fsm->validate("0"));
    EXPECT_TRUE(fsm->validate("5"));
    EXPECT_TRUE(fsm->validate("9"));
    EXPECT_FALSE(fsm->validate("a"));
    EXPECT_FALSE(fsm->validate("Z"));
}

TEST_F(FsmTest, AlphaTransition)
{
    auto fsm = FSM::Builder("alpha")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::alpha())
                   .build();

    EXPECT_TRUE(fsm->validate("a"));
    EXPECT_TRUE(fsm->validate("Z"));
    EXPECT_FALSE(fsm->validate("5"));
    EXPECT_FALSE(fsm->validate("! "));
}

TEST_F(FsmTest, LiteralTransition)
{
    auto fsm = FSM::Builder("literal")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::literal('x'))
                   .build();

    EXPECT_TRUE(fsm->validate("x"));
    EXPECT_FALSE(fsm->validate("y"));
    EXPECT_FALSE(fsm->validate("X"));
}

TEST_F(FsmTest, RangeTransition)
{
    auto fsm = FSM::Builder("range")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::range('a', 'c'))
                   .build();

    EXPECT_TRUE(fsm->validate("a"));
    EXPECT_TRUE(fsm->validate("b"));
    EXPECT_TRUE(fsm->validate("c"));
    EXPECT_FALSE(fsm->validate("d"));
    EXPECT_FALSE(fsm->validate("A"));
}

TEST_F(FsmTest, HexdigitTransition)
{
    auto fsm = FSM::Builder("hexdig")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::hexdig())
                   .build();

    EXPECT_TRUE(fsm->validate("0"));
    EXPECT_TRUE(fsm->validate("9"));
    EXPECT_TRUE(fsm->validate("A"));
    EXPECT_TRUE(fsm->validate("F"));
    EXPECT_TRUE(fsm->validate("a"));
    EXPECT_TRUE(fsm->validate("f"));
    EXPECT_FALSE(fsm->validate("G"));
    EXPECT_FALSE(fsm->validate("z"));
}

// ============================================================================
// Multi-State Validation Tests
// ============================================================================

TEST_F(FsmTest, ThreeDigits)
{
    auto fsm = FSM::Builder("three_digits")
                   .addState("START", StateType::START)
                   .addState("D1")
                   .addState("D2")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "D1", ABNF::digit())
                   .addTransition("D1", "D2", ABNF::digit())
                   .addTransition("D2", "ACCEPT", ABNF::digit())
                   .build();

    EXPECT_TRUE(fsm->validate("123"));
    EXPECT_TRUE(fsm->validate("000"));
    EXPECT_TRUE(fsm->validate("999"));
    EXPECT_FALSE(fsm->validate("12"));   // Too short
    EXPECT_FALSE(fsm->validate("1234")); // Too long
    EXPECT_FALSE(fsm->validate("12a"));  // Invalid char
}

TEST_F(FsmTest, HTTPMethod_GET)
{
    auto fsm = FSM::Builder("http_get")
                   .addState("START", StateType::START)
                   .addState("G")
                   .addState("GE")
                   .addState("GET", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("GET")
                   .addTransition("START", "G", ABNF::literal('G'))
                   .addTransition("G", "GE", ABNF::literal('E'))
                   .addTransition("GE", "GET", ABNF::literal('T'))
                   .build();

    EXPECT_TRUE(fsm->validate("GET"));
    EXPECT_FALSE(fsm->validate("GE"));
    EXPECT_FALSE(fsm->validate("GETS"));
    EXPECT_FALSE(fsm->validate("get")); // Case sensitive
}

// ============================================================================
// Epsilon Transition Tests
// ============================================================================

TEST_F(FsmTest, SingleEpsilonTransition)
{
    auto fsm = FSM::Builder("epsilon")
                   .addState("START", StateType::START)
                   .addState("MIDDLE")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "MIDDLE", ABNF::digit())
                   .addEpsilonTransition("MIDDLE", "ACCEPT")
                   .build();

    EXPECT_TRUE(fsm->validate("5"));
}

TEST_F(FsmTest, MultipleEpsilonPaths)
{
    auto fsm = FSM::Builder("multi_epsilon")
                   .addState("START", StateType::START)
                   .addState("A")
                   .addState("B")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "A", ABNF::digit())
                   .addEpsilonTransition("A", "B")
                   .addEpsilonTransition("B", "ACCEPT")
                   .build();

    EXPECT_TRUE(fsm->validate("7"));
}

TEST_F(FsmTest, OptionalPattern)
{
    // Pattern: digit followed by optional letter
    auto fsm = FSM::Builder("optional")
                   .addState("START", StateType::START)
                   .addState("DIGIT", StateType::ACCEPT)  // Can stop here
                   .addState("LETTER", StateType::ACCEPT) // Or continue
                   .setStartState("START")
                   .addAcceptState("DIGIT")
                   .addAcceptState("LETTER")
                   .addTransition("START", "DIGIT", ABNF::digit())
                   .addTransition("DIGIT", "LETTER", ABNF::alpha())
                   .build();

    EXPECT_TRUE(fsm->validate("5"));  // Just digit
    EXPECT_TRUE(fsm->validate("5a")); // Digit + letter
    EXPECT_FALSE(fsm->validate("a")); // No digit
}

// ============================================================================
// Loop/Repetition Tests
// ============================================================================

TEST_F(FsmTest, RepeatingDigits)
{
    auto fsm = FSM::Builder("digits")
                   .addState("START", StateType::START)
                   .addState("DIGITS", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("DIGITS")
                   .addTransition("START", "DIGITS", ABNF::digit())
                   .addTransition("DIGITS", "DIGITS", ABNF::digit()) // Loop
                   .build();

    EXPECT_TRUE(fsm->validate("1"));
    EXPECT_TRUE(fsm->validate("123"));
    EXPECT_TRUE(fsm->validate("123456789"));
    EXPECT_FALSE(fsm->validate(""));
    EXPECT_FALSE(fsm->validate("12a34"));
}

TEST_F(FsmTest, AlternatingPattern)
{
    // Pattern: digit letter digit letter...
    auto fsm = FSM::Builder("alternating")
                   .addState("START", StateType::START)
                   .addState("DIGIT")
                   .addState("LETTER", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("LETTER")
                   .addTransition("START", "DIGIT", ABNF::digit())
                   .addTransition("DIGIT", "LETTER", ABNF::alpha())
                   .addTransition("LETTER", "DIGIT", ABNF::digit())
                   .build();

    EXPECT_TRUE(fsm->validate("1a"));
    EXPECT_TRUE(fsm->validate("1a2b"));
    EXPECT_TRUE(fsm->validate("1a2b3c"));
    EXPECT_FALSE(fsm->validate("1"));
    EXPECT_FALSE(fsm->validate("1a2"));
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(FsmTest, NoMatchingTransition)
{
    auto fsm = FSM::Builder("digit")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    EXPECT_FALSE(fsm->validate("a"));

    auto error = fsm->getLastError();
    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(FSM::ErrorType::NO_MATCHING_TRANSITION, error->type);
}

TEST_F(FsmTest, NotInAcceptState)
{
    auto fsm = FSM::Builder("two_digits")
                   .addState("START", StateType::START)
                   .addState("D1")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "D1", ABNF::digit())
                   .addTransition("D1", "ACCEPT", ABNF::digit())
                   .build();

    EXPECT_FALSE(fsm->validate("1")); // Only one digit

    auto error = fsm->getLastError();
    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(FSM::ErrorType::NOT_IN_ACCEPT_STATE, error->type);
}

TEST_F(FsmTest, BuilderErrorNoStartState)
{
    EXPECT_THROW({ auto fsm = FSM::Builder("bad")
                                  .addState("ACCEPT", StateType::ACCEPT)
                                  .addAcceptState("ACCEPT")
                                  .build(); }, std::logic_error);
}

TEST_F(FsmTest, BuilderErrorNoAcceptState)
{
    EXPECT_THROW({ auto fsm = FSM::Builder("bad")
                                  .addState("START", StateType::START)
                                  .setStartState("START")
                                  .build(); }, std::logic_error);
}

// ============================================================================
// Reset Tests
// ============================================================================

TEST_F(FsmTest, ResetAndReuse)
{
    auto fsm = FSM::Builder("reusable")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    EXPECT_TRUE(fsm->validate("5"));

    fsm->reset();

    EXPECT_TRUE(fsm->validate("7"));
    EXPECT_FALSE(fsm->validate("a"));
}

// ============================================================================
// Introspection Tests
// ============================================================================

TEST_F(FsmTest, GetStates)
{
    auto fsm = FSM::Builder("test")
                   .addState("S1")
                   .addState("S2")
                   .addState("S3")
                   .setStartState("S1")
                   .addAcceptState("S3")
                   .build();

    auto states = fsm->getStates();
    EXPECT_EQ(3, states.size());
}

TEST_F(FsmTest, GetTransitions)
{
    auto fsm = FSM::Builder("test")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .addTransition("START", "ACCEPT", ABNF::alpha())
                   .build();

    auto transitions = fsm->getTransitions();
    EXPECT_EQ(2, transitions.size());
}

TEST_F(FsmTest, ToStringMethods)
{
    auto fsm = FSM::Builder("test")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    std::string str = fsm->toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("test"), std::string::npos);

    std::string debug = fsm->toDebugString();
    EXPECT_FALSE(debug.empty());
}

// ============================================================================
// Priority Tests
// ============================================================================

TEST_F(FsmTest, TransitionPriority)
{
    auto fsm = FSM::Builder("priority")
                   .addState("START", StateType::START)
                   .addState("HIGH", StateType::ACCEPT)
                   .addState("LOW", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("HIGH")
                   .addAcceptState("LOW")
                   .addTransition("START", "HIGH", ABNF::digit(), Transition::PRIORITY_HIGH)
                   .addTransition("START", "LOW", ABNF::digit(), Transition::PRIORITY_LOW)
                   .build();

    // Should take HIGH priority transition
    EXPECT_TRUE(fsm->validate("5"));
    EXPECT_EQ("HIGH", fsm->getCurrentState().name);
}

// ============================================================================
// Debug Tests
// ============================================================================

TEST_F(FsmTest, DebugFlags)
{
    auto fsm = FSM::Builder("debug")
                   .setDebugFlags(DebugFlags::TRACE_TRANSITIONS | DebugFlags::COLLECT_METRICS)
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    fsm->validate("5");

    const auto &trace = fsm->getTrace();
    EXPECT_EQ(1, trace.size());

    const auto &metrics = fsm->getMetrics();
    EXPECT_EQ(1, metrics.transitions_taken);
    EXPECT_EQ(1, metrics.characters_processed);
}

TEST_F(FsmTest, DOTExport)
{
    auto fsm = FSM::Builder("dot_test")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    std::string dot = fsm->toDot();
    EXPECT_FALSE(dot.empty());
    EXPECT_NE(dot.find("digraph"), std::string::npos);
    EXPECT_NE(dot.find("START"), std::string::npos);
}

// ============================================================================
// Complex Pattern Tests
// ============================================================================

TEST_F(FsmTest, IPv4Octet)
{
    // Simplified: 1-3 digits
    auto fsm = FSM::Builder("ipv4_octet")
                   .addState("START", StateType::START)
                   .addState("D1", StateType::ACCEPT)
                   .addState("D2", StateType::ACCEPT)
                   .addState("D3", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("D1")
                   .addAcceptState("D2")
                   .addAcceptState("D3")
                   .addTransition("START", "D1", ABNF::digit())
                   .addTransition("D1", "D2", ABNF::digit())
                   .addTransition("D2", "D3", ABNF::digit())
                   .build();

    EXPECT_TRUE(fsm->validate("0"));
    EXPECT_TRUE(fsm->validate("25"));
    EXPECT_TRUE(fsm->validate("255"));
    EXPECT_FALSE(fsm->validate(""));
    EXPECT_FALSE(fsm->validate("2555"));
}

TEST_F(FsmTest, SimpleEmail)
{
    // Very simplified: letters + @ + letters
    auto fsm = FSM::Builder("simple_email")
                   .addState("START", StateType::START)
                   .addState("LOCAL")
                   .addState("AT")
                   .addState("DOMAIN", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("DOMAIN")
                   .addTransition("START", "LOCAL", ABNF::alpha())
                   .addTransition("LOCAL", "LOCAL", ABNF::alpha())
                   .addTransition("LOCAL", "AT", ABNF::literal('@'))
                   .addTransition("AT", "DOMAIN", ABNF::alpha())
                   .addTransition("DOMAIN", "DOMAIN", ABNF::alpha())
                   .build();

    EXPECT_TRUE(fsm->validate("user@domain"));
    EXPECT_TRUE(fsm->validate("a@b"));
    EXPECT_FALSE(fsm->validate("@domain"));
    EXPECT_FALSE(fsm->validate("user@"));
    EXPECT_FALSE(fsm->validate("userdomain"));
}
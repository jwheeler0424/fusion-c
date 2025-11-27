#include <gtest/gtest.h>
#include <fsm/fsm.hpp>
#include <abnf/abnf.hpp>

using namespace fsm;
using namespace abnf;

class BacktrackingTest : public ::testing::Test
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
// Basic Backtracking Tests
// ============================================================================

TEST_F(BacktrackingTest, SimpleAmbiguity_CatOrCatch)
{
    auto fsm = FSM::Builder("cat_or_catch")
                   .addState("START", StateType::START)
                   .addState("C")
                   .addState("CA")
                   .addState("CAT", StateType::ACCEPT)
                   .addState("CATC")
                   .addState("CATCH", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("CAT")
                   .addAcceptState("CATCH")
                   .addTransition("START", "C", ABNF::literal('c'))
                   .addTransition("C", "CA", ABNF::literal('a'))
                   .addTransition("CA", "CAT", ABNF::literal('t'))
                   .addTransition("CAT", "CATC", ABNF::literal('c'))
                   .addTransition("CATC", "CATCH", ABNF::literal('h'))
                   .build();

    EXPECT_TRUE(fsm->validate("cat"));

    fsm->reset();
    EXPECT_TRUE(fsm->validateWithBacktracking("catch"));

    const auto &stats = fsm->getBacktrackingStats();
    EXPECT_GT(stats.paths_explored, 0);
}

TEST_F(BacktrackingTest, NoBacktrackingNeeded)
{
    auto fsm = FSM::Builder("simple")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    EXPECT_TRUE(fsm->validateWithBacktracking("5"));

    const auto &stats = fsm->getBacktrackingStats();
    EXPECT_EQ(0, stats.choice_points_created);
    EXPECT_EQ(0, stats.backtracks_performed);
}

TEST_F(BacktrackingTest, MultipleAmbiguousTransitions)
{
    auto fsm = FSM::Builder("three_paths")
                   .addState("START", StateType::START)
                   .addState("PATH1")
                   .addState("PATH2")
                   .addState("PATH3", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("PATH3")
                   .addTransition("START", "PATH1", ABNF::literal('a'), Transition::PRIORITY_NORMAL)
                   .addTransition("START", "PATH2", ABNF::literal('a'), Transition::PRIORITY_NORMAL)
                   .addTransition("START", "PATH3", ABNF::literal('a'), Transition::PRIORITY_NORMAL)
                   .build();

    EXPECT_TRUE(fsm->validateWithBacktracking("a"));

    const auto &stats = fsm->getBacktrackingStats();
    EXPECT_GT(stats.choice_points_created, 0);
    EXPECT_GT(stats.paths_explored, 1);
}

// ============================================================================
// User-Defined Choice Points (Option C)
// ============================================================================

TEST_F(BacktrackingTest, UserDefinedChoicePoint)
{
    auto fsm = FSM::Builder("user_choice")
                   .addState("START", StateType::START)
                   .addState("CHOICE")
                   .addState("PATH1", StateType::ACCEPT)
                   .addState("PATH2", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("PATH1")
                   .addAcceptState("PATH2")
                   .addTransition("START", "CHOICE", ABNF::digit())
                   .markChoicePoint("CHOICE")
                   .addTransition("CHOICE", "PATH1", ABNF::literal('a'))
                   .addTransition("CHOICE", "PATH2", ABNF::literal('b'))
                   .build();

    EXPECT_TRUE(fsm->validateWithBacktracking("1a"));
    EXPECT_TRUE(fsm->validateWithBacktracking("2b"));
}

TEST_F(BacktrackingTest, AutomaticChoicePointDetection)
{
    auto fsm = FSM::Builder("auto_detect")
                   .addState("START", StateType::START)
                   .addState("AMBIG")
                   .addState("ACCEPT1", StateType::ACCEPT)
                   .addState("ACCEPT2", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT1")
                   .addAcceptState("ACCEPT2")
                   .addTransition("START", "AMBIG", ABNF::digit())
                   .addTransition("AMBIG", "ACCEPT1", ABNF::literal('a'))
                   .addTransition("AMBIG", "ACCEPT2", ABNF::literal('a'))
                   .build();

    EXPECT_TRUE(fsm->validateWithBacktracking("5a"));

    const auto &stats = fsm->getBacktrackingStats();
    EXPECT_GT(stats.choice_points_created, 0);
}

// ============================================================================
// Complex Backtracking Scenarios
// ============================================================================

TEST_F(BacktrackingTest, NestedBacktracking)
{
    auto fsm = FSM::Builder("nested")
                   .addState("START", StateType::START)
                   .addState("A1")
                   .addState("A2")
                   .addState("B1")
                   .addState("B2")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "A1", ABNF::literal('a'))
                   .addTransition("START", "A2", ABNF::literal('a'))
                   .addTransition("A1", "B1", ABNF::literal('b'))
                   .addTransition("A1", "B2", ABNF::literal('b'))
                   .addTransition("A2", "B1", ABNF::literal('b'))
                   .addTransition("A2", "B2", ABNF::literal('b'))
                   .addTransition("B2", "ACCEPT", ABNF::literal('c'))
                   .build();

    EXPECT_TRUE(fsm->validateWithBacktracking("abc"));

    const auto &stats = fsm->getBacktrackingStats();
    EXPECT_GT(stats.choice_points_created, 0);
    EXPECT_GT(stats.max_stack_depth, 0);
}

TEST_F(BacktrackingTest, BacktrackingWithFailure)
{
    auto fsm = FSM::Builder("all_fail")
                   .addState("START", StateType::START)
                   .addState("PATH1")
                   .addState("PATH2")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "PATH1", ABNF::literal('a'))
                   .addTransition("START", "PATH2", ABNF::literal('a'))
                   .addTransition("PATH1", "ACCEPT", ABNF::literal('b'))
                   .addTransition("PATH2", "ACCEPT", ABNF::literal('c'))
                   .build();

    EXPECT_FALSE(fsm->validateWithBacktracking("ax"));

    const auto &stats = fsm->getBacktrackingStats();
    EXPECT_GT(stats.backtracks_performed, 0);
}

// ============================================================================
// Backtracking with Captures
// ============================================================================

TEST_F(BacktrackingTest, BacktrackingWithCaptures)
{
    auto fsm = FSM::Builder("captures")
                   .addState("START", StateType::START)
                   .addState("PATH1")
                   .addState("PATH2")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .onStateEntry("PATH1", [](const StateContext &ctx)
                                 {
            auto* fsm_ptr = static_cast<FSM*>(ctx.user_data);
            if (fsm_ptr) {
                fsm_ptr->beginCapture("data");
            } })
                   .onStateEntry("PATH2", [](const StateContext &ctx)
                                 {
            auto* fsm_ptr = static_cast<FSM*>(ctx.user_data);
            if (fsm_ptr) {
                fsm_ptr->beginCapture("data");
            } })
                   .addTransition("START", "PATH1", ABNF::literal('a'))
                   .addTransition("START", "PATH2", ABNF::literal('a'))
                   .addTransition("PATH1", "ACCEPT", ABNF::literal('x'))
                   .addTransition("PATH2", "ACCEPT", ABNF::literal('y'))
                   .onStateEntry("ACCEPT", [](const StateContext &ctx)
                                 {
            auto* fsm_ptr = static_cast<FSM*>(ctx. user_data);
            if (fsm_ptr) {
                try {
                    fsm_ptr->endCapture("data");
                } catch (...) {
                    // Ignore
                }
            } })
                   .build();

    fsm->setUserData(fsm.get());

    EXPECT_TRUE(fsm->validateWithBacktracking("ay"));

    const auto &stats = fsm->getBacktrackingStats();
    EXPECT_GT(stats.backtracks_performed, 0);
}

// ============================================================================
// Maximum Depth Limiting
// ============================================================================

TEST_F(BacktrackingTest, MaxDepthLimit)
{
    auto fsm = FSM::Builder("deep")
                   .addState("START", StateType::START)
                   .addState("L1")
                   .addState("L2")
                   .addState("L3")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "L1", ABNF::literal('a'))
                   .addTransition("START", "L1", ABNF::literal('a'))
                   .addTransition("L1", "L2", ABNF::literal('b'))
                   .addTransition("L1", "L2", ABNF::literal('b'))
                   .addTransition("L2", "L3", ABNF::literal('c'))
                   .addTransition("L2", "L3", ABNF::literal('c'))
                   .addTransition("L3", "ACCEPT", ABNF::literal('d'))
                   .build();

    fsm->setMaxBacktrackDepth(2);

    fsm->validateWithBacktracking("abcd");

    const auto &stats = fsm->getBacktrackingStats();
    EXPECT_LE(stats.max_stack_depth, 2);
}

// ============================================================================
// Real-World Examples
// ============================================================================

TEST_F(BacktrackingTest, RealWorld_HttpOrHttps)
{
    auto fsm = FSM::Builder("http_protocol")
                   .addState("START", StateType::START)
                   .addState("H")
                   .addState("HT")
                   .addState("HTT")
                   .addState("HTTP", StateType::ACCEPT)
                   .addState("HTTPS", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("HTTP")
                   .addAcceptState("HTTPS")
                   .addTransition("START", "H", ABNF::literal('h'))
                   .addTransition("H", "HT", ABNF::literal('t'))
                   .addTransition("HT", "HTT", ABNF::literal('t'))
                   .addTransition("HTT", "HTTP", ABNF::literal('p'))
                   .addTransition("HTTP", "HTTPS", ABNF::literal('s'))
                   .build();

    EXPECT_TRUE(fsm->validate("http"));

    fsm->reset();
    EXPECT_TRUE(fsm->validateWithBacktracking("https"));
}

TEST_F(BacktrackingTest, RealWorld_EmailLocalPart)
{
    auto fsm = FSM::Builder("email_local")
                   .addState("START", StateType::START)
                   .addState("CHARS1")
                   .addState("CHARS2")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "CHARS1", ABNF::alpha())
                   .addTransition("CHARS1", "CHARS1", ABNF::alpha())
                   .addTransition("CHARS1", "ACCEPT", ABNF::literal('@'))
                   .addTransition("CHARS1", "CHARS2", ABNF::literal('.'))
                   .addTransition("CHARS2", "CHARS2", ABNF::alpha())
                   .addTransition("CHARS2", "ACCEPT", ABNF::literal('@'))
                   .build();

    EXPECT_TRUE(fsm->validateWithBacktracking("user@"));

    fsm->reset();
    EXPECT_TRUE(fsm->validateWithBacktracking("username@"));
}

TEST_F(BacktrackingTest, RealWorld_GreedyVsNonGreedy)
{
    auto fsm = FSM::Builder("greedy")
                   .addState("START", StateType::START)
                   .addState("DIGITS", StateType::ACCEPT)
                   .addState("WITH_LETTER", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("DIGITS")
                   .addAcceptState("WITH_LETTER")
                   .addTransition("START", "DIGITS", ABNF::digit())
                   .addTransition("DIGITS", "DIGITS", ABNF::digit())
                   .addTransition("DIGITS", "WITH_LETTER", ABNF::alpha())
                   .build();

    EXPECT_TRUE(fsm->validate("123"));

    fsm->reset();
    EXPECT_TRUE(fsm->validateWithBacktracking("123a"));
}

// ============================================================================
// Statistics and Metrics
// ============================================================================

TEST_F(BacktrackingTest, BacktrackingStatistics)
{
    auto fsm = FSM::Builder("stats")
                   .addState("START", StateType::START)
                   .addState("A")
                   .addState("B")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "A", ABNF::literal('x'))
                   .addTransition("START", "B", ABNF::literal('x'))
                   .addTransition("A", "ACCEPT", ABNF::literal('y'))
                   .addTransition("B", "ACCEPT", ABNF::literal('z'))
                   .build();

    fsm->validateWithBacktracking("xz");

    const auto &stats = fsm->getBacktrackingStats();

    EXPECT_GT(stats.choice_points_created, 0);
    EXPECT_GT(stats.backtracks_performed, 0);
    EXPECT_GT(stats.paths_explored, 0);
    EXPECT_GT(stats.max_stack_depth, 0);

    fsm->resetBacktrackingStats();
    const auto &reset_stats = fsm->getBacktrackingStats();

    EXPECT_EQ(0, reset_stats.choice_points_created);
    EXPECT_EQ(0, reset_stats.backtracks_performed);
    EXPECT_EQ(0, reset_stats.paths_explored);
    EXPECT_EQ(0, reset_stats.max_stack_depth);
}

TEST_F(BacktrackingTest, StatisticsToString)
{
    auto fsm = FSM::Builder("stats")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    fsm->validateWithBacktracking("5");

    const auto &stats = fsm->getBacktrackingStats();
    std::string stats_str = stats.toString();

    EXPECT_FALSE(stats_str.empty());
    EXPECT_NE(stats_str.find("BacktrackingStats"), std::string::npos);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(BacktrackingTest, EdgeCase_EmptyInput)
{
    auto fsm = FSM::Builder("empty")
                   .addState("START", StateType::START)
                   .setStartState("START")
                   .addAcceptState("START")
                   .build();

    EXPECT_TRUE(fsm->validateWithBacktracking(""));
}

TEST_F(BacktrackingTest, EdgeCase_NoChoicePoints)
{
    auto fsm = FSM::Builder("linear")
                   .addState("START", StateType::START)
                   .addState("S1")
                   .addState("S2")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "S1", ABNF::literal('a'))
                   .addTransition("S1", "S2", ABNF::literal('b'))
                   .addTransition("S2", "ACCEPT", ABNF::literal('c'))
                   .build();

    EXPECT_TRUE(fsm->validateWithBacktracking("abc"));

    const auto &stats = fsm->getBacktrackingStats();
    EXPECT_EQ(0, stats.choice_points_created);
}

TEST_F(BacktrackingTest, EdgeCase_AllPathsFail)
{
    auto fsm = FSM::Builder("all_fail")
                   .addState("START", StateType::START)
                   .addState("PATH1")
                   .addState("PATH2")
                   .addState("DEAD_END", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("DEAD_END")
                   .addTransition("START", "PATH1", ABNF::literal('a'))
                   .addTransition("START", "PATH2", ABNF::literal('a'))
                   .addTransition("PATH1", "DEAD_END", ABNF::literal('b'))
                   .addTransition("PATH2", "DEAD_END", ABNF::literal('c'))
                   .build();

    EXPECT_FALSE(fsm->validateWithBacktracking("ax"));

    const auto &stats = fsm->getBacktrackingStats();
    EXPECT_GT(stats.backtracks_performed, 0);
}

// ============================================================================
// Comparison: validate() vs validateWithBacktracking()
// ============================================================================

TEST_F(BacktrackingTest, CompareValidateMethods)
{
    auto fsm = FSM::Builder("compare")
                   .addState("START", StateType::START)
                   .addState("A")
                   .addState("B")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "A", ABNF::literal('x'), Transition::PRIORITY_HIGH)
                   .addTransition("START", "B", ABNF::literal('x'), Transition::PRIORITY_LOW)
                   .addTransition("A", "ACCEPT", ABNF::literal('y'))
                   .addTransition("B", "ACCEPT", ABNF::literal('z'))
                   .build();

    EXPECT_TRUE(fsm->validate("xy"));

    fsm->reset();
    EXPECT_FALSE(fsm->validate("xz"));

    fsm->reset();
    EXPECT_TRUE(fsm->validateWithBacktracking("xy"));

    fsm->reset();
    EXPECT_TRUE(fsm->validateWithBacktracking("xz"));
}
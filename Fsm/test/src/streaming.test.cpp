#include <gtest/gtest.h>
#include <fsm/fsm.hpp>
#include <abnf/abnf.hpp>

using namespace fsm;
using namespace abnf;

class StreamingTest : public ::testing::Test
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
// Basic Streaming Tests
// ============================================================================

TEST_F(StreamingTest, FeedSingleCharacter)
{
    auto fsm = FSM::Builder("single_digit")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    StreamState state = fsm->feed('5');

    EXPECT_EQ(StreamState::COMPLETE, state);
    EXPECT_TRUE(fsm->isStreamComplete());
    EXPECT_TRUE(fsm->isInAcceptState());
}

TEST_F(StreamingTest, FeedMultipleCharacters)
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

    EXPECT_EQ(StreamState::WAITING_FOR_INPUT, fsm->feed('1'));
    EXPECT_EQ(StreamState::WAITING_FOR_INPUT, fsm->feed('2'));
    EXPECT_EQ(StreamState::COMPLETE, fsm->feed('3'));

    EXPECT_TRUE(fsm->isStreamComplete());
}

TEST_F(StreamingTest, FeedChunk)
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

    StreamState state = fsm->feed("123");

    EXPECT_EQ(StreamState::COMPLETE, state);
    EXPECT_TRUE(fsm->isStreamComplete());
}

TEST_F(StreamingTest, FeedError)
{
    auto fsm = FSM::Builder("only_digits")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    StreamState state = fsm->feed('a');

    EXPECT_EQ(StreamState::ERROR, state);
    EXPECT_FALSE(fsm->isStreamComplete());

    auto error = fsm->getLastError();
    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(FSM::ErrorType::NO_MATCHING_TRANSITION, error->type);
}

// ============================================================================
// End of Stream Tests
// ============================================================================

TEST_F(StreamingTest, EndOfStream_Success)
{
    auto fsm = FSM::Builder("digits_optional")
                   .addState("START", StateType::START)
                   .addState("DIGITS")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "DIGITS", ABNF::digit())
                   .addTransition("DIGITS", "DIGITS", ABNF::digit())
                   .addEpsilonTransition("DIGITS", "ACCEPT")
                   .build();

    fsm->feed("123");
    StreamState state = fsm->endOfStream();

    EXPECT_EQ(StreamState::COMPLETE, state);
    EXPECT_TRUE(fsm->isStreamComplete());
}

TEST_F(StreamingTest, EndOfStream_NotInAcceptState)
{
    auto fsm = FSM::Builder("needs_three_digits")
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

    fsm->feed("12");
    StreamState state = fsm->endOfStream();

    EXPECT_EQ(StreamState::ERROR, state);
    EXPECT_FALSE(fsm->isStreamComplete());

    auto error = fsm->getLastError();
    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(FSM::ErrorType::NOT_IN_ACCEPT_STATE, error->type);
}

TEST_F(StreamingTest, EndOfStream_WithoutFeed)
{
    auto fsm = FSM::Builder("test")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    StreamState state = fsm->endOfStream();

    EXPECT_EQ(StreamState::ERROR, state);
}

// ============================================================================
// Reset and Reuse Tests
// ============================================================================

TEST_F(StreamingTest, ResetAndReuse)
{
    auto fsm = FSM::Builder("digit")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    fsm->feed('5');
    EXPECT_TRUE(fsm->isStreamComplete());

    fsm->reset();

    fsm->feed('7');
    EXPECT_TRUE(fsm->isStreamComplete());
}

TEST_F(StreamingTest, ResetStream_OnlyStreamState)
{
    auto fsm = FSM::Builder("digit")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .build();

    fsm->feed('5');
    EXPECT_TRUE(fsm->isStreamComplete());

    fsm->resetStream();

    EXPECT_EQ(StreamState::READY, fsm->getStreamState());
    EXPECT_FALSE(fsm->isStreamComplete());

    EXPECT_TRUE(fsm->isInAcceptState());
}

// ============================================================================
// Streaming with Captures
// ============================================================================

TEST_F(StreamingTest, StreamingWithCaptures)
{
    auto fsm = FSM::Builder("capture_digits")
                   .addState("START", StateType::START)
                   .addState("DIGITS")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .onStateEntry("DIGITS", [](const StateContext &ctx)
                                 {
            auto* fsm_ptr = static_cast<FSM*>(ctx.user_data);
            if (fsm_ptr) {
                fsm_ptr->beginCapture("digits");
            } })
                   .addTransition("START", "DIGITS", ABNF::digit())
                   .addTransition("DIGITS", "DIGITS", ABNF::digit())
                   .onStateExit("DIGITS", [](const StateContext &ctx)
                                {
            auto* fsm_ptr = static_cast<FSM*>(ctx.user_data);
            if (fsm_ptr) {
                fsm_ptr->endCapture("digits");
            } })
                   .addEpsilonTransition("DIGITS", "ACCEPT")
                   .build();

    fsm->setUserData(fsm.get());

    fsm->feed('1');
    fsm->feed('2');
    fsm->feed('3');
    fsm->endOfStream();

    EXPECT_TRUE(fsm->isStreamComplete());

    auto capture = fsm->getCapture("digits");
    ASSERT_TRUE(capture.has_value());
    EXPECT_EQ("123", capture->value);
}

// ============================================================================
// Streaming with Metrics
// ============================================================================

TEST_F(StreamingTest, StreamingWithMetrics)
{
    auto fsm = FSM::Builder("digits")
                   .setDebugFlags(DebugFlags::COLLECT_METRICS)
                   .addState("START", StateType::START)
                   .addState("DIGITS")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "DIGITS", ABNF::digit())
                   .addTransition("DIGITS", "DIGITS", ABNF::digit())
                   .addEpsilonTransition("DIGITS", "ACCEPT")
                   .build();

    fsm->feed("12345");
    fsm->endOfStream();

    const auto &metrics = fsm->getMetrics();

    EXPECT_EQ(5, metrics.characters_processed);
    EXPECT_EQ(5, metrics.transitions_taken);
    EXPECT_EQ(1, metrics.epsilon_transitions);
}

// ============================================================================
// Large Stream Test
// ============================================================================

TEST_F(StreamingTest, LargeStream)
{
    auto fsm = FSM::Builder("many_digits")
                   .addState("START", StateType::START)
                   .addState("DIGITS", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("DIGITS")
                   .addTransition("START", "DIGITS", ABNF::digit())
                   .addTransition("DIGITS", "DIGITS", ABNF::digit())
                   .build();

    for (int i = 0; i < 10000; ++i)
    {
        char digit = '0' + (i % 10);
        StreamState state = fsm->feed(digit);

        EXPECT_NE(StreamState::ERROR, state);
    }

    EXPECT_TRUE(fsm->isStreamComplete());
}

// ============================================================================
// Interleaved Processing
// ============================================================================

TEST_F(StreamingTest, InterleavedFeedAndCheck)
{
    auto fsm = FSM::Builder("http_method")
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

    EXPECT_EQ(StreamState::WAITING_FOR_INPUT, fsm->feed('G'));
    EXPECT_FALSE(fsm->isInAcceptState());

    EXPECT_EQ(StreamState::WAITING_FOR_INPUT, fsm->feed('E'));
    EXPECT_FALSE(fsm->isInAcceptState());

    EXPECT_EQ(StreamState::COMPLETE, fsm->feed('T'));
    EXPECT_TRUE(fsm->isInAcceptState());
}

// ============================================================================
// Comparison: validate() vs streaming
// ============================================================================

TEST_F(StreamingTest, CompareValidateVsStreaming)
{
    auto fsm1 = FSM::Builder("test1")
                    .addState("START", StateType::START)
                    .addState("DIGITS")
                    .addState("ACCEPT", StateType::ACCEPT)
                    .setStartState("START")
                    .addAcceptState("ACCEPT")
                    .addTransition("START", "DIGITS", ABNF::digit())
                    .addTransition("DIGITS", "DIGITS", ABNF::digit())
                    .addEpsilonTransition("DIGITS", "ACCEPT")
                    .build();

    auto fsm2 = FSM::Builder("test2")
                    .addState("START", StateType::START)
                    .addState("DIGITS")
                    .addState("ACCEPT", StateType::ACCEPT)
                    .setStartState("START")
                    .addAcceptState("ACCEPT")
                    .addTransition("START", "DIGITS", ABNF::digit())
                    .addTransition("DIGITS", "DIGITS", ABNF::digit())
                    .addEpsilonTransition("DIGITS", "ACCEPT")
                    .build();

    std::string input = "12345";

    bool result1 = fsm1->validate(input);

    fsm2->feed(input);
    bool result2 = fsm2->endOfStream() == StreamState::COMPLETE;

    EXPECT_EQ(result1, result2);
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
}
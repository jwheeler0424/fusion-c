#include <gtest/gtest.h>
#include <fsm/fsm.hpp>
#include <abnf/abnf.hpp>

using namespace fsm;
using namespace abnf;

class CallbacksCapturesTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ============================================================================
// Action Callbacks Tests
// ============================================================================

TEST_F(CallbacksCapturesTest, StateEntryCallback)
{
    bool entry_called = false;

    auto fsm = FSM::Builder("entry_test")
                   .addState("START", StateType::START)
                   .addState("TARGET", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("TARGET")
                   .onStateEntry("TARGET", [&entry_called](const StateContext &ctx)
                                 { entry_called = true; })
                   .addTransition("START", "TARGET", ABNF::digit())
                   .build();

    fsm->validate("5");
    EXPECT_TRUE(entry_called);
}

TEST_F(CallbacksCapturesTest, StateExitCallback)
{
    bool exit_called = false;

    auto fsm = FSM::Builder("exit_test")
                   .addState("START", StateType::START)
                   .addState("MIDDLE")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .onStateExit("START", [&exit_called](const StateContext &ctx)
                                { exit_called = true; })
                   .addTransition("START", "MIDDLE", ABNF::digit())
                   .addEpsilonTransition("MIDDLE", "ACCEPT")
                   .build();

    fsm->validate("5");
    EXPECT_TRUE(exit_called);
}

TEST_F(CallbacksCapturesTest, TransitionCallback)
{
    bool transition_called = false;

    auto fsm = FSM::Builder("transition_test")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .onTransition([&transition_called](const TransitionContext &ctx)
                                 { transition_called = true; })
                   .build();

    fsm->validate("5");
    EXPECT_TRUE(transition_called);
}

TEST_F(CallbacksCapturesTest, CallbacksWithUserData)
{
    struct UserData
    {
        int counter = 0;
    };

    UserData data;

    auto fsm = FSM::Builder("userdata_test")
                   .addState("START", StateType::START)
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .onStateEntry("ACCEPT", [](const StateContext &ctx)
                                 {
            auto* user_data = static_cast<UserData*>(ctx.user_data);
            if (user_data) {
                user_data->counter++;
            } })
                   .addTransition("START", "ACCEPT", ABNF::digit())
                   .withUserData(&data)
                   .build();

    fsm->validate("5");
    EXPECT_EQ(1, data.counter);
}

// ============================================================================
// Capture Groups Tests
// ============================================================================

TEST_F(CallbacksCapturesTest, BasicCapture)
{
    auto fsm = FSM::Builder("capture_test")
                   .addState("START", StateType::START)
                   .addState("DIGITS")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .onStateEntry("DIGITS", [](const StateContext &ctx)
                                 {
            auto* f = static_cast<FSM*>(ctx.user_data);
            f->beginCapture("number"); })
                   .addTransition("START", "DIGITS", ABNF::digit())
                   .addTransition("DIGITS", "DIGITS", ABNF::digit())
                   .onStateExit("DIGITS", [](const StateContext &ctx)
                                {
            auto* f = static_cast<FSM*>(ctx.user_data);
            f->endCapture("number"); })
                   .addEpsilonTransition("DIGITS", "ACCEPT")
                   .build();

    fsm->setUserData(fsm.get());
    fsm->validate("12345");

    auto capture = fsm->getCapture("number");
    ASSERT_TRUE(capture.has_value());
    EXPECT_EQ("12345", capture->value);
    EXPECT_EQ(5, capture->length());
}

TEST_F(CallbacksCapturesTest, MultipleCaptures)
{
    auto fsm = FSM::Builder("multi_capture")
                   .addState("START", StateType::START)
                   .addState("LETTERS")
                   .addState("DIGITS")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .onStateEntry("LETTERS", [](const StateContext &ctx)
                                 { static_cast<FSM *>(ctx.user_data)->beginCapture("letters"); })
                   .onStateExit("LETTERS", [](const StateContext &ctx)
                                { static_cast<FSM *>(ctx.user_data)->endCapture("letters"); })
                   .onStateEntry("DIGITS", [](const StateContext &ctx)
                                 { static_cast<FSM *>(ctx.user_data)->beginCapture("digits"); })
                   .onStateExit("DIGITS", [](const StateContext &ctx)
                                { static_cast<FSM *>(ctx.user_data)->endCapture("digits"); })
                   .addTransition("START", "LETTERS", ABNF::alpha())
                   .addTransition("LETTERS", "LETTERS", ABNF::alpha())
                   .addEpsilonTransition("LETTERS", "DIGITS")
                   .addTransition("DIGITS", "DIGITS", ABNF::digit())
                   .addEpsilonTransition("DIGITS", "ACCEPT")
                   .build();

    fsm->setUserData(fsm.get());
    fsm->validate("abc123");

    auto letters = fsm->getCapture("letters");
    auto digits = fsm->getCapture("digits");

    ASSERT_TRUE(letters.has_value());
    ASSERT_TRUE(digits.has_value());
    EXPECT_EQ("abc", letters->value);
    EXPECT_EQ("123", digits->value);
}

TEST_F(CallbacksCapturesTest, GetCaptureByIndex)
{
    auto fsm = FSM::Builder("index_capture")
                   .addState("START", StateType::START)
                   .addState("CAP")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .onStateEntry("CAP", [](const StateContext &ctx)
                                 { static_cast<FSM *>(ctx.user_data)->beginCapture("data"); })
                   .onStateExit("CAP", [](const StateContext &ctx)
                                { static_cast<FSM *>(ctx.user_data)->endCapture("data"); })
                   .addTransition("START", "CAP", ABNF::digit())
                   .addEpsilonTransition("CAP", "ACCEPT")
                   .build();

    fsm->setUserData(fsm.get());
    fsm->validate("5");

    auto capture = fsm->getCaptureByIndex(0);
    ASSERT_TRUE(capture.has_value());
    EXPECT_EQ("5", capture->value);
}

TEST_F(CallbacksCapturesTest, ClearCaptures)
{
    auto fsm = FSM::Builder("clear_test")
                   .addState("START", StateType::START)
                   .addState("CAP")
                   .addState("ACCEPT", StateType::ACCEPT)
                   .setStartState("START")
                   .addAcceptState("ACCEPT")
                   .onStateEntry("CAP", [](const StateContext &ctx)
                                 { static_cast<FSM *>(ctx.user_data)->beginCapture("data"); })
                   .onStateExit("CAP", [](const StateContext &ctx)
                                { static_cast<FSM *>(ctx.user_data)->endCapture("data"); })
                   .addTransition("START", "CAP", ABNF::digit())
                   .addEpsilonTransition("CAP", "ACCEPT")
                   .build();

    fsm->setUserData(fsm.get());
    fsm->validate("5");

    EXPECT_TRUE(fsm->hasCapture("data"));

    fsm->clearCaptures();

    EXPECT_FALSE(fsm->hasCapture("data"));
}
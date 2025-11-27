#ifndef FSM_HPP
#define FSM_HPP

#include <abnf/abnf.hpp>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <stack>

using namespace abnf;

namespace fsm
{
    // Forward declarations
    class State;
    class Transition;
    struct TransitionContext;
    struct StateContext;
    class FSM;

    // ============================================================================
    // Debug Flags (Bit Manipulation)
    // ============================================================================

    enum class DebugFlags : uint8_t
    {
        NONE = 0b00000000,
        TRACE_TRANSITIONS = 0b00000001,
        TRACE_STATE_CHANGES = 0b00000010,
        VERBOSE_ERRORS = 0b00000100,
        COLLECT_METRICS = 0b00001000,
        EXPORT_DOT_ON_ERROR = 0b00010000,
        RESERVED_1 = 0b00100000,
        RESERVED_2 = 0b01000000,
        RESERVED_3 = 0b10000000,

        ALL = 0b00011111,
        BASIC = TRACE_TRANSITIONS | VERBOSE_ERRORS,
        FULL = TRACE_TRANSITIONS | TRACE_STATE_CHANGES | VERBOSE_ERRORS | COLLECT_METRICS,

#ifdef NDEBUG
        AUTO = NONE,
#else
        AUTO = BASIC
#endif
    };

    inline DebugFlags operator|(DebugFlags a, DebugFlags b)
    {
        return static_cast<DebugFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline DebugFlags operator&(DebugFlags a, DebugFlags b)
    {
        return static_cast<DebugFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    inline DebugFlags operator~(DebugFlags a)
    {
        return static_cast<DebugFlags>(~static_cast<uint8_t>(a));
    }

    inline DebugFlags &operator|=(DebugFlags &a, DebugFlags b)
    {
        a = a | b;
        return a;
    }

    inline DebugFlags &operator&=(DebugFlags &a, DebugFlags b)
    {
        a = a & b;
        return a;
    }

    inline bool hasFlag(DebugFlags flags, DebugFlags flag)
    {
        return (flags & flag) == flag;
    }

    struct DebugConfig
    {
        DebugFlags flags = DebugFlags::NONE;
        std::ostream *log_stream = nullptr;

        DebugConfig() = default;
        explicit DebugConfig(DebugFlags f) : flags(f) {}
        DebugConfig(DebugFlags f, std::ostream &os) : flags(f), log_stream(&os) {}

        [[nodiscard]] bool isEnabled() const { return flags != DebugFlags::NONE; }
        [[nodiscard]] bool hasTraceTransitions() const { return hasFlag(flags, DebugFlags::TRACE_TRANSITIONS); }
        [[nodiscard]] bool hasTraceStateChanges() const { return hasFlag(flags, DebugFlags::TRACE_STATE_CHANGES); }
        [[nodiscard]] bool hasVerboseErrors() const { return hasFlag(flags, DebugFlags::VERBOSE_ERRORS); }
        [[nodiscard]] bool hasCollectMetrics() const { return hasFlag(flags, DebugFlags::COLLECT_METRICS); }
        [[nodiscard]] bool hasExportDotOnError() const { return hasFlag(flags, DebugFlags::EXPORT_DOT_ON_ERROR); }

        void enable(DebugFlags flag) { flags |= flag; }
        void disable(DebugFlags flag) { flags &= ~flag; }
        void toggle(DebugFlags flag)
        {
            flags = static_cast<DebugFlags>(static_cast<uint8_t>(flags) ^ static_cast<uint8_t>(flag));
        }

        std::ostream &getOutputStream() const
        {
            return log_stream ? *log_stream : std::cerr;
        }
    };

    // ============================================================================
    // StateID - Hybrid Identification
    // ============================================================================

    struct StateID
    {
        uint32_t id;
        std::string name;

        explicit StateID(uint32_t id_val = 0) : id(id_val), name() {}
        StateID(uint32_t id_val, std::string name_val) : id(id_val), name(std::move(name_val)) {}

        bool operator==(const StateID &other) const { return id == other.id; }
        bool operator!=(const StateID &other) const { return id != other.id; }
        bool operator<(const StateID &other) const { return id < other.id; }

        struct Hash
        {
            size_t operator()(const StateID &sid) const
            {
                return std::hash<uint32_t>{}(sid.id);
            }
        };

        [[nodiscard]] std::string toString() const
        {
            return name.empty() ? "State_" + std::to_string(id) : name;
        }

        [[nodiscard]] bool isValid() const { return id != 0; }
    };

    // ============================================================================
    // Callback Type Definitions
    // ============================================================================

    using TransitionCallback = std::function<void(const TransitionContext &)>;
    using StateEntryCallback = std::function<void(const StateContext &)>;
    using StateExitCallback = std::function<void(const StateContext &)>;

    // ============================================================================
    // Context Structures
    // ============================================================================

    struct TransitionContext
    {
        StateID from_state;
        StateID to_state;
        char input_char;
        size_t position;
        const Transition *transition;
        void *user_data;

        TransitionContext(StateID from, StateID to, char ch, size_t pos,
                          const Transition *trans, void *data = nullptr)
            : from_state(from), to_state(to), input_char(ch), position(pos),
              transition(trans), user_data(data) {}
    };

    struct StateContext
    {
        StateID state;
        size_t position;
        char current_char;
        void *user_data;

        StateContext(StateID s, size_t pos, char ch, void *data = nullptr)
            : state(s), position(pos), current_char(ch), user_data(data) {}
    };

    // ============================================================================
    // Capture Groups
    // ============================================================================

    struct CaptureGroup
    {
        std::string name;
        size_t start_position;
        size_t end_position;
        std::string value;

        CaptureGroup() = default;
        CaptureGroup(const std::string &n, size_t start, size_t end, const std::string &val)
            : name(n), start_position(start), end_position(end), value(val) {}

        [[nodiscard]] size_t length() const { return end_position - start_position; }
        [[nodiscard]] bool isEmpty() const { return start_position == end_position; }
    };

    struct ActiveCapture
    {
        std::string name;
        size_t start_position;
        std::string buffer;

        ActiveCapture(const std::string &n, size_t pos)
            : name(n), start_position(pos) {}
    };

    // ============================================================================
    // Streaming State
    // ============================================================================

    enum class StreamState
    {
        READY,
        PROCESSING,
        WAITING_FOR_INPUT,
        COMPLETE,
        ERROR
    };

    // ============================================================================
    // Backtracking Support
    // ============================================================================

    struct BacktrackingStats
    {
        size_t choice_points_created = 0;
        size_t backtracks_performed = 0;
        size_t max_stack_depth = 0;
        size_t paths_explored = 0;

        void reset()
        {
            choice_points_created = 0;
            backtracks_performed = 0;
            max_stack_depth = 0;
            paths_explored = 0;
        }

        [[nodiscard]] std::string toString() const;
    };

    struct ChoicePoint
    {
        StateID state;
        size_t position;
        std::vector<const class Transition *> remaining;

        std::vector<CaptureGroup> captures_snapshot;
        std::vector<ActiveCapture> active_captures_snapshot;
        size_t input_position_snapshot;

        ChoicePoint(StateID s, size_t pos,
                    const std::vector<const class Transition *> &trans,
                    const std::vector<CaptureGroup> &caps,
                    const std::vector<ActiveCapture> &active,
                    size_t input_pos)
            : state(s), position(pos), remaining(trans),
              captures_snapshot(caps), active_captures_snapshot(active),
              input_position_snapshot(input_pos) {}
    };

    // ============================================================================
    // State
    // ============================================================================

    enum class StateType
    {
        NORMAL,
        START,
        ACCEPT,
        ERROR
    };

    class State
    {
    public:
        StateID id;
        StateType type;
        std::string description;
        bool is_choice_point = false;

        StateEntryCallback on_entry;
        StateExitCallback on_exit;

        State() : id(), type(StateType::NORMAL) {}
        explicit State(StateID state_id, StateType state_type = StateType::NORMAL)
            : id(state_id), type(state_type) {}
        State(StateID state_id, StateType state_type, const std::string &desc)
            : id(state_id), type(state_type), description(desc) {}

        [[nodiscard]] std::string toDebugString() const;
        [[nodiscard]] std::string typeToString() const;
    };

    // ============================================================================
    // Transition
    // ============================================================================

    enum class TransitionType
    {
        ABNF_RULE,
        FSM_INSTANCE,
        EPSILON
    };

    class Transition
    {
    public:
        using TransitionID = uint32_t;

        static constexpr int PRIORITY_LOWEST = 0;
        static constexpr int PRIORITY_LOW = 25;
        static constexpr int PRIORITY_NORMAL = 50;
        static constexpr int PRIORITY_HIGH = 75;
        static constexpr int PRIORITY_HIGHEST = 100;

        TransitionID id;
        StateID from;
        StateID to;
        TransitionType type;
        TransitionCallback on_transition;

        std::optional<ABNF> rule;
        std::shared_ptr<FSM> embedded_fsm;

        int priority;
        std::string description;

        Transition(TransitionID trans_id, StateID from_state, StateID to_state,
                   const ABNF &abnf_rule, int prio = PRIORITY_NORMAL);
        Transition(TransitionID trans_id, StateID from_state, StateID to_state,
                   std::shared_ptr<FSM> fsm, int prio = PRIORITY_NORMAL);
        Transition(TransitionID trans_id, StateID from_state, StateID to_state);

        [[nodiscard]] bool matches(char ch) const;
        [[nodiscard]] std::string toDebugString() const;
        [[nodiscard]] std::string typeToString() const;
    };

    // ============================================================================
    // FSM
    // ============================================================================

    class FSM : public std::enable_shared_from_this<FSM>
    {
    public:
        class Builder;

        enum class ErrorType
        {
            NO_MATCHING_TRANSITION,
            UNEXPECTED_END_OF_INPUT,
            NOT_IN_ACCEPT_STATE,
            EMBEDDED_FSM_FAILED,
            INVALID_STATE,
            INVALID_TRANSITION,
            AMBIGUOUS_TRANSITION,
            NO_START_STATE,
            UNREACHABLE_STATES
        };

        struct ValidationError
        {
            ErrorType type;
            size_t position;
            char character;
            StateID current_state;
            std::string message;
            std::vector<StateID> attempted_states;
            std::string input_context;

            [[nodiscard]] std::string toString() const;
        };

        struct TraceEntry
        {
            size_t step;
            StateID from_state;
            StateID to_state;
            char input_char;
            Transition::TransitionID transition_id;
            std::string description;

            [[nodiscard]] std::string toString() const;
        };

        struct Metrics
        {
            size_t transitions_taken = 0;
            size_t states_visited = 0;
            size_t characters_processed = 0;
            size_t epsilon_transitions = 0;
            uint64_t validation_time_ns = 0;
            std::chrono::microseconds processing_time{0};

            void reset();
            [[nodiscard]] std::string toString() const;
        };

        struct InitialConfig
        {
            std::vector<State> states;
            std::vector<Transition> transitions;
            StateID start_state;
            std::vector<StateID> accept_states;
        };

        // Construction
        FSM();
        explicit FSM(const std::string &name);
        FSM(uint32_t id, const std::string &name);
        explicit FSM(const InitialConfig &config);
        FSM(const std::string &name, const InitialConfig &config);

        // State Management
        StateID addState(const std::string &name, StateType type = StateType::NORMAL);
        StateID addState(const std::string &name, const std::string &description,
                         StateType type = StateType::NORMAL);

        void setStartState(StateID state);
        [[nodiscard]] StateID getStartState() const;

        void addAcceptState(StateID state);
        void removeAcceptState(StateID state);
        [[nodiscard]] bool isAcceptState(StateID state) const;
        [[nodiscard]] const std::unordered_set<StateID, StateID::Hash> &getAcceptStates() const;

        [[nodiscard]] StateID getCurrentState() const;
        [[nodiscard]] const State &getState(StateID id) const;
        [[nodiscard]] bool hasState(StateID id) const;

        // Transition Management
        Transition::TransitionID addTransition(StateID from, StateID to, const ABNF &rule,
                                               int priority = Transition::PRIORITY_NORMAL);
        Transition::TransitionID addTransition(StateID from, StateID to, const ABNF &rule,
                                               const std::string &description,
                                               int priority = Transition::PRIORITY_NORMAL);

        Transition::TransitionID addEpsilonTransition(StateID from, StateID to);

        std::vector<Transition::TransitionID> mergeEmbeddedFSM(
            StateID from, StateID to, std::shared_ptr<FSM> embedded_fsm,
            int priority = Transition::PRIORITY_NORMAL);

        // Input Processing
        bool validate(std::string_view input);
        [[nodiscard]] bool isInAcceptState() const;
        void reset();

        // Streaming Input (Phase 2. 4)
        StreamState feed(char ch);
        StreamState feed(std::string_view chunk);
        StreamState endOfStream();
        [[nodiscard]] StreamState getStreamState() const;
        [[nodiscard]] bool isStreamComplete() const;
        [[nodiscard]] bool needsMoreInput() const;
        void resetStream();
        static std::string streamStateToString(StreamState state);

        // Backtracking (Phase 2.5)
        bool validateWithBacktracking(std::string_view input);
        void markAsChoicePoint(StateID state);
        [[nodiscard]] bool isChoicePoint(StateID state) const;
        [[nodiscard]] const BacktrackingStats &getBacktrackingStats() const;
        void resetBacktrackingStats();
        void setMaxBacktrackDepth(size_t depth);
        [[nodiscard]] size_t getMaxBacktrackDepth() const;

        // Error Reporting
        [[nodiscard]] std::optional<ValidationError> getLastError() const;

        // Debug Support
        void setDebugConfig(const DebugConfig &config);
        [[nodiscard]] DebugConfig &getDebugConfig();
        [[nodiscard]] const DebugConfig &getDebugConfig() const;

        [[nodiscard]] std::string toString() const;
        [[nodiscard]] std::string toDebugString() const;
        [[nodiscard]] std::string toDot() const;
        void exportDot(const std::string &filename) const;

        [[nodiscard]] const std::vector<TraceEntry> &getTrace() const;
        void clearTrace();

        [[nodiscard]] const Metrics &getMetrics() const;
        void resetMetrics();

        // Validation
        [[nodiscard]] bool isValid() const;
        [[nodiscard]] std::vector<std::string> validateStructure() const;

        // Introspection
        [[nodiscard]] size_t getStateCount() const;
        [[nodiscard]] size_t getTransitionCount() const;
        [[nodiscard]] std::vector<StateID> getStates() const;
        [[nodiscard]] std::vector<Transition> getTransitions() const;
        [[nodiscard]] std::vector<Transition *> getTransitionsFrom(StateID state) const;

        [[nodiscard]] const std::string &getName() const;
        [[nodiscard]] uint32_t getID() const;

        // Action Callbacks
        void setStateEntryCallback(StateID state, StateEntryCallback callback);
        void setStateExitCallback(StateID state, StateExitCallback callback);
        void setTransitionCallback(Transition::TransitionID transition_id,
                                   TransitionCallback callback);
        void setUserData(void *data);
        void *getUserData() const;

        // Capture Groups
        void beginCapture(const std::string &name);
        CaptureGroup endCapture(const std::string &name);
        std::optional<CaptureGroup> getCapture(const std::string &name) const;
        const std::vector<CaptureGroup> &getAllCaptures() const;
        std::optional<CaptureGroup> getCaptureByIndex(size_t index) const;
        void clearCaptures();
        bool hasCapture(const std::string &name) const;

        // SIMD Support (Phase 4. 2)
        void setSIMDEnabled(bool enabled);
        [[nodiscard]] bool isSIMDEnabled() const;
        [[nodiscard]] std::string getSIMDCapabilities() const;

    private:
        uint32_t id_;
        std::string name_;

        std::unordered_map<StateID, State, StateID::Hash> states_;
        std::vector<Transition> transitions_;
        std::unordered_map<StateID, std::vector<Transition *>, StateID::Hash> transition_map_;

        StateID start_state_;
        std::unordered_set<StateID, StateID::Hash> accept_states_;
        StateID current_state_;

        DebugConfig debug_config_;
        std::vector<TraceEntry> trace_;
        std::optional<ValidationError> last_error_;
        Metrics metrics_;

        uint32_t next_state_id_;
        uint32_t next_transition_id_;

        void *user_data_ = nullptr;

        std::vector<CaptureGroup> captures_;
        std::vector<ActiveCapture> active_captures_;
        size_t current_input_position_ = 0;
        std::string current_input_;

        // Streaming
        StreamState stream_state_ = StreamState::READY;
        bool streaming_mode_ = false;

        // Backtracking
        std::stack<ChoicePoint> choice_stack_;
        BacktrackingStats backtracking_stats_;
        size_t max_backtrack_depth_ = 0;

        // SIMD
        bool simd_enabled_ = true;

        void processEpsilonTransitions(size_t position);
        void recordCharInCaptures(char ch);
        void updateCapturePosition(size_t pos);

        bool processCharImpl(char ch, size_t position);
        void sortTransitionsByPriority();
        void logTransition(const TraceEntry &entry);
        void logStateChange(StateID from, StateID to);
        std::string getInputContext(std::string_view input, size_t position) const;

        struct MergeResult
        {
            std::unordered_map<StateID, StateID, StateID::Hash> state_mapping;
            std::vector<Transition::TransitionID> new_transitions;
        };

        MergeResult mergeStatesAndTransitions(StateID from_state, StateID to_state, const FSM &embedded);
        void rebuildTransitionMap();
        bool transition_map_dirty_ = true;

        // Backtracking helpers
        std::vector<const Transition *> getValidTransitions(char ch);
        bool shouldCreateChoicePoint(const std::vector<const Transition *> &valid_transitions) const;
        void saveChoicePoint(const std::vector<const Transition *> &alternatives, size_t position);
        bool backtrack();
        void restoreFromChoicePoint(const ChoicePoint &cp);
    };

    // ============================================================================
    // Builder
    // ============================================================================

    class FSM::Builder
    {
    public:
        explicit Builder(const std::string &name);
        Builder(uint32_t id, const std::string &name);

        Builder &withInitialConfig(const InitialConfig &config);

        Builder &addState(const std::string &name, StateType type = StateType::NORMAL);
        Builder &addState(const std::string &name, const std::string &description,
                          StateType type = StateType::NORMAL);
        Builder &setStartState(const std::string &name);
        Builder &addAcceptState(const std::string &name);

        Builder &addTransition(const std::string &from, const std::string &to,
                               const ABNF &rule, int priority = Transition::PRIORITY_NORMAL);
        Builder &addTransition(const std::string &from, const std::string &to,
                               const ABNF &rule, const std::string &description,
                               int priority = Transition::PRIORITY_NORMAL);

        Builder &addTransition(const std::string &from, const std::string &to,
                               std::shared_ptr<FSM> fsm, int priority = Transition::PRIORITY_NORMAL);
        Builder &addTransition(const std::string &from, const std::string &to,
                               const FSM &fsm, int priority = Transition::PRIORITY_NORMAL);

        Builder &addEpsilonTransition(const std::string &from, const std::string &to);

        Builder &setDebugFlags(DebugFlags flags);
        Builder &enableDebugFlag(DebugFlags flag);
        Builder &disableDebugFlag(DebugFlags flag);
        Builder &withDebugOutput(std::ostream &os);
        Builder &enableBasicDebug();
        Builder &enableFullDebug();
        Builder &disableDebug();

        Builder &onStateEntry(const std::string &state_name, StateEntryCallback callback);
        Builder &onStateExit(const std::string &state_name, StateExitCallback callback);
        Builder &onTransition(TransitionCallback callback);
        Builder &withUserData(void *data);

        Builder &markChoicePoint(const std::string &state_name);

        [[nodiscard]] std::shared_ptr<FSM> build();

    private:
        std::string name_;
        uint32_t id_;
        std::vector<State> states_;
        std::vector<Transition> transitions_;
        std::unordered_map<std::string, StateID> name_to_id_;
        std::optional<StateID> start_state_;
        std::vector<StateID> accept_states_;
        DebugConfig debug_config_;
        uint32_t next_id_;
        std::vector<std::pair<std::string, std::shared_ptr<FSM>>> pending_fsm_merges_;
        void *user_data_ = nullptr;

        StateID getOrCreateState(const std::string &name);
    };

} // namespace fsm

#endif // FSM_HPP
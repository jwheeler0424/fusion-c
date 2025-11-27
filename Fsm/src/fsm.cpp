#include <fsm/fsm.hpp>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace fsm
{

    // ============================================================================
    // State Implementation
    // ============================================================================

    std::string State::toDebugString() const
    {
        std::ostringstream oss;
        oss << "State{id=" << id.toString()
            << ", type=" << typeToString()
            << ", desc=\"" << description << "\"}";
        return oss.str();
    }

    std::string State::typeToString() const
    {
        switch (type)
        {
        case StateType::NORMAL:
            return "NORMAL";
        case StateType::START:
            return "START";
        case StateType::ACCEPT:
            return "ACCEPT";
        case StateType::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    // ============================================================================
    // Transition Implementation
    // ============================================================================

    Transition::Transition(TransitionID trans_id, StateID from_state, StateID to_state,
                           const ABNF &abnf_rule, int prio)
        : id(trans_id), from(from_state), to(to_state), type(TransitionType::ABNF_RULE),
          rule(abnf_rule), priority(prio), description(abnf_rule.toString())
    {
    }

    Transition::Transition(TransitionID trans_id, StateID from_state, StateID to_state,
                           std::shared_ptr<FSM> fsm, int prio)
        : id(trans_id), from(from_state), to(to_state), type(TransitionType::FSM_INSTANCE),
          embedded_fsm(std::move(fsm)), priority(prio), description("FSM Instance")
    {
    }

    Transition::Transition(TransitionID trans_id, StateID from_state, StateID to_state)
        : id(trans_id), from(from_state), to(to_state), type(TransitionType::EPSILON),
          priority(PRIORITY_NORMAL), description("Epsilon")
    {
    }

    bool Transition::matches(char ch) const
    {
        if (type == TransitionType::ABNF_RULE && rule.has_value())
        {
            return rule->matches(ch);
        }
        return false;
    }

    std::string Transition::toDebugString() const
    {
        std::ostringstream oss;
        oss << "Transition{id=" << id
            << ", from=" << from.toString()
            << ", to=" << to.toString()
            << ", type=" << typeToString()
            << ", priority=" << priority;

        if (!description.empty())
        {
            oss << ", desc=\"" << description << "\"";
        }

        if (type == TransitionType::ABNF_RULE && rule.has_value())
        {
            oss << ", rule=" << rule->toString();
        }
        else if (type == TransitionType::FSM_INSTANCE && embedded_fsm)
        {
            oss << ", fsm=" << embedded_fsm->getName();
        }

        oss << "}";
        return oss.str();
    }

    std::string Transition::typeToString() const
    {
        switch (type)
        {
        case TransitionType::ABNF_RULE:
            return "ABNF_RULE";
        case TransitionType::FSM_INSTANCE:
            return "FSM_INSTANCE";
        case TransitionType::EPSILON:
            return "EPSILON";
        default:
            return "UNKNOWN";
        }
    }

    // ============================================================================
    // FSM::ValidationError Implementation
    // ============================================================================

    std::string FSM::ValidationError::toString() const
    {
        std::ostringstream oss;
        oss << "ValidationError{";

        switch (type)
        {
        case ErrorType::NO_MATCHING_TRANSITION:
            oss << "NO_MATCHING_TRANSITION";
            break;
        case ErrorType::UNEXPECTED_END_OF_INPUT:
            oss << "UNEXPECTED_END_OF_INPUT";
            break;
        case ErrorType::NOT_IN_ACCEPT_STATE:
            oss << "NOT_IN_ACCEPT_STATE";
            break;
        case ErrorType::EMBEDDED_FSM_FAILED:
            oss << "EMBEDDED_FSM_FAILED";
            break;
        case ErrorType::INVALID_STATE:
            oss << "INVALID_STATE";
            break;
        case ErrorType::INVALID_TRANSITION:
            oss << "INVALID_TRANSITION";
            break;
        case ErrorType::AMBIGUOUS_TRANSITION:
            oss << "AMBIGUOUS_TRANSITION";
            break;
        case ErrorType::NO_START_STATE:
            oss << "NO_START_STATE";
            break;
        case ErrorType::UNREACHABLE_STATES:
            oss << "UNREACHABLE_STATES";
            break;
        }

        oss << ", position=" << position
            << ", character='" << character << "' (0x" << std::hex
            << static_cast<int>(static_cast<unsigned char>(character)) << std::dec << ")"
            << ", state=" << current_state.toString()
            << ", message=\"" << message << "\"";

        if (!input_context.empty())
        {
            oss << ", context=\"" << input_context << "\"";
        }

        oss << "}";
        return oss.str();
    }

    // ============================================================================
    // FSM::TraceEntry Implementation
    // ============================================================================

    std::string FSM::TraceEntry::toString() const
    {
        std::ostringstream oss;
        oss << "Step " << step << ": "
            << from_state.toString() << " -> " << to_state.toString()
            << " on '" << input_char << "' (transition #" << transition_id << ")";

        if (!description.empty())
        {
            oss << " [" << description << "]";
        }

        return oss.str();
    }

    // ============================================================================
    // FSM::Metrics Implementation
    // ============================================================================

    void FSM::Metrics::reset()
    {
        transitions_taken = 0;
        states_visited = 0;
        characters_processed = 0;
        epsilon_transitions = 0;
        validation_time_ns = 0;
        processing_time = std::chrono::microseconds{0};
    }

    std::string FSM::Metrics::toString() const
    {
        std::ostringstream oss;
        oss << "Metrics{"
            << "transitions=" << transitions_taken
            << ", states=" << states_visited
            << ", chars=" << characters_processed
            << ", epsilons=" << epsilon_transitions
            << ", validation_time=" << validation_time_ns << "ns"
            << ", processing_time=" << processing_time.count() << "μs"
            << "}";
        return oss.str();
    }

    // ============================================================================
    // BacktrackingStats Implementation
    // ============================================================================

    std::string BacktrackingStats::toString() const
    {
        std::ostringstream oss;
        oss << "BacktrackingStats{"
            << "choice_points=" << choice_points_created
            << ", backtracks=" << backtracks_performed
            << ", max_depth=" << max_stack_depth
            << ", paths=" << paths_explored
            << "}";
        return oss.str();
    }

    // ============================================================================
    // FSM Constructors
    // ============================================================================

    FSM::FSM()
        : id_(0), name_("FSM_0"), start_state_(0), current_state_(0),
          next_state_id_(1), next_transition_id_(1) {}

    FSM::FSM(const std::string &name)
        : id_(0), name_(name), start_state_(0), current_state_(0),
          next_state_id_(1), next_transition_id_(1) {}

    FSM::FSM(uint32_t id, const std::string &name)
        : id_(id), name_(name), start_state_(0), current_state_(0),
          next_state_id_(1), next_transition_id_(1) {}

    FSM::FSM(const InitialConfig &config)
        : FSM()
    {
        for (const auto &state : config.states)
        {
            states_[state.id] = state;
            if (state.id.id >= next_state_id_)
            {
                next_state_id_ = state.id.id + 1;
            }
        }

        for (const auto &trans : config.transitions)
        {
            transitions_.push_back(trans);
            if (trans.id >= next_transition_id_)
            {
                next_transition_id_ = trans.id + 1;
            }
        }

        start_state_ = config.start_state;
        current_state_ = start_state_;

        for (const auto &accept : config.accept_states)
        {
            accept_states_.insert(accept);
        }

        transition_map_dirty_ = true;
        rebuildTransitionMap();
    }

    FSM::FSM(const std::string &name, const InitialConfig &config)
        : FSM(config)
    {
        name_ = name;
    }

    // ============================================================================
    // State Management
    // ============================================================================

    StateID FSM::addState(const std::string &name, StateType type)
    {
        StateID sid(next_state_id_++, name);
        states_[sid] = State(sid, type);
        return sid;
    }

    StateID FSM::addState(const std::string &name, const std::string &description, StateType type)
    {
        StateID sid(next_state_id_++, name);
        states_[sid] = State(sid, type, description);
        return sid;
    }

    void FSM::setStartState(StateID state)
    {
        if (!hasState(state))
        {
            throw std::invalid_argument("Cannot set non-existent state as start state");
        }
        start_state_ = state;
        current_state_ = state;
        states_[state].type = StateType::START;
    }

    StateID FSM::getStartState() const
    {
        return start_state_;
    }

    void FSM::addAcceptState(StateID state)
    {
        if (!hasState(state))
        {
            throw std::invalid_argument("Cannot add non-existent state as accept state");
        }
        accept_states_.insert(state);

        if (states_[state].type != StateType::START)
        {
            states_[state].type = StateType::ACCEPT;
        }
    }

    void FSM::removeAcceptState(StateID state)
    {
        accept_states_.erase(state);
    }

    bool FSM::isAcceptState(StateID state) const
    {
        return accept_states_.find(state) != accept_states_.end();
    }

    const std::unordered_set<StateID, StateID::Hash> &FSM::getAcceptStates() const
    {
        return accept_states_;
    }

    StateID FSM::getCurrentState() const
    {
        return current_state_;
    }

    const State &FSM::getState(StateID id) const
    {
        auto it = states_.find(id);
        if (it == states_.end())
        {
            throw std::invalid_argument("State not found: " + id.toString());
        }
        return it->second;
    }

    bool FSM::hasState(StateID id) const
    {
        return states_.find(id) != states_.end();
    }

    // ============================================================================
    // Transition Management
    // ============================================================================

    Transition::TransitionID FSM::addTransition(StateID from, StateID to, const ABNF &rule, int priority)
    {
        if (!hasState(from) || !hasState(to))
        {
            throw std::invalid_argument("Cannot add transition with non-existent states");
        }

        Transition trans(next_transition_id_++, from, to, rule, priority);
        transitions_.push_back(trans);
        transition_map_dirty_ = true;

        return trans.id;
    }

    Transition::TransitionID FSM::addTransition(StateID from, StateID to, const ABNF &rule,
                                                const std::string &description, int priority)
    {
        auto id = addTransition(from, to, rule, priority);
        transitions_.back().description = description;
        return id;
    }

    Transition::TransitionID FSM::addEpsilonTransition(StateID from, StateID to)
    {
        if (!hasState(from) || !hasState(to))
        {
            throw std::invalid_argument("Cannot add epsilon transition with non-existent states");
        }

        Transition trans(next_transition_id_++, from, to);
        transitions_.push_back(trans);
        transition_map_dirty_ = true;

        return trans.id;
    }

    // ============================================================================
    // FSM Merging
    // ============================================================================

    std::vector<Transition::TransitionID> FSM::mergeEmbeddedFSM(
        StateID from,
        StateID to,
        std::shared_ptr<FSM> embedded_fsm,
        int priority)
    {
        if (!embedded_fsm)
        {
            throw std::invalid_argument("Cannot merge null FSM");
        }

        if (!hasState(from) || !hasState(to))
        {
            throw std::invalid_argument("Cannot merge FSM with non-existent states");
        }

        auto result = mergeStatesAndTransitions(from, to, *embedded_fsm);
        return result.new_transitions;
    }

    FSM::MergeResult FSM::mergeStatesAndTransitions(
        StateID from_state,
        StateID to_state,
        const FSM &embedded)
    {
        MergeResult result;

        result.state_mapping[embedded.start_state_] = from_state;

        for (const auto &accept : embedded.accept_states_)
        {
            result.state_mapping[accept] = to_state;
        }

        for (const auto &[state_id, state] : embedded.states_)
        {
            if (result.state_mapping.find(state_id) != result.state_mapping.end())
            {
                continue;
            }

            std::string new_name = state_id.name.empty()
                                       ? "State_" + std::to_string(state_id.id) + "_from_" + embedded.name_
                                       : state_id.name + "_from_" + embedded.name_;

            StateID new_id = addState(new_name, state.description, StateType::NORMAL);
            result.state_mapping[state_id] = new_id;
        }

        for (const auto &trans : embedded.transitions_)
        {
            StateID mapped_from = result.state_mapping[trans.from];
            StateID mapped_to = result.state_mapping[trans.to];

            Transition::TransitionID new_trans_id;

            switch (trans.type)
            {
            case TransitionType::ABNF_RULE:
                if (trans.rule.has_value())
                {
                    new_trans_id = addTransition(mapped_from, mapped_to, *trans.rule,
                                                 trans.description, trans.priority);
                }
                break;

            case TransitionType::EPSILON:
                new_trans_id = addEpsilonTransition(mapped_from, mapped_to);
                break;

            case TransitionType::FSM_INSTANCE:
                if (trans.embedded_fsm)
                {
                    auto nested_ids = mergeEmbeddedFSM(mapped_from, mapped_to,
                                                       trans.embedded_fsm, trans.priority);
                    result.new_transitions.insert(result.new_transitions.end(),
                                                  nested_ids.begin(), nested_ids.end());
                }
                break;
            }

            if (trans.type != TransitionType::FSM_INSTANCE)
            {
                result.new_transitions.push_back(new_trans_id);
            }
        }

        sortTransitionsByPriority();

        return result;
    }

    // ============================================================================
    // Input Processing
    // ============================================================================

    bool FSM::validate(std::string_view input)
    {
        reset();
        last_error_.reset();

        current_input_ = std::string(input);
        clearCaptures();
        current_input_position_ = 0;

        rebuildTransitionMap();

        auto start_time = std::chrono::high_resolution_clock::now();

        if (!start_state_.isValid())
        {
            last_error_ = ValidationError{
                ErrorType::NO_START_STATE,
                0,
                '\0',
                current_state_,
                "No start state defined",
                {},
                ""};
            return false;
        }

        for (size_t i = 0; i < input.size(); ++i)
        {
            char ch = input[i];

            updateCapturePosition(i);

            if (!processCharImpl(ch, i))
            {
                return false;
            }

            if (debug_config_.hasCollectMetrics())
            {
                metrics_.characters_processed++;
            }

            recordCharInCaptures(ch);
        }

        updateCapturePosition(input.size());

        processEpsilonTransitions(input.size());

        if (!isInAcceptState())
        {
            last_error_ = ValidationError{
                ErrorType::NOT_IN_ACCEPT_STATE,
                input.size(),
                '\0',
                current_state_,
                "Input consumed but not in accept state.  Current state: " +
                    current_state_.toString(),
                {},
                ""};
            return false;
        }

        auto end_time = std::chrono::high_resolution_clock::now();

        if (debug_config_.hasCollectMetrics())
        {
            metrics_.validation_time_ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    end_time - start_time)
                    .count();
            metrics_.processing_time =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    end_time - start_time);
        }

        return true;
    }

    bool FSM::isInAcceptState() const
    {
        return isAcceptState(current_state_);
    }

    void FSM::reset()
    {
        current_state_ = start_state_;
        last_error_.reset();

        if (debug_config_.hasTraceTransitions() || debug_config_.hasTraceStateChanges())
        {
            trace_.clear();
        }

        if (debug_config_.hasCollectMetrics())
        {
            metrics_.reset();
        }

        current_input_position_ = 0;
        current_input_.clear();

        stream_state_ = StreamState::READY;
        streaming_mode_ = false;

        while (!choice_stack_.empty())
        {
            choice_stack_.pop();
        }
        resetBacktrackingStats();
    }

    // ============================================================================
    // Streaming Implementation (Phase 2. 4)
    // ============================================================================

    StreamState FSM::feed(char ch)
    {
        if (!streaming_mode_)
        {
            streaming_mode_ = true;
            stream_state_ = StreamState::PROCESSING;

            current_input_ += ch;

            if (!start_state_.isValid())
            {
                last_error_ = ValidationError{
                    ErrorType::NO_START_STATE,
                    current_input_position_,
                    ch,
                    current_state_,
                    "No start state defined",
                    {},
                    ""};
                stream_state_ = StreamState::ERROR;
                return stream_state_;
            }
        }
        else
        {
            current_input_ += ch;
        }

        updateCapturePosition(current_input_position_);

        if (!processCharImpl(ch, current_input_position_))
        {
            stream_state_ = StreamState::ERROR;
            return stream_state_;
        }

        if (debug_config_.hasCollectMetrics())
        {
            metrics_.characters_processed++;
        }

        recordCharInCaptures(ch);

        current_input_position_++;

        updateCapturePosition(current_input_position_);

        if (isInAcceptState())
        {
            stream_state_ = StreamState::COMPLETE;
        }
        else
        {
            stream_state_ = StreamState::WAITING_FOR_INPUT;
        }

        return stream_state_;
    }

    StreamState FSM::feed(std::string_view chunk)
    {
        for (char ch : chunk)
        {
            StreamState state = feed(ch);

            if (state == StreamState::ERROR)
            {
                return state;
            }
        }

        return stream_state_;
    }

    StreamState FSM::endOfStream()
    {
        if (!streaming_mode_)
        {
            last_error_ = ValidationError{
                ErrorType::UNEXPECTED_END_OF_INPUT,
                0,
                '\0',
                current_state_,
                "End of stream called before any input was fed",
                {},
                ""};
            stream_state_ = StreamState::ERROR;
            return stream_state_;
        }

        processEpsilonTransitions(current_input_position_);

        if (!isInAcceptState())
        {
            last_error_ = ValidationError{
                ErrorType::NOT_IN_ACCEPT_STATE,
                current_input_position_,
                '\0',
                current_state_,
                "End of stream but not in accept state.  Current state: " +
                    current_state_.toString(),
                {},
                ""};
            stream_state_ = StreamState::ERROR;
            return stream_state_;
        }

        stream_state_ = StreamState::COMPLETE;
        return stream_state_;
    }

    StreamState FSM::getStreamState() const
    {
        return stream_state_;
    }

    bool FSM::isStreamComplete() const
    {
        return stream_state_ == StreamState::COMPLETE;
    }

    bool FSM::needsMoreInput() const
    {
        return stream_state_ == StreamState::WAITING_FOR_INPUT;
    }

    void FSM::resetStream()
    {
        stream_state_ = StreamState::READY;
        streaming_mode_ = false;
    }

    std::string FSM::streamStateToString(StreamState state)
    {
        switch (state)
        {
        case StreamState::READY:
            return "READY";
        case StreamState::PROCESSING:
            return "PROCESSING";
        case StreamState::WAITING_FOR_INPUT:
            return "WAITING_FOR_INPUT";
        case StreamState::COMPLETE:
            return "COMPLETE";
        case StreamState::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    // ============================================================================
    // Backtracking Implementation (Phase 2.5)
    // ============================================================================

    void FSM::markAsChoicePoint(StateID state)
    {
        auto it = states_.find(state);
        if (it == states_.end())
        {
            throw std::invalid_argument("Cannot mark non-existent state as choice point: " +
                                        state.toString());
        }
        it->second.is_choice_point = true;
    }

    bool FSM::isChoicePoint(StateID state) const
    {
        auto it = states_.find(state);
        if (it == states_.end())
        {
            return false;
        }
        return it->second.is_choice_point;
    }

    const BacktrackingStats &FSM::getBacktrackingStats() const
    {
        return backtracking_stats_;
    }

    void FSM::resetBacktrackingStats()
    {
        backtracking_stats_.reset();
    }

    void FSM::setMaxBacktrackDepth(size_t depth)
    {
        max_backtrack_depth_ = depth;
    }

    size_t FSM::getMaxBacktrackDepth() const
    {
        return max_backtrack_depth_;
    }

    std::vector<const Transition *> FSM::getValidTransitions(char ch)
    {
        std::vector<const Transition *> valid;
        auto transitions = getTransitionsFrom(current_state_);

        for (const auto *trans : transitions)
        {
            if (trans->type == TransitionType::ABNF_RULE && trans->matches(ch))
            {
                valid.push_back(trans);
            }
        }

        return valid;
    }

    bool FSM::shouldCreateChoicePoint(const std::vector<const Transition *> &valid_transitions) const
    {
        if (valid_transitions.size() <= 1)
        {
            return false;
        }

        if (isChoicePoint(current_state_))
        {
            return true;
        }

        return valid_transitions.size() > 1;
    }

    void FSM::saveChoicePoint(const std::vector<const Transition *> &alternatives,
                              size_t position)
    {
        if (max_backtrack_depth_ > 0 && choice_stack_.size() >= max_backtrack_depth_)
        {
            return;
        }

        choice_stack_.emplace(
            current_state_,
            position,
            alternatives,
            captures_,
            active_captures_,
            current_input_position_);

        backtracking_stats_.choice_points_created++;
        if (choice_stack_.size() > backtracking_stats_.max_stack_depth)
        {
            backtracking_stats_.max_stack_depth = choice_stack_.size();
        }
    }

    bool FSM::backtrack()
    {
        while (!choice_stack_.empty())
        {
            ChoicePoint &cp = choice_stack_.top();

            if (!cp.remaining.empty())
            {
                restoreFromChoicePoint(cp);
                backtracking_stats_.backtracks_performed++;
                return true;
            }

            choice_stack_.pop();
        }

        return false;
    }

    void FSM::restoreFromChoicePoint(const ChoicePoint &cp)
    {
        current_state_ = cp.state;
        captures_ = cp.captures_snapshot;
        active_captures_ = cp.active_captures_snapshot;
        current_input_position_ = cp.input_position_snapshot;
    }

    bool FSM::validateWithBacktracking(std::string_view input)
    {
        reset();
        resetBacktrackingStats();
        last_error_.reset();

        while (!choice_stack_.empty())
        {
            choice_stack_.pop();
        }

        current_input_ = std::string(input);
        clearCaptures();
        current_input_position_ = 0;

        rebuildTransitionMap();

        if (!start_state_.isValid())
        {
            last_error_ = ValidationError{
                ErrorType::NO_START_STATE,
                0,
                '\0',
                current_state_,
                "No start state defined",
                {},
                ""};
            return false;
        }

        size_t position = 0;

        while (position < input.size())
        {
            char ch = input[position];

            updateCapturePosition(position);

            auto valid_transitions = getValidTransitions(ch);

            if (valid_transitions.empty())
            {
                if (backtrack())
                {
                    ChoicePoint &cp = choice_stack_.top();

                    if (!cp.remaining.empty())
                    {
                        const Transition *next_trans = cp.remaining[0];
                        cp.remaining.erase(cp.remaining.begin());

                        StateID old_state = current_state_;
                        current_state_ = next_trans->to;

                        backtracking_stats_.paths_explored++;

                        recordCharInCaptures(ch);

                        position = cp.position + 1;
                        current_input_position_ = position;

                        if (debug_config_.hasCollectMetrics())
                        {
                            metrics_.characters_processed++;
                        }

                        continue;
                    }
                }

                last_error_ = ValidationError{
                    ErrorType::NO_MATCHING_TRANSITION,
                    position,
                    ch,
                    current_state_,
                    "No transition found from " + current_state_.toString() +
                        " for character '" + std::string(1, ch) + "'",
                    {},
                    ""};
                return false;
            }

            if (shouldCreateChoicePoint(valid_transitions))
            {
                std::vector<const Transition *> remaining(
                    valid_transitions.begin() + 1,
                    valid_transitions.end());

                saveChoicePoint(remaining, position);
            }

            const Transition *trans = valid_transitions[0];

            backtracking_stats_.paths_explored++;

            StateID old_state = current_state_;
            StateID new_state = trans->to;
            bool state_changed = (old_state != new_state);

            if (state_changed)
            {
                auto old_state_it = states_.find(old_state);
                if (old_state_it != states_.end() && old_state_it->second.on_exit)
                {
                    StateContext ctx(old_state, position, ch, user_data_);
                    old_state_it->second.on_exit(ctx);
                }
            }

            if (trans->on_transition)
            {
                TransitionContext ctx(old_state, new_state, ch, position, trans, user_data_);
                trans->on_transition(ctx);
            }

            current_state_ = new_state;

            if (state_changed)
            {
                auto new_state_it = states_.find(new_state);
                if (new_state_it != states_.end() && new_state_it->second.on_entry)
                {
                    StateContext ctx(new_state, position, ch, user_data_);
                    new_state_it->second.on_entry(ctx);
                }
            }

            if (debug_config_.hasCollectMetrics())
            {
                metrics_.transitions_taken++;
                if (state_changed)
                {
                    metrics_.states_visited++;
                }
            }

            if (debug_config_.hasTraceStateChanges() && state_changed)
            {
                logStateChange(old_state, current_state_);
            }

            if (debug_config_.hasTraceTransitions())
            {
                TraceEntry entry{trace_.size(), old_state, current_state_,
                                 ch, trans->id, trans->description};
                trace_.push_back(entry);
                logTransition(entry);
            }

            recordCharInCaptures(ch);

            position++;
            current_input_position_++;

            if (debug_config_.hasCollectMetrics())
            {
                metrics_.characters_processed++;
            }
        }

        updateCapturePosition(input.size());

        processEpsilonTransitions(input.size());

        if (!isInAcceptState())
        {
            while (backtrack())
            {
                ChoicePoint &cp = choice_stack_.top();

                if (cp.remaining.empty())
                {
                    choice_stack_.pop();
                    continue;
                }

                const Transition *next_trans = cp.remaining[0];
                cp.remaining.erase(cp.remaining.begin());

                backtracking_stats_.paths_explored++;

                current_state_ = next_trans->to;

                size_t resume_pos = cp.position + 1;

                if (resume_pos >= input.size())
                {
                    processEpsilonTransitions(input.size());

                    if (isInAcceptState())
                    {
                        return true;
                    }
                    continue;
                }

                std::string remaining_input(input.substr(resume_pos));
                StateID saved_start = start_state_;

                start_state_ = current_state_;

                bool success = validateWithBacktracking(remaining_input);

                start_state_ = saved_start;

                if (success)
                {
                    return true;
                }
            }

            last_error_ = ValidationError{
                ErrorType::NOT_IN_ACCEPT_STATE,
                input.size(),
                '\0',
                current_state_,
                "Input consumed but not in accept state. Current state: " +
                    current_state_.toString(),
                {},
                ""};
            return false;
        }

        return true;
    }

    // ============================================================================
    // Internal Helpers
    // ============================================================================

    bool FSM::processCharImpl(char ch, size_t position)
    {
        auto transitions = getTransitionsFrom(current_state_);

        const Transition *best_match = nullptr;

        for (const auto *trans : transitions)
        {
            if (trans->type == TransitionType::ABNF_RULE && trans->matches(ch))
            {
                best_match = trans;
                break;
            }
        }

        if (!best_match)
        {
            last_error_ = ValidationError{
                ErrorType::NO_MATCHING_TRANSITION,
                position,
                ch,
                current_state_,
                "No transition found from " + current_state_.toString() +
                    " for character '" + std::string(1, ch) + "'",
                {},
                ""};
            return false;
        }

        StateID old_state = current_state_;
        StateID new_state = best_match->to;
        bool state_changed = (old_state != new_state);

        if (state_changed)
        {
            auto old_state_it = states_.find(old_state);
            if (old_state_it != states_.end() && old_state_it->second.on_exit)
            {
                StateContext ctx(old_state, position, ch, user_data_);
                old_state_it->second.on_exit(ctx);
            }
        }

        if (best_match->on_transition)
        {
            TransitionContext ctx(old_state, new_state, ch, position, best_match, user_data_);
            best_match->on_transition(ctx);
        }

        current_state_ = new_state;

        if (state_changed)
        {
            auto new_state_it = states_.find(new_state);
            if (new_state_it != states_.end() && new_state_it->second.on_entry)
            {
                StateContext ctx(new_state, position, ch, user_data_);
                new_state_it->second.on_entry(ctx);
            }
        }

        if (debug_config_.hasCollectMetrics())
        {
            metrics_.transitions_taken++;
            if (state_changed)
            {
                metrics_.states_visited++;
            }
        }

        if (debug_config_.hasTraceStateChanges() && state_changed)
        {
            logStateChange(old_state, current_state_);
        }

        if (debug_config_.hasTraceTransitions())
        {
            TraceEntry entry{trace_.size(), old_state, current_state_,
                             ch, best_match->id, best_match->description};
            trace_.push_back(entry);
            logTransition(entry);
        }

        return true;
    }

    void FSM::processEpsilonTransitions(size_t position)
    {
        std::unordered_set<StateID, StateID::Hash> epsilon_visited;
        epsilon_visited.insert(current_state_);

        bool found_epsilon = true;
        while (found_epsilon)
        {
            found_epsilon = false;
            auto transitions = getTransitionsFrom(current_state_);

            for (const auto *trans : transitions)
            {
                if (trans->type == TransitionType::EPSILON)
                {
                    if (epsilon_visited.find(trans->to) != epsilon_visited.end())
                    {
                        continue;
                    }

                    StateID old_state = current_state_;
                    StateID new_state = trans->to;

                    auto old_state_it = states_.find(old_state);
                    if (old_state_it != states_.end() && old_state_it->second.on_exit)
                    {
                        StateContext ctx(old_state, position, '\0', user_data_);
                        old_state_it->second.on_exit(ctx);
                    }

                    if (trans->on_transition)
                    {
                        TransitionContext ctx(old_state, new_state, '\0', position, trans, user_data_);
                        trans->on_transition(ctx);
                    }

                    current_state_ = new_state;
                    epsilon_visited.insert(current_state_);

                    auto new_state_it = states_.find(new_state);
                    if (new_state_it != states_.end() && new_state_it->second.on_entry)
                    {
                        StateContext ctx(new_state, position, '\0', user_data_);
                        new_state_it->second.on_entry(ctx);
                    }

                    if (debug_config_.hasCollectMetrics())
                    {
                        metrics_.epsilon_transitions++;
                    }

                    if (debug_config_.hasTraceStateChanges())
                    {
                        logStateChange(old_state, current_state_);
                    }

                    if (debug_config_.hasTraceTransitions())
                    {
                        TraceEntry entry{trace_.size(), old_state, current_state_,
                                         '\0', trans->id, "Epsilon"};
                        trace_.push_back(entry);
                        logTransition(entry);
                    }

                    found_epsilon = true;
                    break;
                }
            }
        }
    }

    void FSM::rebuildTransitionMap()
    {
        if (!transition_map_dirty_)
        {
            return;
        }

        transition_map_.clear();

        for (auto &trans : transitions_)
        {
            transition_map_[trans.from].push_back(&trans);
        }

        for (auto &[state, trans_list] : transition_map_)
        {
            std::sort(trans_list.begin(), trans_list.end(),
                      [](const Transition *a, const Transition *b)
                      {
                          return a->priority > b->priority;
                      });
        }

        transition_map_dirty_ = false;
    }

    void FSM::sortTransitionsByPriority()
    {
        std::sort(transitions_.begin(), transitions_.end(),
                  [](const Transition &a, const Transition &b)
                  {
                      return a.priority > b.priority;
                  });

        transition_map_dirty_ = true;
    }

    void FSM::logTransition(const TraceEntry &entry)
    {
        auto &os = debug_config_.getOutputStream();
        os << "[FSM:" << name_ << "] " << entry.toString() << std::endl;
    }

    void FSM::logStateChange(StateID from, StateID to)
    {
        auto &os = debug_config_.getOutputStream();
        os << "[FSM:" << name_ << "] State change: "
           << from.toString() << " -> " << to.toString() << std::endl;
    }

    std::string FSM::getInputContext(std::string_view input, size_t position) const
    {
        const size_t context_size = 10;
        size_t start = (position > context_size) ? position - context_size : 0;
        size_t end = std::min(position + context_size, input.size());

        std::string context(input.substr(start, end - start));
        return context;
    }

    // ============================================================================
    // Error Reporting
    // ============================================================================

    std::optional<FSM::ValidationError> FSM::getLastError() const
    {
        return last_error_;
    }

    // ============================================================================
    // Debug Support
    // ============================================================================

    void FSM::setDebugConfig(const DebugConfig &config)
    {
        debug_config_ = config;
    }

    DebugConfig &FSM::getDebugConfig()
    {
        return debug_config_;
    }

    const DebugConfig &FSM::getDebugConfig() const
    {
        return debug_config_;
    }

    std::string FSM::toString() const
    {
        std::ostringstream oss;
        oss << "FSM{name=" << name_
            << ", states=" << states_.size()
            << ", transitions=" << transitions_.size()
            << ", start=" << start_state_.toString()
            << ", accepts=" << accept_states_.size()
            << "}";
        return oss.str();
    }

    std::string FSM::toDebugString() const
    {
        std::ostringstream oss;
        oss << "FSM: " << name_ << " (ID: " << id_ << ")\n";
        oss << "States (" << states_.size() << "):\n";

        for (const auto &[id, state] : states_)
        {
            oss << "  " << state.toDebugString() << "\n";
        }

        oss << "Transitions (" << transitions_.size() << "):\n";
        for (const auto &trans : transitions_)
        {
            oss << "  " << trans.toDebugString() << "\n";
        }

        oss << "Start State: " << start_state_.toString() << "\n";
        oss << "Accept States: ";
        for (const auto &accept : accept_states_)
        {
            oss << accept.toString() << " ";
        }
        oss << "\n";

        return oss.str();
    }

    std::string FSM::toDot() const
    {
        std::ostringstream oss;

        oss << "digraph FSM_" << name_ << " {\n";
        oss << "    rankdir=LR;\n";
        oss << "    node [shape=circle];\n\n";

        for (const auto &[id, state] : states_)
        {
            oss << "    " << id.id << " [";

            if (state.type == StateType::ACCEPT || state.type == StateType::START)
            {
                oss << "shape=doublecircle, ";
            }

            if (state.type == StateType::START)
            {
                oss << "style=filled, fillcolor=lightblue, ";
            }
            else if (state.type == StateType::ACCEPT)
            {
                oss << "style=filled, fillcolor=lightgreen, ";
            }

            oss << "label=\"" << id.toString();
            if (!state.description.empty())
            {
                oss << "\\n"
                    << state.description;
            }
            oss << "\"];\n";
        }

        oss << "\n";

        for (const auto &trans : transitions_)
        {
            oss << "    " << trans.from.id << " -> " << trans.to.id << " [label=\"";

            if (trans.type == TransitionType::ABNF_RULE && trans.rule.has_value())
            {
                oss << trans.rule->toString();
            }
            else if (trans.type == TransitionType::EPSILON)
            {
                oss << "ε";
            }
            else if (trans.type == TransitionType::FSM_INSTANCE && trans.embedded_fsm)
            {
                oss << "FSM:" << trans.embedded_fsm->getName();
            }

            if (!trans.description.empty())
            {
                oss << "\\n"
                    << trans.description;
            }

            if (trans.priority != Transition::PRIORITY_NORMAL)
            {
                oss << "\\n[pri:" << trans.priority << "]";
            }

            oss << "\"];\n";
        }

        oss << "}\n";
        return oss.str();
    }

    void FSM::exportDot(const std::string &filename) const
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file for DOT export: " + filename);
        }

        file << toDot();
        file.close();

        if (debug_config_.isEnabled())
        {
            auto &os = debug_config_.getOutputStream();
            os << "[FSM:" << name_ << "] Exported DOT graph to: " << filename << std::endl;
        }
    }

    const std::vector<FSM::TraceEntry> &FSM::getTrace() const
    {
        return trace_;
    }

    void FSM::clearTrace()
    {
        trace_.clear();
    }

    const FSM::Metrics &FSM::getMetrics() const
    {
        return metrics_;
    }

    void FSM::resetMetrics()
    {
        metrics_.reset();
    }

    // ============================================================================
    // Validation
    // ============================================================================

    bool FSM::isValid() const
    {
        return validateStructure().empty();
    }

    std::vector<std::string> FSM::validateStructure() const
    {
        std::vector<std::string> issues;

        if (!start_state_.isValid() || !hasState(start_state_))
        {
            issues.push_back("No valid start state defined");
        }

        if (accept_states_.empty())
        {
            issues.push_back("No accept states defined");
        }

        for (const auto &accept : accept_states_)
        {
            if (!hasState(accept))
            {
                issues.push_back("Accept state does not exist: " + accept.toString());
            }
        }

        for (const auto &trans : transitions_)
        {
            if (!hasState(trans.from))
            {
                issues.push_back("Transition from non-existent state: " + trans.from.toString());
            }
            if (!hasState(trans.to))
            {
                issues.push_back("Transition to non-existent state: " + trans.to.toString());
            }
        }

        return issues;
    }

    // ============================================================================
    // Introspection
    // ============================================================================

    size_t FSM::getStateCount() const
    {
        return states_.size();
    }

    size_t FSM::getTransitionCount() const
    {
        return transitions_.size();
    }

    std::vector<StateID> FSM::getStates() const
    {
        std::vector<StateID> result;
        result.reserve(states_.size());
        for (const auto &[id, state] : states_)
        {
            result.push_back(id);
        }
        return result;
    }

    std::vector<Transition> FSM::getTransitions() const
    {
        return transitions_;
    }

    std::vector<Transition *> FSM::getTransitionsFrom(StateID state) const
    {
        const_cast<FSM *>(this)->rebuildTransitionMap();

        auto it = transition_map_.find(state);
        if (it == transition_map_.end())
        {
            return {};
        }
        return it->second;
    }

    const std::string &FSM::getName() const
    {
        return name_;
    }

    uint32_t FSM::getID() const
    {
        return id_;
    }

    // ============================================================================
    // Action Callbacks Implementation
    // ============================================================================

    void FSM::setStateEntryCallback(StateID state, StateEntryCallback callback)
    {
        auto it = states_.find(state);
        if (it == states_.end())
        {
            throw std::invalid_argument("Cannot set callback for non-existent state: " +
                                        state.toString());
        }
        it->second.on_entry = std::move(callback);
    }

    void FSM::setStateExitCallback(StateID state, StateExitCallback callback)
    {
        auto it = states_.find(state);
        if (it == states_.end())
        {
            throw std::invalid_argument("Cannot set callback for non-existent state: " +
                                        state.toString());
        }
        it->second.on_exit = std::move(callback);
    }

    void FSM::setTransitionCallback(Transition::TransitionID transition_id,
                                    TransitionCallback callback)
    {
        for (auto &trans : transitions_)
        {
            if (trans.id == transition_id)
            {
                trans.on_transition = std::move(callback);
                return;
            }
        }
        throw std::invalid_argument("Cannot set callback for non-existent transition ID: " +
                                    std::to_string(transition_id));
    }

    void FSM::setUserData(void *data)
    {
        user_data_ = data;
    }

    void *FSM::getUserData() const
    {
        return user_data_;
    }

    // ============================================================================
    // Capture Groups Implementation
    // ============================================================================

    void FSM::beginCapture(const std::string &name)
    {
        for (const auto &active : active_captures_)
        {
            if (active.name == name)
            {
                throw std::logic_error("Capture group '" + name + "' is already active");
            }
        }

        active_captures_.emplace_back(name, current_input_position_);
    }

    CaptureGroup FSM::endCapture(const std::string &name)
    {
        for (auto it = active_captures_.begin(); it != active_captures_.end(); ++it)
        {
            if (it->name == name)
            {
                CaptureGroup capture(
                    it->name,
                    it->start_position,
                    current_input_position_,
                    it->buffer);

                captures_.push_back(capture);
                active_captures_.erase(it);

                return capture;
            }
        }

        throw std::logic_error("No active capture group named '" + name + "'");
    }

    void FSM::recordCharInCaptures(char ch)
    {
        for (auto &active : active_captures_)
        {
            active.buffer += ch;
        }
    }

    std::optional<CaptureGroup> FSM::getCapture(const std::string &name) const
    {
        for (const auto &capture : captures_)
        {
            if (capture.name == name)
            {
                return capture;
            }
        }
        return std::nullopt;
    }

    const std::vector<CaptureGroup> &FSM::getAllCaptures() const
    {
        return captures_;
    }

    std::optional<CaptureGroup> FSM::getCaptureByIndex(size_t index) const
    {
        if (index < captures_.size())
        {
            return captures_[index];
        }
        return std::nullopt;
    }

    void FSM::clearCaptures()
    {
        captures_.clear();
        active_captures_.clear();
    }

    bool FSM::hasCapture(const std::string &name) const
    {
        return getCapture(name).has_value();
    }

    void FSM::updateCapturePosition(size_t pos)
    {
        current_input_position_ = pos;
    }

    // ============================================================================
    // SIMD Support (Phase 4. 2)
    // ============================================================================

    void FSM::setSIMDEnabled(bool enabled)
    {
        simd_enabled_ = enabled;
    }

    bool FSM::isSIMDEnabled() const
    {
        return simd_enabled_;
    }

    std::string FSM::getSIMDCapabilities() const
    {
        // Placeholder - will be implemented with SIMD integration
        return "SIMD support: Not yet integrated";
    }

    // ============================================================================
    // Builder Implementation
    // ============================================================================

    FSM::Builder::Builder(const std::string &name)
        : name_(name), id_(0), next_id_(1) {}

    FSM::Builder::Builder(uint32_t id, const std::string &name)
        : name_(name), id_(id), next_id_(1) {}

    FSM::Builder &FSM::Builder::withInitialConfig(const InitialConfig &config)
    {
        states_ = config.states;
        transitions_ = config.transitions;
        start_state_ = config.start_state;
        accept_states_ = config.accept_states;

        for (const auto &state : states_)
        {
            name_to_id_[state.id.name] = state.id;
            if (state.id.id >= next_id_)
            {
                next_id_ = state.id.id + 1;
            }
        }

        return *this;
    }

    FSM::Builder &FSM::Builder::addState(const std::string &name, StateType type)
    {
        StateID sid(next_id_++, name);
        states_.push_back(State(sid, type));
        name_to_id_[name] = sid;
        return *this;
    }

    FSM::Builder &FSM::Builder::addState(const std::string &name, const std::string &description,
                                         StateType type)
    {
        StateID sid(next_id_++, name);
        states_.push_back(State(sid, type, description));
        name_to_id_[name] = sid;
        return *this;
    }

    FSM::Builder &FSM::Builder::setStartState(const std::string &name)
    {
        start_state_ = getOrCreateState(name);
        return *this;
    }

    FSM::Builder &FSM::Builder::addAcceptState(const std::string &name)
    {
        StateID sid = getOrCreateState(name);
        accept_states_.push_back(sid);
        return *this;
    }

    FSM::Builder &FSM::Builder::addTransition(const std::string &from, const std::string &to,
                                              const ABNF &rule, int priority)
    {
        StateID from_id = getOrCreateState(from);
        StateID to_id = getOrCreateState(to);

        transitions_.push_back(Transition(static_cast<Transition::TransitionID>(transitions_.size() + 1),
                                          from_id, to_id, rule, priority));
        return *this;
    }

    FSM::Builder &FSM::Builder::addTransition(const std::string &from, const std::string &to,
                                              const ABNF &rule, const std::string &description,
                                              int priority)
    {
        addTransition(from, to, rule, priority);
        transitions_.back().description = description;
        return *this;
    }

    FSM::Builder &FSM::Builder::addTransition(const std::string &from, const std::string &to,
                                              std::shared_ptr<FSM> fsm, int priority)
    {
        pending_fsm_merges_.push_back({from + "->" + to, fsm});
        return *this;
    }

    FSM::Builder &FSM::Builder::addTransition(const std::string &from, const std::string &to,
                                              const FSM &fsm, int priority)
    {
        return addTransition(from, to, std::make_shared<FSM>(fsm), priority);
    }

    FSM::Builder &FSM::Builder::addEpsilonTransition(const std::string &from, const std::string &to)
    {
        StateID from_id = getOrCreateState(from);
        StateID to_id = getOrCreateState(to);

        transitions_.push_back(Transition(static_cast<Transition::TransitionID>(transitions_.size() + 1),
                                          from_id, to_id));
        return *this;
    }

    FSM::Builder &FSM::Builder::setDebugFlags(DebugFlags flags)
    {
        debug_config_.flags = flags;
        return *this;
    }

    FSM::Builder &FSM::Builder::enableDebugFlag(DebugFlags flag)
    {
        debug_config_.enable(flag);
        return *this;
    }

    FSM::Builder &FSM::Builder::disableDebugFlag(DebugFlags flag)
    {
        debug_config_.disable(flag);
        return *this;
    }

    FSM::Builder &FSM::Builder::withDebugOutput(std::ostream &os)
    {
        debug_config_.log_stream = &os;
        return *this;
    }

    FSM::Builder &FSM::Builder::enableBasicDebug()
    {
        debug_config_.flags = DebugFlags::BASIC;
        return *this;
    }

    FSM::Builder &FSM::Builder::enableFullDebug()
    {
        debug_config_.flags = DebugFlags::FULL;
        return *this;
    }

    FSM::Builder &FSM::Builder::disableDebug()
    {
        debug_config_.flags = DebugFlags::NONE;
        return *this;
    }

    FSM::Builder &FSM::Builder::onStateEntry(const std::string &state_name,
                                             StateEntryCallback callback)
    {
        StateID sid = getOrCreateState(state_name);

        for (auto &state : states_)
        {
            if (state.id == sid)
            {
                state.on_entry = std::move(callback);
                break;
            }
        }

        return *this;
    }

    FSM::Builder &FSM::Builder::onStateExit(const std::string &state_name,
                                            StateExitCallback callback)
    {
        StateID sid = getOrCreateState(state_name);

        for (auto &state : states_)
        {
            if (state.id == sid)
            {
                state.on_exit = std::move(callback);
                break;
            }
        }

        return *this;
    }

    FSM::Builder &FSM::Builder::onTransition(TransitionCallback callback)
    {
        if (transitions_.empty())
        {
            throw std::logic_error("No transitions added yet.  Call addTransition first.");
        }

        transitions_.back().on_transition = std::move(callback);

        return *this;
    }

    FSM::Builder &FSM::Builder::withUserData(void *data)
    {
        user_data_ = data;
        return *this;
    }

    FSM::Builder &FSM::Builder::markChoicePoint(const std::string &state_name)
    {
        StateID sid = getOrCreateState(state_name);

        for (auto &state : states_)
        {
            if (state.id == sid)
            {
                state.is_choice_point = true;
                break;
            }
        }

        return *this;
    }

    std::shared_ptr<FSM> FSM::Builder::build()
    {
        if (!start_state_.has_value())
        {
            throw std::logic_error("Cannot build FSM without a start state");
        }

        if (accept_states_.empty())
        {
            throw std::logic_error("Cannot build FSM without accept states");
        }

        InitialConfig config;
        config.states = states_;
        config.transitions = transitions_;
        config.start_state = *start_state_;
        config.accept_states = accept_states_;

        auto fsm = std::make_shared<FSM>(name_, config);
        fsm->id_ = id_;
        fsm->setDebugConfig(debug_config_);

        fsm->setUserData(user_data_);

        for (const auto &[transition_key, embedded_fsm] : pending_fsm_merges_)
        {
            size_t arrow_pos = transition_key.find("->");
            if (arrow_pos != std::string::npos)
            {
                std::string from_name = transition_key.substr(0, arrow_pos);
                std::string to_name = transition_key.substr(arrow_pos + 2);

                StateID from_id = name_to_id_[from_name];
                StateID to_id = name_to_id_[to_name];

                fsm->mergeEmbeddedFSM(from_id, to_id, embedded_fsm);
            }
        }

        return fsm;
    }

    StateID FSM::Builder::getOrCreateState(const std::string &name)
    {
        auto it = name_to_id_.find(name);
        if (it != name_to_id_.end())
        {
            return it->second;
        }

        StateID sid(next_id_++, name);
        states_.push_back(State(sid));
        name_to_id_[name] = sid;
        return sid;
    }

} // namespace fsm
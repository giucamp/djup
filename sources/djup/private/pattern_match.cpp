
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern_match.h>
#include <private/builtin_names.h>
#include <private/substitute_by_predicate.h>
#include <private/expression.h>
#include <core/flags.h>
#include <core/diagnostic.h>
#include <vector>
#include <unordered_map>

#define DBG_CREATE_GRAPHVIZ_SVG         1
#define DBG_GRAPHVIZ_EXE                "\"C:\\Program Files\\Graphviz\\bin\\dot.exe\""
#define DBG_DEST_DIR                    "C:\\projects\\djup\\test\\"

#if DBG_CREATE_GRAPHVIZ_SVG
    #include <fstream>
    bool g_enable_graphviz = false;
#endif

namespace djup
{
    namespace
    {
        struct ApplySubstitutions
        {
            const PatternMatch & m_match;

            Tensor operator () (const Tensor & i_candidate) const
            {
                // substitution
                /*{
                    auto it = m_match.m_substitutions.find({i_candidate.GetExpression()->GetName(), 0});
                    if(it != m_match.m_substitutions.end())
                        return it->second;
                }

                // argument expansion
                std::vector<Tensor> new_arguments;
                for(const Tensor & argument : i_candidate.GetExpression()->GetArguments())
                {
                    auto it = m_match.m_expansions.find(argument.GetExpression().get());
                    if(it != m_match.m_expansions.end())
                    {
                        
                    }
                }*/
                return i_candidate;
            }
        };

        bool IsCommutative(const Name & i_function)
        {
            return i_function == "Add" || i_function == "Mul" || i_function == "Equals";
        }

        bool IsAssociative(const Name & i_function)
        {
            return i_function == "Add" || i_function == "Mul" || i_function == "MatMul";
        }

        std::string TensorListToString(Span<const Tensor> i_tensors)
        {
            std::string result;
            for(size_t i = 0; i < i_tensors.size(); i++)
            {
                if(i)
                    result += ", ";
                result += ToSimplifiedStringForm(i_tensors[i]);
            }
            return result;
        }

        constexpr uint32_t g_start_node_index = 1;
        constexpr uint32_t g_end_node_index = 0;
    }

    Tensor SubstitutePatternMatch(const Tensor & i_source, const PatternMatch & i_match)
    {
        return SubstituteByPredicate(i_source, ApplySubstitutions{i_match});
    }

    constexpr uint32_t s_max_reps = std::numeric_limits<uint32_t>::max();

    // by default describes a non-variadic argument
    struct RepRange
    {
        uint32_t m_min = 1;
        uint32_t m_max = 1;
    };

    struct PatternInfo
    {
        size_t m_min_arguments{}, m_max_arguments{};
        std::vector<RepRange> m_pattern_arg_ranges;
        std::vector<RepRange> m_pattern_arg_reiaming_ranges;
    };

    struct PatternSegment
    {
        Span<const Tensor> m_pattern;
        Span<const RepRange> m_ranges;
        Span<const RepRange> m_remaining;

        PatternSegment() = default;

        PatternSegment(Span<const Tensor> i_pattern, Span<const RepRange> i_ranges, Span<const RepRange> i_remaining)
            : m_pattern(i_pattern), m_ranges(i_ranges), m_remaining(i_remaining)
        {
            assert(m_ranges.size() == m_pattern.size());
            assert(m_ranges.size() == m_remaining.size());
        }
    };

    PatternInfo BuildPatternInfo(Span<const Tensor> i_pattern_args)
    {
        PatternInfo result;

        // fill m_pattern_arg_ranges
        result.m_pattern_arg_ranges.resize(i_pattern_args.size());
        bool upper_unbounded = false;
        for(size_t sub_pattern_index = 0; sub_pattern_index < i_pattern_args.size(); sub_pattern_index++)
        {
            const Tensor & arg = i_pattern_args[sub_pattern_index]; 

            RepRange & arg_range = result.m_pattern_arg_ranges[sub_pattern_index];

            if(NameIs(arg, builtin_names::RepetitionsZeroToMany))
            {
                arg_range.m_min = 0;
                arg_range.m_max = s_max_reps;
                upper_unbounded = true;
            }
            else if(NameIs(arg, builtin_names::RepetitionsZeroToOne))
            {
                arg_range.m_min = 0;
                arg_range.m_max = 1;
                result.m_max_arguments++;
            }
            else if(NameIs(arg, builtin_names::RepetitionsOneToMany))
            {
                arg_range.m_min = 1;
                arg_range.m_max = s_max_reps;
                result.m_min_arguments++;
                upper_unbounded = true;
            }
            else
            {
                result.m_min_arguments++;
                result.m_max_arguments++;
            }
        }
        if(upper_unbounded)
            result.m_max_arguments = s_max_reps;

        // fill m_pattern_arg_reiaming_ranges
        result.m_pattern_arg_reiaming_ranges.resize(i_pattern_args.size());
        for(size_t sub_pattern_index = 0; sub_pattern_index < i_pattern_args.size(); sub_pattern_index++)
        {
            uint32_t min = 0, max = 0;
            for(size_t j = sub_pattern_index + 1; j < i_pattern_args.size(); j++)
            {
                min += result.m_pattern_arg_ranges[j].m_min;

                auto new_max = max + result.m_pattern_arg_ranges[j].m_max;
                if(max <= new_max)
                    max = new_max;
                else
                    max = s_max_reps; // overflow, max or m_max were s_max_reps
            }
            result.m_pattern_arg_reiaming_ranges[sub_pattern_index].m_min = min;
            result.m_pattern_arg_reiaming_ranges[sub_pattern_index].m_max = max;
        }

        return result;
    }

    struct VariadicIndex
    {
        uint32_t m_index, m_count;
    };

    struct Substitution
    {
        Name m_variable_name;
        Tensor m_value;
    };

    struct Candidate
    {
        uint32_t m_start_node{};
        uint32_t m_dest_node{};
        Span<const Tensor> m_target_arguments;
        PatternSegment m_pattern;
        uint32_t m_repetitions = 0;
        uint32_t m_version;
        bool m_decayed = false;
        uint32_t m_open{};
        uint32_t m_close{};
        std::vector<Substitution> m_substitutions;
    };

    bool AddSubstitution(Candidate & i_candidate, const Name & i_variable_name, const Tensor & i_value)
    {
        i_candidate.m_substitutions.emplace_back(Substitution{i_variable_name, i_value});
        return true;
    }

    StringBuilder & operator << (StringBuilder & i_dest, const Candidate & i_source)
    {
        i_dest << "Pattern: " << TensorListToString(i_source.m_pattern.m_pattern);
        if(i_source.m_repetitions != 0)
            i_dest << " (" << i_source.m_repetitions << " times)";
        i_dest.NewLine();
        i_dest << " Target: " << TensorListToString(i_source.m_target_arguments);
        return i_dest;
    }

    struct GraphNode
    {
        std::string m_debug_name;
        size_t m_outgoing_edges{};
    };

    struct CandidateRef
    {
        uint32_t m_index = std::numeric_limits<uint32_t>::max();
        uint32_t m_version{};
    };

    struct Edge
    {
        uint32_t m_source_index{};
        CandidateRef m_candidate_ref;
        std::vector<Substitution> m_substitutions;
        uint32_t m_open;
        uint32_t m_close;
    };

    struct MatchingContext
    {
        std::vector<Candidate> m_candidates;
        std::vector<GraphNode> m_graph_nodes;
        std::unordered_multimap<uint32_t, Edge> m_edges; // the key is the destination node
        std::unordered_map<const Expression*, PatternInfo> m_pattern_infos;
        uint32_t m_next_candidate_version{};
        #if DBG_CREATE_GRAPHVIZ_SVG
            std::string m_graph_name;
        #endif
    };

    bool IsCandidateRefValid(const MatchingContext & i_context, CandidateRef i_ref)
    {
        return i_ref.m_index < i_context.m_candidates.size() && i_ref.m_version == i_context.m_candidates[i_ref.m_index].m_version; 
    }

    StringBuilder & operator << (StringBuilder & i_dest, const MatchingContext & i_source)
    {
        i_dest << "digraph G";
        i_dest.NewLine();
        i_dest << "{";
        i_dest.NewLine();
        i_dest.Tab();

        i_dest << "label = \"" << i_source.m_graph_name << "\"";
        i_dest.NewLine();

        const char escaped_newline[] = "\\n";

        for(size_t i = 0; i < i_source.m_graph_nodes.size(); i++)
        {
            const GraphNode & node = i_source.m_graph_nodes[i];
            
            i_dest << "v" << i << "[label = \"";
            if(i == 0)
                i_dest << "Final" << escaped_newline;
            else if(i == 1)
                i_dest << "Initial" << escaped_newline;
            else
                i_dest << "Node " << i << escaped_newline;

            i_dest << "\"]";
            i_dest.NewLine();
        }

        for(const auto & edge : i_source.m_edges)
        {
            std::string labels;
            if(edge.second.m_open)
                labels += " taillabel = \" " + std::string(edge.second.m_open, '+') + " \" ";
            if(edge.second.m_close)
                labels += " headlabel = \" " + std::string(edge.second.m_close, '-') + " \" ";
            
            if(IsCandidateRefValid(i_source, edge.second.m_candidate_ref))
            {
                const Candidate candidate = i_source.m_candidates[edge.second.m_candidate_ref.m_index];

                std::string label;
                if(!candidate.m_target_arguments.empty() && !candidate.m_pattern.m_pattern.empty())
                {
                    label = TensorListToString(candidate.m_target_arguments);
                    label += "\\nis\\n";
                    label += TensorListToString(candidate.m_pattern.m_pattern);
                    if(candidate.m_repetitions != std::numeric_limits<uint32_t>::max())
                        label += ToString(" (", candidate.m_repetitions, " times)");
                }

                i_dest << 'v' << edge.second.m_source_index << " -> v" << edge.first
                    << "[style=\"dashed\", label=\"" << label << "\"" << labels << "]"  << ';';
            }
            else
            {
                std::string label;
                for(const auto & substitution : edge.second.m_substitutions)
                {
                    label += ToString(substitution.m_variable_name, " = ", ToSimplifiedStringForm(substitution.m_value), escaped_newline);
                }
                i_dest << 'v' << edge.second.m_source_index << " -> v" << edge.first
                    << "[label=\"" << label << "\"" << labels << "]"  << ';';
            }
            i_dest.NewLine();
        }

        i_dest.Untab();
        i_dest << "}";
        i_dest.NewLine();

        return i_dest;
    }

    const PatternInfo & GetPatternInfo(MatchingContext & i_context, const Tensor & i_pattern)
    {
        const Expression * expr = i_pattern.GetExpression().get();
        auto it = i_context.m_pattern_infos.find(expr);
        if(it != i_context.m_pattern_infos.end())
            return it->second;
        auto res = i_context.m_pattern_infos.insert({expr, BuildPatternInfo(expr->GetArguments())});
        assert(res.second);
        return res.first->second;
    }

    void AddCandidate(MatchingContext & i_context,
        uint32_t i_start_node, uint32_t i_dest_node,
        Span<const Tensor> i_target, PatternSegment i_pattern,
        uint32_t i_open, uint32_t i_close,
        uint32_t i_repetitions = std::numeric_limits<uint32_t>::max())
    {
        assert(i_start_node != i_dest_node);

        Candidate new_candidate;
        new_candidate.m_start_node = i_start_node;
        new_candidate.m_dest_node = i_dest_node;
        new_candidate.m_pattern = i_pattern;
        new_candidate.m_target_arguments = i_target;
        new_candidate.m_repetitions = i_repetitions;
        new_candidate.m_version = i_context.m_next_candidate_version++;
        new_candidate.m_open = i_open;
        new_candidate.m_close = i_close;

        const uint32_t new_candidate_index = NumericCast<uint32_t>(i_context.m_candidates.size());
        CandidateRef candidate_ref{new_candidate_index, new_candidate.m_version};
        i_context.m_edges.insert({i_dest_node, Edge{i_start_node, candidate_ref, {}, i_open, i_close }});
        i_context.m_graph_nodes[i_start_node].m_outgoing_edges++;

        i_context.m_candidates.push_back(std::move(new_candidate));    
    }

    class LinearPath
    {
    public:
        
        LinearPath(MatchingContext & i_context, const Candidate & i_source_candidate)
            : m_context(i_context),
              m_start_node(i_source_candidate.m_start_node), m_dest_node(i_source_candidate.m_dest_node),
              m_open(i_source_candidate.m_open), m_close(i_source_candidate.m_close)
        {

        }

        LinearPath(const LinearPath &) = delete;
        LinearPath & operator = (const LinearPath &) = delete;

        void AddEdge(Span<const Tensor> i_target, PatternSegment i_pattern,
            bool i_increase_depth = false, uint32_t i_repetitions = std::numeric_limits<uint32_t>::max())
        {
            /*std::string rep_str;
            if(i_repetitions != std::numeric_limits<uint32_t>::max())
                rep_str = ToString(" x", i_repetitions);
            PrintLn("Pattern: ", TensorListToString(i_pattern.m_pattern), rep_str);
            PrintLn("Target: ", TensorListToString(i_target));*/

            if(!i_target.empty() && !i_pattern.m_pattern.empty() && i_repetitions != 0)
            {
                if(!(m_target.empty() && m_pattern.m_pattern.empty()) && m_repetitions != 0)
                {
                    const uint32_t intermediate_node = NumericCast<uint32_t>(m_context.m_graph_nodes.size());
                    m_context.m_graph_nodes.emplace_back();

                    FlushPendingEdgeIfNotEmpty(intermediate_node);

                    m_start_node = intermediate_node;
                }

                // store the pending edge
                m_target = i_target;
                m_pattern = i_pattern;
                m_repetitions = i_repetitions;
                m_increase_depth = i_increase_depth;
            }
        }

        ~LinearPath() noexcept(false)
        {
            if(m_close == 0)
                FlushPendingEdgeIfNotEmpty(m_dest_node);
            else
                FlushPendingEdge(m_dest_node, m_close);
        }

    private:

        void FlushPendingEdgeIfNotEmpty(uint32_t i_dest_node)
        {
            if(!(m_target.empty() && m_pattern.m_pattern.empty()) && m_repetitions != 0)
            {
                FlushPendingEdge(i_dest_node);
            }
        }


        void FlushPendingEdge(uint32_t i_dest_node, uint32_t i_close = {})
        {
            uint32_t open = m_open;
            if(m_increase_depth)
            {
                open++;
                i_close++;
            }
            AddCandidate(m_context, m_start_node, i_dest_node, m_target, m_pattern, open, i_close, m_repetitions);
            m_open = 0;
        }

    private:
        MatchingContext & m_context;
        uint32_t m_start_node;
        uint32_t m_dest_node;
        uint32_t m_open;
        uint32_t m_close;

        // pending edge
        Span<const Tensor> m_target;
        PatternSegment m_pattern;
        uint32_t m_repetitions{};
        bool m_increase_depth{};
    };

    /** Returns false if the matching has failed */
    bool MatchCandidate(MatchingContext & i_context, Candidate & i_candidate)
    {
        const bool nest_index = i_candidate.m_repetitions != std::numeric_limits<uint32_t>::max();
        const uint32_t repetitions = nest_index ? i_candidate.m_repetitions : 1;
        
        size_t target_index = 0;
        for(uint32_t repetition = 0; repetition < repetitions; repetition++)
        {
            for(size_t pattern_index = 0; pattern_index < i_candidate.m_pattern.m_pattern.size(); target_index++, pattern_index++)
            {
                const Tensor & pattern = i_candidate.m_pattern.m_pattern[pattern_index];

                if(i_candidate.m_pattern.m_ranges[pattern_index].m_min != i_candidate.m_pattern.m_ranges[pattern_index].m_max)
                {
                    assert(NameIs(pattern, builtin_names::RepetitionsZeroToMany) ||
                        NameIs(pattern, builtin_names::RepetitionsZeroToOne) ||
                        NameIs(pattern, builtin_names::RepetitionsOneToMany));

                    size_t total_available_targets = i_candidate.m_target_arguments.size() - target_index;

                    size_t sub_pattern_count = pattern.GetExpression()->GetArguments().size();
                    assert(sub_pattern_count != 0); // empty repetitions are illegal and should raise an error when constructed

                    // compute usable range
                    size_t max_usable = total_available_targets - i_candidate.m_pattern.m_remaining[pattern_index].m_min;
                    size_t min_usable = i_candidate.m_pattern.m_remaining[pattern_index].m_max == s_max_reps ?
                        0 :
                        total_available_targets - i_candidate.m_pattern.m_remaining[pattern_index].m_max;
                    if(max_usable > i_candidate.m_pattern.m_ranges[pattern_index].m_max)
                        max_usable = i_candidate.m_pattern.m_ranges[pattern_index].m_max;
                    if(min_usable < i_candidate.m_pattern.m_ranges[pattern_index].m_min)
                        min_usable = i_candidate.m_pattern.m_ranges[pattern_index].m_min;

                    // align the usable range to be a multiple of sub_pattern_count
                    min_usable += sub_pattern_count - 1;
                    min_usable -= min_usable % sub_pattern_count;
                    max_usable -= max_usable % sub_pattern_count;

                    const PatternInfo & pattern_info = GetPatternInfo(i_context, pattern);

                    uint32_t rep = NumericCast<uint32_t>(min_usable / sub_pattern_count);
                    for(size_t used = min_usable; used <= max_usable; used += sub_pattern_count, rep++)
                    {
                        assert(!nest_index); // repetitions can't be nested directly

                        LinearPath path(i_context, i_candidate);

                        // pre-pattern
                        path.AddEdge(i_candidate.m_target_arguments.subspan(target_index, used),
                            PatternSegment{ pattern.GetExpression()->GetArguments(),
                                pattern_info.m_pattern_arg_ranges,
                                pattern_info.m_pattern_arg_reiaming_ranges },
                                true, rep );

                        // post-pattern
                        path.AddEdge(i_candidate.m_target_arguments.subspan(target_index + used),
                            PatternSegment{ i_candidate.m_pattern.m_pattern.subspan(pattern_index + 1),
                                i_candidate.m_pattern.m_ranges.subspan(pattern_index + 1),
                                i_candidate.m_pattern.m_remaining.subspan(pattern_index + 1) } );
                    }
                    return false;
                }

                if(target_index >= i_candidate.m_target_arguments.size())
                    return false;

                const Tensor & target = i_candidate.m_target_arguments[target_index];

                if(IsConstant(pattern))
                {
                    if(!AlwaysEqual(pattern, target))
                        return false;
                }
                else if(NameIs(pattern, builtin_names::Identifier))
                {
                    if(!Is(target, pattern))
                        return false; // type mismatch

                    if(!AddSubstitution(i_candidate, GetIdentifierName(pattern), target))
                        return false; // incompatible substitution
                }
                else 
                {
                    if(pattern.GetExpression()->GetName() != target.GetExpression()->GetName())
                        return false;

                    // build pattern info
                    const PatternInfo & pattern_info = GetPatternInfo(i_context, pattern);

                    // if the target does not have enough arguments, early reject
                    size_t target_arguments = target.GetExpression()->GetArguments().size();
                    if(target_arguments >= pattern_info.m_min_arguments &&
                        target_arguments <= pattern_info.m_max_arguments )
                    {
                        LinearPath path(i_context, i_candidate);

                        // match content
                        path.AddEdge(target.GetExpression()->GetArguments(), 
                            PatternSegment{
                                pattern.GetExpression()->GetArguments(),
                                pattern_info.m_pattern_arg_ranges,
                                pattern_info.m_pattern_arg_reiaming_ranges});

                        // rest of this repetition
                        const size_t remaining_in_pattern = i_candidate.m_pattern.m_pattern.size() - (pattern_index + 1);
                        path.AddEdge(i_candidate.m_target_arguments.subspan(target_index + 1, remaining_in_pattern), 
                            PatternSegment{
                                i_candidate.m_pattern.m_pattern.subspan(pattern_index + 1, remaining_in_pattern),
                                i_candidate.m_pattern.m_ranges.subspan(pattern_index + 1, remaining_in_pattern),
                                i_candidate.m_pattern.m_remaining.subspan(pattern_index + 1, remaining_in_pattern) } );

                        // remaining repetitions
                        const size_t target_start = target_index + 1 + remaining_in_pattern;
                        path.AddEdge(i_candidate.m_target_arguments.subspan(target_start), 
                            i_candidate.m_pattern, false, repetitions - (repetition + 1) );
                    }
                    return false;
                }
            }
        }

        return true;
    }

    #if DBG_CREATE_GRAPHVIZ_SVG
    
        void DumpGraphviz(MatchingContext & i_context, std::string i_name)
        {
            i_context.m_graph_name = i_name;

            std::string dot_file_path(DBG_DEST_DIR + i_name + ".txt");
            std::ofstream(dot_file_path) << ToString(i_context);
            std::string cmd = ToString("\"", DBG_GRAPHVIZ_EXE, " -T png -O ", dot_file_path, "\"");
            int res = std::system(cmd.c_str());
            if(res != 0)
                Error("The command ", cmd, " returned ", res);
         }

    #endif

    void RemoveNode(MatchingContext & i_context, uint32_t i_node_index)
    {
        std::vector<uint32_t> nodes_to_remove;
        nodes_to_remove.push_back(i_node_index);

        while(!nodes_to_remove.empty())
        {
            uint32_t node = nodes_to_remove.back();
            nodes_to_remove.pop_back();

            const auto range = i_context.m_edges.equal_range(node);
            for(auto it = range.first; it != range.second;)
            {
                uint32_t start_node = it->second.m_source_index;

                if(IsCandidateRefValid(i_context, it->second.m_candidate_ref))
                {
                    Candidate & candidate = i_context.m_candidates[it->second.m_candidate_ref.m_index];
                    if(candidate.m_start_node == it->second.m_source_index &&
                        candidate.m_dest_node == it->first)
                    {
                        candidate.m_decayed = true;
                    }
                }

                assert(i_context.m_graph_nodes[it->second.m_source_index].m_outgoing_edges > 0);
                i_context.m_graph_nodes[it->second.m_source_index].m_outgoing_edges--;
                it = i_context.m_edges.erase(it);

                if(i_context.m_graph_nodes[start_node].m_outgoing_edges == 0)
                {
                    // we have removed the last outcoming edge for i_start_node, we can erase it
                    nodes_to_remove.push_back(start_node);
                }
            }
        }
    }

    void RemoveEdge(MatchingContext & i_context, 
        uint32_t i_start_node, uint32_t i_dest_node, 
        CandidateRef i_candidate_ref)
    {
        bool found = false;
        const auto range = i_context.m_edges.equal_range(i_dest_node);
        for(auto it = range.first; it != range.second; it++)
        {
            // the candidate has just been removed from the stack
            // assert(IsCandidateRefValid(i_context, it->second.m_candidate_ref));

            if(it->second.m_source_index == i_start_node &&
                it->second.m_candidate_ref.m_index == i_candidate_ref.m_index &&
                it->second.m_candidate_ref.m_version == i_candidate_ref.m_version)
            {
                assert(i_context.m_graph_nodes[i_start_node].m_outgoing_edges > 0);
                i_context.m_graph_nodes[i_start_node].m_outgoing_edges--;
                i_context.m_edges.erase(it);
                found = true;
                break;
            }
        }
        assert(found);

        if(i_context.m_graph_nodes[i_start_node].m_outgoing_edges == 0)
        {
            // we have removed the last outcoming edge for i_start_node, we can erase it
            RemoveNode(i_context, i_start_node);
        }
    }

    MatchingContext MakeSubstitutionsGraph(const Tensor & i_target, const Tensor & i_pattern)
    {
        MatchingContext context;
        RepRange single_range = {1, 1};
        RepRange single_remaining = {0, 0};

        static_assert(g_start_node_index == 1);
        static_assert(g_end_node_index == 0);
        context.m_graph_nodes.emplace_back().m_debug_name = "End"; // the first node is the final target
        context.m_graph_nodes.emplace_back().m_debug_name = "Start";

        AddCandidate(context, 1, 0, {&i_target, 1}, {{&i_pattern, 1}, {&single_range, 1}, {&single_remaining, 1}}, {}, {});

        #if DBG_CREATE_GRAPHVIZ_SVG
        if(g_enable_graphviz)
        {
            DumpGraphviz(context, "InitialState");
        }
        #endif

        int dbg_step = 0;
        do {
            Candidate candidate = std::move(context.m_candidates.back());
            context.m_candidates.pop_back();
            if(!candidate.m_decayed)
            {
                // MatchCandidate may add other candidates, take the inndex beofore
                const uint32_t candidate_index = NumericCast<uint32_t>(context.m_candidates.size());

                if(!MatchCandidate(context, candidate))
                {
                    RemoveEdge(context, 
                        candidate.m_start_node, candidate.m_dest_node,
                        {candidate_index, candidate.m_version});
                }
                else
                {
                    bool found = false;
                    const auto range = context.m_edges.equal_range(candidate.m_dest_node);
                    for(auto it = range.first; it != range.second; it++)
                    {
                        // the candidate has just been removed from the stack
                        // assert(IsCandidateRefValid(i_context, it->second.m_candidate_ref));

                        if(it->second.m_source_index == candidate.m_start_node &&
                            it->second.m_candidate_ref.m_index == candidate_index &&
                            it->second.m_candidate_ref.m_version == candidate.m_version)
                        {
                            it->second.m_substitutions = std::move(candidate.m_substitutions);
                            found = true;
                            break;
                        }
                    }
                    assert(found);
                }

                #if DBG_CREATE_GRAPHVIZ_SVG
                    if(g_enable_graphviz)
                        DumpGraphviz(context, ToString("Step_", dbg_step));
                    dbg_step++;
                #endif
            }

        } while(!context.m_candidates.empty());

        return context;
    }

    /* this function is recursive and slow, but is used only for testing 
        the correctness of the substitution graph */
    size_t CountSolutions(const MatchingContext & i_context, uint32_t i_node_index)
    {
        size_t solutions = 0;
        const auto range = i_context.m_edges.equal_range(i_node_index);
        for(auto it = range.first; it != range.second; it++)
        {
            if(it->second.m_source_index == g_start_node_index)
                ++solutions;
            else
                solutions += CountSolutions(i_context, it->second.m_source_index);
        }
        return solutions;
    }

    struct VariadicValue
    {
        std::vector<std::vector<Tensor>> m_stack;
    };

    void VariadicAddValue(VariadicValue & i_dest, uint32_t i_depth, const Tensor & i_value)
    {
        if(i_dest.m_stack.size() < i_depth)
            i_dest.m_stack.resize(i_depth);
        i_dest.m_stack.back().push_back(i_value);
    }

    Tensor ReverseToTuple(const std::vector<Tensor> & i_source)
    {
        std::vector<Tensor> arguments;
        arguments.reserve(i_source.size());
        for(auto it = i_source.rbegin(); it != i_source.rend(); ++it)
            arguments.push_back(*it);
        return Tuple(arguments);
    }

    void VariadicReduceDepth(VariadicValue & i_dest)
    {
        assert(i_dest.m_stack.size() >= 2);

        const size_t size = i_dest.m_stack.size();
        i_dest.m_stack[size - 2].push_back(Tuple(i_dest.m_stack[size - 1]));
        i_dest.m_stack.pop_back();
    }

    Tensor VariadicClear(VariadicValue & i_dest)
    {
        assert(i_dest.m_stack.size() >= 1);

        while(i_dest.m_stack.size() > 1)
            VariadicReduceDepth(i_dest);

        Tensor result = ReverseToTuple(i_dest.m_stack.front());
        i_dest.m_stack.clear();
        return result;
    }

    struct Solution
    {
        uint32_t m_node{};
        uint32_t m_depth{};
        std::unordered_map<Name, Tensor> m_substitutions;
        std::unordered_map<Name, VariadicValue> m_variadic_substitutions;
    };

    bool AddSubstitution(Solution & i_dest, const Name & i_variable_name, const Tensor & i_value)
    {
        const auto [it, inserted] = i_dest.m_substitutions.insert(std::pair(i_variable_name, i_value));
        if(!inserted)
            return AlwaysEqual(it->second, i_value);
        else
            return true;
    }

    PatternMatch Match(const Tensor & i_target, const Tensor & i_pattern)
    {
        MatchingContext context = MakeSubstitutionsGraph(i_target, i_pattern);

        std::vector<Solution> solutions;
        solutions.push_back(Solution{g_end_node_index});

        do {
            const Solution source_solution = solutions.back();
            solutions.pop_back();

            const auto range = context.m_edges.equal_range(source_solution.m_node);
            for(auto edge_it = range.first; edge_it != range.second; edge_it++)
            {
                Solution solution = source_solution;

                bool incompatible = false;

                solution.m_depth += edge_it->second.m_close;

                if(solution.m_depth == 0)
                {
                    assert(edge_it->second.m_open == 0);

                    for(const Substitution & substitution : edge_it->second.m_substitutions)
                    {
                        if(!AddSubstitution(solution, substitution.m_variable_name, substitution.m_value))
                        {
                            incompatible = true;
                            break;
                        }
                    }   
                }
                else
                {
                    for(const Substitution & substitution : edge_it->second.m_substitutions)
                    {
                        VariadicAddValue(solution.m_variadic_substitutions[substitution.m_variable_name], solution.m_depth, substitution.m_value);
                    }

                    if(edge_it->second.m_open)
                    {
                        solution.m_depth -= edge_it->second.m_open;
                        assert(solution.m_depth >= 0);

                        if(solution.m_depth == 0)
                        {
                            for(auto & var_subst : solution.m_variadic_substitutions)
                            {
                                Tensor value = VariadicClear(var_subst.second);
                                if(!AddSubstitution(solution, var_subst.first, value))
                                {
                                    incompatible = true;
                                    break;
                                }
                            }
                            solution.m_variadic_substitutions.clear();
                        }
                        else
                        {
                            for(auto & var_subst : solution.m_variadic_substitutions)
                            {
                                while(var_subst.second.m_stack.size() > solution.m_depth)
                                    VariadicReduceDepth(var_subst.second);
                            }
                        }
                    }
                }

                solution.m_node = edge_it->second.m_source_index;
                if(solution.m_node == g_start_node_index)
                {
                    assert(solution.m_depth == 0);
                    return PatternMatch{1, std::move(solution.m_substitutions) };
                }

                if(!incompatible)
                    solutions.push_back(solution);
            }

        } while(!solutions.empty());

        return {};
    }

    size_t PatternMatchingCount(const Tensor & i_target, const Tensor & i_pattern)
    {
        MatchingContext context = MakeSubstitutionsGraph(i_target, i_pattern);
        const size_t solutions = CountSolutions(context, g_end_node_index);
        return solutions;
    }
}

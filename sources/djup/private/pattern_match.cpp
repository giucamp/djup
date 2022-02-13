
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

    struct Candidate
    {
        uint32_t m_start_node{};
        uint32_t m_dest_node{};
        Span<const Tensor> m_target_arguments;
        PatternSegment m_pattern;
        uint32_t m_repetitions = 0;
        uint32_t m_version;
        bool m_has_repetitions = false;
        bool m_decayed = false;
        std::vector<VariadicIndex> m_variadic_indices;
    };

    StringBuilder & operator << (StringBuilder & i_dest, const Candidate & i_source)
    {
        i_dest << "Pattern: " << TensorListToString(i_source.m_pattern.m_pattern);
        if(i_source.m_repetitions != 0)
            i_dest << " (" << i_source.m_repetitions << " times)";
        i_dest.NewLine();
        i_dest << " Target: " << TensorListToString(i_source.m_target_arguments);
        return i_dest;
    }

    struct Substitution
    {
        Name m_variable_name;
        std::vector<VariadicIndex> m_indices;
        Tensor m_value;
    };

    struct GraphNode
    {
        std::string m_debug_name;
        std::vector<Substitution> m_substitutions;
        size_t m_outgoing_edges{};

        bool AddSubstitution(const Name & i_variable_name, Span<VariadicIndex> i_indices, const Tensor & i_value)
        {
            m_substitutions.emplace_back(Substitution{i_variable_name, {i_indices.begin(), i_indices.end()}, i_value});
            return true;
        }
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
            std::unordered_map<uint64_t, std::string> m_edge_labels;
        #endif
    };

    bool IsCandidateRefValid(const MatchingContext & i_context, CandidateRef i_ref)
    {
        return i_ref.m_index < i_context.m_candidates.size() && i_ref.m_version == i_context.m_candidates[i_ref.m_index].m_version; 
    }

    uint64_t CombineNodeIndices(uint32_t i_start_node, uint32_t i_dest_node)
    {
        return (static_cast<uint64_t>(i_start_node) << 32) | i_dest_node;
    }

    std::string GetEdgeLabel(const MatchingContext & i_context, uint32_t i_start_node, uint32_t i_dest_node)
    {
        auto it = i_context.m_edge_labels.find(CombineNodeIndices(i_start_node, i_dest_node));
        if(it != i_context.m_edge_labels.end())
            return it->second;
        else
            return "";
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
            i_dest << "v" << i << "[label = \"";
            if(i == 0)
                i_dest << "Final" << escaped_newline;
            else if(i == 1)
                i_dest << "Initial" << escaped_newline;
            else
                i_dest << "Node " << i << escaped_newline;
            for(const auto & substitution : i_source.m_graph_nodes[i].m_substitutions)
            {
                i_dest << substitution.m_variable_name << " = " << ToSimplifiedStringForm(substitution.m_value) << escaped_newline;
            }
            i_dest << "\"]";
            i_dest.NewLine();
        }

        for(const auto & edge : i_source.m_edges)
        {
            std::string label = GetEdgeLabel(i_source, edge.second.m_source_index, edge.first);

            size_t candidate_count = 0;
            for(const Candidate & candidate : i_source.m_candidates)
            {
                if(candidate.m_start_node == edge.second.m_source_index && 
                    candidate.m_dest_node == edge.first)
                        candidate_count++;
            }
            //assert(candidate_count <= 1);

            if(candidate_count > 0)
            {
                i_dest << 'v' << edge.second.m_source_index << " -> v" << edge.first
                    << "[style=\"dashed\", label=\"" << label << "\"]"  << ';';
            }
            else
            {
                i_dest << 'v' << edge.second.m_source_index << " -> v" << edge.first
                    << "[label=\"" << label << "\"]"  << ';';
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
        uint32_t i_repetitions = std::numeric_limits<uint32_t>::max())
    {
        assert(i_start_node != i_dest_node);

        #if DBG_CREATE_GRAPHVIZ_SVG
            if(!i_target.empty() && !i_pattern.m_pattern.empty())
            {
                std::string label = TensorListToString(i_target);
                label += "\\nis\\n";
                label += TensorListToString(i_pattern.m_pattern);
                if(i_repetitions != std::numeric_limits<uint32_t>::max())
                    label += ToString(" (", i_repetitions, " times)");
                i_context.m_edge_labels[CombineNodeIndices(i_start_node, i_dest_node)] = label;
            }
        #endif

        Candidate new_candidate;
        new_candidate.m_start_node = i_start_node;
        new_candidate.m_dest_node = i_dest_node;
        new_candidate.m_pattern = i_pattern;
        new_candidate.m_target_arguments = i_target;
        new_candidate.m_has_repetitions = i_repetitions != std::numeric_limits<uint32_t>::max();
        new_candidate.m_repetitions = new_candidate.m_has_repetitions ? i_repetitions : 1;
        new_candidate.m_version = i_context.m_next_candidate_version++;
        
        const uint32_t new_candidate_index = NumericCast<uint32_t>(i_context.m_candidates.size());
        CandidateRef candidate_ref{new_candidate_index, new_candidate.m_version};
        i_context.m_edges.insert({i_dest_node, {i_start_node, candidate_ref }});
        i_context.m_graph_nodes[i_start_node].m_outgoing_edges++;

        i_context.m_candidates.push_back(std::move(new_candidate));
        
    }

    class LinearPath
    {
    public:
        
        LinearPath(MatchingContext & i_context, uint32_t i_start_node, uint32_t i_dest_node)
            : m_context(i_context), m_start_node(i_start_node), m_dest_node(i_dest_node)
        {

        }

        LinearPath(const LinearPath &) = delete;
        LinearPath & operator = (const LinearPath &) = delete;

        void AddEdge(Span<const Tensor> i_target, PatternSegment i_pattern,
            uint32_t i_repetitions = std::numeric_limits<uint32_t>::max())
        {
            if(!i_target.empty() && !i_pattern.m_pattern.empty() && i_repetitions != 0)
            {
                if(!m_target.empty() && !m_pattern.m_pattern.empty() && m_repetitions != 0)
                {
                    const uint32_t intermediate_node = NumericCast<uint32_t>(m_context.m_graph_nodes.size());
                    m_context.m_graph_nodes.emplace_back();

                    FlushPendingEdge(intermediate_node);

                    m_start_node = intermediate_node;
                }

                // store the pending edge
                m_target = i_target;
                m_pattern = i_pattern;
                m_repetitions = i_repetitions;
            }
        }

        ~LinearPath() noexcept(false)
        {
            FlushPendingEdge(m_dest_node);
        }

    private:

        void FlushPendingEdge(uint32_t i_dest_node)
        {
            AddCandidate(m_context, m_start_node, i_dest_node, m_target, m_pattern, m_repetitions);
        }

    private:
        MatchingContext & m_context;
        uint32_t m_start_node;
        uint32_t m_dest_node;

        // pemding edge
        Span<const Tensor> m_target;
        PatternSegment m_pattern;
        uint32_t m_repetitions{};
    };

    /** Returns false if the matching has failed */
    bool MatchCandidate(MatchingContext & i_context, Candidate & i_candidate)
    {
        /*std::string rep_str;
        if(i_candidate.m_has_repetitions)
            rep_str = ToString(" x", i_candidate.m_repetitions);
        PrintLn("Pattern: ", TensorListToString(i_candidate.m_pattern.m_pattern), rep_str);
        PrintLn("Target: ", TensorListToString(i_candidate.m_target_arguments));*/
        
        const bool nest_index = i_candidate.m_has_repetitions;
        const uint32_t repetitions = nest_index ? i_candidate.m_repetitions : 1;
        
        if(nest_index)
            i_candidate.m_variadic_indices.push_back(VariadicIndex{0, repetitions});

        size_t target_index = 0;
        for(uint32_t repetition = 0; repetition < repetitions; repetition++)
        {
            if(nest_index)
                i_candidate.m_variadic_indices.back().m_index = repetition;
            
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

                        LinearPath path(i_context, i_candidate.m_start_node, i_candidate.m_dest_node);

                        // pre-pattern
                        path.AddEdge(i_candidate.m_target_arguments.subspan(target_index, used),
                            PatternSegment{ pattern.GetExpression()->GetArguments(),
                                pattern_info.m_pattern_arg_ranges,
                                pattern_info.m_pattern_arg_reiaming_ranges },
                                rep );

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

                    if(!i_context.m_graph_nodes[i_candidate.m_dest_node].AddSubstitution(
                            GetIdentifierName(pattern), i_candidate.m_variadic_indices, target))
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
                        LinearPath path(i_context, i_candidate.m_start_node, i_candidate.m_dest_node);

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
                        path.AddEdge( i_candidate.m_target_arguments.subspan(target_start), 
                            i_candidate.m_pattern, repetitions - (repetition + 1) );
                    }
                    return false;
                }
            }
        }

        return true;
    }

    void FilltMatches(const MatchingContext & i_context, std::vector<PatternMatch> & i_matches)
    {
        
    }

    std::vector<PatternMatch> GetMatches(const MatchingContext & i_context)
    {
        std::vector<PatternMatch> result;

        assert(!i_context.m_graph_nodes.empty());

        std::vector<bool> visited_nodes(i_context.m_graph_nodes.size());

        size_t curr_node = 0;
        //for(;;)
        {
            //curr_node
            
        }

        return result;
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
        /*bool redo;
        do {
            
            redo = false;

            const auto range = i_context.m_edges.equal_range(i_node_index);
            for(auto it = range.first; it != range.second; it++)
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
                i_context.m_edges.erase(it);
                
                if(i_context.m_graph_nodes[start_node].m_outgoing_edges == 0)
                {
                    // we have removed the last outcoming edge for i_start_node, we can erase it
                    RemoveNode(i_context, start_node);
                }
                
                redo = true;
                break;
            }

        } while(redo);*/

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

    std::vector<PatternMatch> Match(const Tensor & i_target, const Tensor & i_pattern)
    {
        MatchingContext context;
        RepRange single_range = {1, 1};
        RepRange single_remaining = {0, 0};

        context.m_graph_nodes.emplace_back().m_debug_name = "End"; // the first node is the final target
        context.m_graph_nodes.emplace_back().m_debug_name = "Start";

        AddCandidate(context, 1, 0, {&i_target, 1}, {{&i_pattern, 1}, {&single_range, 1}, {&single_remaining, 1}});

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

                #if DBG_CREATE_GRAPHVIZ_SVG
                    if(g_enable_graphviz)
                        DumpGraphviz(context, ToString("Step_", dbg_step));
                    dbg_step++;
                #endif
            }

        } while(!context.m_candidates.empty());

        std::vector<PatternMatch> matches = GetMatches(context);
        return matches;
    }
}

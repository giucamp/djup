
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/namespace.h>
#include <private/make_expr.h>
#include <private/substitute_by_predicate.h>
#include <private/o2o_pattern/o2o_pattern_match.h>
#include <private/o2o_pattern/o2o_pattern_info.h>
#include <private/o2o_pattern/o2o_debug_utils.h>
#include <private/o2o_pattern/o2o_substitutions_builder.h>
#include <private/builtin_names.h>
#include <core/flags.h>
#include <core/pool.h>
#include <core/graph_wiz.h>

namespace djup
{
    namespace o2o_pattern
    {
        namespace
        {
            Tensor PreprocessPattern(const Namespace & i_namespace, const Tensor & i_pattern)
            {
                return SubstituteByPredicate(i_namespace, i_pattern, [&i_namespace](const Tensor & i_candidate) {
                    FunctionFlags flags = GetFunctionFlags(*i_candidate.GetExpression());

                    bool some_substitution = false;
                    std::vector<Tensor> new_arguments;

                    const std::vector<Tensor> & arguments = i_candidate.GetExpression()->GetArguments();
                    const size_t argument_count = arguments.size();

                    // substitute identifiers in associative functions with AssociativeIdentifier()
                    if (HasFlag(flags, FunctionFlags::Associative))
                    {
                        size_t index = 0;

                        for (; index < argument_count; index++)
                        {
                            const Tensor & argument = arguments[index];
                            if (!IsConstant(argument))
                            {
                                new_arguments = arguments;
                                some_substitution = true;
                                break;
                            }
                        }

                        for (; index < argument_count; index++)
                        {
                            const Tensor & argument = arguments[index];
                            if (!IsConstant(argument))
                            {
                                new_arguments[index] = MakeExpression(
                                    i_namespace,
                                    argument.GetExpression()->GetType(),
                                    builtin_names::AssociativeIdentifier,
                                    { argument },
                                    argument.GetExpression()->GetMetadata());
                            }
                        }
                    }

                    if (some_substitution)
                        return MakeExpression(i_namespace,
                            i_candidate.GetExpression()->GetType(),
                            i_candidate.GetExpression()->GetName(),
                            new_arguments, i_candidate.GetExpression()->GetMetadata());
                    else
                        return i_candidate;
                    });
            }
        }

        struct PatternSegment
        {
            FunctionFlags m_flags = FunctionFlags::None;
            Span<const Tensor> m_pattern;
            Span<const ArgumentInfo> m_arg_infos;

            PatternSegment() = default;

            PatternSegment(FunctionFlags i_flags, Span<const Tensor> i_pattern,
                    Span<const ArgumentInfo> i_arguments)
                : m_flags(i_flags), m_pattern(i_pattern), m_arg_infos(i_arguments)
            {
                DJUP_ASSERT(m_arg_infos.size() == m_pattern.size());
            }
        };

        struct GraphNode
        {
            size_t m_incoming_edges{};
        };

        struct Candidate
        {
            uint32_t m_start_node{};
            uint32_t m_dest_node{};
            Span<const Tensor> m_target_arguments;
            PatternSegment m_segment;
            uint32_t m_repetitions = 0;
            uint32_t m_version{};
            uint32_t m_open{};
            uint32_t m_close{};
            std::vector<Substitution> m_substitutions;
        };

        StringBuilder & operator << (StringBuilder & i_dest, const Candidate & i_source)
        {
            i_dest << "Pattern: " << TensorSpanToString(i_source.m_segment.m_pattern);
            if (i_source.m_repetitions != 1)
                i_dest << " (" << i_source.m_repetitions << " times)";
            i_dest.NewLine();
            i_dest << " Target: " << TensorSpanToString(i_source.m_target_arguments);
            return i_dest;
        }

        struct Edge
        {
            uint32_t m_dest_index{};
            Pool<Candidate>::Handle m_candidate_ref;
            std::vector<Substitution> m_substitutions;
            uint32_t m_open;
            uint32_t m_close;
        };

        struct MatchingContext
        {
            const Namespace * m_namespace;
            Pool<Candidate> m_candidates;
            std::vector<Pool<Candidate>::Handle> m_candidate_queue;
            std::vector<GraphNode> m_graph_nodes;
            std::unordered_multimap<uint32_t, Edge> m_edges; // the key is the source node
            std::unordered_map<const Expression*, PatternInfo> m_pattern_infos;
            const char * m_artifact_path{nullptr};
        };

        constexpr uint32_t g_start_node_index = 0;
        constexpr uint32_t g_end_node_index = 1;

        GraphWizGraph MakeSolutionGraphWiz(const MatchingContext & i_source,
            std::string i_graph_name)
        {
            GraphWizGraph graph(i_graph_name);
            
            // next_candidate
            const Candidate * next_candidate = nullptr;
            for (auto it = i_source.m_candidate_queue.rbegin();
                it != i_source.m_candidate_queue.rend();
                ++it)
            {
                next_candidate = i_source.m_candidates.TryGetObject(*it);
                if (next_candidate != nullptr)
                    break;
            }

            for (size_t i = 0; i < i_source.m_graph_nodes.size(); i++)
            {
                GraphWizGraph::Node & dest_node = graph.AddNode({});

                if (i == g_end_node_index)
                    dest_node.SetLabel("Final (1)");
                else if (i == g_start_node_index)
                    dest_node.SetLabel("Initial (0)");
                else
                    dest_node.SetLabel(ToString("Node ", i));
            }

            for (const auto & edge : i_source.m_edges)
            {
                GraphWizGraph::Edge & dest_edge = graph.AddEdge(
                    edge.first, edge.second.m_dest_index);

                auto Append = [&](std::string & i_dest, const std::string & i_str) {
                    if (!i_dest.empty())
                        i_dest += "\n";
                    i_dest += i_str;
                };

                std::string label, head, tail;
                if (i_source.m_candidates.IsValid(edge.second.m_candidate_ref))
                {
                    const Candidate & candidate = i_source.m_candidates.GetObject(edge.second.m_candidate_ref);
                    if (HasAllFlags(candidate.m_segment.m_flags, CombineFlags(FunctionFlags::Associative, FunctionFlags::Commutative)))
                        Append(label, "ac");
                    else if (HasFlag(candidate.m_segment.m_flags, FunctionFlags::Associative))
                        Append(label, "a");
                    else if (HasFlag(candidate.m_segment.m_flags, FunctionFlags::Commutative))
                        Append(label, "a");
                }

                if (edge.second.m_open)
                    Append(tail, std::string(edge.second.m_open, '+'));

                if (edge.second.m_close)
                    Append(head, std::string(edge.second.m_close, '-'));

                dest_edge.SetHeadLabel(head);
                dest_edge.SetTailLabel(tail);

                // if this edge is associated with an alive candidate
                if (i_source.m_candidates.IsValid(edge.second.m_candidate_ref))
                {
                    const Candidate & candidate = i_source.m_candidates.GetObject(edge.second.m_candidate_ref);

                    std::string label;
                    if (!candidate.m_target_arguments.empty() && !candidate.m_segment.m_pattern.empty())
                    {
                        Append(label, TensorSpanToString(candidate.m_target_arguments));
                        Append(label, "is" );
                        Append(label, TensorSpanToString(candidate.m_segment.m_pattern));
                        if (candidate.m_repetitions != 1)
                            Append(label, ToString(" (", candidate.m_repetitions, " times)"));
                    }

                    for (const auto & substitution : candidate.m_substitutions)
                    {
                        Append(label, ToString(substitution.m_identifier_name, 
                            " = ", ToSimplifiedString(substitution.m_value)));
                    }

                    dest_edge.SetLabel(label);

                    dest_edge.SetStyle(GraphWizGraph::EdgeStyle::Dashed);

                    // make the next edge to be processed red
                    if (&candidate == next_candidate)
                    {
                        dest_edge.SetDrawingColor({ 100, 0, 0 });
                        dest_edge.SetFontColor({ 100, 0, 0 });
                    }
                }
                else
                {
                    std::string label;
                    for (const auto & substitution : edge.second.m_substitutions)
                    {
                        Append(label, ToString(substitution.m_identifier_name,
                            " = ", ToSimplifiedString(substitution.m_value)));
                    }
                    dest_edge.SetLabel(label);
                }
            }

            return graph;
        }


        const PatternInfo & GetPatternInfo(MatchingContext & i_context, const Tensor & i_pattern)
        {
            const Expression * expr = i_pattern.GetExpression().get();
            auto it = i_context.m_pattern_infos.find(expr);
            if (it != i_context.m_pattern_infos.end())
                return it->second;
            auto res = i_context.m_pattern_infos.insert({ expr, BuildPatternInfo(i_pattern) });
            DJUP_ASSERT(res.second);
            return res.first->second;
        }

        void AddCandidate(MatchingContext & i_context,
            uint32_t i_start_node, uint32_t i_dest_node,
            Span<const Tensor> i_target, PatternSegment i_pattern,
            std::vector<Substitution> i_substitutions,
            uint32_t i_open, uint32_t i_close,
            uint32_t i_repetitions = 1)
        {
            DJUP_ASSERT(i_start_node != i_dest_node);

            auto cand_handle = i_context.m_candidates.New();
            Candidate & new_candidate = i_context.m_candidates.GetObject(cand_handle);
            i_context.m_candidate_queue.push_back(cand_handle);
            new_candidate.m_start_node = i_start_node;
            new_candidate.m_dest_node = i_dest_node;
            new_candidate.m_segment = i_pattern;
            new_candidate.m_target_arguments = i_target;
            new_candidate.m_repetitions = i_repetitions;
            new_candidate.m_open = i_open;
            new_candidate.m_close = i_close;
            new_candidate.m_substitutions = std::move(i_substitutions);

            i_context.m_edges.insert({ i_start_node, Edge{i_dest_node, cand_handle, {}, i_open, i_close } });
            i_context.m_graph_nodes[i_dest_node].m_incoming_edges++;
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
                std::vector<Substitution> i_substitutions,
                bool i_increase_depth = false, uint32_t i_repetitions = 1)
            {
                bool empty_edge = i_target.empty() && i_substitutions.empty() && !i_increase_depth;

                empty_edge = empty_edge || i_repetitions == 0;

                if(!empty_edge)
                {
                    /*std::string rep_str;
                    if(i_repetitions != std::numeric_limits<uint32_t>::max())
                        rep_str = ToString(" x", i_repetitions);
                    PrintLn("Pattern: ", TensorSpanToString(i_pattern.m_pattern), rep_str);
                    PrintLn("Target: ", TensorSpanToString(i_target));*/

                    if(m_has_edge)
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
                    m_increase_depth = i_increase_depth;
                    m_substitutions = std::move(i_substitutions);
                    m_has_edge = true;
                }
            }

            ~LinearPath() noexcept(false)
            {
                if(m_has_edge)
                    FlushPendingEdge(m_dest_node, m_close);
                
                if (!m_any_edge_added)
                {
                    AddCandidate(m_context, m_start_node, m_dest_node,
                        {}, {}, std::move(m_substitutions),
                        0, 0, 1);
                }
            }

        private:

            void FlushPendingEdge(uint32_t i_dest_node, uint32_t i_close = {})
            {
                DJUP_ASSERT(m_has_edge);
                
                uint32_t open = m_open;
                if (m_increase_depth)
                {
                    open++;
                    i_close++;
                }

                AddCandidate(m_context, m_start_node, i_dest_node,
                    m_target, m_pattern, std::move(m_substitutions),
                    open, i_close, m_repetitions);
                m_open = 0;
                m_any_edge_added = true;
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
            bool m_increase_depth{ false };
            bool m_has_edge{ false };
            bool m_any_edge_added{ false };
            std::vector<Substitution> m_substitutions;
        };

        /** Returns false if the matching has failed */
        bool MatchCandidate(MatchingContext & i_context, Candidate & i_candidate)
        {
            const uint32_t repetitions = i_candidate.m_repetitions;

            uint32_t target_index = 0;
            for (uint32_t repetition = 0; repetition < repetitions; repetition++)
            {
                for (uint32_t pattern_index = 0; pattern_index < i_candidate.m_segment.m_pattern.size(); target_index++, pattern_index++)
                {
                    const Tensor & pattern = i_candidate.m_segment.m_pattern[pattern_index];

                    const ArgumentInfo & arg_info = i_candidate.m_segment.m_arg_infos[pattern_index];

                    if (arg_info.m_cardinality.m_min != arg_info.m_cardinality.m_max)
                    {
                        uint32_t total_available_targets = static_cast<uint32_t>(
                            i_candidate.m_target_arguments.size()) - target_index;

                        uint32_t sub_pattern_count = static_cast<uint32_t>(pattern.GetExpression()->GetArguments().size());
                        DJUP_ASSERT(sub_pattern_count != 0); // empty repetitions are illegal and should raise an error when constructed

                        // compute usable range
                        UIntInterval usable;
                        usable.m_max = total_available_targets - arg_info.m_remaining.m_min;
                        usable.m_min = arg_info.m_remaining.m_max ==
                            std::numeric_limits<uint32_t>::max() ?
                            0 :
                            total_available_targets - arg_info.m_remaining.m_max;

                        usable = arg_info.m_cardinality.ClampRange(usable);

                        // align the usable range to be a multiple of sub_pattern_count
                        usable.m_min += static_cast<int32_t>(sub_pattern_count - 1);
                        usable.m_min -= usable.m_min % sub_pattern_count;
                        usable.m_max -= usable.m_max % sub_pattern_count;

                        const PatternInfo & pattern_info = GetPatternInfo(i_context, pattern);

                        uint32_t rep = NumericCast<uint32_t>(usable.m_min / sub_pattern_count);
                        for (uint32_t used = usable.m_min; used <= usable.m_max; used += sub_pattern_count, rep++)
                        {
                            LinearPath path(i_context, i_candidate);

                            // pre-pattern
                            PatternSegment pre_segment;
                            pre_segment.m_flags = pattern_info.m_flags;
                            pre_segment.m_pattern = pattern.GetExpression()->GetArguments();
                            pre_segment.m_arg_infos = pattern_info.m_arguments_info;
                            path.AddEdge(
                                i_candidate.m_target_arguments.subspan(target_index, used),
                                pre_segment, std::move(i_candidate.m_substitutions), 
                                true, rep);

                            // post-pattern
                            PatternSegment post_segment;
                            post_segment.m_flags = pattern_info.m_flags;
                            post_segment.m_pattern = i_candidate.m_segment.m_pattern.subspan(pattern_index + 1);
                            post_segment.m_arg_infos = i_candidate.m_segment.m_arg_infos.subspan(pattern_index + 1);
                            auto target = i_candidate.m_target_arguments.subspan(target_index + used);
                            path.AddEdge(target, post_segment, {});
                        }
                        return false;
                    }

                    if (target_index >= i_candidate.m_target_arguments.size())
                        return false;

                    const Tensor & target = i_candidate.m_target_arguments[target_index];

                    if (IsConstant(pattern))
                    {
                        if (!AlwaysEqual(pattern, target))
                            return false;
                    }
                    else if (IsIdentifier(pattern))
                    {
                        if (!i_context.m_namespace->TypeBelongsTo(
                            target.GetExpression()->GetType(),
                            pattern.GetExpression()->GetType()))
                            return false;

                        i_candidate.m_substitutions.push_back({ 
                            pattern.GetExpression()->GetName(), target });
                    }
                    else
                    {
                        if (pattern.GetExpression()->GetName() != target.GetExpression()->GetName())
                            return false;

                        // build pattern info
                        const PatternInfo & pattern_info = GetPatternInfo(i_context, pattern);

                        // if the target does not have enough arguments, early reject
                        size_t target_arguments = target.GetExpression()->GetArguments().size();
                        if (target_arguments >= pattern_info.m_arguments_range.m_min &&
                            target_arguments <= pattern_info.m_arguments_range.m_max)
                        {
                            LinearPath path(i_context, i_candidate);

                            // match content
                            path.AddEdge(target.GetExpression()->GetArguments(),
                                PatternSegment{ pattern_info.m_flags,
                                    pattern.GetExpression()->GetArguments(),
                                    pattern_info.m_arguments_info },
                                    std::move(i_candidate.m_substitutions));

                            // rest of this repetition
                            const size_t remaining_in_pattern = i_candidate.m_segment.m_pattern.size() - (pattern_index + 1);
                            path.AddEdge(i_candidate.m_target_arguments.subspan(target_index + 1, remaining_in_pattern),
                                PatternSegment{ pattern_info.m_flags,
                                    i_candidate.m_segment.m_pattern.subspan(pattern_index + 1),
                                    i_candidate.m_segment.m_arg_infos.subspan(pattern_index + 1) }, {});

                            // remaining repetitions
                            const size_t target_start = target_index + 1 + remaining_in_pattern;
                            path.AddEdge(i_candidate.m_target_arguments.subspan(target_start),
                                i_candidate.m_segment, {}, false, repetitions - (repetition + 1));
                        }
                        return false;
                    }
                }
            }

            return true;
        }

        void RemoveNode(MatchingContext & i_context, uint32_t i_node_index)
        {
            std::vector<uint32_t> nodes_to_remove;
            nodes_to_remove.push_back(i_node_index);

            while (!nodes_to_remove.empty())
            {
                const uint32_t node = nodes_to_remove.back();
                nodes_to_remove.pop_back();

                // for all nodes outgoing from this node...
                const auto range = i_context.m_edges.equal_range(node);
                for (auto it = range.first; it != range.second;)
                {
                    // we are going to delete the edge from this node to it->dest
                    const uint32_t dest_node = it->second.m_dest_index;

                    // if the edge has an associated candidate, delete it
                    if (i_context.m_candidates.IsValid(it->second.m_candidate_ref))
                    {
                        Candidate & candidate = i_context.m_candidates.GetObject(it->second.m_candidate_ref);
                        if (candidate.m_start_node == it->first && 
                            candidate.m_dest_node == it->second.m_dest_index)
                        {
                            i_context.m_candidates.Delete(it->second.m_candidate_ref);
                        }
                    }

                    // decrement the incoming edge on the dest
                    DJUP_ASSERT(i_context.m_graph_nodes[it->second.m_dest_index].m_incoming_edges > 0);
                    i_context.m_graph_nodes[it->second.m_dest_index].m_incoming_edges--;
                    
                    it = i_context.m_edges.erase(it);

                    // if this was the last incoming edge to the dest node, delete it
                    if (i_context.m_graph_nodes[dest_node].m_incoming_edges == 0)
                    {
                        nodes_to_remove.push_back(dest_node);
                    }
                }
            }
        }

        void RemoveEdge(MatchingContext & i_context,
            uint32_t i_start_node, uint32_t i_dest_node,
            Pool<Candidate>::Handle i_candidate_ref)
        {
            bool found = false;
            const auto range = i_context.m_edges.equal_range(i_start_node);
            for (auto it = range.first; it != range.second; it++)
            {
                if (it->second.m_dest_index == i_dest_node &&
                    it->second.m_candidate_ref == i_candidate_ref)
                {
                    DJUP_ASSERT(i_context.m_graph_nodes[i_dest_node].m_incoming_edges > 0);
                    i_context.m_graph_nodes[i_dest_node].m_incoming_edges--;
                    i_context.m_edges.erase(it);
                    found = true;
                    break;
                }
            }
            DJUP_ASSERT(found);

            if (i_context.m_graph_nodes[i_dest_node].m_incoming_edges == 0)
            {
                // we have removed the last outcoming edge for i_start_node, we can erase it
                RemoveNode(i_context, i_dest_node);
            }
        }
                
        void MakeSubstitutionsGraph(MatchingContext & i_context, 
            const Tensor & i_target, const Tensor & i_pattern)
        {
            Tensor pattern = PreprocessPattern(*i_context.m_namespace, i_pattern);
            const Tensor & target = i_target;

            UIntInterval single_range = {1, 1};
            UIntInterval single_remaining = {0, 0};

            static_assert(g_start_node_index == 0 && g_end_node_index == 1);
            i_context.m_graph_nodes.emplace_back(); // start node
            i_context.m_graph_nodes.emplace_back(); // final node

            PatternSegment segment;
            segment.m_flags = FunctionFlags::None;
            segment.m_pattern = {&pattern, 1};
            ArgumentInfo arg_info{single_range, single_remaining};
            segment.m_arg_infos = {&arg_info, 1};
            AddCandidate(i_context, g_start_node_index, g_end_node_index, { &target, 1 }, segment, {}, {}, {});

            if (i_context.m_artifact_path != nullptr)
            {
                GraphWizGraph graph = MakeSolutionGraphWiz(i_context, "Initial");
                graph.SaveAsImage(std::filesystem::path(i_context.m_artifact_path) / "Initial.png");
            }

            int dbg_step = 0;
            do {
                Pool<Candidate>::Handle candidate_handle = std::move(i_context.m_candidate_queue.back());
                i_context.m_candidate_queue.pop_back();
                if(i_context.m_candidates.IsValid(candidate_handle))
                {
                    Candidate candidate = i_context.m_candidates.GetObject(candidate_handle);
                    i_context.m_candidates.Delete(candidate_handle);

                    dbg_step++;
                    
                    const bool match = MatchCandidate(i_context, candidate);

                    if(!match)
                    {
                        RemoveEdge(i_context, 
                            candidate.m_start_node, candidate.m_dest_node,
                            candidate_handle);
                    }
                    else
                    {
                        // find the edge and sets the substitutions
                        bool found = false;
                        const auto range = i_context.m_edges.equal_range(candidate.m_start_node);
                        for(auto it = range.first; it != range.second; it++)
                        {
                            // the candidate has just been removed from the stack
                            // DJUP_ASSERT(IsCandidateRefValid(i_context, it->second.m_candidate_ref));

                            if(it->second.m_dest_index == candidate.m_dest_node &&
                                it->second.m_candidate_ref == candidate_handle)
                            {
                                it->second.m_candidate_ref = {};

                                DJUP_ASSERT(it->second.m_substitutions.empty());
                                it->second.m_substitutions = std::move(candidate.m_substitutions);
                                found = true;
                                break;
                            }
                        }
                        DJUP_ASSERT(found);
                    }

                    if (i_context.m_artifact_path != nullptr)
                    {
                        std::string title = ToString("Step_", dbg_step);
                        GraphWizGraph graph = MakeSolutionGraphWiz(i_context, title);
                        graph.SaveAsImage(std::filesystem::path(i_context.m_artifact_path) / (title + ".png"));
                    }
                }

            } while(!i_context.m_candidate_queue.empty());
        }

        struct SolutionBuilder
        {
            uint32_t m_curr_node;
            SubstitutionsBuilder m_builder;
        };

        std::vector<MatchResult> GetAllSolutions(const MatchingContext & i_context)
        {
            std::vector<SolutionBuilder> builders;
            std::vector<MatchResult> solutions;

            builders.emplace_back().m_curr_node = g_start_node_index;

            do {
                for (auto bld_it = builders.begin(); bld_it != builders.end(); )
                {
                    SolutionBuilder copy;
                    uint32_t outgoing_edges = 0;

                    // for all edges starting from bld_it->m_curr_node
                    const auto equal_range = i_context.m_edges.equal_range(bld_it->m_curr_node);
                    for (auto edge_it = equal_range.first; edge_it != equal_range.second;
                        ++edge_it, ++outgoing_edges)
                    {
                        /* if this is the first edge, save a copy of it and remove it. For each edge
                           we use it as starting point and add the encountered substitutions */

                        if (outgoing_edges == 0)
                        {
                            copy = *bld_it;
                            bld_it = builders.erase(bld_it);
                        }

                        bld_it = builders.insert(bld_it, copy);

                        if (edge_it->second.m_open)
                            bld_it->m_builder.Open(edge_it->second.m_open);
                        
                        bool compatible = bld_it->m_builder.Add(edge_it->second.m_substitutions);
                        bld_it->m_curr_node = edge_it->second.m_dest_index;

                        if (edge_it->second.m_close)
                            compatible = compatible && bld_it->m_builder.Close(edge_it->second.m_close);
                        bld_it->m_curr_node = edge_it->second.m_dest_index;
                        
                        if (!compatible)
                        {
                            bld_it = builders.erase(bld_it);
                        }
                        else if (bld_it->m_curr_node == g_end_node_index)
                        {
                            // complete non-contradictory solution, save it
                            solutions.emplace_back().m_substitutions = 
                                bld_it->m_builder.StealSubstitutions();

                            bld_it = builders.erase(bld_it);
                        }
                        else
                        {
                            ++bld_it;
                        }
                    }

                    if (outgoing_edges == 0)
                        bld_it = builders.erase(bld_it);
                }

            } while (!builders.empty());
            return solutions;
        }

        Pattern::Pattern(const Namespace & i_namespace,
            const Tensor & i_pattern)
            : m_namespace(i_namespace)
        {
            m_pattern = PreprocessPattern(i_namespace, i_pattern);
        }

        Pattern::Pattern(const Namespace & i_namespace,
            const Tensor & i_pattern, const Tensor & i_when)
            : m_namespace(i_namespace)
        {
            m_pattern = PreprocessPattern(i_namespace, i_pattern);
            m_when = PreprocessPattern(i_namespace, i_when);
        }

        std::vector<MatchResult> Pattern::MatchAll(const Tensor & i_target,
            const char * i_artifact_path) const
        {
            MatchingContext context;
            context.m_namespace = &m_namespace;
            context.m_artifact_path = i_artifact_path;
            MakeSubstitutionsGraph(context, i_target, m_pattern);

            std::vector<MatchResult> solutions = GetAllSolutions(context);

            if (!IsEmpty(m_when))
            {
                for (auto it = solutions.begin(); it != solutions.end(); )
                {
                    const std::vector<Substitution> substitutions = it->m_substitutions;
                    const Tensor when_result = ApplySubstitutions(m_namespace, m_when, substitutions);
                    if (Always(when_result))
                    {
                        ++it;
                    }
                    else
                    {
                        it = solutions.erase(it);
                    }
                }
            }

            return { solutions };
        }

        std::optional<MatchResult> Pattern::MatchOne(const Tensor & i_target,
            const char * i_artifact_path) const
        {
            std::vector<MatchResult> solutions = MatchAll(i_target, i_artifact_path);
            if (solutions.empty())
                return {};
            else
                return solutions[0];
        }

    } // namespace o2o_pattern

} // namespace djup


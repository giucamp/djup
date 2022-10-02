
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/substitution_graph.h>
#include <private/substitute_by_predicate.h>
#include <private/builtin_names.h>
#include <core/flags.h>
#include <core/to_string.h>

namespace djup
{
    namespace pattern
    {
        namespace
        {
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

            Tensor PreprocessPattern(const Tensor & i_pattern)
            {
                return SubstituteByPredicate(i_pattern, [](const Tensor & i_candidate){
                    FunctionFlags flags = GetFunctionFlags(i_candidate.GetExpression()->GetName());

                    bool some_substitution = false;
                    std::vector<Tensor> new_arguments;

                    const std::vector<Tensor> & arguments = i_candidate.GetExpression()->GetArguments();
                    const size_t argument_count = arguments.size();

                    // substitute identifiers in associative functions with AssociativeIdentifier()
                    if(HasFlag(flags, FunctionFlags::Associative))
                    {
                        size_t index = 0;

                        for(; index < argument_count; index++)
                        {
                            const Tensor & argument = arguments[index];
                            if(IsIdentifier(argument))
                            {
                                new_arguments = arguments;
                                some_substitution = true;
                                break;
                            }
                        }

                        for(; index < argument_count; index++)
                        {
                            const Tensor & argument = arguments[index];
                            if(IsIdentifier(argument))
                            {
                                new_arguments[index] = MakeExpression(builtin_names::AssociativeIdentifier, 
                                    {argument}, 
                                    argument.GetExpression()->GetMetadata());
                            }
                        }
                    }

                    if(some_substitution)
                        return MakeExpression(i_candidate.GetExpression()->GetName(), new_arguments, i_candidate.GetExpression()->GetMetadata());
                    else
                        return i_candidate;
                });
            }
        }

        struct SubstitutionGraph::Node
        {
            std::string m_debug_name;
            size_t m_outgoing_edges{};
        };

        struct SubstitutionGraph::Substitution
        {
            Name m_variable_name;
            Tensor m_value;
        };

        struct SubstitutionGraph::Candidate
        {
            uint32_t m_start_node{};
            uint32_t m_repetitions{};
            uint32_t m_version{};
            bool m_decayed = false;
            uint32_t m_open{};
            uint32_t m_close{};
            std::vector<Substitution> m_substitutions;
        };

        struct SubstitutionGraph::CandidateRef
        {
            uint32_t m_index = std::numeric_limits<uint32_t>::max();
            uint32_t m_version{};
        };

        struct SubstitutionGraph::Edge
        {
            uint32_t m_source_index{};
            CandidateRef m_candidate_ref;
            std::vector<Substitution> m_substitutions;
            uint32_t m_open{};
            uint32_t m_close{};
        };

    } // namespace pattern

} // namespace djup

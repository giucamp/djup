
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/o2o_pattern/o2o_debug_utils.h>
#include <private/o2o_pattern/o2o_pattern_info.h>

namespace djup
{
    namespace o2o_pattern
    {
        std::vector<Span<const Tensor>> Tokenize(const Tensor& i_tensor)
        {
            std::vector<Span<const Tensor>> result;
            result.push_back({ &i_tensor, 1 });

            for (size_t i = 0; i < result.size(); i++)
            {
                Span<const Tensor> arguments = result[i];

                for (size_t j = 0; j < arguments.size(); j++)
                {
                    if (!IsLiteral(arguments[j]) && !IsRepetition(arguments[j]))
                    {
                        result.push_back(Span(arguments[j].GetExpression()->GetArguments()));
                    }
                }
            }

            return result;
        }

        std::string TensorSpanToString(Span<const Tensor> i_tensor, FormatFlags i_format_flags, size_t i_depth)
        {
            std::string result;
            for (size_t i = 0; i < i_tensor.size(); i++)
            {
                if (i != 0)
                    result += ", ";
                result += ToSimplifiedString(i_tensor[i], i_format_flags, i_depth);
            }
            return result;
        }
    
    } // namespace o2o_pattern

} // namespace djup

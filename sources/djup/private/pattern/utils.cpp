
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/utils.h>
#include <private/pattern/pattern_info.h>

namespace djup
{
    namespace pattern
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
                    if (!IsLiteral(arguments[j]) && !IsIdentifier(arguments[j]) && !IsRepetition(arguments[j]))
                    {
                        result.push_back(Span(arguments[j].GetExpression()->GetArguments()));
                    }
                }
            }

            return result;
        }

        std::string TensorSpanToString(Span<const Tensor> i_tensor, size_t i_depth)
        {
            std::string result;
            for (size_t i = 0; i < i_tensor.size(); i++)
            {
                if (i != 0)
                    result += ", ";
                result += ToSimplifiedStringForm(i_tensor[i], i_depth);
            }
            return result;
        }

    } // namespace pattern

} // namespace djup

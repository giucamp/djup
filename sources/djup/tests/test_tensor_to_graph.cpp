
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <djup/tensor.h>
#include <tests/test_utils.h>
#include <djup/expression.h>
#include <fstream>
#include <filesystem>

namespace djup
{
    namespace tests
    {
        void TensorToGraph()
        {
            Print("Test: djup - TensorToGraph...");
            
            std::string dot = TensorToGraph("3 + 2 * 5"_t).ToDotLanguage();
            // SaveGraph(test_dir + "\\expr.txt", "expr", dot);
            std::string expected = R"tensor_to_graph(digraph
{
	label = "Add(3, Mul(2, 5))"
	dpi = 384
	v0[shape = ellipse label = "Add" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v1[shape = ellipse label = "3" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v2[shape = ellipse label = "Mul" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v3[shape = ellipse label = "2" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v4[shape = ellipse label = "5" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v0 -> v1[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v2 -> v3[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v2 -> v4[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v0 -> v2[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
}
)tensor_to_graph";
            CORE_EXPECTS_EQ(dot, expected);

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup

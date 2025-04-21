
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <djup/tensor.h>
#include <core/diagnostic.h>
#include <private/diagnostic.h>
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
            std::string expected = R"(digraph unnamed_graph
{
	v0[shape = ellipse label = "Add" color = "#000000" fontcolor = "#000000" style = "filled" fillcolor = "#FFFFFF"]
	v1[shape = ellipse label = "Mul" color = "#000000" fontcolor = "#000000" style = "filled" fillcolor = "#FFFFFF"]
	v2[shape = ellipse label = "2" color = "#000000" fontcolor = "#000000" style = "filled" fillcolor = "#FFFFFF"]
	v3[shape = ellipse label = "5" color = "#000000" fontcolor = "#000000" style = "filled" fillcolor = "#FFFFFF"]
	v4[shape = ellipse label = "3" color = "#000000" fontcolor = "#000000" style = "filled" fillcolor = "#FFFFFF"]
	v1 -> v2[label = "" style = "solid"]
	v1 -> v3[label = "" style = "solid"]
	v0 -> v1[label = "" style = "solid"]
	v0 -> v4[label = "" style = "solid"]
}
)";
            CORE_EXPECTS_EQ(dot, expected);

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup

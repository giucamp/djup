
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/graph_wiz.h>
#include <core/diagnostic.h>

namespace core
{
    std::string RgbaToHex(uint8_t i_red, uint8_t i_green, uint8_t i_blue, uint8_t i_alpha);
        // implemented in graph_wiz.cpp

    namespace tests
    {
        void GraphWiz()
        {
            Print("Test: core - GraphWiz...");

            CORE_EXPECTS_EQ(RgbaToHex(0, 0, 0, 0),          "#00000000");
            CORE_EXPECTS_EQ(RgbaToHex(255, 255, 255, 255),  "#FFFFFFFF");
            CORE_EXPECTS_EQ(RgbaToHex(89, 115, 158, 255),   "#59739EFF");
            CORE_EXPECTS_EQ(RgbaToHex(75, 77, 63, 255), "#4B4D3FFF");

            GraphWizGraph graph;
            
            graph.AddNode("abc");
            graph.AddNode("def");

            graph.SetFontColor(255, 255, 0);
            graph.AddNode("g\"h\"i");

            graph.SetNodeShape(GraphWizGraph::NodeShape::Box);
            graph.SetDrawingColor(0, 0, 255);
            graph.AddNode("\"lmno\"");

            graph.SetDrawingColor(0, 255, 0);
            graph.AddNode("1\n2\n3");

            graph.AddEdge(0, 1);
            graph.AddEdge(1, 2);
            graph.AddEdge(3, 0);
            graph.AddEdge(4, 0);

            auto EscapeTestChain = [&] (const char * label, bool first = false) {
                graph.AddNode(label);
                if (!first)
                    graph.AddEdge(graph.GetNodeCount() - 1, graph.GetNodeCount() - 2);
            };

            EscapeTestChain("single quote: \'", true);
            EscapeTestChain("double quote: \"");
            EscapeTestChain("question mark: \?");
            EscapeTestChain("backslash: \\");
            EscapeTestChain("audible bell: \a");
            EscapeTestChain("backspace: \b");
            EscapeTestChain("form feed - new page: \f");
            EscapeTestChain("line feed - new line: \n");
            EscapeTestChain("carriage return: \r");            
            EscapeTestChain("horizontal tab: \t");
            EscapeTestChain("vertical tab: \v");

            const std::string dot = graph.ToDotLanguage();
            const std::string expected = R"(digraph unnamed_graph
{
	v0[shape = ellipse label = "abc" color = "#000000" fontcolor = "#000000" style = "filled" fillcolor = "#FFFFFF"]
	v1[shape = ellipse label = "def" color = "#000000" fontcolor = "#000000" style = "filled" fillcolor = "#FFFFFF"]
	v2[shape = ellipse label = "g\"h\"i" color = "#000000" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v3[shape = box label = "\"lmno\"" color = "#0000FFFF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v4[shape = box label = "1\n2\n3" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v5[shape = box label = "single quote: \'" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v6[shape = box label = "double quote: \"" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v7[shape = box label = "question mark: \?" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v8[shape = box label = "backslash: \\" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v9[shape = box label = "audible bell: \" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v10[shape = box label = "backspace: \" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v11[shape = box label = "form feed - new page: \" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v12[shape = box label = "line feed - new line: \n" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v13[shape = box label = "carriage return: \r" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v14[shape = box label = "horizontal tab: \t" color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v15[shape = box label = "vertical tab: " color = "#00FF00FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFF"]
	v0 -> v1[label = "" style = "solid"]
	v1 -> v2[label = "" style = "solid"]
	v3 -> v0[label = "" style = "solid"]
	v4 -> v0[label = "" style = "solid"]
	v6 -> v5[label = "" style = "solid"]
	v7 -> v6[label = "" style = "solid"]
	v8 -> v7[label = "" style = "solid"]
	v9 -> v8[label = "" style = "solid"]
	v10 -> v9[label = "" style = "solid"]
	v11 -> v10[label = "" style = "solid"]
	v12 -> v11[label = "" style = "solid"]
	v13 -> v12[label = "" style = "solid"]
	v14 -> v13[label = "" style = "solid"]
	v15 -> v14[label = "" style = "solid"]
}
)";
            CORE_EXPECTS_EQ(dot, expected);
            //graph.SaveAsImage("C:\\repos\\djup\\tests\\graph");

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup

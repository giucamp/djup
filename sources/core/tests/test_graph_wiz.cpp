
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/graph_wiz.h>
#include <core/diagnostic.h>

namespace core
{
    std::string RgbaToHex(GraphWizGraph::Color i_color);
        // implemented in graph_wiz.cpp

    namespace tests
    {
        void GraphWiz()
        {
            Print("Test: core - GraphWiz...");

            CORE_EXPECTS_EQ(RgbaToHex({ 0, 0, 0, 0}), "#00000000");
            CORE_EXPECTS_EQ(RgbaToHex({ 255, 255, 255, 255 }), "#FFFFFFFF");
            CORE_EXPECTS_EQ(RgbaToHex({ 89, 115, 158, 255 }), "#59739EFF");
            CORE_EXPECTS_EQ(RgbaToHex({ 75, 77, 63, 255 }), "#4B4D3FFF");

            GraphWizGraph graph;
            
            graph.AddNode("abc");
            graph.AddNode("def");

            graph.AddNode("g\"h\"i")
                .SetFontColor({ 255, 255, 0 });

            graph.AddNode("\"lmno\"")
                .SetShape(GraphWizGraph::NodeShape::Box)
                .SetDrawingColor({ 0, 0, 255 });

            graph.AddNode("1\n2\n3")
                .SetDrawingColor({ 0, 255, 0 });

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
	v0[shape = ellipse label = "abc" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v1[shape = ellipse label = "def" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v2[shape = ellipse label = "g\"h\"i" color = "#000000FF" fontcolor = "#FFFF00FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v3[shape = box label = "\"lmno\"" color = "#0000FFFF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v4[shape = ellipse label = "1\n2\n3" color = "#00FF00FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v5[shape = ellipse label = "single quote: \'" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v6[shape = ellipse label = "double quote: \"" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v7[shape = ellipse label = "question mark: \?" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v8[shape = ellipse label = "backslash: \\" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v9[shape = ellipse label = "audible bell: \" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v10[shape = ellipse label = "backspace: \" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v11[shape = ellipse label = "form feed - new page: \" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v12[shape = ellipse label = "line feed - new line: \n" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v13[shape = ellipse label = "carriage return: \r" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v14[shape = ellipse label = "horizontal tab: \t" color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v15[shape = ellipse label = "vertical tab: " color = "#000000FF" fontcolor = "#000000FF" style = "filled" fillcolor = "#FFFFFFFF"]
	v0 -> v1[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v1 -> v2[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v3 -> v0[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v4 -> v0[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v6 -> v5[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v7 -> v6[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v8 -> v7[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v9 -> v8[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v10 -> v9[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v11 -> v10[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v12 -> v11[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v13 -> v12[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v14 -> v13[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
	v15 -> v14[label = "" color = "#000000FF" fontcolor = "#000000FF" style = "solid"]
}
)";
            CORE_EXPECTS_EQ(dot, expected);
            //graph.SaveAsImage("C:\\repos\\djup\\tests\\graph");

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup

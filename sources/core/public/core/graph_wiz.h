
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <string_view>

namespace core
{
    /** Constructs a GraphWiz graph (https://graphviz.org/) in immediate
        mode, i.e. a state of attributes is maintained and can be changed,
        and nodes and edges can be added inheriting that attributes.
        Nodes and edges can be added in immediate mode, that is setting
        attributes such colors an shapes prior adding them. The path of the
        dot executable must be provided in order to the class to work 
        (see SetDotExe).
        Inside labels escape sequences are processed so that double-escaping
        is not necessary.
        Currently only a small subset of features of GraphWiz is supported. */
    class GraphWizGraph
    {
    public:

        /** Sets the path of the dot executable, necessary to build the
            graph. On Win32 platforms the default is:
            "C:\Program Files\Graphviz\bin\dot.exe".
            Multiple threads calling this function without synchronization 
            cause a race condition. */
        void SetDotExe(const std::filesystem::path& i_dot_exe);

        GraphWizGraph(std::string_view i_graph_name = "unnamed_graph");

        ~GraphWizGraph();

        enum NodeShape
        {
            Ellipse,
            Box
        };

        void SetNodeShape(NodeShape i_shape);

        void SetDrawingColor(std::string_view i_color);

        void SetFontColor(std::string_view i_color);

        void SetFillColor(std::string_view i_color);

        /** Adds a node using the currently set attributes and
            the provided label. Returns the index of the new node. */
        uint32_t AddNode(std::string_view i_label);

        /** Returns the number of nodes added so far */
        uint32_t GetNodeCount() const;

        /** Returns the number of edges added so far */
        uint32_t GetEdgeCount() const;

        /** Adds an edge given a source and a destination node index */
        void AddEdge(uint32_t i_from, uint32_t i_to);

        /** Renders the graph to dot language */
        std::string ToDotLanguage() const;

        /** Save the graph as a .png image */
        void SaveAsImage(std::filesystem::path i_path) const;

    private: // internal structures

        struct Node;
        struct Edge;

        struct Attributes
        {
            NodeShape m_node_shape = NodeShape::Ellipse;
            std::string m_drawing_color = "#000000"; //black
            std::string m_font_color = "#000000"; //black
            std::string m_fill_color = "#FFFFFF"; // white
        };

    private: // data members
        std::string m_graph_name;
        Attributes m_current_attributes;
        std::vector<Node> m_nodes;
        std::vector<Edge> m_edges;

        static std::filesystem::path s_dot_exe;
    };

} // namespace core

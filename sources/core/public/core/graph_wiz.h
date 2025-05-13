
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
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
    /** Constructs a GraphWiz graph (https://graphviz.org/) 
        GraphWiz must be installed in the system. By default the dot executable
        is C:/Program Files/Graphviz/bin/dot.exe on Windows and /usr/bin/dot on
        Linux. It can be changed with the static function SetDotExe.
        Inside labels escape sequences are processed so that double-escaping
        is not necessary.
        Currently only a small subset of features of GraphWiz is supported. */
    class GraphWizGraph
    {
    public:

        GraphWizGraph(std::string_view i_graph_name = "unnamed_graph");

        GraphWizGraph(const GraphWizGraph& i_source);

        GraphWizGraph(GraphWizGraph && i_source);

        ~GraphWizGraph();

        /** Renders the graph to dot language */
        std::string ToDotLanguage() const;

        /** Save the graph as an image */
        void SaveAsImage(std::filesystem::path i_path) const;

        /** Sets the path of the dot executable, necessary to build the
            graph. The default is C:/Program Files/Graphviz/bin/dot.exe
            on Windows and /usr/bin/dot on Linux.
            Multiple threads calling this function without synchronization 
            cause a race condition. */
        static void SetDotExe(const std::filesystem::path& i_dot_exe);


                /** graph */

        /** Sets the output image format. The default is png.
            Supported format at https://graphviz.org/docs/outputs/ */
        GraphWizGraph & SetOutputFormat(std::string i_ouput_format)
            { m_output_format = std::move(i_ouput_format); return *this; }

        /** Sets the layout engine. The default is dot.
            Supported engine at https://graphviz.org/docs/layouts/ */
        GraphWizGraph & SetLayoutEngine(std::string i_layout_engine)
            { m_layout_engine = std::move(i_layout_engine); return *this; }

        /* Sets the resolution of the image. The default is 384. */
        GraphWizGraph & SetDPI(uint32_t i_dpi)
            { m_dpi = i_dpi; return *this; }

                /* nodes */

        enum class NodeShape
        {
            Ellipse,
            Box
        };

        struct Color
        {
            constexpr Color(uint8_t i_red, uint8_t i_green, uint8_t i_blue, uint8_t i_alpha = 255)
                : m_red(i_red), m_green(i_green), m_blue(i_blue), m_alpha(i_alpha) { }

            uint8_t m_red, m_green, m_blue, m_alpha;
        };

        class Node
        {
        public:

            Node & SetLabel(std::string i_label) { m_label = std::move(i_label); return *this; }

            Node & SetShape(NodeShape i_shape) { m_shape = i_shape; return *this; }

            Node & SetDrawingColor(Color i_color) { m_drawing_color = i_color; return *this; }

            Node & SetFontColor(Color i_color) { m_font_color = i_color; return *this; }

            Node & SetFillColor(Color i_color) { m_fill_color = i_color; return *this; }

        private:
            std::string m_label;
            NodeShape m_shape = NodeShape::Ellipse;
            Color m_drawing_color = {0, 0, 0};
            Color m_font_color = {0, 0, 0};
            Color m_fill_color = {255, 255, 255}; // white
            friend class GraphWizGraph;
        };

        /** Adds a node using the currently set attributes and
            the provided label. Returns the index of the new node. */
        Node & AddNode(std::string i_label);

        /** Returns a reference to the node given its index. */
        Node & GetNodeAt(size_t i_index);

        /** Returns a reference to the node given its index. */
        const Node & GetNodeAt(size_t i_index) const;

        /** Returns the number of nodes added so far */
        size_t GetNodeCount() const;


                        /* edges */

        enum class EdgeStyle
        {
            Solid,
            Dashed,
            Dotted,
            Bold,
            SyleCount
        };

        struct Edge
        {
        public:

            Edge & SetLabel(std::string i_label) { m_label = std::move(i_label); return *this; }

            Edge & SetHeadLabel(std::string i_head_label) { m_head_label = std::move(i_head_label); return *this; }

            Edge & SetTailLabel(std::string i_tail_label) { m_tail_label = std::move(i_tail_label); return *this; }

            Edge & SetStyle(EdgeStyle i_style) { m_style = i_style; return *this; }

            Edge & SetDrawingColor(Color i_color) { m_drawing_color = i_color; return *this; }

            Edge & SetFontColor(Color i_color) { m_font_color = i_color; return *this; }

        private:
            size_t m_from;
            size_t m_to;
            std::string m_label, m_head_label, m_tail_label;
            EdgeStyle m_style = EdgeStyle::Solid;
            Color m_drawing_color = {0, 0, 0};
            Color m_font_color = {0, 0, 0};
            friend class GraphWizGraph;
        };

        /** Adds an edge given a source and a destination node index */
        Edge & AddEdge(size_t i_from, size_t i_to, std::string i_label = {});

        /** Returns a reference to an node given its index. */
        Edge & GetEdgeAt(size_t i_index);

        /** Returns a reference to an edge given its index. */
        const Edge & GetEdgeAt(size_t i_index) const;

        /** Returns the number of edges added so far */
        size_t GetEdgeCount() const;

    private: // data members
        std::string m_graph_name;
        std::string m_output_format = "png";
        std::string m_layout_engine = "dot";
        uint32_t m_dpi = 384;
        std::vector<Node> m_nodes;
        std::vector<Edge> m_edges;
        static std::filesystem::path s_dot_exe;
    };

} // namespace core

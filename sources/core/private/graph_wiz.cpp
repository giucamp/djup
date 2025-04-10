
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/graph_wiz.h>
#include <core/diagnostic.h>
#include <core/to_string.h>
#include <core/numeric_cast.h>
#include <utility>
#include <fstream>
#include <filesystem>

namespace core
{
    namespace
    {
        void ReplaceAllInPlace(std::string & i_where, const std::string & i_what, const std::string& i_whith)
        {
            size_t curr_pos = 0;
            for (;;)                
            {
                curr_pos = i_where.find(i_what, curr_pos);
                if (curr_pos == std::string::npos)
                    break;
                i_where = i_where.replace(curr_pos, i_what.size(), i_whith);
                curr_pos += i_whith.length();
            }
        }

        std::string GetEscaped(const std::string & i_text)
        {
            std::string result = i_text;
            ReplaceAllInPlace(result, "\\", "\\\\");
            ReplaceAllInPlace(result, "\'", "\\\'");
            ReplaceAllInPlace(result, "\"", "\\\"");
            ReplaceAllInPlace(result, "\t", "\\t");
            
            ReplaceAllInPlace(result, "\?", "\\\?");
            
            ReplaceAllInPlace(result, "\a", "\\\a");
            ReplaceAllInPlace(result, "\b", "\\\b");
            ReplaceAllInPlace(result, "\f", "\\\f");
            ReplaceAllInPlace(result, "\n", "\\n");
            ReplaceAllInPlace(result, "\r", "\\r"); 
            return result;
        }
    }

    struct GraphWizGraph::Node
    {
        std::string m_label;
        Attributes m_attributes;
    };

    struct GraphWizGraph::Edge
    {
        uint32_t m_from;
        uint32_t m_to;
    };

    std::filesystem::path GraphWizGraph::s_dot_exe = "C:\\Program Files\\Graphviz\\bin\\dot.exe";

    void GraphWizGraph::SetDotExe(const std::filesystem::path& i_dot_exe)
    {
        s_dot_exe = i_dot_exe;
    }

    GraphWizGraph::GraphWizGraph(std::string_view i_graph_name)
        : m_graph_name(i_graph_name)
    {
        if (!std::filesystem::exists(s_dot_exe))
            Error("The path ", s_dot_exe.string(), " does not exist. "
                "Install GraphWiz on your system or call SetDotExe to" 
                "set the correct executable location");
    }

    /* the destructor of m_nodes and m_edges cannot appear prior the
       definition of the structs */
    GraphWizGraph::~GraphWizGraph() = default;

    void GraphWizGraph::SetNodeShape(GraphWizGraph::NodeShape i_shape)
    {
        m_current_attributes.m_node_shape = i_shape;
    }

    void GraphWizGraph::SetDrawingColor(std::string_view i_color)
    {
        m_current_attributes.m_drawing_color = i_color;
    }

    void GraphWizGraph::SetFontColor(std::string_view i_color)
    {
        m_current_attributes.m_font_color = i_color;
    }

    void GraphWizGraph::SetFillColor(std::string_view i_color)
    {
        m_current_attributes.m_fill_color = i_color;
    }

    uint32_t GraphWizGraph::AddNode(std::string_view i_label)
    {
        Node new_node;
        new_node.m_label = i_label;
        new_node.m_attributes = m_current_attributes;
        uint32_t index = NumericCast<uint32_t>(m_nodes.size());
        m_nodes.push_back(std::move(new_node));
        return index;
    }

    void GraphWizGraph::AddEdge(uint32_t i_from, uint32_t i_to)
    {
        m_edges.push_back({i_from, i_to});
    }

    /** Returns the number of nodes added so far */
    uint32_t GraphWizGraph::GetNodeCount() const
    { 
        return static_cast<uint32_t>(m_nodes.size());
    }

    /** Returns the number of edges added so far */
    uint32_t GraphWizGraph::GetEdgeCount() const
    {
        return static_cast<uint32_t>(m_edges.size());
    }

    std::string GraphWizGraph::ToDotLanguage() const
    {
        StringBuilder builder;

        builder << "digraph " << m_graph_name;
        builder.NewLine();
        builder << "{";
        builder.NewLine();
        builder.Tab();

        for (size_t node_index = 0; node_index < m_nodes.size(); ++node_index)
        {
            const Node& node = m_nodes[node_index];
            builder << "v" << node_index << "[";

            switch (node.m_attributes.m_node_shape)
            {
            case NodeShape::Ellipse:
                builder << "shape = ellipse";
                break;
            
            case NodeShape::Box:
                builder << "shape = box";
                break;
            
            default:
                Error("Unrecognized shape");
                break;
            }

            builder << " label = \"" << GetEscaped(node.m_label) << "\"";
            builder << " color = \"" << node.m_attributes.m_drawing_color << "\"";
            builder << " fontcolor = \"" << node.m_attributes.m_font_color << "\"";
            builder << "]";
            builder.NewLine();
        }

        for (const Edge & edge : m_edges)
        {
            builder << "v" << edge.m_from << " -> v" << edge.m_to;
            builder.NewLine();
        }


        builder.Untab();
        builder << "}";
        builder.NewLine();

        std::string dot = builder.StealString();
        return dot;
    }

    void GraphWizGraph::SaveAsImage(std::filesystem::path i_path) const
    {
        using std::filesystem::path;

        // content of the file
        const std::string dot = ToDotLanguage();
        PrintLn(dot);

        // path of the source file
        path dot_file_path = i_path;
        dot_file_path += ".txt";

        // write file content
        std::ofstream(dot_file_path) << dot;

        std::string cmd = ToString("\"\"", s_dot_exe.string(),
            "\" -T png -O ", dot_file_path.string(), "\"");
        int res = std::system(cmd.c_str());
        std::filesystem::remove(dot_file_path);
        if (res != 0)
            Error("The command ", cmd, " returned ", res);

        std::filesystem::remove(dot_file_path);
    }

} // namespace core

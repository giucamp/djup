
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
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

    std::string RgbaToHex(GraphWizGraph::Color i_color)
    {
        char digits[] = "0123456789ABCDEF";

        std::string chars;
        chars.resize(9);
        chars[0] = '#';
        chars[1] = digits[(i_color.m_red >> 4) & 15];
        chars[2] = digits[i_color.m_red & 15];
        chars[3] = digits[(i_color.m_green >> 4) & 15];
        chars[4] = digits[i_color.m_green & 15];
        chars[5] = digits[(i_color.m_blue >> 4) & 15];
        chars[6] = digits[i_color.m_blue & 15];
        chars[7] = digits[(i_color.m_alpha >> 4) & 15];
        chars[8] = digits[i_color.m_alpha & 15];
        chars[9] = 0;
        return chars;
    }

    #ifdef _WIN32
        std::filesystem::path GraphWizGraph::s_dot_exe = "C:\\Program Files\\Graphviz\\bin\\dot.exe";
    #elif __linux__
        std::filesystem::path GraphWizGraph::s_dot_exe = "/usr/bin/dot";
    #endif

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

    /* definition of copy, move and delete of m_nodes and m_edges
       cannot appear prior the definition of the structs */

    GraphWizGraph::GraphWizGraph(const GraphWizGraph& i_source) = default;

    GraphWizGraph::GraphWizGraph(GraphWizGraph&& i_source) = default;

    GraphWizGraph::~GraphWizGraph() = default;

    GraphWizGraph::Node & GraphWizGraph::AddNode(std::string i_label)
    {
        Node & new_node = m_nodes.emplace_back();
        new_node.m_label = std::move(i_label);
        return new_node;
    }

    GraphWizGraph::Edge & GraphWizGraph::AddEdge(size_t i_from, size_t i_to, std::string i_label)
    {
        Edge & new_edge = m_edges.emplace_back();
        new_edge.m_from = i_from;
        new_edge.m_to = i_to;
        new_edge.m_label = std::move(i_label);
        return new_edge;
    }

    GraphWizGraph::Node & GraphWizGraph::GetNodeAt(size_t i_index)
    {
        return m_nodes.at(i_index);
    }
    
    const GraphWizGraph::Node & GraphWizGraph::GetNodeAt(size_t i_index) const
    {
        return m_nodes.at(i_index);
    }

    size_t GraphWizGraph::GetNodeCount() const
    { 
        return m_nodes.size();
    }

    GraphWizGraph::Edge & GraphWizGraph::GetEdgeAt(size_t i_index)
    {
        return m_edges.at(i_index);
    }

    const GraphWizGraph::Edge & GraphWizGraph::GetEdgeAt(size_t i_index) const
    {
        return m_edges.at(i_index);
    }

    size_t GraphWizGraph::GetEdgeCount() const
    {
        return size_t(m_edges.size());
    }

    std::string GraphWizGraph::ToDotLanguage() const
    {
        StringBuilder builder;

        builder << "digraph";
        builder.NewLine();
        builder << "{";
        builder.NewLine();
        builder.Tab();

        builder << "label = \"" << GetEscaped(m_graph_name) << "\""; builder.NewLine();
        builder << "dpi = " << m_dpi; builder.NewLine();

        for (size_t node_index = 0; node_index < m_nodes.size(); ++node_index)
        {
            const Node & node = m_nodes[node_index];
            builder << "v" << node_index << "[";

            switch (node.m_shape)
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
            builder << " color = \"" << RgbaToHex(node.m_drawing_color) << "\"";
            builder << " fontcolor = \"" << RgbaToHex(node.m_font_color) << "\"";
            builder << " style = \"filled\" fillcolor = \"" << RgbaToHex(node.m_fill_color) << "\"";
            builder << "]";
            builder.NewLine();
        }

        for (const Edge & edge : m_edges)
        {
            const char* edge_styles[] = {"solid", "dashed", "dotted", "bold"};
            static_assert(std::size(edge_styles) == static_cast<size_t>(EdgeStyle::SyleCount) );

            builder << "v" << edge.m_from << " -> v" << edge.m_to;
            builder << "[";
            builder << "label = \"" << GetEscaped(edge.m_label) << "\"";
            if(!edge.m_head_label.empty())
                builder << " headlabel  = \"" << GetEscaped(edge.m_head_label) << "\"";
            if (!edge.m_tail_label.empty())
                builder << " taillabel = \"" << GetEscaped(edge.m_tail_label) << "\"";
            builder << " fontcolor = \"" << RgbaToHex(edge.m_font_color) << "\"";
            builder << " style = \"" << edge_styles[static_cast<int>(edge.m_style)] << "\"";
            builder << " color = \"" << RgbaToHex(edge.m_drawing_color) << "\"";
            builder << "]";
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

        // path of the source file
        path dot_file_path = i_path;
        dot_file_path += ".txt";

        // write file content
        std::ofstream dot_file(dot_file_path);
        if (dot_file.fail())
            Error("Could not write file ", dot_file_path.string());
        dot_file << dot;
        dot_file.close();

        std::string cmd = s_dot_exe.string();
        if (cmd.find(' ') != std::string::npos)
            cmd = "\"" + s_dot_exe.string() + "\"";
        
        cmd += " -o" + i_path.string();
        cmd += " -T" + m_output_format;
        cmd += " -K" + m_layout_engine;
        
        std::string dot_file_path_str = dot_file_path.string();
        cmd += " " + dot_file_path_str;
        #ifdef _WIN32
            cmd = "\"" + cmd + "\"";
        #endif  
        
        int res = std::system(cmd.c_str());

        std::filesystem::remove(dot_file_path);
        if (res != 0)
             Error("The command ", cmd, " returned ", res);

        std::filesystem::remove(dot_file_path);
    }

} // namespace core

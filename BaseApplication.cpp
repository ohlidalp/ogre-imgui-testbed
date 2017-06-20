
/*
NOTES
=====

NODE EDITORS FOUND ON THE INTERNET
    * https://github.com/ocornut/imgui/issues/306#issuecomment-134657997 -- Demo by Omar Cornut. Nodelinks not editable :(
    * https://github.com/ocornut/imgui/issues/306#issuecomment-151167133 -- Looks very good. Editable nodelinks. Has online emscripten demo.
    * https://www.youtube.com/watch?v=m6eteEPQ0Lg -- Fully editable, looks interesting.

*/

#include "stdafx.h" // Precompiled

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include "ImguiManager.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

static float G_truck_node_x[100] = {0}; // dummy 100-node truck

class NodeGraphTool
{
public:
    NodeGraphTool():
        m_scroll(0.0f, 0.0f),
        m_last_scaled_node(nullptr),
        m_link_mouse_src(nullptr),
        m_link_mouse_dst(nullptr),
        m_hovered_slot_node(nullptr),
        m_hovered_slot_input(-1),
        m_hovered_slot_output(-1),
        m_num_ticks(0)
    {
        m_fake_mouse_node.num_inputs = 1;
        m_fake_mouse_node.num_outputs = 1;
        m_fake_mouse_node.size = ImVec2(1,1);

        // test nodes
        m_reading_nodes.push_back(new ReadingNode());
        m_reading_nodes.push_back(new ReadingNode());
        m_reading_nodes.push_back(new ReadingNode());

        m_reading_nodes[1]->pos += ImVec2(300.f, -10.f);
        m_reading_nodes[2]->pos += ImVec2(250.f, 133.f);

        // Links
   //     m_links.emplace_back(m_nodes[0], m_nodes[1], 0, 0); // X output -> graph input
   //     m_links.emplace_back(m_nodes[0], m_nodes[2], 2, 0); // Z output -> graph input

    }
private:
    struct Vec3 { float x, y, z; };

    struct Style { // Copypaste from https://github.com/Flix01/imgui/tree/2015-10-Addons/addons/imguinodegrapheditor
        //ImVec4 color_background;
        ImU32 color_grid;
        float grid_line_width;
        float grid_size;
        ImU32 color_node;
        ImU32 color_node_frame;
        //ImU32 color_node_selected;
        ImU32 color_node_active;
        //ImU32 color_node_frame_selected;
        ImU32 color_node_frame_active;
        ImU32 color_node_hovered;
        ImU32 color_node_frame_hovered;
        float node_rounding;
        ImVec2 node_window_padding;
        ImU32 color_input_slot;
        ImU32 color_input_slot_hover;
        //ImU32 color_node_input_slots_border;
        ImU32 color_output_slot;
        ImU32 color_output_slot_hover;
        //ImU32 color_node_output_slots_border;
        float node_slots_radius;
        //int node_slots_num_segments;
        ImU32 color_link;
        float link_line_width;
        ImVec2 slot_hoverbox_extent;
        //float link_control_point_distance;
        //int link_num_segments;  // in AddBezierCurve(...)
        //ImVec4 color_node_title;
        //ImU32 color_node_title_background;
        //float color_node_title_background_gradient;
        //ImVec4 color_node_input_slots_names;
        //ImVec4 color_node_output_slots_names;        
        //ImU32 color_mouse_rectangular_selection;
        //ImU32 color_mouse_rectangular_selection_frame;
        Style() {
            //color_background =          ImColor(60,60,70,200);
            color_grid =                ImColor(200,200,200,40);
            grid_line_width =           1.f;
            grid_size =                 64.f;
            //
            color_node =                ImColor(30,30,35);
            color_node_frame =          ImColor(100,100,100);
            //color_node_selected =       ImColor(75,75,85);
            //color_node_active =         ImColor(45,45,49);
            //color_node_frame_selected = ImColor(115,115,115);
            color_node_frame_active =   ImColor(100,100,100);
            color_node_hovered =        ImColor(45,45,49);
            color_node_frame_hovered =  ImColor(125,125,125);
            node_rounding =             4.f;
            node_window_padding =       ImVec2(8.f,8.f);
            slot_hoverbox_extent        = ImVec2(15.f, 10.f);
            //
            color_input_slot =    ImColor(150,150,150,150);
            color_output_slot =   ImColor(150,150,150,150);
            color_input_slot_hover =    ImColor(144,155,222,245);
            color_output_slot_hover =   ImColor(144,155,222,245);
            node_slots_radius =         5.f;
            //
            color_link =                ImColor(200,200,100);
            link_line_width =           3.f;
            //link_control_point_distance = 50.f;
            //link_num_segments =         0;
            //
            //color_node_title = ImGui::GetStyle().Colors[ImGuiCol_Text];
            //color_node_title_background = 0;//ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive]);
            //color_node_title_background_gradient = 0.f;   // in [0,0.5f] used only if available (performance is better when 0)
            //color_node_input_slots_names = ImGui::GetStyle().Colors[ImGuiCol_Text];color_node_input_slots_names.w=0.75f;
            //color_node_output_slots_names = ImGui::GetStyle().Colors[ImGuiCol_Text];color_node_output_slots_names.w=0.75f;
            //
            //color_mouse_rectangular_selection =         ImColor(255,0,0,45);
            //color_mouse_rectangular_selection_frame =   ImColor(45,0,0,175);
            //
            //color_node_input_slots_border = color_node_output_slots_border = ImColor(60,60,60,0);
            //node_slots_num_segments = 12;
        }
    }; // struct Style

    struct Node; // Forward
    struct ReadingNode;
    struct DisplayNode;

    struct Link
    {
        Link(): node_src(nullptr), node_dst(nullptr), slot_src(0), slot_dst(0) {}

        Link(Node* src_n, Node* dst_n, size_t src_s, size_t dst_s): node_src(src_n), node_dst(dst_n), slot_src(src_s), slot_dst(dst_s) {}

        Node* node_src;
        Node* node_dst;
        size_t slot_src;
        size_t slot_dst;
    };

    struct Node
    {
        enum class Type { INVALID, READING, DISPLAY };

        Node(): num_inputs(0), num_outputs(-1), type(Type::INVALID), pos(100.f, 100.f), size(150.f, 100.f)
        {
            static int new_id = 1;
            id = new_id;
            ++new_id;
        }

        ReadingNode* ToReading() { assert(type == Type::READING); if (type == Type::READING) { return static_cast<ReadingNode*>(this); } else { return nullptr; } }
        DisplayNode* ToDisplay() { assert(type == Type::DISPLAY); if (type == Type::DISPLAY) { return static_cast<DisplayNode*>(this); } else { return nullptr; } }

        size_t num_inputs;
        size_t num_outputs;
        Type type;
        ImVec2 pos;
        ImVec2 size;
        int id;

        inline ImVec2 GetInputSlotPos(size_t slot_idx)  { return ImVec2(pos.x,          pos.y + (size.y * (static_cast<float>(slot_idx+1) / static_cast<float>(num_inputs+1)))); }
        inline ImVec2 GetOutputSlotPos(size_t slot_idx) { return ImVec2(pos.x + size.x, pos.y + (size.y * (static_cast<float>(slot_idx+1) / static_cast<float>(num_outputs+1)))); }
    };

    /// reports XYZ position of node in world space
    /// Inputs: none
    /// Outputs(3): X position, Y position, Z position
    struct ReadingNode: public Node
    {
        ReadingNode()
        {
            num_inputs = 0;
            num_outputs = 3;
            type = Type::READING;
            softbody_node_id = -1;
            data_offset = 0;
            memset(data_buffer, 0, sizeof(data_buffer));
            size = ImVec2(250.f, 85.f);
        }

        int softbody_node_id; // -1 means 'none'
        Vec3 data_buffer[2000]; // 1 second worth of data
        int data_offset;

        inline void PushData(Vec3 entry) { data_buffer[data_offset] = entry; data_offset = (data_offset+1)%2000; }
    };

    /// Displays a graph
    /// Inputs(1): value
    /// Outputs(1): value (pass-through)
    struct DisplayNode: public Node
    {
        DisplayNode() { num_inputs = 1; num_outputs = 1; type = Type::DISPLAY; }
    };

public:
    void Draw()
    {
        // Create a window
        if (!ImGui::Begin("RigsOfRods NodeGraph"))
            return; // No window -> nothing to do.

        ImGui::Text("MouseDrag - src: 0x%p, dst: 0x%p | mousenode - X:%.1f, Y:%.1f", m_link_mouse_src, m_link_mouse_dst, m_fake_mouse_node.pos.x, m_fake_mouse_node.pos.y);
        ImGui::Text("SlotHover - node: 0x%p, input: %d, output: %d", m_hovered_slot_node, m_hovered_slot_input, m_hovered_slot_output);

        m_scroll_offset = ImGui::GetCursorScreenPos() - m_scroll;
        m_is_any_slot_hovered = false;

        this->DrawNodeGraphPane();

        // Finalize the window
        ImGui::End();
    }

    void PhysicsTick()
    {
        for (ReadingNode* rn: m_reading_nodes)
        {
            if (rn->softbody_node_id >= 0)
            {
                Vec3 data;
                data.x = G_truck_node_x[rn->softbody_node_id];
                data.y = G_truck_node_x[rn->softbody_node_id];
                data.z = G_truck_node_x[rn->softbody_node_id];
                rn->PushData(data);
            }
        }
        ++m_num_ticks;
    }
private:

    inline bool IsInside(ImRect& rect, ImVec2& point) { return ((point.x > rect.Min.x) && (point.y > rect.Min.y)) && ((point.x < rect.Max.x) && (point.y < rect.Max.y)); }
    inline bool IsLinkDragInProgress() { return (m_link_mouse_src != nullptr) || (m_link_mouse_dst != nullptr); }

    void DrawLink(Link& link)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->ChannelsSetCurrent(0); // background + curves
        ImVec2 offset =m_scroll_offset;
        ImVec2 p1 = offset + link.node_src->GetOutputSlotPos(link.slot_src) + (m_style.node_window_padding * 2.f);
        ImVec2 p2 = offset + link.node_dst->GetInputSlotPos(link.slot_dst);
        ImRect window = ImGui::GetCurrentWindow()->Rect();

        if (this->IsInside(window, p1) || this->IsInside(window, p2)) // very basic clipping
        {
            float bezier_pt_dist = fmin(50.f, fmin(fabs(p1.x - p2.x)*0.75f, fabs(p1.y - p2.y)*0.75f)); // Maximum: 50; minimum: 75% of shorter-axis distance between p1 and p2
            draw_list->AddBezierCurve(p1, p1+ImVec2(+bezier_pt_dist,0), p2+ImVec2(-bezier_pt_dist,0), p2, m_style.color_link, m_style.link_line_width);
        }
    }

    void DrawGrid()
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->ChannelsSetCurrent(0); // background + curves
        const ImVec2 win_pos = ImGui::GetCursorScreenPos();
        const ImVec2 offset = ImGui::GetCursorScreenPos() - m_scroll;
        const ImVec2 canvasSize = ImGui::GetWindowSize();

        for (float x = fmodf(offset.x,m_style.grid_size); x < canvasSize.x; x += m_style.grid_size)
            draw_list->AddLine(ImVec2(x,0.0f)+win_pos, ImVec2(x,canvasSize.y)+win_pos, m_style.color_grid, m_style.grid_line_width);
        for (float y = fmodf(offset.y,m_style.grid_size); y < canvasSize.y; y += m_style.grid_size)
            draw_list->AddLine(ImVec2(0.0f,y)+win_pos, ImVec2(canvasSize.x,y)+win_pos, m_style.color_grid, m_style.grid_line_width);
    }

    Link* FindLinkByDestination(Node* node, size_t slot)
    {
        for (Link& link: m_links)
        {
            if (link.node_dst == node && link.slot_dst == slot)
                return &link;
        }
        return nullptr;
    }

    Link* FindLinkBySource(Node* node, size_t slot)
    {
        for (Link& link: m_links)
        {
            if (link.node_src == node && link.slot_src == slot)
                return &link;
        }
        return nullptr;
    }

    Link* AddLink(Node* src, Node* dst, size_t src_slot, size_t dst_slot) ///< creates new link or fetches existing unused one
    {
        for (Link& link: m_links)
        {
            if (link.node_dst == nullptr || link.node_src == nullptr)
            {
                link.node_src = src;
                link.node_dst = dst;
                link.slot_src = src_slot;
                link.slot_dst = dst_slot;
                return &link;
            }
        }
        m_links.emplace_back(src, dst, src_slot, dst_slot);
        return &m_links.back();
    }

    inline void DrawInputSlot(Node* node, const size_t index) { this->DrawSlotUni(node, index, true); }
    inline void DrawOutputSlot(Node* node, const size_t index) { this->DrawSlotUni(node, index, false); }

    inline bool IsSlotHovered(ImVec2 slot_center) const
    {
        ImVec2 slot_rect_min = slot_center - m_style.slot_hoverbox_extent;
        ImVec2 slot_rect_max = slot_center + m_style.slot_hoverbox_extent;
        return this->IsRectHovered(slot_rect_min, slot_rect_max);
    }

    inline bool IsRectHovered(ImVec2 min, ImVec2 max) const
    {
        return ((m_nodegraph_mouse_pos.x >= min.x) && (m_nodegraph_mouse_pos.y >= min.y) &&
                (m_nodegraph_mouse_pos.x <= max.x) && (m_nodegraph_mouse_pos.y <= max.y));
    }

    void DrawSlotUni(Node* node, const size_t index, const bool input)
    {
        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        drawlist->ChannelsSetCurrent(1);
        ImVec2 slot_center_pos =  ((input) ? node->GetInputSlotPos(index) : (node->GetOutputSlotPos(index) + (m_style.node_window_padding * 2.f)));
        ImGui::SetCursorScreenPos((slot_center_pos + m_scroll_offset) - m_style.slot_hoverbox_extent);
        ImU32 color = (input) ? m_style.color_input_slot : m_style.color_output_slot;
        if (this->IsSlotHovered(slot_center_pos))
        {
            m_is_any_slot_hovered = true;
            m_hovered_slot_node = node;
            if (input)
                m_hovered_slot_input = static_cast<int>(index);
            else
                m_hovered_slot_output = static_cast<int>(index);
            color = (input) ? m_style.color_input_slot_hover : m_style.color_output_slot_hover;
            if (ImGui::IsMouseDragging(0) && !this->IsLinkDragInProgress())
            {
                // Start link drag!
                Link* link = (input) ? this->FindLinkByDestination(node, index) : this->FindLinkBySource(node, index);
                if (link)
                {
                    // drag existing link
                    if (input)
                    {
                        link->node_dst = &m_fake_mouse_node;
                        m_link_mouse_dst = link;
                    }
                    else
                    {
                        link->node_src = &m_fake_mouse_node;
                        m_link_mouse_src = link;
                    }
                }
                else
                {
                    // Create a new link
                    if (input)
                    {
                        m_link_mouse_src = this->AddLink(&m_fake_mouse_node, node, 0, index);
                    }
                    else
                    {
                        m_link_mouse_dst = this->AddLink(node, &m_fake_mouse_node, index, 0);
                    }
                }
            }
        }
        drawlist->AddCircleFilled(slot_center_pos+m_scroll_offset, m_style.node_slots_radius, color);
    }

    void DrawNodeGraphPane()
    {
        const bool draw_border = false;
        const int flags = ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollWithMouse;
        if (!ImGui::BeginChild("scroll-region", ImVec2(0,0), draw_border, flags))
            return; // Nothing more to do.

        const float baseNodeWidth = 120.f; // same as reference, but hardcoded
        float currentNodeWidth = baseNodeWidth;
        ImGui::PushItemWidth(currentNodeWidth);
        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        drawlist->ChannelsSplit(3); // 0 = background (grid, curves); 1 = node rectangle/slots; 2 = node content

        // Update mouse drag
        const ImVec2 nodepane_screen_pos = ImGui::GetCursorScreenPos();
        m_nodegraph_mouse_pos = (ImGui::GetIO().MousePos - nodepane_screen_pos);
        if (ImGui::IsMouseDragging(0))
        {
            if (m_link_mouse_src != nullptr)
                m_fake_mouse_node.pos = m_nodegraph_mouse_pos;
            else if (m_link_mouse_dst != nullptr)
                m_fake_mouse_node.pos = m_nodegraph_mouse_pos;
        }
        else
        {
            if (m_link_mouse_src != nullptr)
            {
                if (m_hovered_slot_node != nullptr && m_hovered_slot_output != -1)
                {
                    m_link_mouse_src->node_src = m_hovered_slot_node;
                    m_link_mouse_src->slot_src = static_cast<size_t>(m_hovered_slot_output);
                }
                else
                {
                    m_link_mouse_src->node_dst = nullptr; // Make 'dead' link -> rendering will skip it
                    m_link_mouse_src->node_src = nullptr;
                }
                m_link_mouse_src = nullptr;
            }
            else if (m_link_mouse_dst != nullptr)
            {
                if (m_hovered_slot_node != nullptr && m_hovered_slot_input != -1)
                {
                    m_link_mouse_dst->node_dst = m_hovered_slot_node;
                    m_link_mouse_dst->slot_dst = static_cast<size_t>(m_hovered_slot_input);
                }
                else
                {
                    m_link_mouse_dst->node_dst = nullptr; // Make 'dead' link -> rendering will skip it
                    m_link_mouse_dst->node_src = nullptr;
                }
                m_link_mouse_dst = nullptr;
            }
        }

        // Draw grid
        this->DrawGrid();

        // Draw links
        drawlist->ChannelsSetCurrent(0);
        for (Link& link: m_links)
        {
            if (link.node_src!= nullptr && link.node_dst != nullptr) // Skip disconnected links
                this->DrawLink(link);
        }

        for (ReadingNode* node: m_reading_nodes)
        {
                ImGui::PushID(node->id);
                // Scalable node!
                ImVec2 node_rect_min = m_scroll_offset + node->pos;
                ImVec2 node_rect_max = node_rect_min + node->size + (m_style.node_window_padding * 2.f);

                // Draw content
                drawlist->ChannelsSetCurrent(2);
                ImGui::SetCursorScreenPos(node_rect_min + m_style.node_window_padding);
                ImGui::BeginGroup(); // Locks horizontal position
                ImVec2 cursor_start = ImGui::GetCursorPos();
                ImGui::InputInt("SoftBody Node ID", &node->softbody_node_id);
                ImVec2 cursor_plot = ImGui::GetCursorPos();
                ImVec2 plot_size((node->size.x), (node->size.y - (cursor_plot.y - cursor_start.y)));
                const char* plot_title = (node->softbody_node_id < 0) ? "~~ Inactive ~~" : "";
                const float* plot_data_ptr = &node->data_buffer[0].x;
                ImGui::PlotLines("", plot_data_ptr, 2000, node->data_offset, plot_title, std::numeric_limits<float>::min(), std::numeric_limits<float>::max(), plot_size, sizeof(Vec3));
                // ... the scale anchor
         //       ImVec2 scaler_size(30.f, 30.f);
         //       ImGui::SetCursorScreenPos(node_rect_max - m_style.node_window_padding - scaler_size);
         //       ImGui::Button("#", scaler_size);
         //       // Note: cursor easily escapes from the scaler when dragging, we need to track it...
         //       if (ImGui::IsMouseDragging(0))
         //       {
         //           if (ImGui::IsItemHovered() || m_last_scaled_node == node)
         //           {
         //               node->size += ImGui::GetIO().MouseDelta;
         //               if (ImGui::IsItemHovered())
         //               {
         //                   m_last_scaled_node = node;
         //               }
         //           }
         //       }
         //       else
         //       {
         //           m_last_scaled_node = nullptr;
         //       }
                ImGui::EndGroup();

                // Handle mouse dragging
                ImGui::SetCursorScreenPos(node_rect_min);
                ImGui::InvisibleButton("node", node->size + m_style.node_window_padding + m_style.node_window_padding);
                    // NOTE: Using 'InvisibleButton' enables dragging by node body but not by contained widgets
                    // NOTE: This MUST be done AFTER widgets are drawn, otherwise their input is blocked by the invis. button
                bool is_hovered = ImGui::IsItemHovered();
                bool node_moving_active = ImGui::IsItemActive();
                if (node_moving_active && ImGui::IsMouseDragging(0))
                {
                    node->pos += ImGui::GetIO().MouseDelta;
                }
                // Draw outline
                drawlist->ChannelsSetCurrent(1);
                ImU32 bg_color = (is_hovered) ? m_style.color_node_hovered : m_style.color_node;
                ImU32 border_color = (is_hovered) ? m_style.color_node_frame_hovered : m_style.color_node_frame;
                drawlist->AddRectFilled(node_rect_min, node_rect_max, bg_color, m_style.node_rounding);
                drawlist->AddRect(node_rect_min, node_rect_max, border_color, m_style.node_rounding);

                // Draw slots: 0 inputs, 3 outputs (XYZ)
                this->DrawOutputSlot(node, 0);
                this->DrawOutputSlot(node, 1);
                this->DrawOutputSlot(node, 2);

                ImGui::PopID();
        }

    /*    for (Node* node: m_nodes)
        {
            switch (node->type)
            {
            case Node::Type::READING:
            {
                ReadingNode* rnode = node->ToReading();
                ImGui::PushID(node->id);

                // Draw node content.
                drawlist->ChannelsSetCurrent(2);
                ImVec2 node_rect_min = m_scroll_offset + node->pos;
                ImGui::SetCursorScreenPos(node_rect_min + m_style.node_window_padding);
                bool old_any_active = ImGui::IsAnyItemActive();
                ImGui::BeginGroup(); // Locks horizontal position
                if (rnode->softbody_node_id >= 0)
                {
                    ImGui::Text("SoftBody positon (Working)", rnode->softbody_node_id);
                }
                else
                {
                    ImGui::Text("SoftBody positon (Inactive)");
                }
                ImGui::Text("Outputs: X Y Z");
                ImGui::InputInt("SB Node ID", &rnode->softbody_node_id);
                ImGui::EndGroup();
                bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
                node->size = ImGui::GetItemRectSize() + m_style.node_window_padding + m_style.node_window_padding;
                ImVec2 node_rect_max = node_rect_min + node->size;

                // Draw node rect
                drawlist->ChannelsSetCurrent(1);
                ImGui::SetCursorScreenPos(node_rect_min);
                ImGui::InvisibleButton("node", node->size);
                if (ImGui::IsItemHovered())
                {
                    m_hovered_node = node;
                }
                bool node_moving_active = ImGui::IsItemActive();
                if (node_moving_active && ImGui::IsMouseDragging(0))
                {
                    node->pos += ImGui::GetIO().MouseDelta;
                }
                ImU32 bg_color = (node == m_hovered_node) ? m_style.color_node_hovered : m_style.color_node;
                ImU32 border_color = (node == m_hovered_node) ? m_style.color_node_frame_hovered : m_style.color_node_frame;
                drawlist->AddRectFilled(node_rect_min, node_rect_max, bg_color, m_style.node_rounding);
                drawlist->AddRect(node_rect_min, node_rect_max, border_color, m_style.node_rounding);

                // Draw slots: 0 inputs, 3 outputs (XYZ)
                this->DrawOutputSlot(node, 0);
                this->DrawOutputSlot(node, 1);
                this->DrawOutputSlot(node, 2);

                m_hovered_node = nullptr;
                break;
            }
            case Node::Type::DISPLAY:
            {
                DisplayNode* dnode = node->ToDisplay();
                ImGui::PushID(node->id);

                // Scalable node!
                ImVec2 node_rect_min = m_scroll_offset + node->pos;
                ImVec2 node_rect_max = node_rect_min + node->size + (m_style.node_window_padding * 2.f);
                // Handle mouse dragging
                ImGui::SetCursorScreenPos(node_rect_min);
                ImGui::InvisibleButton("node", node->size);
                if (ImGui::IsItemHovered())
                {
                    m_hovered_node = node;
                }
                bool node_moving_active = ImGui::IsItemActive();
                if (node_moving_active && ImGui::IsMouseDragging(0))
                {
                    node->pos += ImGui::GetIO().MouseDelta;
                }
                // Draw outline
                ImU32 bg_color = (node == m_hovered_node) ? m_style.color_node_hovered : m_style.color_node;
                ImU32 border_color = (node == m_hovered_node) ? m_style.color_node_frame_hovered : m_style.color_node_frame;
                drawlist->AddRectFilled(node_rect_min, node_rect_max, bg_color, m_style.node_rounding);
                drawlist->AddRect(node_rect_min, node_rect_max, border_color, m_style.node_rounding);

                // Draw content
                ImGui::SetCursorScreenPos(node_rect_min + m_style.node_window_padding);
                ImGui::BeginGroup(); // Locks horizontal position
                const char* title = "Display";
                ImGui::Text(title);
                float data[] = {1.f, 2,5,6,9,4,3,8,5,1,7,6};
                ImVec2 plot_size((node->size.x - 15.f), (node->size.y - ImGui::CalcTextSize("Display").y)); // x: leave space for the scaler
                ImGui::PlotLines(".", data, 12, 0, nullptr, std::numeric_limits<float>::min(), std::numeric_limits<float>::max(), plot_size);
                // ... the scale anchor
                ImVec2 scaler_size(20.f, 20.f);
                ImGui::SetCursorScreenPos(node_rect_max - m_style.node_window_padding - scaler_size);
                ImGui::Button("#", scaler_size);
                // Note: cursor easily escapes from the scaler when dragging, we need to track it...
                if (ImGui::IsMouseDragging(0))
                {
                    if (ImGui::IsItemHovered() || m_last_scaled_node == node)
                    {
                        node->size += ImGui::GetIO().MouseDelta;
                        if (ImGui::IsItemHovered())
                        {
                            m_last_scaled_node = node;
                        }
                    }
                }
                else
                {
                    m_last_scaled_node = nullptr;
                }

                // Draw slots 1 input, 1 output (pass-thru)
                this->DrawInputSlot(node, 0);
                this->DrawOutputSlot(node, 0);

                ImGui::EndGroup();
            }
            default:;
            }
        }*/

        // Slot hover cleanup
        if (!m_is_any_slot_hovered)
        {
            m_hovered_slot_node = nullptr;
            m_hovered_slot_input = -1;
            m_hovered_slot_output = -1;
        }

        // Scrolling
        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
        {
            m_scroll = m_scroll - ImGui::GetIO().MouseDelta;
        }

        ImGui::EndChild();
        drawlist->ChannelsMerge();
    }

    std::vector<ReadingNode*> m_reading_nodes;
    std::vector<Link> m_links;
    Style    m_style;
    ImVec2   m_scroll;
    ImVec2   m_scroll_offset;
    ImVec2   m_nodegraph_mouse_pos;
    Node*    m_hovered_slot_node;
    int      m_hovered_slot_input;  // -1 = none
    int      m_hovered_slot_output; // -1 = none
    bool     m_is_any_slot_hovered;
    Node*    m_last_scaled_node;
    Node     m_fake_mouse_node;     ///< Used while dragging link with mouse
    Link*    m_link_mouse_src;      ///< Link being mouse-dragged by it's input end.
    Link*    m_link_mouse_dst;      ///< Link being mouse-dragged by it's output end.
    int      m_num_ticks;
};

// #################################################### END CUSTOM NODE EDITOR ############################################################# //

#ifdef _DEBUG
    static const std::string mPluginsCfg = "plugins_d.cfg";
#else
    static const std::string mPluginsCfg = "plugins.cfg";
#endif

class DemoApp: public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener,  public Ogre::WindowEventListener
{
public:
    void DrawGui()
    {
        static bool show_nodegraph = false;
        static bool show_test = false;
        static bool show_plot_test = false;

        if (ImGui::BeginMainMenuBar())
        {
            ImVec2 pos = ImGui::GetCursorPos();
            ImGui::Checkbox("Test", &show_test);
            ImGui::SameLine();
            ImGui::Checkbox("Nodegraph", &show_nodegraph);
            ImGui::SameLine();
            ImGui::Checkbox("Plot test", &show_plot_test);

            ImGui::EndMainMenuBar();
        }

        if (show_test)
        {
            ImGui::ShowTestWindow(&show_test);
        }

        if (show_nodegraph)
            m_nodegraph.Draw();

        if (show_plot_test)
        {
            float data[] = {0,2,1,4,3,5,4,6,7,5,8,6,9,7,10,8,11,9};
            ImGui::Begin("graph test");
            ImGui::PlotLines("offset0", data, IM_ARRAYSIZE(data), 0, "", 0, 12, ImVec2(0, 50));
            ImGui::PlotLines("offset5", data, IM_ARRAYSIZE(data), 5, "", 0, 12, ImVec2(0, 50));
            ImGui::PlotLines("offset10", data, IM_ARRAYSIZE(data), 10, "", 0, 12, ImVec2(0, 50));
            ImGui::End();
        }
    }

    void Go()
    {

        mRoot = new Ogre::Root(mPluginsCfg);

        // Show the configuration dialog and initialise the system.
        // NOTE: If you have valid file 'ogre.cfg', you can use `root.restoreConfig()` instead.
        if(!mRoot->showConfigDialog())
        {
            // User abort
            delete mRoot;
            return;
        }

        const bool create_window = true; // Let's be descriptive :)
        const char* window_name = "OGRE/ImGui demo app";
        mWindow = mRoot->initialise(create_window, window_name);

        mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
        mCamera = mSceneMgr->createCamera("PlayerCam");

        // Create one viewport, entire window
        Ogre::Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

        // Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

        Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        mSceneMgr->setAmbientLight(Ogre::ColourValue(.7f, .7f, .7f));
        mCamera->getViewport()->setBackgroundColour(Ogre::ColourValue(0.f, 0.2f, 0.1f)); // Dark green

        // OIS setup
        OIS::ParamList pl;

        size_t windowHnd = 0;
        std::ostringstream windowHndStr;
        mWindow->getCustomAttribute("WINDOW", &windowHnd);
        windowHndStr << windowHnd;
        pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
        pl.insert(OIS::ParamList::value_type("x11_mouse_hide", "false"));
        pl.insert(OIS::ParamList::value_type("XAutoRepeatOn", "false"));
        pl.insert(OIS::ParamList::value_type("x11_mouse_grab", "false"));
#else
        pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_FOREGROUND"));
        pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_NONEXCLUSIVE"));
#endif
        mInputManager = OIS::InputManager::createInputSystem(pl);

        mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
        mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));

        // Set initial mouse clipping size
        this->windowResized(mWindow);

        // Register as a Window listener
        Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

        // === Create IMGUI ====
        Ogre::ImguiManager::createSingleton();
        Ogre::ImguiManager::getSingleton().init(mSceneMgr, mKeyboard, mMouse); // OIS mouse + keyboard

        mRoot->addFrameListener(this);

        mMouse->setEventCallback(this);
        mKeyboard->setEventCallback(this);

        srand (time(NULL));

        mRoot->startRendering();

        this->Shutdown();
    }

private:
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) override
    {
        return (!mWindow->isClosed() && (!mShutDown)); // False means "exit the application"
    }

    virtual bool frameStarted(const Ogre::FrameEvent& evt) override
    {
        // Need to capture/update each device
        mKeyboard->capture();
        mMouse->capture();

        // ===== Start IMGUI frame =====
        int left, top, width, height;
        mWindow->getViewport(0)->getActualDimensions(left, top, width, height); // output params
        Ogre::ImguiManager::getSingleton().newFrame(evt.timeSinceLastFrame, Ogre::Rect(left, top, width, height));

        // generate input data
        static int jitter_max = 0;
        static float last_time = ImGui::GetTime();
        static float time_to_jitter_update = 2.f;

        while (last_time < ImGui::GetTime())
        {
            static float base_phase = 0.f;
            // Update the dummy truck
            for (int i = 0; i < IM_ARRAYSIZE(G_truck_node_x); ++i)
            {
                float phase = base_phase + static_cast<float>(i) * 0.001;
                G_truck_node_x[i] = cosf(phase); // clean input
                G_truck_node_x[i] + static_cast<float>(rand() % jitter_max); // add jitter
            }
            base_phase += 0.001f;

            // Push data to nodegraph
            m_nodegraph.PhysicsTick();

            // update time
            last_time += (1.f / 2000.f);
            time_to_jitter_update -= (1.f / 2000.f);
        }

        if (time_to_jitter_update < 0.f)
        {
            jitter_max = (rand() % 20);
            time_to_jitter_update = static_cast<float>(rand() % 4) + 1.f;
        }

        // ===== Draw IMGUI  ====
        this->DrawGui();



        return true;
    }

    bool keyPressed( const OIS::KeyEvent &arg ) override
    {
        if (arg.key == OIS::KC_ESCAPE) // All extras we need
        {
            mShutDown = true;
        }

        Ogre::ImguiManager::getSingleton().keyPressed(arg);
        return true;
    }

    bool keyReleased(const OIS::KeyEvent &arg) override
    {
        Ogre::ImguiManager::getSingleton().keyReleased(arg);
        return true;
    }

    bool mouseMoved(const OIS::MouseEvent &arg) override
    {
        Ogre::ImguiManager::getSingleton().mouseMoved(arg);
        return true;
    }

    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override
    {
        Ogre::ImguiManager::getSingleton().mousePressed(arg, id);
        return true;
    }

    bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) override
    {
        Ogre::ImguiManager::getSingleton().mouseReleased(arg, id);
        return true;
    }

    // Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow* rw) override
    {
        unsigned int width, height, depth;
        int left, top;
        rw->getMetrics(width, height, depth, left, top);

        const OIS::MouseState &ms = mMouse->getMouseState();
        ms.width = width;
        ms.height = height;
    }

    virtual void windowClosed(Ogre::RenderWindow* rw) override
    {
        this->Shutdown(); // Unattach OIS before window shutdown (very important under Linux)
        mShutDown = true; // Stop application
    }

    void Shutdown()
    {
        if(mInputManager)
        {
            mInputManager->destroyInputObject(mMouse);
            mInputManager->destroyInputObject(mKeyboard);

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
        // Remove ourself as a Window listener
        Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
        delete mRoot;
        mRoot = nullptr;
    }

    Ogre::Root*                 mRoot    ;
    Ogre::Camera*               mCamera  ;
    Ogre::SceneManager*         mSceneMgr;
    Ogre::RenderWindow*         mWindow  ;

    bool                        mShutDown;

    //OIS Input devices
    OIS::InputManager*          mInputManager;
    OIS::Mouse*                 mMouse       ;
    OIS::Keyboard*              mKeyboard    ;

    NodeGraphTool               m_nodegraph;
};



int main(int argc, char *argv[])
{
    try
    {
        DemoApp demo_app;
        demo_app.Go();
    }
    catch(Ogre::Exception& e)
    {
        std::cerr << "An exception has occurred: " << e.getFullDescription().c_str() << std::endl;
    }

    return 0;
}


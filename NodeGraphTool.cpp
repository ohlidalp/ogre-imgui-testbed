#include "NodeGraphTool.h"

#include <imgui_internal.h> // For ImRect

#include "sigpack/sigpack.h" // *Bundled*

FakeTruck G_fake_truck; // declared extern in "NodeGraphTool.h"

RoR::NodeGraphTool::NodeGraphTool():
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
    m_reading_nodes.push_back(new SourceNode());
    m_reading_nodes.push_back(new SourceNode());
    m_reading_nodes.push_back(new SourceNode());

    m_reading_nodes[1]->pos += ImVec2(300.f, -10.f);
    m_reading_nodes[2]->pos += ImVec2(250.f, 133.f);

    // Links
//     m_links.emplace_back(m_nodes[0], m_nodes[1], 0, 0); // X output -> graph input
//     m_links.emplace_back(m_nodes[0], m_nodes[2], 2, 0); // Z output -> graph input

}

RoR::NodeGraphTool::Link* RoR::NodeGraphTool::FindLinkByDestination(Node* node, size_t slot)
{
    for (Link& link: m_links)
    {
        if (link.node_dst == node && link.slot_dst == slot)
            return &link;
    }
    return nullptr;
}

RoR::NodeGraphTool::Style::Style()
{
    color_grid                = ImColor(200,200,200,40);
    grid_line_width           = 1.f;
    grid_size                 = 64.f;
    color_node                = ImColor(30,30,35);
    color_node_frame          = ImColor(100,100,100);
    color_node_frame_active   = ImColor(100,100,100);
    color_node_hovered        = ImColor(45,45,49);
    color_node_frame_hovered  = ImColor(125,125,125);
    node_rounding             = 4.f;
    node_window_padding       = ImVec2(8.f,8.f);
    slot_hoverbox_extent      = ImVec2(15.f, 10.f);
    color_input_slot          = ImColor(150,150,150,150);
    color_output_slot         = ImColor(150,150,150,150);
    color_input_slot_hover    = ImColor(144,155,222,245);
    color_output_slot_hover   = ImColor(144,155,222,245);
    node_slots_radius         = 5.f;
    color_link                = ImColor(200,200,100);
    link_line_width           = 3.f;
}

RoR::NodeGraphTool::Link* RoR::NodeGraphTool::FindLinkBySource(Node* node, size_t slot)
{
    for (Link& link: m_links)
    {
        if (link.node_src == node && link.slot_src == slot)
            return &link;
    }
    return nullptr;
}

void RoR::NodeGraphTool::Draw()
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

void RoR::NodeGraphTool::PhysicsTick()
{
    for (SourceNode* rn: m_reading_nodes)
    {
        if (rn->softbody_node_id >= 0)
        {
            Vec3 data;
            data.x = G_fake_truck.nodes_x[rn->softbody_node_id];
            data.y = G_fake_truck.nodes_x[rn->softbody_node_id];
            data.z = G_fake_truck.nodes_x[rn->softbody_node_id];
            rn->PushData(data);
        }
    }
    ++m_num_ticks;
}

void RoR::NodeGraphTool::DrawGrid()
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

void RoR::NodeGraphTool::DrawLink(Link& link)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->ChannelsSetCurrent(0); // background + curves
    ImVec2 offset =m_scroll_offset;
    ImVec2 p1 = offset + link.node_src->GetOutputSlotPos(link.slot_src) + (m_style.node_window_padding * 2.f);
    ImVec2 p2 = offset + link.node_dst->GetInputSlotPos(link.slot_dst);
    ImRect window = ImGui::GetCurrentWindow()->Rect();

    if (this->IsInside(window.Min, window.Max, p1) || this->IsInside(window.Min, window.Max, p1)) // very basic clipping
    {
        float bezier_pt_dist = fmin(50.f, fmin(fabs(p1.x - p2.x)*0.75f, fabs(p1.y - p2.y)*0.75f)); // Maximum: 50; minimum: 75% of shorter-axis distance between p1 and p2
        draw_list->AddBezierCurve(p1, p1+ImVec2(+bezier_pt_dist,0), p2+ImVec2(-bezier_pt_dist,0), p2, m_style.color_link, m_style.link_line_width);
    }
}

void RoR::NodeGraphTool::DrawSlotUni(Node* node, const size_t index, const bool input)
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

RoR::NodeGraphTool::Link* RoR::NodeGraphTool::AddLink(Node* src, Node* dst, size_t src_slot, size_t dst_slot) ///< creates new link or fetches existing unused one
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

void RoR::NodeGraphTool::DrawNodeGraphPane()
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

    for (SourceNode* node: m_reading_nodes)
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
            const float PLOT_MIN = -1.7f;//std::numeric_limits<float>::min();
            const float PLOT_MAX = +1.7f; //std::numeric_limits<float>::max();
            ImGui::PlotLines("", plot_data_ptr, 2000, node->data_offset, plot_title, PLOT_MIN, PLOT_MAX, plot_size, sizeof(Vec3));
            // FIXME ... the scale anchor
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

void RoR::NodeGraphTool::CalcGraph()
{
    // Reset link states
    for (Link& link: m_links)
    {
        link.processed = false;
    }

    bool has_unprocessed = false;
    do
    {
        for (Link& link: m_links)
        {
            if (link.processed)
                continue;

            // Ty to process
            float val;
            if (link.node_src->type == Node::Type::SOURCE)
            {
                SourceNode* src_node = static_cast<SourceNode*>(link.node_src);
                // get data from past tick
                int buf_pos = src_node->data_offset - (m_num_ticks - 1);
                if (buf_pos < 0)
                {
                    buf_pos = 2000 + buf_pos;
                }
                val = src_node->data_buffer[buf_pos].x; // temporary = using only X
            }
            else if (link.node_src->type == Node::Type::TRANSFORM)
            {
                TransformNode* tf_node = static_cast<TransformNode*>(link.node_src);
                if (!tf_node->result_ready)
                {
                    has_unprocessed = false;
                    continue;
                }
                val = tf_node->result_val;
            }

            // Process the DST node
            if (link.node_dst->type == Node::Type::TRANSFORM) // there will be more...
            {
                
            }
        }
    }
    while (has_unprocessed);


    for (;;)
    {
        bool repeat = false;
        for (Link& link: m_links)
        {
            switch (link.node_dst->type)
            {
            case Node::Type::TRANSFORM:
            {
                switch (link.node_src->type);
                break;
            }
            }
        }
    }
}

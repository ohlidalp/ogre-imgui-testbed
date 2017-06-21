#include "NodeGraphTool.h"

#include <imgui_internal.h> // For ImRect

//#include "sigpack/sigpack.h" // *Bundled*

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
    for (ReadingNode* rn: m_read_nodes)
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
    ImVec2 p1 = offset + link.node_src->GetOutputSlotPos(link.slot_src);
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
    ImVec2 slot_center_pos =  ((input) ? node->GetInputSlotPos(index) : (node->GetOutputSlotPos(index)));
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
                    // HACK: replace with "NodeLinkChanged(N1, n2, bool connected)" callback function
                    if (node->type == Node::Type::DISPLAY)
                    {
                        static_cast<DisplayNode*>(node)->source_node = nullptr;
                    }
                    // END HACK
                }
                else
                {
                    link->node_src = &m_fake_mouse_node;
                    m_link_mouse_src = link;
                    // HACK: replace with "NodeLinkChanged(N1, n2, bool connected)" callback function
                    if (link->node_dst->type == Node::Type::DISPLAY)
                    {
                        static_cast<DisplayNode*>(link->node_dst)->source_node = nullptr;
                    }
                    // END HACK
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

void RoR::NodeGraphTool::DrawNodeBegin(Node* node)
{
    ImGui::PushID(node->id);
    node->draw_rect_min = m_scroll_offset + node->pos;
    // Draw content
    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    drawlist->ChannelsSetCurrent(2);
    ImGui::SetCursorScreenPos(node->draw_rect_min + m_style.node_window_padding);
    ImGui::BeginGroup(); // Locks horizontal position
}

void RoR::NodeGraphTool::DrawNodeFinalize(Node* node)
{
    ImGui::EndGroup();
    node->calc_size = ImGui::GetItemRectSize() + (m_style.node_window_padding * 2.f);

    // Handle mouse dragging
    ImGui::SetCursorScreenPos(node->draw_rect_min);
    ImGui::InvisibleButton("node", node->calc_size);
        // NOTE: Using 'InvisibleButton' enables dragging by node body but not by contained widgets
        // NOTE: This MUST be done AFTER widgets are drawn, otherwise their input is blocked by the invis. button
    bool is_hovered = ImGui::IsItemHovered();
    bool node_moving_active = ImGui::IsItemActive();
    if (node_moving_active && ImGui::IsMouseDragging(0))
    {
        node->pos += ImGui::GetIO().MouseDelta;
    }
    // Draw outline
    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    drawlist->ChannelsSetCurrent(1);
    ImU32 bg_color = (is_hovered) ? m_style.color_node_hovered : m_style.color_node;
    ImU32 border_color = (is_hovered) ? m_style.color_node_frame_hovered : m_style.color_node_frame;
    ImVec2 draw_rect_max = node->draw_rect_min + node->calc_size;
    drawlist->AddRectFilled(node->draw_rect_min, draw_rect_max, bg_color, m_style.node_rounding);
    drawlist->AddRect(node->draw_rect_min, draw_rect_max, border_color, m_style.node_rounding);

    // Draw slots: 0 inputs, 3 outputs (XYZ)
    drawlist->ChannelsSetCurrent(2);
    for (size_t i = 0; i<node->num_inputs; ++i)
        this->DrawInputSlot(node, i);
    for (size_t i = 0; i<node->num_outputs; ++i)
        this->DrawOutputSlot(node, i);

    ImGui::PopID();
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
                // HACK: replace with "NodeLinkChanged(N1, n2, bool connected)" callback function
                if (m_hovered_slot_node->type == Node::Type::DISPLAY)
                {
                    DisplayNode* d_node = static_cast<DisplayNode*>(m_hovered_slot_node);
                    d_node->source_node = m_link_mouse_dst->node_src;
                }
                // END HACK
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

    for (ReadingNode* node: m_read_nodes)
    {
        this->DrawNodeBegin(node);

        ImGui::InputInt("SoftBody Node ID", &node->softbody_node_id);
        if (node->softbody_node_id >= 0)
        {
            RoR::Vec3 value = node->data_buffer[node->data_offset];
            ImGui::Text("Value: %7.2f %7.2f %7.2f", value.x, value.y, value.z);
        }
        else
        {
            ImGui::Text("Value: ~~inactive~~");
        }

        this->DrawNodeFinalize(node);
    }

    static const float DUMMY_PLOT[] = {0,0,0,1,-1,1,0,0,0};

    for (DisplayNode* node: m_disp_nodes)
    {
        this->DrawNodeBegin(node);

        const float* data_ptr = DUMMY_PLOT;;
        int data_length = IM_ARRAYSIZE(DUMMY_PLOT);
        int data_offset = 0;
        int stride = sizeof(float);
        const char* title = "~~disconnected~~";
        if ((node->source_node != nullptr) && (node->source_node->type == Node::Type::SOURCE))
        {
            ReadingNode* rd_node = static_cast<ReadingNode*>(node->source_node);
            data_ptr = &rd_node->data_buffer[rd_node->data_offset].x;
            data_offset = rd_node->data_offset;
            stride = sizeof(Vec3);
            title = "";
            data_length = 2000;
        }
        const float PLOT_MIN = -1.7f;//std::numeric_limits<float>::min();
        const float PLOT_MAX = +1.7f; //std::numeric_limits<float>::max();
        ImGui::PlotLines("", data_ptr, data_length, data_offset, title, PLOT_MIN, PLOT_MAX, node->user_size, stride);

        this->DrawNodeFinalize(node);
    }

    // Slot hover cleanup
    if (!m_is_any_slot_hovered)
    {
        m_hovered_slot_node = nullptr;
        m_hovered_slot_input = -1;
        m_hovered_slot_output = -1;
    }

    // Open context menu
    bool open_context_menu = false;
    if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
    {
        open_context_menu = true;
    }
    if (open_context_menu)
    {
        ImGui::OpenPopup("context_menu");
    }

    // Draw context menu
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8,8));
    if (ImGui::BeginPopup("context_menu"))
    {
        ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - m_scroll_offset;
        ImGui::Text("New node:");
        if (ImGui::MenuItem("Reading"))
        {
            m_read_nodes.push_back(new ReadingNode(scene_pos));
        }
        if (ImGui::MenuItem("Display"))
        {
            m_disp_nodes.push_back(new DisplayNode(scene_pos));
        }
        if (ImGui::MenuItem("Transform"))
        {
            m_xform_nodes.push_back(new TransformNode(scene_pos));
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();

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
    // Reset states
    for (TransformNode* n: m_xform_nodes)
    {
        n->done = false;
    }

    bool keep_working = false;
    do
    {
        for (TransformNode* n: m_xform_nodes)
        {
            
        }
    }
    while (keep_working);

}

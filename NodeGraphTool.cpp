#include "NodeGraphTool.h"

#include <imgui_internal.h> // For ImRect

// Bundled libs
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/prettywriter.h"
// END bundled

#include <map>

RoR::NodeGraphTool::NodeGraphTool():
    m_scroll(0.0f, 0.0f),
    m_last_scaled_node(nullptr),
    m_link_mouse_src(nullptr),
    m_link_mouse_dst(nullptr),
    m_hovered_slot_node(nullptr),
    m_hovered_node(nullptr),
    m_context_menu_node(nullptr),
    m_header_mode(HeaderMode::NORMAL),
    m_hovered_slot_input(-1),
    m_hovered_slot_output(-1),
    m_free_id(0),
    m_fake_mouse_node(this, ImVec2()) // Used for dragging links with mouse
{
    memset(m_filename, 0, sizeof(m_filename));
    snprintf(m_motionsim_ip, IM_ARRAYSIZE(m_motionsim_ip), "localhost");
    m_motionsim_port = 1234;
}

RoR::NodeGraphTool::Link* RoR::NodeGraphTool::FindLinkByDestination(Node* node, const int slot)
{
    for (Link* link: m_links)
    {
        if (link->node_dst == node && link->slot_dst == slot)
            return link;
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

RoR::NodeGraphTool::Link* RoR::NodeGraphTool::FindLinkBySource(Node* node, const int slot)
{
    for (Link* link: m_links)
    {
        if (link->node_src == node && link->buff_src->slot == slot)
            return link;
    }
    return nullptr;
}

void RoR::NodeGraphTool::Draw()
{
    // Create a window
    if (!ImGui::Begin("RigsOfRods NodeGraph"))
        return; // No window -> nothing to do.

    // Debug outputs
    //ImGui::Text("MouseDrag - src: 0x%p, dst: 0x%p | mousenode - X:%.1f, Y:%.1f", m_link_mouse_src, m_link_mouse_dst, m_fake_mouse_node.pos.x, m_fake_mouse_node.pos.y);
    //ImGui::Text("SlotHover - node: 0x%p, input: %d, output: %d", m_hovered_slot_node, m_hovered_slot_input, m_hovered_slot_output);
    ImGui::SameLine();
    if (ImGui::Button("Open file"))
    {
        m_header_mode = HeaderMode::LOAD_FILE;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save file"))
    {
        m_header_mode = HeaderMode::SAVE_FILE;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear all"))
    {
        m_header_mode = HeaderMode::CLEAR_ALL;
    }
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50.f);
  //TODO  ImGui::PushItemWidth(150.f);
  //TODO  ImGui::InputText("IP", m_motionsim_ip, IM_ARRAYSIZE(m_motionsim_ip));
  //TODO  ImGui::SameLine();
  //TODO  ImGui::InputInt("Port", &m_motionsim_port);
  //TODO  ImGui::PopItemWidth();
  //TODO  ImGui::SameLine();
  //TODO  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50.f);
    static bool transmit = false;
    ImGui::Checkbox("Transmit", &transmit); // TODO: Enable/disable networking

    if (m_header_mode != HeaderMode::NORMAL)
    {
        if (ImGui::Button("Cancel"))
        {
            m_header_mode = HeaderMode::NORMAL;
        }
        if ((m_header_mode == HeaderMode::CLEAR_ALL))
        {
            ImGui::SameLine();
            ImGui::Text("Really clear all?");
            ImGui::SameLine();
            if (ImGui::Button("Confirm"))
            {
                this->ClearAll();
                m_header_mode = HeaderMode::NORMAL;
            }
        }
        else
        {
            ImGui::SameLine();
            ImGui::InputText("Filename", m_filename, IM_ARRAYSIZE(m_filename));
            ImGui::SameLine();
            if ((m_header_mode == HeaderMode::SAVE_FILE) && ImGui::Button("Save now"))
            {
                this->SaveAsJson();
                m_header_mode = HeaderMode::NORMAL;
            }
            else if ((m_header_mode == HeaderMode::LOAD_FILE) && ImGui::Button("Load now"))
            {
                this->LoadFromJson();
                m_header_mode = HeaderMode::NORMAL;
            }
        }
    }

    // Scripting engine messages
    if (! m_messages.empty())
    {
        if (ImGui::CollapsingHeader("Messages"))
        {
            for (std::string& msg: m_messages)
            {
                ImGui::BulletText(msg.c_str());
            }
            if (ImGui::Button("Clear messages"))
            {
                m_messages.clear();
            }
        }
    }

    m_scroll_offset = ImGui::GetCursorScreenPos() - m_scroll;
    m_is_any_slot_hovered = false;

    this->DrawNodeGraphPane();

    // Finalize the window
    ImGui::End();
}

void RoR::NodeGraphTool::PhysicsTick()
{
    for (Node* node: m_nodes)
    {
        if (node->type != Node::Type::GENERATOR)
            continue;

        GeneratorNode* gen_node = static_cast<GeneratorNode*>(node);
        gen_node->elapsed += 0.002f;

        float result = cosf((gen_node->elapsed / 2.f) * 3.14f * gen_node->frequency) * gen_node->amplitude;

        // add noise
        if (gen_node->noise_max != 0)
        {
            int r = rand() % gen_node->noise_max;
            result += static_cast<float>((r*2)-r) * 0.1f;
        }

        // save to buffer
        gen_node->buffer_out.Push(result);
    }
    this->CalcGraph();
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

void RoR::NodeGraphTool::DrawLink(Link* link)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->ChannelsSetCurrent(0); // background + curves
    ImVec2 offset =m_scroll_offset;
    ImVec2 p1 = offset + link->node_src->GetOutputSlotPos(link->buff_src->slot);
    ImVec2 p2 = offset + link->node_dst->GetInputSlotPos(link->slot_dst);
    ImRect window = ImGui::GetCurrentWindow()->Rect();

    if (this->IsInside(window.Min, window.Max, p1) || this->IsInside(window.Min, window.Max, p1)) // very basic clipping
    {
        float bezier_pt_dist = fmin(50.f, fmin(fabs(p1.x - p2.x)*0.75f, fabs(p1.y - p2.y)*0.75f)); // Maximum: 50; minimum: 75% of shorter-axis distance between p1 and p2
        draw_list->AddBezierCurve(p1, p1+ImVec2(+bezier_pt_dist,0), p2+ImVec2(-bezier_pt_dist,0), p2, m_style.color_link, m_style.link_line_width);
    }
}

void RoR::NodeGraphTool::DrawSlotUni(Node* node, const int index, const bool input)
{
    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    drawlist->ChannelsSetCurrent(2);
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
                node->DetachLink(link);
                if (input)
                {
                    m_fake_mouse_node.BindDst(link, 0);
                    m_link_mouse_dst = link;
                }
                else
                {
                    m_fake_mouse_node.BindSrc(link, 0);
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

RoR::NodeGraphTool::Link* RoR::NodeGraphTool::AddLink(Node* src, Node* dst, int src_slot, int dst_slot)
{
    Link* link = new Link();
    src->BindSrc(link, src_slot);
    dst->BindDst(link, dst_slot);
    m_links.push_back(link);
    return link;
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

    // Draw slots: 0 inputs, 3 outputs (XYZ)
    for (size_t i = 0; i<node->num_inputs; ++i)
        this->DrawInputSlot(node, i);
    for (size_t i = 0; i<node->num_outputs; ++i)
        this->DrawOutputSlot(node, i);

    // Handle mouse dragging
    bool is_hovered = false;
    if (!m_is_any_slot_hovered)
    {
        ImGui::SetCursorScreenPos(node->draw_rect_min);
        ImGui::InvisibleButton("node", node->calc_size);
            // NOTE: Using 'InvisibleButton' enables dragging by node body but not by contained widgets
            // NOTE: This MUST be done AFTER widgets are drawn, otherwise their input is blocked by the invis. button
        is_hovered = ImGui::IsItemHovered();
        bool node_moving_active = ImGui::IsItemActive();
        if (node_moving_active && ImGui::IsMouseDragging(0))
        {
            node->pos += ImGui::GetIO().MouseDelta;
        }
    }
    // Draw outline
    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    drawlist->ChannelsSetCurrent(1);
    ImU32 bg_color = (is_hovered) ? m_style.color_node_hovered : m_style.color_node;
    ImU32 border_color = (is_hovered) ? m_style.color_node_frame_hovered : m_style.color_node_frame;
    ImVec2 draw_rect_max = node->draw_rect_min + node->calc_size;
    drawlist->AddRectFilled(node->draw_rect_min, draw_rect_max, bg_color, m_style.node_rounding);
    drawlist->AddRect(node->draw_rect_min, draw_rect_max, border_color, m_style.node_rounding);

    ImGui::PopID();

    if (is_hovered)
        m_hovered_node = node;
}

void RoR::NodeGraphTool::DetachAndDeleteLink(Link* link)
{
    if (link->node_dst != nullptr)
        link->node_dst->DetachLink(link);
    if (link->node_src != nullptr)
        link->node_src->DetachLink(link);
    this->DeleteLink(link);
}

void RoR::NodeGraphTool::DeleteLink(Link* link)
{
    auto itor = m_links.begin();
    auto endi = m_links.end();
    for (; itor != endi; ++itor)
    {
        if (link == *itor)
        {
            delete link;
            m_links.erase(itor);
            return;
        }
    }
    assert(false && "NodeGraphTool::DeleteLink(): stray link - not in list");
}

void RoR::NodeGraphTool::DrawNodeGraphPane()
{
    const bool draw_border = false;
    const int window_flags = ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollWithMouse;
    if (!ImGui::BeginChild("scroll-region", ImVec2(0,0), draw_border, window_flags))
        return; // Nothing more to do.

    const float baseNodeWidth = 120.f; // same as reference, but hardcoded
    float currentNodeWidth = baseNodeWidth;
    ImGui::PushItemWidth(currentNodeWidth);
    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    drawlist->ChannelsSplit(3); // 0 = background (grid, curves); 1 = node rectangle/slots; 2 = node content

    // Update mouse drag
    const ImVec2 nodepane_screen_pos = ImGui::GetCursorScreenPos();
    m_nodegraph_mouse_pos = (ImGui::GetIO().MousePos - nodepane_screen_pos);
    if (ImGui::IsMouseDragging(0) && this->IsLinkDragInProgress())
    {
        m_fake_mouse_node.pos = m_nodegraph_mouse_pos;
    }
    else // drag ended
    {
        if (m_link_mouse_src != nullptr)
        {
            if (m_hovered_slot_node != nullptr && m_hovered_slot_output != -1)
            {
                m_hovered_slot_node->BindSrc(m_link_mouse_src, m_hovered_slot_output);
            }
            else
            {
                this->DetachAndDeleteLink(m_link_mouse_src);
            }
            m_link_mouse_src = nullptr;
        }
        else if (m_link_mouse_dst != nullptr)
        {
            if (m_hovered_slot_node != nullptr && m_hovered_slot_input != -1)
            {
                m_hovered_slot_node->BindDst(m_link_mouse_dst, m_hovered_slot_input);
            }
            else
            {
                this->DetachAndDeleteLink(m_link_mouse_dst);
            }
            m_link_mouse_dst = nullptr;
        }
    }

    // Draw grid
    this->DrawGrid();

    // DRAW LINKS

    drawlist->ChannelsSetCurrent(0);
    for (Link* link: m_links)
    {
        this->DrawLink(link);
    }

    // DRAW NODES
    m_hovered_node = nullptr;
    for (Node* node: m_nodes)
    {
        node->Draw();
    }

    // Slot hover cleanup
    if (!m_is_any_slot_hovered)
    {
        m_hovered_slot_node = nullptr;
        m_hovered_slot_input = -1;
        m_hovered_slot_output = -1;
    }

    // Open context menu
    if (ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
    {
        if (!ImGui::IsAnyItemHovered())
        {
            ImGui::OpenPopup("context_menu");
            m_context_menu_node = nullptr;
        }
        else if (m_hovered_node != nullptr)
        {
            m_context_menu_node = m_hovered_node;
            ImGui::OpenPopup("context_menu");
        }
    }

    // Draw context menu
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8,8));
    if (ImGui::BeginPopup("context_menu"))
    {
        ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - m_scroll_offset;
        if (m_context_menu_node != nullptr)
        {
            ImGui::Text("Existing node:");
            if (ImGui::MenuItem("Delete"))
            {
                this->DetachAndDeleteNode(m_context_menu_node);
                m_context_menu_node = nullptr;
            }
        }
        else
        {
            ImGui::Text("New node:");
            if (ImGui::MenuItem("Generator"))
            {
                m_nodes.push_back(new GeneratorNode(this, scene_pos));
            }
            if (ImGui::MenuItem("Display"))
            {
                m_nodes.push_back(new DisplayNode(this, scene_pos));
            }
            if (ImGui::MenuItem("Transform"))
            {
                m_nodes.push_back(new TransformNode(this, scene_pos));
            }
            if (ImGui::MenuItem("Script"))
            {
                m_nodes.push_back(new ScriptNode(this, scene_pos));
            }
        }
        ImGui::EndPopup();
    }
    else
    {
        m_context_menu_node = nullptr;
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
    for (Node* n: m_nodes)
    {
        n->done = false;
    }

    bool all_done = true;
    do
    {
        for (Node* n: m_nodes)
        {
            if (! n->done)
            {
                all_done &= n->Process();
            }
        }
    }
    while (!all_done);
}

void RoR::NodeGraphTool::ScriptMessageCallback(const AngelScript::asSMessageInfo *msg, void *param)
{
    const char *type = "ERR ";
    if( msg->type == AngelScript::asMSGTYPE_WARNING ) 
        type = "WARN";
    else if( msg->type == AngelScript::asMSGTYPE_INFORMATION ) 
        type = "INFO";

    char buf[500];
    snprintf(buf, 500, "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
    std::cout << buf; // TODO: show in nodegraph window!!
}

void RoR::NodeGraphTool::AddMessage(const char* format, ...)
{
    char buffer[2000] = {};

    va_list args;
    va_start(args, format);
        vsprintf(buffer, format, args);
    va_end(args);

    m_messages.push_back(buffer);
}

void RoR::NodeGraphTool::NodeToJson(rapidjson::Value& j_data, Node* node, rapidjson::Document& doc)
{
    j_data.AddMember("pos_x",       node->pos.x,       doc.GetAllocator());
    j_data.AddMember("pos_y",       node->pos.y,       doc.GetAllocator());
    j_data.AddMember("user_size_x", node->user_size.x, doc.GetAllocator());
    j_data.AddMember("user_size_y", node->user_size.y, doc.GetAllocator());
    j_data.AddMember("id",          node->id,          doc.GetAllocator());
}

void RoR::NodeGraphTool::JsonToNode(Node* node, const rapidjson::Value& j_object)
{
    node->pos = ImVec2(j_object["pos_x"].GetFloat(), j_object["pos_y"].GetFloat());
    node->user_size = ImVec2(j_object["user_size_x"].GetFloat(), j_object["user_size_y"].GetFloat());
    node->id = j_object["id"].GetInt();
    this->UpdateFreeId(node->id);
}

void RoR::NodeGraphTool::SaveAsJson()
{
    rapidjson::Document doc(rapidjson::kObjectType);
    auto& j_alloc = doc.GetAllocator();

    // EXPORT NODES

    rapidjson::Value j_xform_nodes(rapidjson::kArrayType);
    rapidjson::Value j_disp_nodes(rapidjson::kArrayType);
    rapidjson::Value j_script_nodes(rapidjson::kArrayType);
    rapidjson::Value j_gen_nodes(rapidjson::kArrayType);
    for (Node* node: m_nodes)
    {
        rapidjson::Value j_data(rapidjson::kObjectType); // Common properties....
        this->NodeToJson(j_data, node, doc);

        switch (node->type) // Specifics...
        {
        case Node::Type::GENERATOR:
            j_data.AddMember("amplitude", static_cast<GeneratorNode*>(node)->amplitude, j_alloc);
            j_data.AddMember("frequency", static_cast<GeneratorNode*>(node)->frequency, j_alloc);
            j_data.AddMember("noise_max", static_cast<GeneratorNode*>(node)->noise_max, j_alloc);
            j_gen_nodes.PushBack(j_data, j_alloc);
            break;

        case Node::Type::TRANSFORM:
            j_xform_nodes.PushBack(j_data, j_alloc);
            break;

        case Node::Type::DISPLAY:
            j_disp_nodes.PushBack(j_data, j_alloc);
            break;

        case Node::Type::SCRIPT:
            j_data.AddMember("source_code", rapidjson::StringRef(static_cast<ScriptNode*>(node)->code_buf), j_alloc);
            j_script_nodes.PushBack(j_data, j_alloc);
            break;

        } // end switch
    }

    // EXPORT LINKS

    rapidjson::Value j_links(rapidjson::kArrayType);
    for (Link* link: m_links)
    {
        rapidjson::Value j_data(rapidjson::kObjectType);
        j_data.AddMember("node_src_id",  link->node_src->id,  j_alloc);
        j_data.AddMember("node_dst_id",  link->node_dst->id,  j_alloc);
        j_data.AddMember("slot_src",     link->buff_src->slot, j_alloc);
        j_data.AddMember("slot_dst",     link->slot_dst,      j_alloc);
        j_links.PushBack(j_data, j_alloc);
    }

    // COMBINE

    doc.AddMember("generator_nodes", j_gen_nodes,    j_alloc);
    doc.AddMember("transform_nodes", j_xform_nodes,  j_alloc);
    doc.AddMember("script_nodes",    j_script_nodes, j_alloc);
    doc.AddMember("display_nodes",   j_disp_nodes,   j_alloc);
    doc.AddMember("links",           j_links,        j_alloc);

    // SAVE FILE

    FILE* file = nullptr;
    errno_t fopen_result = 0;
#ifdef _WIN32
    // Binary mode recommended by RapidJSON tutorial: http://rapidjson.org/md_doc_stream.html#FileWriteStream
    fopen_result = fopen_s(&file, m_filename, "wb");
#else
    fopen_result = fopen_s(&file, m_filename, "w");
#endif
    if ((fopen_result != 0) || (file == nullptr))
    {
        std::stringstream msg;
        msg << "[RoR|RigEditor] Failed to save JSON project file (path: "<< m_filename << ")";
        if (fopen_result != 0)
        {
            msg<<" Tech details: function [fopen_s()] returned ["<<fopen_result<<"]";
        }
        this->AddMessage(msg.str().c_str());
        return; 
    }

    char* buffer = new char[100000]; // 100kb
    rapidjson::FileWriteStream j_out_stream(file, buffer, sizeof(buffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> j_writer(j_out_stream);
    doc.Accept(j_writer);
    fclose(file);
    delete buffer;
}

void RoR::NodeGraphTool::LoadFromJson()
{
    this->ClearAll();

#ifdef _WIN32
    // Binary mode recommended by RapidJSON tutorial: http://rapidjson.org/md_doc_stream.html#FileReadStream
    FILE* fp = fopen(m_filename, "rb");
#else
    FILE* fp = fopen(m_filename, "r");
#endif
    if (fp == nullptr)
    {
        this->AddMessage("failed to open file '%s'", m_filename);
        return;
    }

    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document d;
    d.ParseStream(is);
    fclose(fp);

    // IMPORT NODES
    std::unordered_map<int, Node*> lookup;

    const char* field = "generator_nodes";
    if (d.HasMember(field) && d[field].IsArray())
    {
        auto& j_gen = d[field];
        rapidjson::Value::ConstValueIterator itor = j_gen.Begin();
        rapidjson::Value::ConstValueIterator endi = j_gen.End();
        for (; itor != endi; ++itor)
        {
            GeneratorNode* node = new GeneratorNode(this, ImVec2());
            this->JsonToNode(node, *itor);
            node->amplitude = (*itor)["amplitude"].GetFloat();
            node->frequency = (*itor)["frequency"].GetFloat();
            node->noise_max = (*itor)["noise_max"].GetInt();
            m_nodes.push_back(node);
            lookup.insert(std::make_pair(node->id, node));
        }
    }
    field = "display_nodes";
    if (d.HasMember(field) && d[field].IsArray())
    {
        auto& j_array = d[field];
        rapidjson::Value::ConstValueIterator itor = j_array.Begin();
        rapidjson::Value::ConstValueIterator endi = j_array.End();
        for (; itor != endi; ++itor)
        {
            DisplayNode* node = new DisplayNode(this, ImVec2());
            this->JsonToNode(node, *itor);
            m_nodes.push_back(node);
            lookup.insert(std::make_pair(node->id, node));
        }
    }
    field = "transform_nodes";
    if (d.HasMember(field) && d[field].IsArray())
    {
        auto& j_array = d[field];
        rapidjson::Value::ConstValueIterator itor = j_array.Begin();
        rapidjson::Value::ConstValueIterator endi = j_array.End();
        for (; itor != endi; ++itor)
        {
            TransformNode* node = new TransformNode(this, ImVec2());
            this->JsonToNode(node, *itor);
            m_nodes.push_back(node);
            lookup.insert(std::make_pair(node->id, node));
        }
    }
    field = "script_nodes";
    if (d.HasMember(field) && d[field].IsArray())
    {
        auto& j_array = d[field];
        rapidjson::Value::ConstValueIterator itor = j_array.Begin();
        rapidjson::Value::ConstValueIterator endi = j_array.End();
        for (; itor != endi; ++itor)
        {
            ScriptNode* node = new ScriptNode(this, ImVec2());
            this->JsonToNode(node, *itor);
            strncpy(node->code_buf, (*itor)["source_code"].GetString(), IM_ARRAYSIZE(node->code_buf));
            m_nodes.push_back(node);
            lookup.insert(std::make_pair(node->id, node));
        }
    }

    // IMPORT LINKS

    field = "links";
    if (d.HasMember(field) && d[field].IsArray())
    {
        auto& j_array = d[field];
        rapidjson::Value::ConstValueIterator itor = j_array.Begin();
        rapidjson::Value::ConstValueIterator endi = j_array.End();
        for (; itor != endi; ++itor)
        {
            Link* link = new Link();
            lookup.find((*itor)["node_src_id"].GetInt())->second->BindSrc(link, (*itor)["slot_src"].GetInt());
            lookup.find((*itor)["node_dst_id"].GetInt())->second->BindDst(link, (*itor)["slot_dst"].GetInt());
            m_links.push_back(link);
        }
    }
}

void RoR::NodeGraphTool::ClearAll()
{
    for (Link* link: m_links)
        this->DeleteLink(link);
    m_links.clear();

    for (Node* n: m_nodes)
        delete n;
    m_nodes.clear();
}

template<typename N> void DeleteNodeFromVector(std::vector<N*>& vec, RoR::NodeGraphTool::Node* node)
{
    for (auto itor = vec.begin(); itor != vec.end(); ++itor)
    {
        if (*itor == node)
        {
            delete node;
            vec.erase(itor);
            return;
        }
    }
}

void RoR::NodeGraphTool::DeleteNode(Node* node)
{
    auto itor = m_nodes.begin();
    auto endi = m_nodes.end();
    for (; itor != endi; ++itor)
    {
        if (*itor == node)
        {
            m_nodes.erase(itor);
            delete node;
            return;
        }
    }
}

void RoR::NodeGraphTool::DetachAndDeleteNode(Node* node)
{
    // Disconnect inputs
    for (int i = 0; i<node->num_inputs; ++i)
    {
        Link* found_link = this->FindLinkByDestination(node, i);
        if (found_link != nullptr)
        {
            node->DetachLink(found_link);
            this->DeleteLink(found_link); // De-allocates link
        }
    }
    // Disconnect outputs
    for (int i=0; i< node->num_outputs; ++i)
    {
        Link* found_link = this->FindLinkBySource(node, i);
        if (found_link != nullptr)
        {
            node->DetachLink(found_link);
            this->DeleteLink(found_link); // De-allocates link
        }
    }
    // Erase the node
    this->DeleteNode(node);
}

// -------------------------------- Buffer object -----------------------------------

void RoR::NodeGraphTool::Buffer::CopyKeepOffset(Buffer* src) // Copies source buffer as-is, including the offset; fastest
{
    memcpy(this->data, src->data, Buffer::SIZE*sizeof(float));
    this->offset = src->offset;
}

void RoR::NodeGraphTool::Buffer::CopyResetOffset(Buffer* src) // Copies source buffer with 0-offset
{
    int src_upper_size = (Buffer::SIZE - src->offset);
    memcpy(this->data, src->data + src->offset, src_upper_size*sizeof(float)); // Upper portion
    memcpy(this->data + src_upper_size, src->data, src->offset*sizeof(float)); // Lower portion
    this->offset = 0; // Reset offset
}

void RoR::NodeGraphTool::Buffer::Fill(const float* const src, int offset, int len) // offset: default=0; len: default=Buffer::SIZE
{
    memcpy(this->data + offset, src, len);
}

// -------------------------------- Display node -----------------------------------

RoR::NodeGraphTool::DisplayNode::DisplayNode(NodeGraphTool* nodegraph, ImVec2 _pos):
    Node(nodegraph, Type::DISPLAY, _pos), link_in(nullptr)
{
    num_outputs = 0;
    num_inputs = 1;
    user_size = ImVec2(250.f, 85.f);
    done = false; // Irrelevant for this node type - no outputs
    plot_extent = 1.5f;
}

static const float DUMMY_PLOT[] = {0,0,0,0,0};

void RoR::NodeGraphTool::DisplayNode::Draw()
{
    graph->DrawNodeBegin(this);

    const float* data_ptr = DUMMY_PLOT;;
    int data_length = IM_ARRAYSIZE(DUMMY_PLOT);
    int data_offset = 0;
    int stride = sizeof(float);
    const char* title = "~~ disconnected ~~";
    if (this->link_in != nullptr)
    {
        data_ptr    = this->link_in->buff_src->data;
        data_offset = this->link_in->buff_src->offset;
        stride = sizeof(float);
        title = "";
        data_length = Buffer::SIZE;
    }
    ImGui::PlotLines("", data_ptr, data_length, data_offset, title, -this->plot_extent, this->plot_extent, this->user_size, stride);
    ImGui::InputFloat("Scale", &this->plot_extent);

    graph->DrawNodeFinalize(this);
}

void RoR::NodeGraphTool::DisplayNode::DetachLink(Link* link)
{
    assert (link->node_src != this); // Check discrepancy - this node has no outputs!

    if (link->node_dst == this)
    {
        assert(this->link_in == link); // Check discrepancy
        this->link_in = nullptr; // Clear local link
        link->node_dst = nullptr;
        link->slot_dst = -1;
    }
}

// -------------------------------- Generator node -----------------------------------

void RoR::NodeGraphTool::GeneratorNode::Draw()
{
    this->graph->DrawNodeBegin(this);

    ImGui::Text("Sine generator");

    float freq = this->frequency;
    if (ImGui::InputFloat("Freq", &freq))
    {
        this->frequency = freq;
    }

    float ampl = this->amplitude;
    if (ImGui::InputFloat("Ampl", &ampl))
    {
        this->amplitude = ampl;
    }

    int noise = this->noise_max;
    if (ImGui::InputInt("Noise", &noise))
    {
        this->noise_max = noise;
    }

    this->graph->DrawNodeFinalize(this);
}

void RoR::NodeGraphTool::GeneratorNode::DetachLink(Link* link)
{
    assert(link->node_dst != this); // discrepancy - no inputs in this node

    if (link->node_src == this)
    {
        assert(link->buff_src == &this->buffer_out); // check discrepancy
        link->buff_src = nullptr;
        link->node_src = nullptr;
    }
}

void RoR::NodeGraphTool::GeneratorNode::BindSrc(Link* link, int slot)
{
    assert(slot == 0); // Check invalid input
    if (slot == 0)
    {
        link->node_src = this;
        link->buff_src = &buffer_out;
    }
}

// -------------------------------- Reading node -----------------------------------

void RoR::NodeGraphTool::ReadingNode::BindSrc(Link* link, int slot)
{
    switch (slot)
    {
    case 0:    link->buff_src = &buffer_x;    link->node_src = this;     return;
    case 1:    link->buff_src = &buffer_y;    link->node_src = this;     return;
    case 2:    link->buff_src = &buffer_z;    link->node_src = this;     return;
    default: assert(false && "ReadingNode::BindSrc(): invalid slot index");
    }
}

void RoR::NodeGraphTool::ReadingNode::DetachLink(Link* link)
{
    assert(link->node_dst != this); // Check discrepancy - this node has no inputs

    if (link->node_src == this)
    {
        assert (link->buff_src == &buffer_x || link->buff_src == &buffer_y || link->buff_src == &buffer_z); // check discrepancy
        link->buff_src = nullptr;
        link->node_src = nullptr;
    }
}

void RoR::NodeGraphTool::ReadingNode::Draw()
{
    this->graph->DrawNodeBegin(this);
    ImGui::Text("SoftBody reading");
    ImGui::InputInt("Node", &softbody_node_id);
    this->graph->DrawNodeFinalize(this);
}

// -------------------------------- Script node -----------------------------------

const char* EXAMPLE_CODE =
    "// Static variables here"     "\n"
    ""                             "\n"
    "// Update func. (mandatory)"  "\n"
    "void step() {"                "\n"
    "    // Pass-thru"             "\n"
    "    Write(0,Read(0,0));"      "\n"
    "}";

RoR::NodeGraphTool::ScriptNode::ScriptNode(NodeGraphTool* _graph, ImVec2 _pos):
    Node(_graph, Type::SCRIPT, _pos), 
    script_func(nullptr), script_engine(nullptr), script_context(nullptr), enabled(false),
    outputs{{0},{1},{2},{3},{4},{5},{6},{7},{8}} // C++11 mandatory :)
{
    num_outputs = 9;
    num_inputs = 9;
    memset(code_buf, 0, sizeof(code_buf));
    sprintf(code_buf, EXAMPLE_CODE);
    memset(inputs, 0, sizeof(inputs));
    user_size = ImVec2(250, 200);
    snprintf(node_name, 10, "Node %d", id);
    this->InitScripting();
}

void RoR::NodeGraphTool::ScriptNode::InitScripting()
{
    script_engine = AngelScript::asCreateScriptEngine(ANGELSCRIPT_VERSION);
    if (script_engine == nullptr)
    {
        graph->AddMessage("%s: failed to create scripting engine", node_name);
        return;
    }

    int result = script_engine->SetMessageCallback(AngelScript::asMETHOD(NodeGraphTool, ScriptMessageCallback), this, AngelScript::asCALL_THISCALL);
    if (result < 0)
    {
        graph->AddMessage("%s: failed to register message callback function, res: %d", node_name, result);
        return;
    }

    result = script_engine->RegisterGlobalFunction("void Write(int, float)", AngelScript::asMETHOD(RoR::NodeGraphTool::ScriptNode, Write), AngelScript::asCALL_THISCALL_ASGLOBAL, this);
    if (result < 0)
    {
        graph->AddMessage("%s: failed to register function `Write`, res: %d", node_name, result);
        return;
    }

    result = script_engine->RegisterGlobalFunction("float Read(int, int)", AngelScript::asMETHOD(RoR::NodeGraphTool::ScriptNode, Read), AngelScript::asCALL_THISCALL_ASGLOBAL, this);
    if (result < 0)
    {
        graph->AddMessage("%s: failed to register function `Read`, res: %d", node_name, result);
        return;
    }
}

void RoR::NodeGraphTool::ScriptNode::Apply()
{
    AngelScript::asIScriptModule* module = script_engine->GetModule(nullptr, AngelScript::asGM_ALWAYS_CREATE);
    if (module == nullptr)
    {
        graph->AddMessage("%s: Failed to create module", node_name);
        module->Discard();
        return;
    }

    //char sourcecode[1100];
    //snprintf(sourcecode, 1100, "void main() {\n%s\n}", code_buf);
    int result = module->AddScriptSection("body", code_buf, strlen(code_buf));
    if (result < 0)
    {
        graph->AddMessage("%s: failed to `AddScriptSection()`, res: %d", node_name, result);
        module->Discard();
        return;
    }

    result = module->Build();
    if (result < 0)
    {
        graph->AddMessage("%s: failed to `Build()`, res: %d", node_name, result);
        module->Discard();
        return;
    }

    script_func = module->GetFunctionByDecl("void step()");
    if (script_func == nullptr)
    {
        graph->AddMessage("%s: failed to `GetFunctionByDecl()`", node_name);
        module->Discard();
        return;
    }

    script_context = script_engine->CreateContext();
    if (script_context == nullptr)
    {
        graph->AddMessage("%s: failed to `CreateContext()`", node_name);
        module->Discard();
        return;
    }

    enabled = true;
}

        // Script functions
float RoR::NodeGraphTool::ScriptNode::Read(int slot, int offset)
{
    if (slot < 0 || slot > (num_inputs - 1) || inputs[slot] == nullptr)
        return 0.f;

    if (offset > 0 || offset < -(Buffer::SIZE - 1))
        return 0.f;

    Buffer* buff_src = inputs[slot]->buff_src;
    int pos = (buff_src->offset + offset);
    pos = (pos < 0) ? (pos + Buffer::SIZE) : pos;
    return buff_src->data[pos];
}

void RoR::NodeGraphTool::ScriptNode::Write(int slot, float val)
{
    this->outputs[slot].data[this->outputs[slot].offset] = val;
}

bool RoR::NodeGraphTool::ScriptNode::Process()
{
    if (! enabled)
    {
        this->done = true;
        return true;
    }

    bool ready = true; // If completely disconnected, we're good to go. Otherwise, all inputs must be ready.
    for (int i=0; i<num_inputs; ++i)
    {
        if ((inputs[i] != nullptr) && (! inputs[i]->node_src->done))
            ready = false;
    }

    if (! ready)
        return false;

    int prep_result = script_context->Prepare(script_func);
    if (prep_result < 0)
    {
        graph->AddMessage("%s: failed to `Prepare()`, res: %d", node_name, prep_result);
        script_engine->ReturnContext(script_context);
        script_context = nullptr;
        enabled = false;
        done = true;
        return true;
    }

    int exec_result = script_context->Execute();
    if (exec_result != AngelScript::asEXECUTION_FINISHED)
    {
        graph->AddMessage("%s: failed to `Execute()`, res: %d", node_name, exec_result);
        script_engine->ReturnContext(script_context);
        script_context = nullptr;
        enabled = false;
        done = true;
    }

    for (int i=0; i<num_outputs; ++i)
    {
        outputs[i].Step();
    }

    done = true;
    return true;
}

void RoR::NodeGraphTool::ScriptNode::BindSrc(Link* link, int slot)
{
    assert(slot >= 0 && slot < num_outputs); // Check input
    assert(link != nullptr); // Check input
    link->node_src = this;
    link->buff_src = &outputs[slot];
}

void RoR::NodeGraphTool::ScriptNode::BindDst(Link* link, int slot)
{
    assert(slot >= 0 && slot < num_inputs); // Check input
    assert(link != nullptr); // Check input
    inputs[slot] = link;
    link->node_dst = this;
    link->slot_dst = slot;
}

void RoR::NodeGraphTool::ScriptNode::DetachLink(Link* link)
{
    if (link->node_dst == this)
    {
        assert(inputs[link->slot_dst] == link); // Check discrepancy
        inputs[link->slot_dst] = nullptr;
        link->node_dst = nullptr;
        link->slot_dst = -1;
    }
    else if (link->node_src == this)
    {
        assert((link->buff_src != nullptr)); // Check discrepancy
        link->buff_src = nullptr;
        link->node_src = nullptr;
    }
    else assert(false && "ScriptNode::DetachLink() called on unrelated node");
}

void RoR::NodeGraphTool::ScriptNode::Draw()
{
    graph->DrawNodeBegin(this);
    const int flags = ImGuiInputTextFlags_AllowTabInput;
    const ImVec2 size = this->user_size;
    ImGui::Text((this->enabled)? "Enabled" : "Disabled");
    ImGui::SameLine();
    if (ImGui::SmallButton("Update"))
    {
        this->Apply();
    }
    ImGui::InputTextMultiline("##source", this->code_buf, IM_ARRAYSIZE(this->code_buf), size, flags);
    graph->DrawNodeFinalize(this);
}

// -------------------------------- Transform node -----------------------------------

RoR::NodeGraphTool::TransformNode::TransformNode(NodeGraphTool* _graph, ImVec2 _pos):
    Node(_graph, Type::TRANSFORM, _pos), buffer_out(0), link_in(nullptr)
{
    num_inputs = 1;
    num_outputs = 1;
    user_size.x = 200.f;
}

void RoR::NodeGraphTool::TransformNode::Draw()
{
    graph->DrawNodeBegin(this);
    ImGui::PushItemWidth(this->user_size.x);
    ImGui::Text("Transform");
    ImGui::Text("No options");
    graph->DrawNodeFinalize(this);
}

void RoR::NodeGraphTool::TransformNode::DetachLink(Link* link)
{
    if (link->node_dst == this)
    {
        assert(this->link_in == link); // Check discrepancy
        link->node_dst = nullptr;
        link->slot_dst = -1;
        this->link_in = nullptr;
    }
    else if (link->node_src == this)
    {
        assert(&this->buffer_out == link->buff_src); // Check discrepancy
        link->node_src = nullptr;
        link->buff_src = nullptr;
    }
    else assert(false && "TransformNode::DetachLink() called on unrelated node");
}


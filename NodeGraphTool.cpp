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
        m_hovered_slot_output(-1)
{
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
        if (link->node_src == node && link->slot_src == slot)
            return link;
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
    for (GeneratorNode* gen_node: m_gen_nodes)
    {
        gen_node->elapsed += 0.002f;
        gen_node->data_offset = (gen_node->data_offset + 1) % Node::BUF_SIZE;
        gen_node->data_buffer[gen_node->data_offset] = cosf((gen_node->elapsed / 2.f) * 3.14f * gen_node->frequency) * gen_node->amplitude;
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
    ImVec2 p1 = offset + link->node_src->GetOutputSlotPos(link->slot_src);
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
                this->NodeLinkChanged(link, false); // Link removed
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

RoR::NodeGraphTool::Link* RoR::NodeGraphTool::AddLink(Node* src, Node* dst, int src_slot, int dst_slot) ///< creates new link or fetches existing unused one
{
    m_links.push_back(new Link(src, dst, src_slot, dst_slot));
    return m_links.back();
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
}

void RoR::NodeGraphTool::NodeLinkChanged(Link* link, bool added)
{
    link->node_dst->links_in[link->slot_dst] = (added) ? link : nullptr;
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
                m_link_mouse_src->node_src = m_hovered_slot_node;
                m_link_mouse_src->slot_src = static_cast<size_t>(m_hovered_slot_output);
                this->NodeLinkChanged(m_link_mouse_src, true); // Link added
            }
            else
            {
                this->DeleteLink(m_link_mouse_src);
            }
            m_link_mouse_src = nullptr;
        }
        else if (m_link_mouse_dst != nullptr)
        {
            if (m_hovered_slot_node != nullptr && m_hovered_slot_input != -1)
            {
                m_link_mouse_dst->node_dst = m_hovered_slot_node;
                m_link_mouse_dst->slot_dst = static_cast<size_t>(m_hovered_slot_input);
                this->NodeLinkChanged(m_link_mouse_dst, true); // Link added
            }
            else
            {
                this->DeleteLink(m_link_mouse_dst);
            }
            m_link_mouse_dst = nullptr;
        }
    }

    // Draw grid
    this->DrawGrid();

    // Draw links
    drawlist->ChannelsSetCurrent(0);
    for (Link* link: m_links)
    {
        this->DrawLink(link);
    }

    for (GeneratorNode* node: m_gen_nodes)
    {
        this->DrawNodeBegin(node);

        float freq = node->frequency;
        if (ImGui::InputFloat("Freq", &freq))
        {
            node->frequency = freq;
        }

        float ampl = node->amplitude;
        if (ImGui::InputFloat("Ampl", &ampl))
        {
            node->amplitude = ampl;
        }

        this->DrawNodeFinalize(node);
    }

    static const float DUMMY_PLOT[] = {0,0,0,0,0};

    for (DisplayNode* node: m_disp_nodes)
    {
        this->DrawNodeBegin(node);

        const float* data_ptr = DUMMY_PLOT;;
        int data_length = IM_ARRAYSIZE(DUMMY_PLOT);
        int data_offset = 0;
        int stride = sizeof(float);
        const char* title = "~~disconnected~~";
        if (node->links_in[0] != nullptr)
        {
            Node* node_src = node->links_in[0]->node_src;

            data_ptr = node_src->data_buffer;
            data_offset = node_src->data_offset;
            stride = sizeof(float);
            title = "";
            data_length = Node::BUF_SIZE;
        }
        const float PLOT_MIN = -1.7f;//std::numeric_limits<float>::min();
        const float PLOT_MAX = +1.7f; //std::numeric_limits<float>::max();
        ImGui::PlotLines("", data_ptr, data_length, data_offset, title, PLOT_MIN, PLOT_MAX, node->user_size, stride);

        this->DrawNodeFinalize(node);
    }

    for (TransformNode* node: m_xform_nodes)
    {
        this->DrawNodeBegin(node);

        int method_id = static_cast<int>(node->method);
        const char* mode_options[] = { "~ None ~", "FIR - direct", "FIR - adaptive" };
        if (ImGui::Combo("Method", &method_id, mode_options, 3))
        {
            node->method = static_cast<TransformNode::Method>(method_id);
        }

        if (node->method == TransformNode::Method::FIR_DIRECT)
        {
            ImGui::InputText("Coefs", node->input_field, sizeof(node->input_field));
        }

        this->DrawNodeFinalize(node);
    }

    for (ScriptNode* node: m_script_nodes)
    {
        this->DrawNodeBegin(node);
        const int flags = ImGuiInputTextFlags_AllowTabInput;
        const ImVec2 size = node->user_size;
        ImGui::Text((node->enabled)? "Enabled" : "Disabled");
        ImGui::SameLine();
        if (ImGui::SmallButton("Update"))
        {
            node->Apply();
        }
        ImGui::InputTextMultiline("##source", node->code_buf, IM_ARRAYSIZE(node->code_buf), size, flags);
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
        if (ImGui::MenuItem("Generator"))
        {
            m_gen_nodes.push_back(new GeneratorNode(scene_pos));
        }
        if (ImGui::MenuItem("Display"))
        {
            m_disp_nodes.push_back(new DisplayNode(scene_pos));
        }
        if (ImGui::MenuItem("Script"))
        {
            m_script_nodes.push_back(new ScriptNode(this, scene_pos));
            m_script_nodes.back()->InitScripting();
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

/* TODO
void Sigpack_FIR_Direct(RoR::NodeGraphTool::TransformNode* node, float* src_data)
{
    sp::FIR_filt<float, float, float> G;
    arma::fvec b = node->input_field;
    G.set_coeffs(b);

    arma::fvec input_vec(2000);
    for (int i = 0; i < 2000; ++i)
        input_vec(i) = node->data_buffer[i];

}*/

void RoR::NodeGraphTool::CalcGraph()
{
    // Reset states
    for (TransformNode* n: m_xform_nodes)
    {
        n->done = false;
    }
    for (ScriptNode* n: m_script_nodes)
    {
        n->done = false;
    }

    bool keep_working = false;
    do
    {
        for (TransformNode* n: m_xform_nodes)
        {
            if (n->done)
                continue; // Next, please

            if (n->links_in[0] == nullptr)
            {
                n->done = true; // Nothing to transform here
                continue;
            }

            Node* node_src = n->links_in[0]->node_src;
            int slot_src = n->links_in[0]->slot_src;
            if (! node_src->done)
            {
                keep_working = true;
                continue; // Not ready for processing yet
            }

            // Get data
            memcpy(n->data_buffer, node_src->data_buffer, 2000*sizeof(float));
            n->data_offset = node_src->data_offset;
            n->done = true;
        }

        for (ScriptNode* n: m_script_nodes)
        {
            if (n->done)
                continue; // Next, please

            n->Exec();
        }
    }
    while (keep_working);
}

void RoR::NodeGraphTool::ScriptMessageCallback(const asSMessageInfo *msg, void *param)
{
    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING ) 
        type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION ) 
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

// -------------------------------- Nodes -----------------------------------

RoR::NodeGraphTool::ScriptNode::ScriptNode(NodeGraphTool* _nodegraph, ImVec2 _pos):
    Node(), nodegraph(_nodegraph), script_func(nullptr), script_engine(nullptr), script_context(nullptr)
{
    num_outputs = 1;
    num_inputs = 4;
    memset(code_buf, 0, sizeof(code_buf));
    done = false;
    type = Type::SCRIPT;
    pos = _pos;
    user_size = ImVec2(200, 100);
    snprintf(node_name, 10, "Node %d", id);
    enabled = false;
}

void RoR::NodeGraphTool::ScriptNode::InitScripting()
{
    script_engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    if (script_engine == nullptr)
    {
        nodegraph->AddMessage("%s: failed to create scripting engine", node_name);
        return;
    }

    int result = script_engine->SetMessageCallback(asMETHOD(NodeGraphTool, ScriptMessageCallback), this, asCALL_THISCALL);
    if (result < 0)
    {
        nodegraph->AddMessage("%s: failed to register message callback function, res: %d", node_name, result);
        return;
    }

    result = script_engine->RegisterGlobalFunction("void Write(float)", asMETHOD(RoR::NodeGraphTool::ScriptNode, Write), asCALL_THISCALL_ASGLOBAL, this);
    if (result < 0)
    {
        nodegraph->AddMessage("%s: failed to register function `Write`, res: %d", node_name, result);
        return;
    }

    result = script_engine->RegisterGlobalFunction("float Read(int, int)", asMETHOD(RoR::NodeGraphTool::ScriptNode, Read), asCALL_THISCALL_ASGLOBAL, this);
    if (result < 0)
    {
        nodegraph->AddMessage("%s: failed to register function `Read`, res: %d", node_name, result);
        return;
    }
}

void RoR::NodeGraphTool::ScriptNode::Apply()
{
    asIScriptModule* module = script_engine->GetModule(nullptr, asGM_ALWAYS_CREATE);
    if (module == nullptr)
    {
        nodegraph->AddMessage("%s: Failed to create module", node_name);
        module->Discard();
        return;
    }

    char sourcecode[1100];
    snprintf(sourcecode, 1100, "void main() {\n%s\n}", code_buf);
    int result = module->AddScriptSection("body", sourcecode, strlen(sourcecode));
    if (result < 0)
    {
        nodegraph->AddMessage("%s: failed to `AddScriptSection()`, res: %d", node_name, result);
        module->Discard();
        return;
    }

    result = module->Build();
    if (result < 0)
    {
        nodegraph->AddMessage("%s: failed to `Build()`, res: %d", node_name, result);
        module->Discard();
        return;
    }

    script_func = module->GetFunctionByDecl("void main()");
    if (script_func == nullptr)
    {
        nodegraph->AddMessage("%s: failed to `GetFunctionByDecl()`", node_name);
        module->Discard();
        return;
    }

    script_context = script_engine->CreateContext();
    if (script_context == nullptr)
    {
        nodegraph->AddMessage("%s: failed to `CreateContext()`", node_name);
        module->Discard();
        return;
    }

    enabled = true;
}

        // Script functions
float RoR::NodeGraphTool::ScriptNode::Read(int slot, int offset)
{
    if (slot < 0 || slot > (num_inputs - 1) || links_in[slot] == nullptr)
        return 0.f;

    if (offset > 0 || offset < -(Node::BUF_SIZE - 1))
        return 0.f;

    Node* src_node = links_in[slot]->node_src;
    int pos = (src_node->data_offset + offset);
    pos = (pos < 0) ? (pos + Node::BUF_SIZE) : pos;
    return src_node->data_buffer[pos];
}

void RoR::NodeGraphTool::ScriptNode::Write(float val)
{
    data_buffer[data_offset] = val;
}

void RoR::NodeGraphTool::ScriptNode::Exec()
{
    if (! enabled)
        return;

    bool ready = true; // If completely disconnected, we're good to go. Otherwise, all inputs must be ready.
    for (int i=0; i<num_inputs; ++i)
    {
        if ((links_in[i] != nullptr) && (! links_in[i]->node_src->done))
            ready = false;
    }

    if (! ready)
        return;

    int prep_result = script_context->Prepare(script_func);
    if (prep_result < 0)
    {
        nodegraph->AddMessage("%s: failed to `Prepare()`, res: %d", node_name, prep_result);
        script_engine->ReturnContext(script_context);
        script_context = nullptr;
        enabled = false;
        return;
    }

    int exec_result = script_context->Execute();
    if (exec_result != asEXECUTION_FINISHED)
    {
        nodegraph->AddMessage("%s: failed to `Execute()`, res: %d", node_name, exec_result);
        script_engine->ReturnContext(script_context);
        script_context = nullptr;
        enabled = false;
    }

    data_offset = (data_offset+1)%Node::BUF_SIZE;
    done = true;
}


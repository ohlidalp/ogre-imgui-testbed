
#pragma once

#include "ImguiManager.h"

    // ############# testing dummy #############
struct FakeTruck
{
    static const size_t NUM_NODES = 100;

    FakeTruck() { memset(nodes_x, 0, sizeof(nodes_x)); }

    float nodes_x[NUM_NODES];
};

extern FakeTruck G_fake_truck;
    // ############# END dummy #############


namespace RoR {

struct Vec3 { float x, y, z; };

#define RoR_ARRAYSIZE(_ARR)  (sizeof(_ARR)/sizeof(*_ARR))

class NodeGraphTool
{
public:
    struct Style
    {
        ImU32 color_grid;
        float grid_line_width;
        float grid_size;
        ImU32 color_node;
        ImU32 color_node_frame;
        ImU32 color_node_active;
        ImU32 color_node_frame_active;
        ImU32 color_node_hovered;
        ImU32 color_node_frame_hovered;
        float node_rounding;
        ImVec2 node_window_padding;
        ImU32 color_input_slot;
        ImU32 color_input_slot_hover;
        ImU32 color_output_slot;
        ImU32 color_output_slot_hover;
        float node_slots_radius;
        ImU32 color_link;
        float link_line_width;
        ImVec2 slot_hoverbox_extent;

        Style();
    };

    NodeGraphTool();

    void Draw();
    void PhysicsTick();
    void CalcGraph();

private:

    struct Node; // Forward
    struct SourceNode;
    struct DisplayNode;

    struct Link
    {
        Link(): node_src(nullptr), node_dst(nullptr), slot_src(0), slot_dst(0) {}

        Link(Node* src_n, Node* dst_n, size_t src_s, size_t dst_s): node_src(src_n), node_dst(dst_n), slot_src(src_s), slot_dst(dst_s) {}

        Node* node_src;
        Node* node_dst;
        size_t slot_src;
        size_t slot_dst;
        bool processed;
    };

    struct Node
    {
        enum class Type { INVALID, SOURCE, TRANSFORM };

        Node(): num_inputs(0), num_outputs(-1), pos(100.f, 100.f), size(150.f, 100.f), type(Type::INVALID)
        {
            static int new_id = 1;
            id = new_id;
            ++new_id;
        }

        size_t num_inputs;
        size_t num_outputs;
        ImVec2 pos;
        ImVec2 size;
        int id;
        Type type;

        inline ImVec2 GetInputSlotPos(size_t slot_idx)  { return ImVec2(pos.x,          pos.y + (size.y * (static_cast<float>(slot_idx+1) / static_cast<float>(num_inputs+1)))); }
        inline ImVec2 GetOutputSlotPos(size_t slot_idx) { return ImVec2(pos.x + size.x, pos.y + (size.y * (static_cast<float>(slot_idx+1) / static_cast<float>(num_outputs+1)))); }
    };

    /// reports XYZ position of node in world space
    /// Inputs: none
    /// Outputs(3): X position, Y position, Z position
    struct SourceNode: public Node
    {
        SourceNode()
        {
            num_inputs = 0;
            num_outputs = 3;
            softbody_node_id = -1;
            data_offset = 0;
            memset(data_buffer, 0, sizeof(data_buffer));
            size = ImVec2(250.f, 85.f);
            type = Type::SOURCE;
        }

        int softbody_node_id; // -1 means 'none'
        Vec3 data_buffer[2000]; // 1 second worth of data
        int data_offset;

        inline void PushData(Vec3 entry) { data_buffer[data_offset] = entry; data_offset = (data_offset+1)%2000; }
    };

    struct TransformNode: public Node
    {
        TransformNode()
        {
            num_inputs = 1;
            num_outputs = 1;
            data_offset = 0;
            memset(data_buffer, 0, sizeof(data_buffer));
            result_ready = false;
            result_val = 0.f;
            type = Type::TRANSFORM;
        }

        Vec3 data_buffer[2000];
        int data_offset;
        bool result_ready;
        float result_val;
    };

    inline bool     IsInside (ImVec2 min, ImVec2 max, ImVec2 point) const                { return ((point.x > min.x) && (point.y > min.y)) && ((point.x < max.x) && (point.y < max.y)); }
    inline bool     IsLinkDragInProgress () const                                        { return (m_link_mouse_src != nullptr) || (m_link_mouse_dst != nullptr); }
    inline bool     IsRectHovered(ImVec2 min, ImVec2 max) const                          { return this->IsInside(min, max, m_nodegraph_mouse_pos); }
    inline void     DrawInputSlot (Node* node, const size_t index)                       { this->DrawSlotUni(node, index, true); }
    inline void     DrawOutputSlot (Node* node, const size_t index)                      { this->DrawSlotUni(node, index, false); }
    void            DrawSlotUni (Node* node, const size_t index, const bool input);
    Link*           AddLink (Node* src, Node* dst, size_t src_slot, size_t dst_slot);     ///< creates new link or fetches existing unused one
    Link*           FindLinkByDestination (Node* node, size_t slot);
    Link*           FindLinkBySource (Node* node, size_t slot);
    void            DrawNodeGraphPane ();
    void            DrawGrid ();
    void            DrawLink (Link& link);

    inline bool IsSlotHovered(ImVec2 center_pos) const
    {
        ImVec2 min = center_pos - m_style.slot_hoverbox_extent;
        ImVec2 max = center_pos + m_style.slot_hoverbox_extent;
        return this->IsInside(min, max, m_nodegraph_mouse_pos);
    }

    std::vector<SourceNode*> m_reading_nodes;
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

} // namespace RoR

#pragma once

#include "ImguiManager.h"

#include <angelscript.h>

#include <list>
#include <string>
#include <vector>

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
    static const int MAX_SLOTS = 8;

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

    struct Node; // Forward

    struct Link
    {
        Link(): node_src(nullptr), node_dst(nullptr), slot_src(-1), slot_dst(-1) {}

        Link(Node* src_n, Node* dst_n, int src_s, int dst_s): node_src(src_n), node_dst(dst_n), slot_src(src_s), slot_dst(dst_s) {}

        Node* node_src;
        Node* node_dst;
        int slot_src;
        int slot_dst;
    };

    struct Node
    {
        enum class Type { INVALID, GENERATOR, TRANSFORM, SCRIPT, DISPLAY };
        static const int BUF_SIZE = 2000; // Physics tick is 2Khz

        Node(): num_inputs(0), num_outputs(0), pos(100.f, 100.f), type(Type::INVALID), data_offset(0)
        {
            static int new_id = 1;
            id = new_id;
            ++new_id;
            memset(links_in, 0, sizeof(links_in));
            memset(data_buffer, 0, sizeof(float)*Node::BUF_SIZE);
        }

        size_t num_inputs;
        size_t num_outputs;
        ImVec2 pos;
        ImVec2 draw_rect_min; // Updated by `DrawNodeBegin()`
        ImVec2 calc_size;
        ImVec2 user_size;
        int id;
        Type type;
        bool done; // Are data ready in this processing step?
        Link* links_in[MAX_SLOTS];
        float data_buffer[Node::BUF_SIZE];
        int data_offset;

        inline ImVec2 GetInputSlotPos(size_t slot_idx)  { return ImVec2(pos.x,               pos.y + (calc_size.y * (static_cast<float>(slot_idx+1) / static_cast<float>(num_inputs+1)))); }
        inline ImVec2 GetOutputSlotPos(size_t slot_idx) { return ImVec2(pos.x + calc_size.x, pos.y + (calc_size.y * (static_cast<float>(slot_idx+1) / static_cast<float>(num_outputs+1)))); }
    };

    /* ############## TODO ##############
    /// reports XYZ position of node in world space
    /// Inputs: none
    /// Outputs(3): X position, Y position, Z position
    struct ReadingNode: public Node
    {
        ReadingNode(ImVec2 _pos):Node()
        {
            num_inputs = 0;
            num_outputs = 3;
            softbody_node_id = -1;
            data_offset = 0;
            memset(data_buffer, 0, sizeof(data_buffer));
            type = Type::SOURCE;
            pos = _pos;
            done = true;
        }

        int softbody_node_id; // -1 means 'none'
        Vec3 data_buffer[BUF_SIZE]; // 1 second worth of data
        int data_offset;

        inline void PushData(Vec3 entry) { data_buffer[data_offset] = entry; data_offset = (data_offset+1)%2000; }
    };
    */

    struct GeneratorNode: public Node
    {
        GeneratorNode(ImVec2 _pos): Node(), amplitude(1.f), frequency(1.f), noise_max(0.f), elapsed(0.f)
        {
            num_inputs = 0;
            num_outputs = 1;
            pos = _pos;
            done = true; // Always ready
        }

        float frequency; // Hz
        float amplitude;
        float noise_max;
        float elapsed;
    };

    struct ScriptNode: public Node
    {
        ScriptNode(NodeGraphTool* _nodegraph, ImVec2 _pos);
        void InitScripting();
        void Apply();
        void  Exec();
        // Script functions
        float Read(int slot, int offset);
        void  Write(float val);

        char code_buf[1000];
        NodeGraphTool* nodegraph;
        asIScriptContext* script_context;
        asIScriptEngine*  script_engine;
        asIScriptFunction* script_func;
        char node_name[10];
        bool enabled; // Disables itself on script error
    };

    struct TransformNode: public Node
    {
        enum class Method
        {
            NONE, // Pass-through
            FIR_DIRECT,
            FIR_ADAPTIVE
        };

        TransformNode(ImVec2 _pos):Node() // Data offset is always '0' here
        {
            num_inputs = 1;
            num_outputs = 1;
            done = false;
            type = Type::TRANSFORM;
            pos = _pos;
            method = Method::NONE;
            memset(input_field, 0, sizeof(input_field));
            done = false;
        }

        char input_field[100];
        Method method;
        bool done;
    };

    struct DisplayNode: public Node
    {
        DisplayNode(ImVec2 _pos):Node()
        {
            pos = _pos;
            num_outputs = 0;
            num_inputs = 1;
            type = Type::DISPLAY;
            user_size = ImVec2(250.f, 85.f);
            done = false; // Irrelevant for this node type - no outputs
        }
    };

private:

    inline bool     IsInside (ImVec2 min, ImVec2 max, ImVec2 point) const                { return ((point.x > min.x) && (point.y > min.y)) && ((point.x < max.x) && (point.y < max.y)); }
    inline bool     IsLinkDragInProgress () const                                        { return (m_link_mouse_src != nullptr) || (m_link_mouse_dst != nullptr); }
    inline bool     IsRectHovered(ImVec2 min, ImVec2 max) const                          { return this->IsInside(min, max, m_nodegraph_mouse_pos); }
    inline void     DrawInputSlot (Node* node, const int index)                          { this->DrawSlotUni(node, index, true); }
    inline void     DrawOutputSlot (Node* node, const int index)                         { this->DrawSlotUni(node, index, false); }
    void            DrawSlotUni (Node* node, const int index, const bool input);
    Link*           AddLink (Node* src, Node* dst, int src_slot, int dst_slot);     ///< creates new link or fetches existing unused one
    Link*           FindLinkByDestination (Node* node, const int slot);
    Link*           FindLinkBySource (Node* node, const int slot);
    void            DrawNodeGraphPane ();
    void            DrawGrid ();
    void            DrawLink(Link* link);
    void            DeleteLink(Link* link);
    void            DrawNodeBegin(Node* node);
    void            DrawNodeFinalize(Node* node);
    void            NodeLinkChanged(Link* link, bool added);
    void            ScriptMessageCallback(const asSMessageInfo *msg, void *param);
    void            AddMessage(const char* fmt, ...);
    void            SaveAsJson(const char* filepath);
    void            LoadFromJson(const char* filepath);
    void            NodeToJson(rapidjson::Value& j_data, Node* node);


    inline bool IsSlotHovered(ImVec2 center_pos) const /// Slots can't use the "InvisibleButton" technique because it won't work when dragging.
    {
        ImVec2 min = center_pos - m_style.slot_hoverbox_extent;
        ImVec2 max = center_pos + m_style.slot_hoverbox_extent;
        return this->IsInside(min, max, m_nodegraph_mouse_pos);
    }

    //std::vector<ReadingNode*>   m_read_nodes;
    std::vector<TransformNode*> m_xform_nodes;
    std::vector<DisplayNode*>   m_disp_nodes;
    std::vector<GeneratorNode*> m_gen_nodes;
    std::vector<ScriptNode*>    m_script_nodes;
    std::vector<Link*>          m_links;
    std::list<std::string>      m_messages;
    char     m_filename[100];
    char     m_motionsim_ip[40];
    int      m_motionsim_port;
    Style    m_style;
    ImVec2   m_scroll;
    ImVec2   m_scroll_offset;
    ImVec2   m_nodegraph_mouse_pos;
    Node*    m_hovered_slot_node;
    int      m_hovered_slot_input;  // -1 = none
    int      m_hovered_slot_output; // -1 = none
    bool     m_is_any_slot_hovered;
    bool     m_show_filepath_input;
    Node*    m_last_scaled_node;
    Node     m_fake_mouse_node;     ///< Used while dragging link with mouse
    Link*    m_link_mouse_src;      ///< Link being mouse-dragged by it's input end.
    Link*    m_link_mouse_dst;      ///< Link being mouse-dragged by it's output end.
};

} // namespace RoR

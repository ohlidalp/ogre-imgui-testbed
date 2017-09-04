#pragma once

/// @file
/// @author Petr Ohlidal, 07/2017
/// @brief  Data representing the 'actor' being created/edited.

#include "Application.h"

#include <vector>

namespace RoR
{
    typedef Ogre::Vector3 Vec3;
}

namespace RigEditor {

// Types
typedef RoR::GStr<64> IdStr;

// Forward declarations
struct SoftbodyNode;
struct SoftbodyBeam;

/// NOTE ON DESIGN: RoR has many 'beam' types with various arguments, but RigEditor has only `SoftbodyBeam` with all these parameters mixed. Reason: I think RoR needs
///      to have only 1 beam type + external modifiers which read/update it's properties (to simulate hydros/shocks/commands and whatever). My plan is to rewrite
///      physics code with this approach in the future, and I'm preparing grounds for it in the editor: one unified beam class, temporarily carrying other parameters.
///      ~ only_a_ptr, 07/2017
struct SoftbodyBeam
{
    static const char* TypeAsStr[];

    enum class Type
    {
        PLAIN,
        STEERING_HYDRO,   ///< Truckfile section 'hydro'
        COMMAND_HYDRO,    ///< Truckfile section 'command' or 'command2'
        SHOCK_ABSORBER,   ///< Truckfile section 'shock'
        SHOCK_ABSORBER_2, ///< Truckfile section 'shocks2'
        ROPE,             ///< Truckfile section 'ropes'
        TRIGGER,          ///< Truckfile section 'triggers'
        GENERATED         ///< Generated, i.e. from section 'cinecam'
    };

    struct Options ///< Actor data; all beam types merged.
    {
        Options() { memset(this, 0, sizeof(Options)); }

        bool alltypes_i_invisible;

        bool plain_r_rope;
        bool plain_s_support;

        bool hydro_s_disable_on_high_speed;
        bool hydro_a_input_aileron;
        bool hydro_r_input_rudder;
        bool hydro_e_input_elevator;
        bool hydro_u_input_aileron_elevator;
        bool hydro_v_input_invaileron_elevator;
        bool hydro_x_input_aileron_rudder;
        bool hydro_y_input_invaileron_rudder;
        bool hydro_g_input_elevator_rudder;
        bool hydro_h_input_invelevator_rudder;

        bool command_r_rope;
        bool command_c_auto_center;
        bool command_f_not_faster;
        bool command_p_1press;
        bool command_o_1press_center;

        bool shock_L_active_left;
        bool shock_R_active_right;
        bool shock_m_metric;

        bool shock2_s_soft_bump_bounds;
        bool shock2_m_metric;
        bool shock2_M_absolute_metric;
    };

    /// Represents directive 'set_beam_defaults_scale' in Truckfile - allows fine-tuning many presets at once, used frequently by content creators
    struct PresetScaler
    {
        PresetScaler(): spring(1.f), damp(1.f), deform_threshold(1.f), strength(1.f) {}

        float         spring;
        float         damp;
        float         deform_threshold;
        float         strength; ///< a.k.a. 'breaking threshold'
    };

    /// Represents directive 'set_beam_defaults' in Truckfile.
    struct Preset
    {
        IdStr         name;
        PresetScaler* scaler;

        float         spring;
        float         damp;
        float         deform_threshold;
        float         strength;
        float         plastic_deform;
        bool          enable_advanced_deform; ///< Is this preset affected by Truckfile directive 'enable_advanced_deformation'?

        // TODO: Gfx attrs shouldn't be here, but it eases Truckfile import/export.
        float         visual_diameter; ///< in meters
        std::string   material_name;
    };

    struct Selection
    {
        // Metadata
        int                    num_selected;
        IdStr                  name;              ///< Only valid if 1 beam is selected.

        // Aggregate data, with uniformity states
        Type                   type;
        bool                   type_is_uniform;
        int                    detacher_group;
        bool                   detacher_group_is_uniform;
        Options                option_values;
        Options                option_uniformity;
        // * Plain beam
        float                  support_break_limit; ///< Extension break limit in %
        bool                   support_break_limit_is_uniform;
        // * Steer hydro
        float                  extension; ///< Max. extension in %
        bool                   extension_is_uniform;

        // TODO: other types
    };

    SoftbodyNode* base_node;
    SoftbodyNode* tip_node;
    Type          type;
    int           detacher_group;
    float         extension_break_limit; ///< Type: PLAIN; -1 means 'not set'
    float         max_extension;         ///< Types: hydro, command[2], shock[2], triggers
    float         max_contraction;       ///< Types: command[2], shock[2], triggers
    // TODO: STEERING_HYDRO/COMMAND_HYDRO inertia!

    // Command2 (unified) attrs;
    float         command_shorten_rate;
    float         command_lengthen_rate;
    int           command_contract_key;
    int           command_extend_key;
    std::string   command_description;
    float         command_affect_engine;
    bool          command_needs_engine;
    bool          command_plays_sound;

    // Shock2 attrs
    float         shock_precompression;
    float         shock_spring_in;           ///< Spring value applied when the shock is compressing.
    float         shock_damp_in;             ///< Damping value applied when the shock is compressing. 
    float         shock_spring_in_progress;  ///< Progression factor for springin. A value of 0 disables this option. 1...x as multipliers, example:maximum springrate == springrate + (factor*springrate)
    float         shock_damp_in_progress;    ///< Progression factor for dampin. 0 = disabled, 1...x as multipliers, example:maximum dampingrate == springrate + (factor*dampingrate)
    float         shock_spring_out;          ///< spring value applied when shock extending
    float         shock_damp_out;            ///< damping value applied when shock extending
    float         shock_spring_out_progress; ///< Progression factor springout, 0 = disabled, 1...x as multipliers, example:maximum springrate == springrate + (factor*springrate)
    float         shock_damp_out_progress;   ///< Progression factor dampout, 0 = disabled, 1...x as multipliers, example:maximum dampingrate == springrate + (factor*dampingrate)

    // Trigger attrs
    float         trigger_boundary_timer;
    int           trigger_shortlimit_action;
    int           trigger_longlimit_action;
};

struct SoftbodyNode
{
    struct Options ///< Actor data
    {
        Options() { memset(this, 0, sizeof(Options)); }

        bool      option_m_no_mouse_grab;
        bool      option_f_no_sparks;
        bool      option_x_exhaust_point;       ///< Legacy; use truckfile section 'exhausts'.
        bool      option_y_exhaust_direction;   ///< Legacy; use truckfile section 'exhausts'.
        bool      option_c_no_ground_contact;
        bool      option_h_hook_point;
        bool      option_b_extra_buoyancy;
        bool      option_p_no_particles;
        bool      option_L_log;
        bool      option_l_load_weight;
    };

    struct Preset ///< Actor data
    {
        Preset(): load_weight(0.f), friction(0.f), volume(0.f), surface(0.f)
        {}

        IdStr     name;
        float     load_weight;
        float     friction;
        float     volume;
        float     surface;
        Options   options;
    };

    struct Selection ///< Editor state: aggregate info about selected nodes in the actor project.
    {
        // Metadata
        int                    num_selected;
        IdStr                  name;              ///< Only valid if 1 node is selected.

        // Aggregates, with uniformity states
        SoftbodyNode::Options  options_values;
        SoftbodyNode::Options  options_uniform;
        float                  weight_override;
        bool                   weight_override_is_uniform;
        int                    detacher_group_id;
        bool                   detacher_group_is_uniform;
        SoftbodyNode::Preset*  node_preset;
        bool                   node_preset_is_uniform;
        SoftbodyBeam::Preset*  beam_preset;
        bool                   beam_preset_is_uniform;
    };

    SoftbodyNode():
        position(0,0,0), weight_override(-1), node_preset(nullptr), beam_preset(nullptr), detacher_group_id(-1), editor_group_id(-1),
        state_is_hovered(false), state_is_selected(false)
    {}

    IdStr                  name;
    RoR::Vec3              position;
    SoftbodyNode::Options  options;
    float                  weight_override;    ///< -1 means 'not set'
    SoftbodyNode::Preset*  node_preset;
    SoftbodyBeam::Preset*  beam_preset;        ///< Needed for truckfile feature 'hooks'
    int                    detacher_group_id;  ///< -1 means 'not set'
    int                    editor_group_id;    ///< -1 means 'not set'
    // Bitfields
    bool                   state_is_hovered:1;
    bool                   state_is_selected:1;
};

struct Project
{
    struct Softbody ///< Represents the raw softbody part of the actor project
    {
        std::vector<SoftbodyNode*>            nodes;
        std::vector<SoftbodyNode::Preset*>    node_presets;
        SoftbodyNode::Selection               node_selection;

        std::vector<SoftbodyBeam*>            beams;
        std::vector<SoftbodyBeam::Preset*>    beam_presets;
        SoftbodyBeam::Selection               beam_selection;

        int GetNodePresetArrayIndex(SoftbodyNode::Preset* query)
        {
            int num_presets = static_cast<int>(node_presets.size());
            for (int i = 0; i < num_presets; ++i)
            {
                if (query == node_presets[i])
                    return i;
            }
            return -1;
        }

        int GetBeamPresetArrayIndex(SoftbodyBeam::Preset* query)
        {
            int num_presets = static_cast<int>(beam_presets.size());
            for (int i = 0; i < num_presets; ++i)
            {
                if (query == beam_presets[i])
                    return i;
            }
            return -1;
        }
    };

    Softbody softbody;
};

} // namespace RigEditor

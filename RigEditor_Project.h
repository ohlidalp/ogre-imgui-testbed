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

// Forward declarations
struct SoftbodyNode;
struct SoftbodyBeam;

struct SoftbodyBeam
{
    struct Preset
    {
    //TODO
    };

    struct Selection
    {
    //TODO
    };

    SoftbodyNode* node_src; // NOTE: RoR's nodes don't have orientation - I'm adding it to the editor just in case ~ only_a_ptr, 07/2017
    SoftbodyNode* node_dst;
};

struct SoftbodyNode
{
    typedef RoR::GStr<64> IdStr;

    struct Options ///< Actor data
    {
        Options() { memset(this, 0, sizeof(Options)); }

        bool      option_m_no_mouse_grab     ;
        bool      option_f_no_sparks         ;
        bool      option_x_exhaust_point     ; ///< Legacy; use truckfile section 'exhausts'.
        bool      option_y_exhaust_direction ; ///< Legacy; use truckfile section 'exhausts'.
        bool      option_c_no_ground_contact ;
        bool      option_h_hook_point        ;
        bool      option_b_extra_buoyancy    ;
        bool      option_p_no_particles      ;
        bool      option_L_log               ;
        bool      option_l_load_weight       ;
    };

    struct Preset ///< Actor data
    {
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
        SoftbodyNode::IdStr    id;              ///< Only valid if 1 node is selected.

        // Aggregates, with uniformity states
        Options                options_values;
        Options                options_uniform;
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

    IdStr                  id;
    RoR::Vec3              position;
    Options                options;
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
    };

    Softbody softbody;
};

} // namespace RigEditor

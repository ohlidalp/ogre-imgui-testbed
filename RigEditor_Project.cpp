
#include "RigEditor_Gui.h"
#include "Application.h"

const char* RigEditor::SoftbodyBeam::TypeAsStr[] = // static member init
{
    "PLAIN",
    "STEERING_HYDRO",   ///< Truckfile section 'hydro'
    "COMMAND_HYDRO",    ///< Truckfile section 'command' or 'command2'
    "SHOCK_ABSORBER",   ///< Truckfile section 'shock'
    "SHOCK_ABSORBER_2", ///< Truckfile section 'shocks2'
    "ROPE",             ///< Truckfile section 'ropes'
    "TRIGGER",          ///< Truckfile section 'triggers'
    "GENERATED"         ///< Generated, i.e. from section 'cinecam'
};

int RigEditor::Project::Softbody::GetNodePresetArrayIndex(SoftbodyNode::Preset* query)
{
    int num_presets = static_cast<int>(node_presets.size());
    for (int i = 0; i < num_presets; ++i)
    {
        if (query == node_presets[i])
            return i;
    }
    return -1;
}

int RigEditor::Project::Softbody::GetBeamPresetArrayIndex(SoftbodyBeam::Preset* query)
{
    int num_presets = static_cast<int>(beam_presets.size());
    for (int i = 0; i < num_presets; ++i)
    {
        if (query == beam_presets[i])
            return i;
    }
    return -1;
}

void RigEditor::SoftbodyNode::Options::SetAll(bool value)
{
    option_m_no_mouse_grab     = value;
    option_f_no_sparks         = value;
    option_x_exhaust_point     = value;  
    option_y_exhaust_direction = value;  
    option_c_no_ground_contact = value;
    option_h_hook_point        = value;
    option_b_extra_buoyancy    = value;
    option_p_no_particles      = value;
    option_L_log               = value;
    option_l_load_weight       = value;
}

void RigEditor::SoftbodyBeam::Options::SetAll(bool value)
{
    alltypes_i_invisible              = value;

    plain_r_rope                      = value;
    plain_s_support                   = value;

    hydro_s_disable_on_high_speed     = value;
    hydro_a_input_aileron             = value;
    hydro_r_input_rudder              = value;
    hydro_e_input_elevator            = value;
    hydro_u_input_aileron_elevator    = value;
    hydro_v_input_invaileron_elevator = value;
    hydro_x_input_aileron_rudder      = value;
    hydro_y_input_invaileron_rudder   = value;
    hydro_g_input_elevator_rudder     = value;
    hydro_h_input_invelevator_rudder  = value;

    command_r_rope                    = value;
    command_c_auto_center             = value;
    command_f_not_faster              = value;
    command_p_1press                  = value;
    command_o_1press_center           = value;

    shock_L_active_left               = value;
    shock_R_active_right              = value;
    shock_m_metric                    = value;

    shock2_s_soft_bump_bounds         = value;
    shock2_m_metric                   = value;
    shock2_M_absolute_metric          = value;
}

template <typename T>
void MergeValue(T& aggregate_val, bool& is_uniform, T const& incoming_val)
{
    is_uniform = (is_uniform && (aggregate_val == incoming_val));
    aggregate_val = incoming_val;
}

void RigEditor::SoftbodyNode::Selection::Merge(SoftbodyNode* n)
{
    if (num_selected == 0)
    {
        num_selected = 1;
        name = n->name;
        options_values = n->options;
        options_uniform.SetAll(true);
        weight_override = n->weight_override;
        weight_override_is_uniform = true;
        detacher_group_id = n->detacher_group_id;
        detacher_group_is_uniform = true;
        node_preset = n->node_preset;
        node_preset_is_uniform = true;
        beam_preset = n->beam_preset;
        beam_preset_is_uniform = true;

        return;
    }

    ++num_selected;
    name.Clear();

    // Merge attributes
    MergeValue(weight_override  ,  weight_override_is_uniform, n->weight_override  );
    MergeValue(detacher_group_id,  detacher_group_is_uniform , n->detacher_group_id);
    MergeValue(node_preset      ,  node_preset_is_uniform    , n->node_preset      );
    MergeValue(beam_preset      ,  beam_preset_is_uniform    , n->beam_preset      );

    // Merge options
    options_uniform.option_m_no_mouse_grab        = (options_uniform.option_m_no_mouse_grab        && (options_values.option_m_no_mouse_grab     == n->options.option_m_no_mouse_grab    ));
    options_uniform.option_f_no_sparks            = (options_uniform.option_f_no_sparks            && (options_values.option_f_no_sparks         == n->options.option_f_no_sparks        ));
    options_uniform.option_x_exhaust_point        = (options_uniform.option_x_exhaust_point        && (options_values.option_x_exhaust_point     == n->options.option_x_exhaust_point    ));
    options_uniform.option_y_exhaust_direction    = (options_uniform.option_y_exhaust_direction    && (options_values.option_y_exhaust_direction == n->options.option_y_exhaust_direction));
    options_uniform.option_c_no_ground_contact    = (options_uniform.option_c_no_ground_contact    && (options_values.option_c_no_ground_contact == n->options.option_c_no_ground_contact));
    options_uniform.option_h_hook_point           = (options_uniform.option_h_hook_point           && (options_values.option_h_hook_point        == n->options.option_h_hook_point       ));
    options_uniform.option_b_extra_buoyancy       = (options_uniform.option_b_extra_buoyancy       && (options_values.option_b_extra_buoyancy    == n->options.option_b_extra_buoyancy   ));
    options_uniform.option_p_no_particles         = (options_uniform.option_p_no_particles         && (options_values.option_p_no_particles      == n->options.option_p_no_particles     ));
    options_uniform.option_L_log                  = (options_uniform.option_L_log                  && (options_values.option_L_log               == n->options.option_L_log              ));
    options_uniform.option_l_load_weight          = (options_uniform.option_l_load_weight          && (options_values.option_l_load_weight       == n->options.option_l_load_weight      ));

    options_values = n->options;
}

void RigEditor::SoftbodyNode::Selection::Propagate(SoftbodyNode* n)
{
    if (num_selected == 1)
    {
        n->name = name;
    }

    // Update attributes
    if (weight_override_is_uniform) { n->weight_override   = weight_override  ; }
    if (detacher_group_is_uniform ) { n->detacher_group_id = detacher_group_id; }
    if (node_preset_is_uniform    ) { n->node_preset       = node_preset      ; }
    if (beam_preset_is_uniform    ) { n->beam_preset       = beam_preset      ; }

    // Update options
    if (options_uniform.option_m_no_mouse_grab    ) { n->options.option_m_no_mouse_grab     = options_values.option_m_no_mouse_grab    ; }
    if (options_uniform.option_f_no_sparks        ) { n->options.option_f_no_sparks         = options_values.option_f_no_sparks        ; }
    if (options_uniform.option_x_exhaust_point    ) { n->options.option_x_exhaust_point     = options_values.option_x_exhaust_point    ; }
    if (options_uniform.option_y_exhaust_direction) { n->options.option_y_exhaust_direction = options_values.option_y_exhaust_direction; }
    if (options_uniform.option_c_no_ground_contact) { n->options.option_c_no_ground_contact = options_values.option_c_no_ground_contact; }
    if (options_uniform.option_h_hook_point       ) { n->options.option_h_hook_point        = options_values.option_h_hook_point       ; }
    if (options_uniform.option_b_extra_buoyancy   ) { n->options.option_b_extra_buoyancy    = options_values.option_b_extra_buoyancy   ; }
    if (options_uniform.option_p_no_particles     ) { n->options.option_p_no_particles      = options_values.option_p_no_particles     ; }
    if (options_uniform.option_L_log              ) { n->options.option_L_log               = options_values.option_L_log              ; }
    if (options_uniform.option_l_load_weight      ) { n->options.option_l_load_weight       = options_values.option_l_load_weight      ; }
}

void RigEditor::Project::RefreshNodeSelectionAggregates()
{
    this->softbody.node_selection.Reset();
    for (SoftbodyNode* n: this->softbody.nodes)
    {
        if (n->state_is_selected)
        {
            this->softbody.node_selection.Merge(n);
        }
    }
}

void RigEditor::SoftbodyBeam::Selection::SetSingle(SoftbodyBeam* b)
{
    num_selected = 1;
    option_values = b->options;
    option_uniformity.SetAll(true);

    type                  = b->type;                             type_is_uniform                  = true;
    detacher_group        = b->detacher_group;                   detacher_group_is_uniform        = true;
    extension_break_limit = b->extension_break_limit;            extension_break_limit_is_uniform = true;
    max_extension         = b->max_extension;                    max_extension_is_uniform         = true;
    max_contraction       = b->max_contraction;                  max_contraction_is_uniform       = true;

    // Command2 (unified) attrs;
    command_shorten_rate  = b->command_shorten_rate;             command_shorten_rate_is_uniform  = true;
    command_lengthen_rate = b->command_lengthen_rate;            command_lengthen_rate_is_uniform = true;
    command_contract_key  = b->command_contract_key;             command_contract_key_is_uniform  = true;
    command_extend_key    = b->command_extend_key;               command_extend_key_is_uniform    = true;
    command_affect_engine = b->command_affect_engine;            command_affect_engine_is_uniform = true;
    command_needs_engine  = b->command_needs_engine;             command_needs_engine_is_uniform  = true;
    command_plays_sound   = b->command_plays_sound;              command_plays_sound_is_uniform   = true;

    // Shock2 attrs
    shock_precompression       = b->shock_precompression;          shock_precompression_is_uniform      = true;
    shock_spring_in            = b->shock_spring_in;               shock_spring_in_is_uniform           = true;
    shock_damp_in              = b->shock_damp_in;                 shock_damp_in_is_uniform             = true;
    shock_spring_in_progress   = b->shock_spring_in_progress;      shock_spring_in_progress_is_uniform  = true;
    shock_damp_in_progress     = b->shock_damp_in_progress;        shock_damp_in_progress_is_uniform    = true;
    shock_spring_out           = b->shock_spring_out;              shock_spring_out_is_uniform          = true;
    shock_damp_out             = b->shock_damp_out;                shock_damp_out_is_uniform            = true;
    shock_spring_out_progress  = b->shock_spring_out_progress;     shock_spring_out_progress_is_uniform = true;
    shock_damp_out_progress    = b->shock_damp_out_progress;       shock_damp_out_progress_is_uniform   = true;

    // Trigger attrs
    trigger_boundary_timer     = b->trigger_boundary_timer;        trigger_boundary_timer_is_uniform    = true;
    trigger_shortlimit_action  = b->trigger_shortlimit_action;     trigger_shortlimit_action_is_uniform = true;
    trigger_longlimit_action   = b->trigger_longlimit_action;      trigger_longlimit_action_is_uniform  = true;
}

void RigEditor::SoftbodyBeam::Selection::Merge(SoftbodyBeam* b)
{
    if (num_selected == 0)
    {
        this->SetSingle(b);
        return;
    }

    ++num_selected;

    MergeValue(type,                       type_is_uniform,                        b->type);
    MergeValue(detacher_group,             detacher_group_is_uniform,              b->detacher_group);
    MergeValue(extension_break_limit,      extension_break_limit_is_uniform,       b->extension_break_limit);
    MergeValue(max_extension,              max_extension_is_uniform,               b->max_extension);
    MergeValue(max_contraction,            max_contraction_is_uniform,             b->max_contraction);

    MergeValue(command_shorten_rate,       command_shorten_rate_is_uniform,        b->command_shorten_rate);
    MergeValue(command_lengthen_rate,      command_lengthen_rate_is_uniform,       b->command_lengthen_rate);
    MergeValue(command_contract_key,       command_contract_key_is_uniform,        b->command_contract_key);
    MergeValue(command_extend_key,         command_extend_key_is_uniform,          b->command_extend_key);
    MergeValue(command_affect_engine,      command_affect_engine_is_uniform,       b->command_affect_engine);
    MergeValue(command_needs_engine,       command_needs_engine_is_uniform,        b->command_needs_engine);
    MergeValue(command_plays_sound,        command_plays_sound_is_uniform,         b->command_plays_sound);

    MergeValue(shock_precompression,       shock_precompression_is_uniform,        b->shock_precompression);
    MergeValue(shock_spring_in,            shock_spring_in_is_uniform,             b->shock_spring_in);
    MergeValue(shock_damp_in,              shock_damp_in_is_uniform,               b->shock_damp_in);
    MergeValue(shock_spring_in_progress,   shock_spring_in_progress_is_uniform,    b->shock_spring_in_progress);
    MergeValue(shock_damp_in_progress,     shock_damp_in_progress_is_uniform,      b->shock_damp_in_progress);
    MergeValue(shock_spring_out,           shock_spring_out_is_uniform,            b->shock_spring_out);
    MergeValue(shock_damp_out,             shock_damp_out_is_uniform,              b->shock_damp_out);
    MergeValue(shock_spring_out_progress,  shock_spring_out_progress_is_uniform,   b->shock_spring_out_progress);
    MergeValue(shock_damp_out_progress,    shock_damp_out_progress_is_uniform,     b->shock_damp_out_progress);

    MergeValue(trigger_boundary_timer,     trigger_boundary_timer_is_uniform,      b->trigger_boundary_timer);
    MergeValue(trigger_shortlimit_action,  trigger_shortlimit_action_is_uniform,   b->trigger_shortlimit_action);
    MergeValue(trigger_longlimit_action,   trigger_longlimit_action_is_uniform,    b->trigger_longlimit_action);
}

void RigEditor::Project::RefreshBeamSelectionAggregates()
{
    this->softbody.beam_selection.num_selected = 0;
    for (SoftbodyBeam* b: this->softbody.beams)
    {
        if (b->state_is_selected)
        {
            this->softbody.beam_selection.Merge(b);
        }
    }
}

void RigEditor::Project::PropagateNodeAggregateUpdates()
{
    for (SoftbodyNode* n: this->softbody.nodes)
    {
        if (n->state_is_selected)
        {
            this->softbody.node_selection.Propagate(n);
        }
    }
}

void RigEditor::Project::PropagateBeamAggregateUpdates()
{}


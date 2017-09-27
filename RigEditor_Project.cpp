
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

void RigEditor::SoftbodyNode::Selection::Push(SoftbodyNode* n)
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

    SoftbodyBeam::Options::Merge(option_values, option_uniformity, b->options);
}

void RigEditor::SoftbodyBeam::Selection::Push(SoftbodyBeam* b)
{
    if (type_is_uniform                     ) { b->type                      = type;                      }
    if (detacher_group_is_uniform           ) { b->detacher_group            = detacher_group;            }
    if (extension_break_limit_is_uniform    ) { b->extension_break_limit     = extension_break_limit;     }
    if (max_extension_is_uniform            ) { b->max_extension             = max_extension;             }
    if (max_contraction_is_uniform          ) { b->max_contraction           = max_contraction;           }

    if (command_shorten_rate_is_uniform     ) { b->command_shorten_rate      = command_shorten_rate;      }
    if (command_lengthen_rate_is_uniform    ) { b->command_lengthen_rate     = command_lengthen_rate;     }
    if (command_contract_key_is_uniform     ) { b->command_contract_key      = command_contract_key;      }
    if (command_extend_key_is_uniform       ) { b->command_extend_key        = command_extend_key;        }
    if (command_affect_engine_is_uniform    ) { b->command_affect_engine     = command_affect_engine;     }
    if (command_needs_engine_is_uniform     ) { b->command_needs_engine      = command_needs_engine;      }
    if (command_plays_sound_is_uniform      ) { b->command_plays_sound       = command_plays_sound;       }

    if (shock_precompression_is_uniform     ) { b->shock_precompression      = shock_precompression;      }
    if (shock_spring_in_is_uniform          ) { b->shock_spring_in           = shock_spring_in;           }
    if (shock_damp_in_is_uniform            ) { b->shock_damp_in             = shock_damp_in;             }
    if (shock_spring_in_progress_is_uniform ) { b->shock_spring_in_progress  = shock_spring_in_progress;  }
    if (shock_damp_in_progress_is_uniform   ) { b->shock_damp_in_progress    = shock_damp_in_progress;    }
    if (shock_spring_out_is_uniform         ) { b->shock_spring_out          = shock_spring_out;          }
    if (shock_damp_out_is_uniform           ) { b->shock_damp_out            = shock_damp_out;            }
    if (shock_spring_out_progress_is_uniform) { b->shock_spring_out_progress = shock_spring_out_progress; }
    if (shock_damp_out_progress_is_uniform  ) { b->shock_damp_out_progress   = shock_damp_out_progress;   }

    if (trigger_boundary_timer_is_uniform   ) { b->trigger_boundary_timer    = trigger_boundary_timer;    }
    if (trigger_shortlimit_action_is_uniform) { b->trigger_shortlimit_action = trigger_shortlimit_action; }
    if (trigger_longlimit_action_is_uniform ) { b->trigger_longlimit_action  = trigger_longlimit_action;  }

    SoftbodyBeam::Options::Push(b->options, option_values, option_uniformity);
}

inline void MergeOption(bool& value, bool& uni, const bool incoming)
{
    uni = (value != incoming) ? false : uni;
    value = (!uni) ? false : value;
}

void RigEditor::SoftbodyBeam::Options::Merge(Options& values, Options& uni, const Options& incoming)
{
    MergeOption(values.alltypes_i_invisible             , uni.alltypes_i_invisible             , incoming.alltypes_i_invisible             );

    MergeOption(values.plain_r_rope                     , uni.plain_r_rope                     , incoming.plain_r_rope                     );
    MergeOption(values.plain_s_support                  , uni.plain_s_support                  , incoming.plain_s_support                  );

    MergeOption(values.hydro_s_disable_on_high_speed    , uni.hydro_s_disable_on_high_speed    , incoming.hydro_s_disable_on_high_speed    );
    MergeOption(values.hydro_a_input_aileron            , uni.hydro_a_input_aileron            , incoming.hydro_a_input_aileron            );
    MergeOption(values.hydro_r_input_rudder             , uni.hydro_r_input_rudder             , incoming.hydro_r_input_rudder             );
    MergeOption(values.hydro_e_input_elevator           , uni.hydro_e_input_elevator           , incoming.hydro_e_input_elevator           );
    MergeOption(values.hydro_u_input_aileron_elevator   , uni.hydro_u_input_aileron_elevator   , incoming.hydro_u_input_aileron_elevator   );
    MergeOption(values.hydro_v_input_invaileron_elevator, uni.hydro_v_input_invaileron_elevator, incoming.hydro_v_input_invaileron_elevator);
    MergeOption(values.hydro_x_input_aileron_rudder     , uni.hydro_x_input_aileron_rudder     , incoming.hydro_x_input_aileron_rudder     );
    MergeOption(values.hydro_y_input_invaileron_rudder  , uni.hydro_y_input_invaileron_rudder  , incoming.hydro_y_input_invaileron_rudder  );
    MergeOption(values.hydro_g_input_elevator_rudder    , uni.hydro_g_input_elevator_rudder    , incoming.hydro_g_input_elevator_rudder    );
    MergeOption(values.hydro_h_input_invelevator_rudder , uni.hydro_h_input_invelevator_rudder , incoming.hydro_h_input_invelevator_rudder );

    MergeOption(values.command_r_rope                   , uni.command_r_rope                   , incoming.command_r_rope                   );
    MergeOption(values.command_c_auto_center            , uni.command_c_auto_center            , incoming.command_c_auto_center            );
    MergeOption(values.command_f_not_faster             , uni.command_f_not_faster             , incoming.command_f_not_faster             );
    MergeOption(values.command_p_1press                 , uni.command_p_1press                 , incoming.command_p_1press                 );
    MergeOption(values.command_o_1press_center          , uni.command_o_1press_center          , incoming.command_o_1press_center          );

    MergeOption(values.shock_L_active_left              , uni.shock_L_active_left              , incoming.shock_L_active_left              );
    MergeOption(values.shock_R_active_right             , uni.shock_R_active_right             , incoming.shock_R_active_right             );
    MergeOption(values.shock_m_metric                   , uni.shock_m_metric                   , incoming.shock_m_metric                   );

    MergeOption(values.shock2_s_soft_bump_bounds        , uni.shock2_s_soft_bump_bounds        , incoming.shock2_s_soft_bump_bounds        );
    MergeOption(values.shock2_m_metric                  , uni.shock2_m_metric                  , incoming.shock2_m_metric                  );
    MergeOption(values.shock2_M_absolute_metric         , uni.shock2_M_absolute_metric         , incoming.shock2_M_absolute_metric         );
}

inline void PushOption(bool& dst, const bool value, const bool uni)
{
    dst = (uni) ? value : dst;
}

void RigEditor::SoftbodyBeam::Options::Push(Options& target, const Options& values, const Options& uni)
{
    PushOption(target.alltypes_i_invisible             , values.alltypes_i_invisible             , uni.alltypes_i_invisible             );

    PushOption(target.plain_r_rope                     , values.plain_r_rope                     , uni.plain_r_rope                     );
    PushOption(target.plain_s_support                  , values.plain_s_support                  , uni.plain_s_support                  );

    PushOption(target.hydro_s_disable_on_high_speed    , values.hydro_s_disable_on_high_speed    , uni.hydro_s_disable_on_high_speed    );
    PushOption(target.hydro_a_input_aileron            , values.hydro_a_input_aileron            , uni.hydro_a_input_aileron            );
    PushOption(target.hydro_r_input_rudder             , values.hydro_r_input_rudder             , uni.hydro_r_input_rudder             );
    PushOption(target.hydro_e_input_elevator           , values.hydro_e_input_elevator           , uni.hydro_e_input_elevator           );
    PushOption(target.hydro_u_input_aileron_elevator   , values.hydro_u_input_aileron_elevator   , uni.hydro_u_input_aileron_elevator   );
    PushOption(target.hydro_v_input_invaileron_elevator, values.hydro_v_input_invaileron_elevator, uni.hydro_v_input_invaileron_elevator);
    PushOption(target.hydro_x_input_aileron_rudder     , values.hydro_x_input_aileron_rudder     , uni.hydro_x_input_aileron_rudder     );
    PushOption(target.hydro_y_input_invaileron_rudder  , values.hydro_y_input_invaileron_rudder  , uni.hydro_y_input_invaileron_rudder  );
    PushOption(target.hydro_g_input_elevator_rudder    , values.hydro_g_input_elevator_rudder    , uni.hydro_g_input_elevator_rudder    );
    PushOption(target.hydro_h_input_invelevator_rudder , values.hydro_h_input_invelevator_rudder , uni.hydro_h_input_invelevator_rudder );

    PushOption(target.command_r_rope                   , values.command_r_rope                   , uni.command_r_rope                   );
    PushOption(target.command_c_auto_center            , values.command_c_auto_center            , uni.command_c_auto_center            );
    PushOption(target.command_f_not_faster             , values.command_f_not_faster             , uni.command_f_not_faster             );
    PushOption(target.command_p_1press                 , values.command_p_1press                 , uni.command_p_1press                 );
    PushOption(target.command_o_1press_center          , values.command_o_1press_center          , uni.command_o_1press_center          );

    PushOption(target.shock_L_active_left              , values.shock_L_active_left              , uni.shock_L_active_left              );
    PushOption(target.shock_R_active_right             , values.shock_R_active_right             , uni.shock_R_active_right             );
    PushOption(target.shock_m_metric                   , values.shock_m_metric                   , uni.shock_m_metric                   );

    PushOption(target.shock2_s_soft_bump_bounds        , values.shock2_s_soft_bump_bounds        , uni.shock2_s_soft_bump_bounds        );
    PushOption(target.shock2_m_metric                  , values.shock2_m_metric                  , uni.shock2_m_metric                  );
    PushOption(target.shock2_M_absolute_metric         , values.shock2_M_absolute_metric         , uni.shock2_M_absolute_metric         );
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
            this->softbody.node_selection.Push(n);
        }
    }
}

void RigEditor::Project::PropagateBeamAggregateUpdates()
{
    for (SoftbodyBeam* b: this->softbody.beams)
    {
        if (b->state_is_selected)
        {
            this->softbody.beam_selection.Push(b);
        }
    }
}


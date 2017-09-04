/*
    This source file is part of Rigs of Rods
    Copyright 2013-2017 Petr Ohlidal

    For more information, see http://www.rigsofrods.org/

    Rigs of Rods is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3, as
    published by the Free Software Foundation.

    Rigs of Rods is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rigs of Rods. If not, see <http://www.gnu.org/licenses/>.
*/


/// @file   Application.h
/// @author Petr Ohlidal
/// @date   05/2014
/// @brief  Central state/object manager and communications hub

#pragma once

#include <cstring>


namespace RoR {

/// Wrapper for classic c-string. Why 'G'string? Because it was originally written for use with our GVars (global vars).
template<size_t L> class GStr
{
public:
    // Constructors
    inline             GStr()                                { std::memset(m_buffer, 0, L); }
    inline             GStr(GStr<L> const & src)             { this->Assign(src); }
    inline             GStr(const char* src)                 { this->Assign(src); }

    // Reading
    inline const char* ToCStr() const                        { return m_buffer; }
    inline bool        IsEmpty() const                       { return m_buffer[0] == '\0'; }
    inline char*       GetBuffer()                           { return m_buffer; }
    inline size_t      GetCapacity() const                   { return m_capacity; }
    inline int         Compare(const char* str) const        { return std::strncmp(m_buffer, str, L); }
    inline size_t      GetLength() const                     { return std::strlen(m_buffer); }

    // Writing
    inline GStr&       Clear()                               { m_buffer[0] = '\0'; return *this; }
    inline GStr&       Assign(const char* src)               { std::strncpy(m_buffer, src, L); return *this; }
    inline GStr&       Append(const char* src)               { std::strncat(m_buffer, src, (L-(strlen(src)+1))); return *this; }
    inline GStr&       Append(float f)                       { char buf[50]; std::snprintf(buf, 50, "%f", f); this->Append(buf); return *this; }
    inline GStr&       Append(int i)                         { char buf[50]; std::snprintf(buf, 50, "%d", i); this->Append(buf); return *this; }
    inline GStr&       Append(size_t z)                      { char buf[50]; std::snprintf(buf, 50, "%lu", static_cast<unsigned long>(z)); this->Append(buf); return *this; }
    inline GStr&       Append(char c)                        { char buf[2] = {}; buf[0] = c; this->Append(buf); return *this; }

    // Operators
    inline             operator const char*() const          { return this->ToCStr(); }
    inline GStr&       operator=  (const char* src)          { return this->Assign(src); }
    inline GStr&       operator=  (GStr<L> g_src)            { return this->Assign(g_src.GetBuffer()); }
    inline GStr&       operator<< (const char* src)          { return this->Append(src); }
    inline GStr&       operator<< (float f)                  { return this->Append(f); }
    inline GStr&       operator<< (int i)                    { return this->Append(i); }
    inline GStr&       operator<< (size_t z)                 { return this->Append(z); }
    inline GStr&       operator<< (char c)                   { return this->Append(c); }
    inline bool        operator== (const char* other) const  { return (this->Compare(other) == 0); }

private:
    char         m_buffer[L];
    const size_t m_capacity = L;
};


struct GVarBase
{
    GVarBase(const char* name, const char* conf_name):
        name(name), conf_name(conf_name)
    {}

    // Logging
    void LogSetPendingS  (const char* input,   const char* pending, const char* active) const;
    void LogSetPending   (        int input,           int pending,         int active) const;
    void LogSetPending   (      float input,         float pending,       float active) const;

    void LogSetActiveS   (const char* input,   const char* active) const;
    void LogSetActive    (        int input,           int active) const;
    void LogSetActive    (      float input,         float active) const;

    void LogApplyPendingS(const char* pending, const char* active) const;
    void LogApplyPending (        int pending,         int active) const;
    void LogApplyPending (      float pending,       float active) const;

    inline const char* BoolToStr(bool b) const                       { return (b) ? "true" : "false"; }
    void        LogSetPending  (bool a, bool b, bool c) const { this->LogSetPendingS  (this->BoolToStr(a), this->BoolToStr(b), this->BoolToStr(c)); }
    void        LogSetActive   (bool a, bool b) const         { this->LogSetActiveS   (this->BoolToStr(a), this->BoolToStr(b)); }
    void        LogApplyPending(bool a, bool b) const         { this->LogApplyPendingS(this->BoolToStr(a), this->BoolToStr(b)); }

    const char* name;
    const char* conf_name;
};


template <typename T> class GVarPod: public GVarBase
{
public:
    GVarPod(const char* name, const char* conf, T active_val, T pending_val):
        GVarBase(name, conf), m_value_active(active_val), m_value_pending(pending_val)
    {}

    inline T     GetActive() const        { return m_value_active; }
    inline T     GetPending() const       { return m_value_pending; }
    void         SetPending(T val);
    void         ApplyPending();
    void         SetActive(T val);

protected:
    void         LogSetPending  (T val) const  { GVarBase::LogSetPending  (val, m_value_pending, m_value_active); }
    void         LogSetActive   (T val) const  { GVarBase::LogSetActive   (val, m_value_active); }
    void         LogApplyPending()      const  { GVarBase::LogApplyPending(m_value_pending, m_value_active); }

    T            m_value_active;
    T            m_value_pending;
};


template <typename E> class GVarEnum: public GVarPod<E>
{
public:
    GVarEnum(const char* name, const char* conf, E active_val, E pending_val):
        GVarPod(name, conf, active_val, pending_val)
    {}

    const char*  GetActiveAsStr () const       { return EnumToStr(m_value_active); }
    const char*  GetPendingAsStr() const       { return EnumToStr(m_value_pending); }
    void         SetPending(E val);
    void         ApplyPending();
    void         SetActive(E val);
    void         LogSetPending  (E val) const  { GVarBase::LogSetPendingS  (EnumToStr(val), EnumToStr(m_value_pending), EnumToStr(m_value_active)); }
    void         LogSetActive   (E val) const  { GVarBase::LogSetActiveS   (EnumToStr(val), EnumToStr(m_value_active)); }
    void         LogApplyPending()      const  { GVarBase::LogApplyPendingS(EnumToStr(m_value_pending), EnumToStr(m_value_active)); }
};


template <size_t L> class GVarStr: public GVarBase
{
public:
    GVarStr(const char* name, const char* conf, const char* active_val, const char* pending_val):
        GVarBase(name, conf), m_value_active(active_val), m_value_pending(pending_val)
    {}

    inline const char*     GetActive() const        { return m_value_active; }
    inline bool            IsActiveEmpty() const    { return m_value_active.IsEmpty(); }
    inline GStr<L> &       GetPending()             { return m_value_pending; }

    void                   SetActive (const char* val)   { GVarBase::LogSetActiveS   (val, m_value_active); m_value_active = val; }
    void                   SetPending(const char* val)   { GVarBase::LogSetPendingS  (val, m_value_pending, m_value_active); m_value_pending = val; }
    void                   ApplyPending()                { GVarBase::LogApplyPendingS(m_value_pending, m_value_active); m_value_active = m_value_pending; }

protected:
    GStr<L>         m_value_active;
    GStr<L>         m_value_pending;
};


enum class AppState
{
    NONE,               ///< Only valid for GVar 'app_state_pending'. Means no change is requested.
    BOOTSTRAP,          ///< Initial state
    MAIN_MENU,
    CHANGE_MAP,         ///< Enter main menu & immediatelly launch singleplayer map selector.
    SIMULATION,
    SHUTDOWN,
    PRINT_HELP_EXIT,
    PRINT_VERSION_EXIT,
};
const char* EnumToStr(AppState v);

enum class MpState
{
    NONE,      ///< Only valid for GVar 'app_state_pending'. Means no change is requested.
    DISABLED,  ///< Not connected for whatever reason.
    CONNECTED,
};
const char* EnumToStr(MpState v);

enum class SimState
{
    NONE,
    RUNNING,
    PAUSED,
    SELECTING,  ///< The selector GUI window is displayed.
    EDITOR_MODE ///< Hacky, but whatever... added by Ulteq, 2016
};
const char* EnumToStr(SimState v);

enum class SimGearboxMode
{
    AUTO,          ///< Automatic shift
    SEMI_AUTO,     ///< Manual shift - Auto clutch
    MANUAL,        ///< Fully Manual: sequential shift
    MANUAL_STICK,  ///< Fully manual: stick shift
    MANUAL_RANGES, ///< Fully Manual: stick shift with ranges
};
const char* EnumToStr(SimGearboxMode v);

enum class GfxShadowType
{
    NONE,
    TEXTURE,
    PSSM
};
const char* EnumToStr(SimGearboxMode v);

enum class GfxExtCamMode
{
    NONE,
    STATIC,
    PITCHING,
};
const char* EnumToStr(GfxExtCamMode v);

enum class GfxTexFilter
{
    NONE,
    BILINEAR,
    TRILINEAR,
    ANISOTROPIC,
};
const char* EnumToStr(GfxTexFilter v);

enum class GfxVegetation
{
    NONE,
    x20PERC,
    x50PERC,
    FULL,
};
const char* EnumToStr(GfxVegetation v);

enum class GfxFlaresMode
{
    NONE,                    ///< None (fastest)
    NO_LIGHTSOURCES,         ///< No light sources
    CURR_VEHICLE_HEAD_ONLY,  ///< Only current vehicle, main lights
    ALL_VEHICLES_HEAD_ONLY,  ///< All vehicles, main lights
    ALL_VEHICLES_ALL_LIGHTS, ///< All vehicles, all lights
};
const char* EnumToStr(GfxFlaresMode v);

enum class GfxWaterMode
{
    NONE,       ///< None
    BASIC,      ///< Basic (fastest)
    REFLECT,    ///< Reflection
    FULL_FAST,  ///< Reflection + refraction (speed optimized)
    FULL_HQ,    ///< Reflection + refraction (quality optimized)
    HYDRAX,     ///< HydraX
};
const char* EnumToStr(GfxWaterMode v);

enum class GfxSkyMode
{
    SANDSTORM,  ///< Sandstorm (fastest)
    CAELUM,     ///< Caelum (best looking, slower)
    SKYX,       ///< SkyX (best looking, slower)
};
const char* EnumToStr(GfxSkyMode v);

enum class IoInputGrabMode
{
    NONE,
    ALL,
    DYNAMIC,
};
const char* EnumToStr(IoInputGrabMode v);

namespace App {

// App
extern GVarEnum<AppState>      app_state;
extern GVarStr<100>            app_language;
extern GVarStr<50>             app_locale;
extern GVarPod<bool>           app_multithread;
extern GVarStr<50>             app_screenshot_format;

// Simulation
extern GVarEnum<SimState>      sim_state;
extern GVarStr<200>            sim_terrain_name;
extern GVarPod<bool>           sim_replay_enabled;
extern GVarPod<int>            sim_replay_length;
extern GVarPod<int>            sim_replay_stepping;
extern GVarPod<bool>           sim_position_storage;
extern GVarEnum<SimGearboxMode>sim_gearbox_mode;

// Multiplayer
extern GVarEnum<MpState>       mp_state;
extern GVarStr<200>            mp_server_host;
extern GVarPod<int>            mp_server_port;
extern GVarStr<100>            mp_server_password;
extern GVarStr<100>            mp_player_name;
extern GVarStr<400>            mp_portal_url;

// Diagnostic
extern GVarPod<bool>           diag_trace_globals;
extern GVarPod<bool>           diag_rig_log_node_import;
extern GVarPod<bool>           diag_rig_log_node_stats;
extern GVarPod<bool>           diag_rig_log_messages;
extern GVarPod<bool>           diag_collisions;
extern GVarPod<bool>           diag_truck_mass;
extern GVarPod<bool>           diag_envmap;
extern GVarPod<bool>           diag_videocameras;
extern GVarStr<100>            diag_preset_terrain;
extern GVarStr<100>            diag_preset_vehicle;
extern GVarStr<100>            diag_preset_veh_config;
extern GVarPod<bool>           diag_preset_veh_enter;
extern GVarPod<bool>           diag_log_console_echo;
extern GVarPod<bool>           diag_log_beam_break;
extern GVarPod<bool>           diag_log_beam_deform;
extern GVarPod<bool>           diag_log_beam_trigger;
extern GVarPod<bool>           diag_dof_effect;

// System
extern GVarStr<300>            sys_process_dir;
extern GVarStr<300>            sys_user_dir;
extern GVarStr<300>            sys_config_dir;
extern GVarStr<300>            sys_cache_dir;
extern GVarStr<300>            sys_logs_dir;
extern GVarStr<300>            sys_resources_dir;
extern GVarStr<300>            sys_profiler_dir;
extern GVarStr<300>            sys_screenshot_dir;

// Input - Output
extern GVarPod<bool>           io_ffb_enabled;
extern GVarPod<float>          io_ffb_camera_gain;
extern GVarPod<float>          io_ffb_center_gain;
extern GVarPod<float>          io_ffb_master_gain;
extern GVarPod<float>          io_ffb_stress_gain;
extern GVarEnum<IoInputGrabMode>io_input_grab_mode;
extern GVarPod<bool>           io_arcade_controls;
extern GVarPod<int>            io_outgauge_mode;
extern GVarStr<50>             io_outgauge_ip;
extern GVarPod<int>            io_outgauge_port;
extern GVarPod<float>          io_outgauge_delay;
extern GVarPod<int>            io_outgauge_id;

// Audio
extern GVarPod<float>          audio_master_volume;
extern GVarPod<bool>           audio_enable_creak;
extern GVarStr<100>            audio_device_name;
extern GVarPod<bool>           audio_menu_music;

// Graphics
extern GVarEnum<GfxFlaresMode> gfx_flares_mode;
extern GVarEnum<GfxShadowType> gfx_shadow_type;
extern GVarEnum<GfxExtCamMode> gfx_extcam_mode;
extern GVarEnum<GfxSkyMode>    gfx_sky_mode;
extern GVarEnum<GfxTexFilter>  gfx_texture_filter;
extern GVarEnum<GfxVegetation> gfx_vegetation_mode;
extern GVarEnum<GfxWaterMode>  gfx_water_mode;
extern GVarPod<bool>           gfx_enable_sunburn;
extern GVarPod<bool>           gfx_water_waves;
extern GVarPod<bool>           gfx_minimap_disabled;
extern GVarPod<int>            gfx_particles_mode;
extern GVarPod<bool>           gfx_enable_glow;
extern GVarPod<bool>           gfx_enable_hdr;
extern GVarPod<bool>           gfx_enable_heathaze;
extern GVarPod<bool>           gfx_enable_videocams;
extern GVarPod<bool>           gfx_envmap_enabled;
extern GVarPod<int>            gfx_envmap_rate;
extern GVarPod<int>            gfx_skidmarks_mode;
extern GVarPod<float>          gfx_sight_range;
extern GVarPod<float>          gfx_fov_external;
extern GVarPod<float>          gfx_fov_internal;
extern GVarPod<int>            gfx_fps_limit;

} // namespace App

template <typename T> void GVarPod<T>::SetPending(T val)
{
    if (val != m_value_pending)
    {
        if (App::diag_trace_globals.GetActive())
            this->LogSetPending(val);

        m_value_pending = val;
    }
}

template <typename T> void GVarPod<T>::ApplyPending()
{
    if (m_value_active != m_value_pending)
    {
        if (App::diag_trace_globals.GetActive())
            this->LogApplyPending();

        m_value_active = m_value_pending;
    }
}

template <typename T> void GVarPod<T>::SetActive(T val)
{
    if (val != m_value_active)
    {
        if (App::diag_trace_globals.GetActive())
            this->LogSetActive(val);

        m_value_active = val;
        m_value_pending = val;
    }
}

template <typename T> void GVarEnum<T>::SetPending(T val)
{
    if (val != m_value_pending)
    {
        if (App::diag_trace_globals.GetActive())
            this->LogSetPending(val);

        m_value_pending = val;
    }
}

template <typename T> void GVarEnum<T>::ApplyPending()
{
    if (m_value_active != m_value_pending)
    {
        if (App::diag_trace_globals.GetActive())
            this->LogApplyPending();

        m_value_active = m_value_pending;
    }
}

template <typename T> void GVarEnum<T>::SetActive(T val)
{
    if (val != m_value_active)
    {
        if (App::diag_trace_globals.GetActive())
            this->LogSetActive(val);

        m_value_active = val;
        m_value_pending = val;
    }
}



} // namespace RoR
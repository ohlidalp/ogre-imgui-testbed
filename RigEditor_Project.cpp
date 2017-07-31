
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


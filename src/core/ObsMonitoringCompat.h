#pragma once

// Include after <obs-module.h> and any JUCE headers: <obs.h> pulls in libobs's SIMD shim,
// which clashes with compiler intrinsics headers included after it on Linux.
#include <obs.h>

// Whether a source is set to "monitor only", i.e. audible on the monitoring device but
// muted in the outputs.
//
// OBS Studio 33 replaced the monitoring type with a plain on/off switch
// (obs_source_get_monitoring_enabled) and dropped "monitor only": monitored sources are
// always also sent to the outputs. obs_source_get_monitoring_type() is deprecated there and
// logs a warning on every call, so it is only queried when running on an older OBS.
inline bool obsSourceIsMonitorOnly(obs_source_t* source)
{
#if LIBOBS_API_MAJOR_VER >= 33
    (void)source;
    return false;
#else
    if (source == nullptr || obs_get_version() >= MAKE_SEMANTIC_VERSION(33, 0, 0))
        return false;

    return obs_source_get_monitoring_type(source) == OBS_MONITORING_TYPE_MONITOR_ONLY;
#endif
}

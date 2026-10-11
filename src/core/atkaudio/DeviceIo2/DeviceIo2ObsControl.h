#pragma once

#include <string>

struct obs_source;

namespace atk::deviceIo2ObsControl
{
struct SourceLevelState
{
    float sourceVolume = 1.0f;
    bool sourceMuted = false;
};

bool resolveDesiredBypass(const std::string& parentSourceUuid, bool& desiredBypass);
bool readSourceLevelState(obs_source* source, SourceLevelState& state);
double getCurrentTransitionFadeSeconds();
} // namespace atk::deviceIo2ObsControl

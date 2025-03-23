#include "../debuglib.h"

#include "debuglocationsplitter.h"

#include "Meta/keyvalue/valuetype.h"

namespace Engine {
namespace Debug {

    DebugLocationSplitter::DebugLocationSplitter(size_t channelCount)
        : mChildLocations(channelCount - 1)
    {
    }

    ParentLocation *DebugLocationSplitter::channel(size_t index)
    {
        return index == 0 ? this : &mChildLocations[index - 1];
    }

    const std::vector<ParentLocation> &DebugLocationSplitter::channels() const
    {
        return mChildLocations;
    }

    void DebugLocationSplitter::stepInto(ParentLocation *parent)
    {
        DebugLocation::stepInto(parent);
        for (ParentLocation& location : mChildLocations) {
            location.mContext = mContext;
        }
    }

    std::string DebugLocationSplitter::toString() const
    {
        return "Splitter";
    }

    std::map<std::string_view, ValueType> DebugLocationSplitter::localVariables() const
    {
        return {};
    }

    bool DebugLocationSplitter::wantsPause(Debug::ContinuationType type) const
    {
        return false;
    }

}
}
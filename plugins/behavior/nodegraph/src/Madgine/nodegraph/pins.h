#pragma once

#include "Generic/indextype.h"

#include "Meta/keyvalue/valuetype_desc.h"

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        struct MADGINE_NODEGRAPH_EXPORT Pin {
            IndexType<uint32_t, 0> mNode;
            IndexType<uint32_t> mIndex;
            uint32_t mGroup = 0;

            explicit operator bool() const;
            bool operator==(const Pin &other)
            {
                return mNode == other.mNode && mIndex == other.mIndex && mGroup == other.mGroup;
            }
        };

        struct FlowOutPinPrototype {
            Pin mTarget;
        };

        struct FlowInPinPrototype {
            std::vector<Pin> mSources;
        };

        struct DataInPinPrototype {
            Pin mSource;
        };

        struct DataOutPinPrototype {
            std::vector<Pin> mTargets;
        };

        enum class PinDir {
            In,
            Out
        };

        enum class PinType {
            Flow,
            Data
        };

        struct PinDesc {
            PinDir mDir;
            PinType mType;
            Pin mPin;

            bool isCompatible(const PinDesc &other) const
            {
                return mType == other.mType;
            }
        };

    }
}
}
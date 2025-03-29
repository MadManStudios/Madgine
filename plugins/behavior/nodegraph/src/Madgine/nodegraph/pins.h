#pragma once

#include "Generic/indextype.h"
#include "Meta/keyvalue/valuetype_desc.h"

namespace Engine {
namespace NodeGraph {

    struct MADGINE_NODEGRAPH_EXPORT Pin {
        IndexType<uint32_t, 0> mNode;
        IndexType<uint32_t> mIndex;
        uint32_t mGroup = 0;

        explicit operator bool() const;
        bool operator==(const Pin& other) {
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
        Pin mTarget;
    };

    struct DataReceiverPinPrototype {        
        std::vector<Pin> mSources;
    };

    struct DataProviderPinPrototype {
        std::vector<Pin> mTargets;
    };

    enum class PinDir {
        In,
        Out
    };

    enum class PinType {
        Flow,
        Data,
        DataInstance
    };

    struct PinDesc {
        PinDir mDir;
        PinType mType;
        Pin mPin;

        bool isCompatible(const PinDesc& other) const {
            if (mDir == other.mDir)
                return false;
            if (mType == PinType::Flow || other.mType == PinType::Flow)
                return mType == other.mType;
            return mType != other.mType;
        }
    };

}
}


#pragma once

#include "../../nodebase.h"
#include "../../nodegraph.h"

namespace Engine {
namespace NodeGraph {

    template <typename Base = NodeBase>
    struct AutoMaskNode : Base {
        using Base::Base;

        uint32_t inMask(uint32_t index, uint32_t group, bool bidir = true) const
        {
            uint32_t mask = NodeExecutionMask::ALL;
            for (auto &group : this->mFlowOutPins) {
                for (const FlowOutPinPrototype &pin : group) {
                    if (pin.mTarget) {
                        mask &= this->mGraph.flowInMask(pin.mTarget, false);
                    }
                }
            }
            for (auto &group : this->mDataProviderPins) {
                for (const DataProviderPinPrototype &pin : group) {
                    for (Pin target : pin.mTargets) {
                        mask &= this->mGraph.dataInMask(target, false);
                    }
                }
            }
            for (auto &group : this->mDataOutPins) {
                for (const DataOutPinPrototype &pin : group) {
                    if (pin.mTarget) {
                        mask &= this->mGraph.dataReceiverMask(pin.mTarget, false);
                    }
                }
            }
            if (bidir) {
                mask &= outMask(0, 0, false);
            }
            return mask;
        }

        uint32_t outMask(uint32_t index, uint32_t group, bool bidir = true) const
        {
            uint32_t mask = NodeExecutionMask::ALL;
            for (auto &group : this->mFlowInPins) {
                for (const FlowInPinPrototype &pin : group) {
                    for (Pin source : pin.mSources) {
                        mask &= this->mGraph.flowOutMask(source, false);
                    }
                }
            }
            for (auto &group : this->mDataInPins) {
                for (const DataInPinPrototype &pin : group) {
                    if (pin.mSource) {
                        mask &= this->mGraph.dataProviderMask(pin.mSource, false);
                    }
                }
            }
            for (auto &group : this->mDataReceiverPins) {
                for (const DataReceiverPinPrototype &pin : group) {
                    for (Pin source : pin.mSources) {
                        mask &= this->mGraph.dataOutMask(source, false);
                    }
                }
            }
            if (bidir) {
                mask &= inMask(0, 0, false);
            }
            return mask;
        }
        
        virtual uint32_t flowInMask(uint32_t index, uint32_t group, bool bidir = true) const override
        {
            return inMask(0, 0, bidir);
        }

        virtual uint32_t flowOutMask(uint32_t index, uint32_t group, bool bidir = true) const override
        {
            return outMask(0, 0, bidir);
        }

        virtual uint32_t dataInMask(uint32_t index, uint32_t group, bool bidir = true) const override
        {
            return inMask(0, 0, bidir);
        }

        virtual uint32_t dataOutMask(uint32_t index, uint32_t group, bool bidir = true) const override
        {
            return outMask(0, 0, bidir);
        }

        virtual uint32_t dataReceiverMask(uint32_t index, uint32_t group, bool bidir = true) const override
        {
            return inMask(0, 0, bidir);
        }

        virtual uint32_t dataProviderMask(uint32_t index, uint32_t group, bool bidir = true) const override
        {
            return outMask(0, 0, bidir);
        }


    private:
    };

}

}
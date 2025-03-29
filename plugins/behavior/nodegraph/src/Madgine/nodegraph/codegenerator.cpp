#include "../nodegraphlib.h"

#include "codegenerator.h"

#include "nodegraph.h"

namespace Engine {
namespace NodeGraph {

    CodeGenerator::CodeGenerator(const NodeGraph &graph)
        : mGraph(graph)
        , mData(graph.nodes().size())
    {
    }

    void CodeGenerator::generate(uint32_t flowIn, const NodeBase *startNode)
    {
        Pin pin;
        if (startNode) {
            pin.mIndex = flowIn;
            pin.mNode = mGraph.nodeIndex(startNode);
        } else {
            pin = mGraph.mFlowOutPins[flowIn].mTarget;
        }

        generate(pin);
    }

    void CodeGenerator::generate(Pin pin)
    {
        if (pin && pin.mNode) {
            const NodeBase *node = mGraph.node(pin.mNode);
            node->generate(*this, mData[pin.mNode - 1], pin.mIndex, pin.mGroup);
        }
    }

    CodeGen::Statement CodeGenerator::read(Pin pin)
    {
        if (!pin) {
            throw 0;
        } else if (!pin.mNode) {
            //retVal = mCurrentArguments->at(pin.mIndex);
            throw 0;
        } else {
            return mGraph.node(pin.mNode)->generateRead(*this, mData[pin.mNode - 1], pin.mIndex, pin.mGroup);
        }
    }

    CodeGen::Statement CodeGenerator::write(Pin pin, CodeGen::Statement statement)
    {
        if (pin)
            return mGraph.node(pin.mNode)->generateWrite(*this, mData[pin.mNode - 1], pin.mIndex, pin.mGroup);
        else
            throw 0;
    }

}
}
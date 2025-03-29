#pragma once

#include "../toolbase.h"
#include "../toolscollector.h"

#include "Generic/flags.h"
#include "Generic/genericresult.h"

#include "Meta/keyvalue/valuetype.h"

namespace Engine {
namespace Tools {

    struct TestTool : Tool<TestTool> {

        TestTool(ImRoot &root);

        virtual void render() override;
        virtual void update() override;

        std::string_view key() const override;

        void logTest();
        void logValue(const ValueType &v);
        int dummy();

        ValueType mV;

        Flags<GenericResult> mFlags;

    private:
        bool mRenderSphere = false;
        int mSphereDetail = 1;

        bool mRenderText = false;
        std::string m3DText = "Test_1234 Hello World!";

        bool mRenderArrow = false;
    };

}
}

REGISTER_TYPE(Engine::Tools::TestTool)
#pragma once

#include "Modules/debug/history.h"

#include "DirectX12/util/directx12texture.h"
#include "Madgine_Tools/render/rendercontexttool.h"
#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {
namespace Tools {

    struct DirectX12RenderContextTool : public ToolVirtualImpl<DirectX12RenderContextTool, RenderContextTool> {

        SERIALIZABLEUNIT(DirectX12RenderContextTool)

        DirectX12RenderContextTool(ImRoot &root);

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        void update() override;
        void renderMetrics() override;

        std::string_view key() const override;

    private:
        size_t mLastFrameTempBytes = 0;        
        float mTimeBank = 0.0f;

        Debug::History<float, 120> mTempBytesPerFrame;
        Debug::History<float, 100> mTempBytesPerFrameTrend;

        Debug::History<float, 120> mUploadBytesPerFrame;
        Debug::History<float, 100> mUploadBytesPerFrameTrend;
    };

}
}
#pragma once

namespace Engine {

namespace Core {

    struct MainWindow;
    struct ToolWindow;

    struct MainWindowComponentBase;
    struct MainWindowComponentComparator;
}

namespace Render {
    struct RendererBase;
    struct RenderContext;
    struct RenderTarget;
    struct RenderTextureConfig;
    struct RenderPass;
    struct RenderData;
    struct RenderDebuggable;
    struct Camera;

    struct ResourceBlockSignature;
    struct PipelineSignature;

    enum ShaderType {
        VertexShader,
        PixelShader
    };

    struct ShaderObjectBase;

}

}

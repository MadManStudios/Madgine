#pragma once

#include "Generic/execution/intervalclock.h"

#include "Platform/filesystem/path.h"

#include "Meta/math/vector2.h"

#include "Madgine/imageloader/imageloader.h"
#include "Madgine/render/pipelineloader.h"
#include "Madgine/render/renderdata.h"
#include "Madgine/window/mainwindowcomponent.h"
#include "Madgine/window/mainwindowcomponentcollector.h"

#include "Madgine_Tools/renderer/imroot.h"

struct ImGuiDockNode;
struct ImGuiViewport;
struct ImTextureData;

namespace Engine {
namespace Tools {

    struct ClientImRoot;

    struct ImGuiRenderData : Render::RenderData {
        ImGuiRenderData(ClientImRoot &root);

        Threading::ImmediateTask<Render::RenderFuture> render(Render::RenderContext *context) override;

        ClientImRoot &mRoot;

        Execution::IntervalClock<> mFrameClock = std::chrono::steady_clock::now();
    };

    struct MADGINE_CLIENT_TOOLS_EXPORT ClientImRoot : Core::MainWindowComponent<ClientImRoot>,
                                                      ImRoot {

        ClientImRoot(Core::MainWindow &window);
        ~ClientImRoot();

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        void setup(Render::RenderTarget *target) override;
        void render(Render::RenderTarget *target, size_t iteration) override;
        void shutdown(Render::RenderTarget *target) override;

        void renderViewport(Render::RenderTarget *target, ImGuiViewport *vp);
        void updateTexture(ImTextureData *tex);

        void addViewportMapping(Render::RenderTarget *target, ImGuiViewport *vp);
        void removeViewportMapping(Render::RenderTarget *target);

        bool onWindowEvent(const Platform::Window::WindowEvent &arg) override;
        bool injectKeyPress(const Platform::Input::KeyPressEvent &arg);
        bool injectKeyRelease(const Platform::Input::KeyReleaseEvent &arg);
        bool injectPointerPress(const Platform::Input::PointerPressEvent &arg);
        bool injectPointerRelease(const Platform::Input::PointerReleaseEvent &arg);
        bool injectPointerMove(const Platform::Input::PointerMoveEvent &arg);
        bool injectAxisEvent(const Platform::Input::AxisEvent &arg);

        bool wantsSoftwareKeyboard() const override;

        void setCentralNode();

        Math::Rect2i getChildClientSpace() override;

        bool includeInLayout() const override;

        Platform::Filesystem::Path findDataFile(std::string_view name) const override;

        Threading::TaskQueue *taskQueue() const override;

        void Image(Render::ConstTexturePtr tex, Math::Vector2i image_size = { -1, -1 }, const Math::Vector2 &uv0 = { 0, 0 }, const Math::Vector2 &uv1 = { 1, 1 }) override;
        void Image(const Platform::Filesystem::Path &path, Math::Vector2i image_size = { -1, -1 }) override;
        void DrawImage(const Platform::Filesystem::Path &path, Math::Vector2i pos, Math::Vector2i image_size = { -1, -1 }, float spinnerRadius = 15) override;

        void addRenderTarget(Render::RenderTarget *target);
        void removeRenderTarget(Render::RenderTarget *target);

        Math::Vector2 mLeftControllerStick, mRightControllerStick;
        int mDPadState = 0;
        float mZAxis = 0;

    private:
        std::map<Render::RenderTarget *, ImGuiViewport *> mViewportMappings;

        Math::Vector2 mAreaPos = Math::Vector2::ZERO;
        Math::Vector2 mAreaSize = Math::Vector2::ZERO;

        Platform::Filesystem::Path mImGuiIniFilePath;

        std::vector<Render::RenderTarget *> mRenderTargets;

        Memory::ByteBuffer mIconsData;

        struct CachedImage {
            Resources::ImageLoader::Handle mHandle;
            Render::TexturePtr mTexture;
        };
        std::map<Platform::Filesystem::Path, CachedImage> mImageCache;
        std::vector<Render::ConstTexturePtr> mTextureCache;

        friend struct ImGuiRenderData;
        ImGuiRenderData mRenderData;
    };

}
}

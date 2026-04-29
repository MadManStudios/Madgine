#pragma once

#include "Generic/intervalclock.h"

#include "Interfaces/filesystem/path.h"

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

        IntervalClock<> mFrameClock = std::chrono::steady_clock::now();
    };

    struct MADGINE_CLIENT_TOOLS_EXPORT ClientImRoot : Window::MainWindowComponent<ClientImRoot>,
                                                      ImRoot {

        ClientImRoot(Window::MainWindow &window);
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

        bool onWindowEvent(const Window::WindowEvent &arg) override;
        bool injectKeyPress(const Input::KeyPressEvent &arg);
        bool injectKeyRelease(const Input::KeyReleaseEvent &arg);
        bool injectPointerPress(const Input::PointerPressEvent &arg);
        bool injectPointerRelease(const Input::PointerReleaseEvent &arg);
        bool injectPointerMove(const Input::PointerMoveEvent &arg);
        bool injectAxisEvent(const Engine::Input::AxisEvent &arg);

        bool wantsSoftwareKeyboard() const override;

        void setCentralNode();

        Rect2i getChildClientSpace() override;

        bool includeInLayout() const override;

        Filesystem::Path findDataFile(std::string_view name) const override;

        Threading::TaskQueue *taskQueue() const override;

        void Image(Render::ConstTexturePtr tex, Vector2i image_size = { -1, -1 }, const Vector2 &uv0 = { 0, 0 }, const Vector2 &uv1 = { 1, 1 }) override;
        void Image(const Filesystem::Path &path, Vector2i image_size = { -1, -1 }) override;
        void DrawImage(const Filesystem::Path &path, Vector2i pos, Vector2i image_size = { -1, -1 }, float spinnerRadius = 15) override;

        void addRenderTarget(Render::RenderTarget *target);
        void removeRenderTarget(Render::RenderTarget *target);

        Vector2 mLeftControllerStick, mRightControllerStick;
        int mDPadState = 0;
        float mZAxis = 0;

    private:
        std::map<Render::RenderTarget *, ImGuiViewport *> mViewportMappings;

        Vector2 mAreaPos = Vector2::ZERO;
        Vector2 mAreaSize = Vector2::ZERO;

        Filesystem::Path mImGuiIniFilePath;

        std::vector<Render::RenderTarget *> mRenderTargets;

        ByteBuffer mIconsData;

        struct CachedImage {
            Resources::ImageLoader::Handle mHandle;
            Render::TexturePtr mTexture;
        };
        std::map<Filesystem::Path, CachedImage> mImageCache;
        std::vector<Render::ConstTexturePtr> mTextureCache;

        friend struct ImGuiRenderData;
        ImGuiRenderData mRenderData;
    };

}
}

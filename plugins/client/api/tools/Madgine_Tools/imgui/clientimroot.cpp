#include "../clienttoolslib.h"

#include "clientimroot.h"

#include "Platform/filesystem/fsapi.h"
#include "Platform/input/inputevents.h"
#include "Platform/window/windowapi.h"
#include "Platform/window/windowsettings.h"

#include "Modules/debug/profiler/profile.h"

#include "Madgine/imageloader/imageloader.h"
#include "Madgine/render/fonts/fontloader.h"
#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/render/texture.h"
#include "Madgine/resources/resourcemanager.h"
#include "Madgine/window/mainwindow.h"
#include "Madgine/window/toolwindow.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/toolbase.h"
#include "im3d/im3d.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"
#include "imgui/misc/freetype/imgui_freetype.h"
#include "imgui_hlsl.h"

METATABLE_BEGIN_BASE(Engine::Tools::ClientImRoot, Engine::Tools::ImRoot)
METATABLE_END(Engine::Tools::ClientImRoot)

SERIALIZETABLE_BEGIN(Engine::Tools::ClientImRoot)
SERIALIZETABLE_END(Engine::Tools::ClientImRoot)

NAMED_UNIQUECOMPONENT(ClientImRoot, Engine::Tools::ClientImRoot)

namespace Engine {
namespace Tools {

    static std::map<Platform::Input::Key::Key, ImGuiKey> sKeyMap {
        { Platform::Input::Key::Tabulator, ImGuiKey_Tab },
        { Platform::Input::Key::LeftArrow, ImGuiKey_LeftArrow },
        { Platform::Input::Key::RightArrow, ImGuiKey_RightArrow },
        { Platform::Input::Key::UpArrow, ImGuiKey_UpArrow },
        { Platform::Input::Key::DownArrow, ImGuiKey_DownArrow },
        { Platform::Input::Key::PageUp, ImGuiKey_PageUp },
        { Platform::Input::Key::PageDown, ImGuiKey_PageDown },
        { Platform::Input::Key::Home, ImGuiKey_Home },
        { Platform::Input::Key::End, ImGuiKey_End },
        { Platform::Input::Key::Insert, ImGuiKey_Insert },
        { Platform::Input::Key::Delete, ImGuiKey_Delete },
        { Platform::Input::Key::Backspace, ImGuiKey_Backspace },
        { Platform::Input::Key::Space, ImGuiKey_Space },
        { Platform::Input::Key::Return, ImGuiKey_Enter },
        { Platform::Input::Key::Escape, ImGuiKey_Escape },
        { Platform::Input::Key::LShift, ImGuiKey_LeftShift },
        { Platform::Input::Key::Shift, ImGuiKey_LeftShift },
        { Platform::Input::Key::RShift, ImGuiKey_RightShift },
        { Platform::Input::Key::LAlt, ImGuiKey_LeftAlt },
        { Platform::Input::Key::Alt, ImGuiKey_LeftAlt },
        { Platform::Input::Key::RAlt, ImGuiKey_RightAlt },
        { Platform::Input::Key::LControl, ImGuiKey_LeftCtrl },
        { Platform::Input::Key::Control, ImGuiKey_LeftCtrl },
        { Platform::Input::Key::RControl, ImGuiKey_RightCtrl },
        { Platform::Input::Key::A, ImGuiKey_A },
        { Platform::Input::Key::B, ImGuiKey_B },
        { Platform::Input::Key::C, ImGuiKey_C },
        { Platform::Input::Key::D, ImGuiKey_D },
        { Platform::Input::Key::E, ImGuiKey_E },
        { Platform::Input::Key::F, ImGuiKey_F },
        { Platform::Input::Key::G, ImGuiKey_G },
        { Platform::Input::Key::H, ImGuiKey_H },
        { Platform::Input::Key::I, ImGuiKey_I },
        { Platform::Input::Key::J, ImGuiKey_J },
        { Platform::Input::Key::K, ImGuiKey_K },
        { Platform::Input::Key::L, ImGuiKey_L },
        { Platform::Input::Key::M, ImGuiKey_M },
        { Platform::Input::Key::N, ImGuiKey_N },
        { Platform::Input::Key::O, ImGuiKey_O },
        { Platform::Input::Key::P, ImGuiKey_P },
        { Platform::Input::Key::Q, ImGuiKey_Q },
        { Platform::Input::Key::R, ImGuiKey_R },
        { Platform::Input::Key::S, ImGuiKey_S },
        { Platform::Input::Key::T, ImGuiKey_T },
        { Platform::Input::Key::U, ImGuiKey_U },
        { Platform::Input::Key::V, ImGuiKey_V },
        { Platform::Input::Key::W, ImGuiKey_W },
        { Platform::Input::Key::X, ImGuiKey_X },
        { Platform::Input::Key::Y, ImGuiKey_Y },
        { Platform::Input::Key::Z, ImGuiKey_Z },
        { Platform::Input::Key::Alpha0, ImGuiKey_0 },
        { Platform::Input::Key::Alpha1, ImGuiKey_1 },
        { Platform::Input::Key::Alpha2, ImGuiKey_2 },
        { Platform::Input::Key::Alpha3, ImGuiKey_3 },
        { Platform::Input::Key::Alpha4, ImGuiKey_4 },
        { Platform::Input::Key::Alpha5, ImGuiKey_5 },
        { Platform::Input::Key::Alpha6, ImGuiKey_6 },
        { Platform::Input::Key::Alpha7, ImGuiKey_7 },
        { Platform::Input::Key::Alpha8, ImGuiKey_8 },
        { Platform::Input::Key::Alpha9, ImGuiKey_9 },
        { Platform::Input::Key::Comma, ImGuiKey_Comma },
        { Platform::Input::Key::Period, ImGuiKey_Period },
        { Platform::Input::Key::Minus, ImGuiKey_Minus }
    };

    static void CreateImGuiToolWindow(ImGuiViewport *vp)
    {

        ImGuiIO &io = ImGui::GetIO();
        Core::MainWindow *topLevel = static_cast<Core::MainWindow *>(io.BackendPlatformUserData);

        Platform::Window::WindowSettings settings;
        settings.mHeadless = true;
        settings.mHidden = true;
        Core::ToolWindow *window = topLevel->createToolWindow(settings);
        vp->PlatformUserData = window;
        vp->PlatformHandle = window->osWindow();
        vp->PlatformHandleRaw = window->osWindow()->ptrHandle();

        ClientImRoot *root = static_cast<ClientImRoot *>(io.UserData);
        root->addViewportMapping(window->getRenderer(), vp);
        window->getRenderer()->addRenderPass(root);
    }
    static void DestroyImGuiToolWindow(ImGuiViewport *vp)
    {
        if (vp->PlatformUserData) {
            Core::ToolWindow *toolWindow = static_cast<Core::ToolWindow *>(vp->PlatformUserData);
            vp->PlatformUserData = nullptr;
            vp->PlatformHandle = nullptr;
            vp->PlatformHandleRaw = nullptr;
            toolWindow->close();

            ImGuiIO &io = ImGui::GetIO();
            static_cast<ClientImRoot *>(io.UserData)->removeViewportMapping(toolWindow->getRenderer());
        }
    }
    static void ShowImGuiToolWindow(ImGuiViewport *vp)
    {
        Platform::Window::OSWindow *w = static_cast<Platform::Window::OSWindow *>(vp->PlatformHandle);
        w->show();
    }
    static void SetImGuiToolWindowPos(ImGuiViewport *vp, ImVec2 pos)
    {
        ImGuiIO &io = ImGui::GetIO();
        Platform::Window::OSWindow *w = static_cast<Platform::Window::OSWindow *>(vp->PlatformHandle);
        w->setRenderPos({ static_cast<int>(pos.x * io.DisplayFramebufferScale.x), static_cast<int>(pos.y * io.DisplayFramebufferScale.y) });
    }
    static ImVec2 GetImGuiToolWindowPos(ImGuiViewport *vp)
    {
        ImGuiIO &io = ImGui::GetIO();
        Platform::Window::OSWindow *w = static_cast<Platform::Window::OSWindow *>(vp->PlatformHandle);
        return { static_cast<float>(w->renderPos().x / io.DisplayFramebufferScale.x), static_cast<float>(w->renderPos().y / io.DisplayFramebufferScale.y) };
    }
    static void SetImGuiToolWindowSize(ImGuiViewport *vp, ImVec2 size)
    {
        ImGuiIO &io = ImGui::GetIO();
        Platform::Window::OSWindow *w = static_cast<Platform::Window::OSWindow *>(vp->PlatformHandle);
        w->setRenderSize({ static_cast<int>(size.x * io.DisplayFramebufferScale.x), static_cast<int>(size.y * io.DisplayFramebufferScale.y) });
    }
    static ImVec2 GetImGuiToolWindowSize(ImGuiViewport *vp)
    {
        ImGuiIO &io = ImGui::GetIO();
        Platform::Window::OSWindow *w = static_cast<Platform::Window::OSWindow *>(vp->PlatformHandle);
        return { static_cast<float>(w->renderSize().x / io.DisplayFramebufferScale.x), static_cast<float>(w->renderSize().y / io.DisplayFramebufferScale.y) };
    }
    static void SetImGuiToolWindowFocus(ImGuiViewport *vp)
    {
        Platform::Window::OSWindow *w = static_cast<Platform::Window::OSWindow *>(vp->PlatformHandle);
        w->focus();
    }
    static bool GetImGuiToolWindowFocus(ImGuiViewport *vp)
    {
        Platform::Window::OSWindow *w = static_cast<Platform::Window::OSWindow *>(vp->PlatformHandle);
        return w->hasFocus();
    }
    static bool GetImGuiToolWindowMinimized(ImGuiViewport *vp)
    {
        Platform::Window::OSWindow *w = static_cast<Platform::Window::OSWindow *>(vp->PlatformHandle);
        return w->isMinimized();
    }
    static void SetImGuiToolWindowTitle(ImGuiViewport *vp, const char *title)
    {
        Platform::Window::OSWindow *w = static_cast<Platform::Window::OSWindow *>(vp->PlatformHandle);
        w->setTitle(title);
    }

    ClientImRoot::ClientImRoot(Core::MainWindow &window)
        : MainWindowComponent(window, 80)
        , mImGuiIniFilePath(Platform::Filesystem::appDataPath() / "imgui.ini")
        , mRenderData(*this)
    {
    }

    ClientImRoot::~ClientImRoot()
    {
    }

    Threading::Task<bool> ClientImRoot::init()
    {
        if (!co_await MainWindowComponentBase::init())
            co_return false;

        ImGui::CreateContext();
        Im3D::CreateContext();

        ImGuiIO &io = ImGui::GetIO();
        io.UserData = this;

        io.IniFilename = mImGuiIniFilePath.c_str();

        io.DisplayFramebufferScale = ImVec2 { Platform::Window::platformCapabilities.mScalingFactor, Platform::Window::platformCapabilities.mScalingFactor };

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        if (Platform::Window::platformCapabilities.mSupportMultipleWindows) {
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
            platform_io.Platform_CreateWindow = CreateImGuiToolWindow;
            platform_io.Platform_DestroyWindow = DestroyImGuiToolWindow;
            platform_io.Platform_ShowWindow = ShowImGuiToolWindow;
            platform_io.Platform_SetWindowPos = SetImGuiToolWindowPos;
            platform_io.Platform_GetWindowPos = GetImGuiToolWindowPos;
            platform_io.Platform_SetWindowSize = SetImGuiToolWindowSize;
            platform_io.Platform_GetWindowSize = GetImGuiToolWindowSize;
            platform_io.Platform_SetWindowFocus = SetImGuiToolWindowFocus;
            platform_io.Platform_GetWindowFocus = GetImGuiToolWindowFocus;
            platform_io.Platform_GetWindowMinimized = GetImGuiToolWindowMinimized;
            platform_io.Platform_SetWindowTitle = SetImGuiToolWindowTitle;
            /*platform_io.Platform_RenderWindow = RenderImGuiToolWindow;
                platform_io.Platform_SwapBuffers = SwapImGuiToolWindowBuffers;*/

            io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports | ImGuiBackendFlags_PlatformHasViewports;

            platform_io.Monitors.clear();
            for (Platform::Window::MonitorInfo info : Platform::Window::listMonitors()) {
                ImGuiPlatformMonitor monitor;
                monitor.MainPos = monitor.WorkPos = ImVec2 { static_cast<float>(info.mPosition.x), static_cast<float>(info.mPosition.y) };
                monitor.MainSize = monitor.WorkSize = ImVec2 { static_cast<float>(info.mSize.x), static_cast<float>(info.mSize.y) };
                platform_io.Monitors.push_back(monitor);
            }

            ImGuiViewport *main_viewport = ImGui::GetMainViewport();
            main_viewport->PlatformHandle = mWindow.osWindow();
        }

        Im3D::GetIO().mFetchFont = [](const char *fontName) {
            Render::FontLoader::Handle font;
            font.load(fontName);
            font.info()->setPersistent(true);

            if (font.available()) {
                return Im3DFont {
                    (Im3DTextureId)font->mTexture->resourceBlock(),
                    font->mTexture->size(),
                    font->mFonts.at(Render::FontStyle::Default).data()
                };
            } else {
                return Im3DFont {};
            }
        };

        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_RendererHasTextures;

        ImGui::FilesystemPickerOptions *filepickerOptions = ImGui::GetFilesystemPickerOptions();

        filepickerOptions->mIconLookup = [](const Platform::Filesystem::Path &path, bool isDir) {
            if (isDir)
                return IMGUI_ICON_FOLDER " ";
            else
                return IMGUI_ICON_FILE " ";
        };

        co_await Engine::Resources::ResourceManager::getSingleton().state();

        if (!co_await ImRoot::init())
            co_return false;

        ImFontConfig defaultConfig {};
        defaultConfig.SizePixels = 13.0f * Platform::Window::platformCapabilities.mScalingFactor;
        defaultConfig.RasterizerDensity = 2.0f;
        io.FontDefault = io.Fonts->AddFontDefaultVector(&defaultConfig);
        ImGui::GetStyle().FontSizeBase = 16.0f;

        Platform::Filesystem::Path iconsPath = Resources::ResourceManager::getSingleton().findResourceFile("icons.ttf");
        auto iconsResult = co_await Platform::Filesystem::readFileAsync(iconsPath);
        if (iconsResult.is_value()) {
            mIconsData = std::move(iconsResult).value().get();

            static const ImWchar icons_ranges[] = { 0xf100, 0xf1ff, 0 };

            ImFontConfig config;
            config.MergeMode = true;
            config.FontLoaderFlags |= ImGuiFreeTypeLoaderFlags_LoadColor;
            // config.GlyphMinAdvanceX = 13.0f;
            config.GlyphOffset = { 0.0f, 3.0f * Platform::Window::platformCapabilities.mScalingFactor };
            config.FontDataOwnedByAtlas = false;
            config.RasterizerDensity = 5.0f;

            io.Fonts->AddFontFromMemoryTTF(const_cast<void *>(mIconsData.mData), mIconsData.mSize, 13.0f * Platform::Window::platformCapabilities.mScalingFactor, &config, icons_ranges);
        } else {
            LOG_ERROR("Reading icons.ttf failed!");
        }

        //io.FontGlobalScale = 1.0f / Window::platformCapabilities.mScalingFactor;

        addDependency(&mRenderData);

        co_return true;
    }

    Threading::Task<void> ClientImRoot::finalize()
    {
        for (ImTextureData *tex : ImGui::GetPlatformIO().Textures) {
            if (Render::TexturePtr *ptr = static_cast<Render::TexturePtr *>(tex->BackendUserData)) {
                ptr->reset();
            }
        }

        mTextureCache.clear();

        removeDependency(&mRenderData);

        ImGuiIO &io = ImGui::GetIO();

        ImGui::SaveIniSettingsToDisk(io.IniFilename);

        io.IniFilename = nullptr;

        ImGui::DestroyPlatformWindows();

        co_await ImRoot::finalize();

        if (Platform::Window::platformCapabilities.mSupportMultipleWindows) {
            ImGuiViewport *main_viewport = ImGui::GetMainViewport();
            main_viewport->PlatformHandle = nullptr;
        }

        Im3D::DestroyContext();
        ImGui::DestroyContext();

        mIconsData.clear();
        mImageCache.clear();

        co_await MainWindowComponentBase::finalize();

        co_return;
    }

    void ClientImRoot::addRenderTarget(Render::RenderTarget *target)
    {
        addDependency(target);
    }

    void ClientImRoot::removeRenderTarget(Render::RenderTarget *target)
    {
        removeDependency(target);
    }

    static Platform::Input::CursorIcon convertCursorIcon(ImGuiMouseCursor cursor)
    {
#define HELPER(x)              \
    case ImGuiMouseCursor_##x: \
        return Platform::Input::CursorIcon::x;
        switch (cursor) {
            HELPER(Arrow)
            HELPER(TextInput)
            HELPER(ResizeAll)
            HELPER(ResizeNS)
            HELPER(ResizeEW)
            HELPER(ResizeNESW)
            HELPER(ResizeNWSE)
            HELPER(Hand)
            HELPER(NotAllowed)
        default:
            throw 0;
        }
    }

    void ClientImRoot::setup(Render::RenderTarget *target)
    {
        if (mWindow.getRenderWindow() == target) {
            setupImpl(target, HLSL::imgui_VS, HLSL::imgui_PS, { sizeof(Math::Matrix4) }, false);
        }

        MainWindowComponentBase::setup(target);
    }

    void ClientImRoot::render(Render::RenderTarget *target, size_t iteration)
    {
        PROFILE();

        if (mWindow.getRenderWindow() == target) {

            ImGuiIO &io = ImGui::GetIO();

            MainWindowComponentBase::render(target, iteration);

            io.MouseWheel += mZAxis * 0.3f;

            mWindow.osWindow()->setCursorIcon(convertCursorIcon(ImGui::GetMouseCursor()));

            ImGuiViewport *main_viewport = ImGui::GetMainViewport();
            main_viewport->Flags |= ImGuiViewportFlags_NoRendererClear; // TODO: Is that necessary every Frame?

            if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) {
                ImGui::Render();
            }

            renderViewport(target, main_viewport);
        } else {
            renderViewport(target, mViewportMappings.at(target));
        }
    }

    void ClientImRoot::shutdown(Render::RenderTarget *target)
    {
        if (mWindow.getRenderWindow() == target) {
            RenderPass::shutdown(target);
        }
    }

    void ClientImRoot::renderViewport(Render::RenderTarget *target, ImGuiViewport *vp)
    {
        if (!mPipeline.available())
            return;

        ImDrawData *draw_data = vp->DrawData;

        if (draw_data->Textures != nullptr)
            for (ImTextureData *tex : *draw_data->Textures)
                if (tex->Status != ImTextureStatus_OK)
                    updateTexture(tex);

        draw_data->ScaleClipRects(ImGui::GetIO().DisplayFramebufferScale);

        {
            auto mvp = mPipeline->mapParameters<Math::Matrix4>(0);

            float L = draw_data->DisplayPos.x;
            float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x /* / ImGui::GetIO().DisplayFramebufferScale.x*/;
            float T = draw_data->DisplayPos.y;
            float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y /* / ImGui::GetIO().DisplayFramebufferScale.y*/;
            *mvp.mData = target->getClipSpaceMatrix() * Math::Matrix4 { 2.0f / (R - L), 0.0f, 0.0f, (R + L) / (L - R), 0.0f, 2.0f / (T - B), 0.0f, (T + B) / (B - T), 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f };
        }

        using Vertex = Compound<Render::VertexPos2, Render::VertexColor, Render::VertexUV>;

        size_t vertexBufferCount = 0;
        size_t indexBufferCount = 0;
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList *cmd_list = draw_data->CmdLists[n];
            vertexBufferCount += cmd_list->VtxBuffer.Size;
            indexBufferCount += cmd_list->IdxBuffer.Size;
        }

        {
            auto vertices = mPipeline->mapVertices<Vertex[]>(target, vertexBufferCount);
            auto indices = mPipeline->mapIndices(target, indexBufferCount);

            Vertex *vertexTarget = vertices.mData;
            uint32_t *indexTarget = indices.mData;
            for (int n = 0; n < draw_data->CmdListsCount; n++) {
                const ImDrawList *cmd_list = draw_data->CmdLists[n];
                std::ranges::transform(cmd_list->VtxBuffer, vertexTarget, [](const ImDrawVert &v) {
                    Vertex result;
                    result.mPos2 = v.pos;
                    result.mColor = ImGui::ColorConvertU32ToFloat4(v.col);
                    result.mUV = v.uv;
                    return result;
                });
                std::ranges::copy(cmd_list->IdxBuffer, indexTarget);
                vertexTarget += cmd_list->VtxBuffer.Size;
                indexTarget += cmd_list->IdxBuffer.Size;
            }
        }

        mPipeline->setGroupSize(3);

        int global_vtx_offset = 0;
        int global_idx_offset = 0;
        ImVec2 clip_off = draw_data->DisplayPos;
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList *cmd_list = draw_data->CmdLists[n];
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != NULL) {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                        throw 0;
                    else
                        pcmd->UserCallback(cmd_list, pcmd);
                } else {
                    // Apply Scissor, Bind texture, Draw
                    const Math::Rect2i r = { { (int)(pcmd->ClipRect.x - clip_off.x), (int)(pcmd->ClipRect.y - clip_off.y) }, { (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y) } };
                    if (r.bottomRight().x > r.mTopLeft.x && r.bottomRight().y > r.mTopLeft.y) {
                        ImTextureID tex = pcmd->GetTexID();
                        const Render::Texture *texture = reinterpret_cast<const Render::Texture *>(tex);
                        mPipeline->bindResources(target, 2, texture->resourceBlock());

                        target->setScissorsRect(r);
                        mPipeline->renderRange(target, pcmd->ElemCount, pcmd->VtxOffset + global_vtx_offset, pcmd->IdxOffset + global_idx_offset);
                    }
                }
            }
            global_idx_offset += cmd_list->IdxBuffer.Size;
            global_vtx_offset += cmd_list->VtxBuffer.Size;
        }
    }

    void ClientImRoot::updateTexture(ImTextureData *tex)
    {
        switch (tex->Status) {
        case ImTextureStatus_WantCreate: {
            Render::TexturePtr ptr = Render::RenderContext::getSingleton().createTexture(Render::TextureType_2D, Render::FORMAT_RGBA8_SRGB, { tex->Width, tex->Height }, { tex->GetPixels(), static_cast<size_t>(tex->Width * tex->Height * 4) });

            tex->SetTexID(reinterpret_cast<ImTextureID>(ptr.get()));
            tex->BackendUserData = new Render::TexturePtr { std::move(ptr) };

            tex->SetStatus(ImTextureStatus_OK);
            break;
        }
        case ImTextureStatus_WantUpdates: {
            Render::TexturePtr &ptr = *static_cast<Render::TexturePtr *>(tex->BackendUserData);

            for (int y = 0; y < tex->UpdateRect.h; y++) {
                const void *row = static_cast<uint32_t *>(tex->GetPixels()) + ((tex->UpdateRect.y + y) * tex->Width + tex->UpdateRect.x);
                mWindow.getRenderer()->setTextureSubData(ptr, { tex->UpdateRect.x, tex->UpdateRect.y + y }, { tex->UpdateRect.w, 1 }, { row, static_cast<size_t>(tex->UpdateRect.w * 4) });
            }

            tex->SetStatus(ImTextureStatus_OK);
            break;
        }
        case ImTextureStatus_WantDestroy:
            if (tex->UnusedFrames > 0) {
                Render::TexturePtr &ptr = *static_cast<Render::TexturePtr *>(tex->BackendUserData);
                tex->BackendUserData = nullptr;
                tex->SetTexID(ImTextureID_Invalid);
                ptr.reset();
                tex->SetStatus(ImTextureStatus_Destroyed);
            }
            break;
        }
    }

    void ClientImRoot::addViewportMapping(Render::RenderTarget *target, ImGuiViewport *vp)
    {
        mViewportMappings[target] = vp;
    }

    void ClientImRoot::removeViewportMapping(Render::RenderTarget *target)
    {
        mViewportMappings.erase(target);
    }

    Math::Rect2i ClientImRoot::getChildClientSpace()
    {
        ImGuiIO &io = ImGui::GetIO();
        if (mAreaSize == Math::Vector2 { 0, 0 })
            return mClientSpace;
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            return { (mAreaPos - getScreenSpace().mTopLeft).floor(), mAreaSize.floor() };
        else
            return { mAreaPos.floor(), mAreaSize.floor() };
    }

    bool ClientImRoot::includeInLayout() const
    {
        return false;
    }

    bool ClientImRoot::onWindowEvent(const Platform::Window::WindowEvent &arg)
    {
        return std::visit(overloaded {
                              [&](const Platform::Window::ResizeEvent &e) { return false; },
                              [&](const Platform::Window::CloseEvent &e) { dialogs().show(closeDialog(), [this]() { mWindow.onWindowEvent(Platform::Window::CloseEvent {}, this); }); return true; },
                              [&](const Platform::Window::RepaintEvent &e) { return false; },
                              [&](const Platform::Input::KeyPressEvent &e) { return injectKeyPress(e); },
                              [&](const Platform::Input::KeyReleaseEvent &e) { return injectKeyRelease(e); },
                              [&](const Platform::Input::PointerPressEvent &e) { return injectPointerPress(e); },
                              [&](const Platform::Input::PointerReleaseEvent &e) { return injectPointerRelease(e); },
                              [&](const Platform::Input::PointerMoveEvent &e) { return injectPointerMove(e); },
                              [&](const Platform::Input::AxisEvent &e) { return injectAxisEvent(e); } },
            arg);
    }

    bool ClientImRoot::injectKeyPress(const Platform::Input::KeyPressEvent &arg)
    {
        ImGuiIO &io = ImGui::GetIO();

        auto it = sKeyMap.find(arg.mScancode);
        if (it == sKeyMap.end()) {
            LOG_ERROR("Unhandled Keycode encountered in ClientImRoot: " << (int)arg.mScancode << ", text: " << arg.mText);
        } else {
            io.AddKeyEvent(it->second, true);
        }
        if (arg.mText > 0)
            io.AddInputCharacter(arg.mText);

        io.AddKeyEvent(ImGuiMod_Ctrl, arg.mControlKeys.mCtrl);
        io.AddKeyEvent(ImGuiMod_Shift, arg.mControlKeys.mShift);
        io.AddKeyEvent(ImGuiMod_Alt, arg.mControlKeys.mAlt);
        // io.AddKeyEvent(ImGuiMod_Super, al_key_down(&keys, ALLEGRO_KEY_LWIN) || al_key_down(&keys, ALLEGRO_KEY_RWIN));

        return io.WantCaptureKeyboard;
    }

    bool ClientImRoot::injectKeyRelease(const Platform::Input::KeyReleaseEvent &arg)
    {
        ImGuiIO &io = ImGui::GetIO();

        auto it = sKeyMap.find(arg.mScancode);
        if (it == sKeyMap.end()) {
            LOG_ERROR("Unhandled Keycode encountered in ClientImRoot: " << arg.mScancode << ", text: " << arg.mText);
        } else {
            io.AddKeyEvent(it->second, false);
        }

        io.AddKeyEvent(ImGuiMod_Ctrl, arg.mControlKeys.mCtrl);
        io.AddKeyEvent(ImGuiMod_Shift, arg.mControlKeys.mShift);
        io.AddKeyEvent(ImGuiMod_Alt, arg.mControlKeys.mAlt);
        // io.AddKeyEvent(ImGuiMod_Super, al_key_down(&keys, ALLEGRO_KEY_LWIN) || al_key_down(&keys, ALLEGRO_KEY_RWIN));

        return io.WantCaptureKeyboard;
    }

    bool ClientImRoot::injectPointerPress(const Platform::Input::PointerPressEvent &arg)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.AddMouseButtonEvent(arg.mButton - 1, true);

        return io.WantCaptureMouse;
    }

    bool ClientImRoot::injectPointerRelease(const Platform::Input::PointerReleaseEvent &arg)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.AddMouseButtonEvent(arg.mButton - 1, false);

        return io.WantCaptureMouse;
    }

    bool ClientImRoot::injectPointerMove(const Platform::Input::PointerMoveEvent &arg)
    {
        ImGuiIO &io = ImGui::GetIO();

        Math::Vector2 pos;
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            pos = Math::Vector2 { static_cast<float>(arg.mScreenPosition.x), static_cast<float>(arg.mScreenPosition.y) } / Math::Vector2 { io.DisplayFramebufferScale };
        else
            pos = Math::Vector2 { static_cast<float>(arg.mWindowPosition.x), static_cast<float>(arg.mWindowPosition.y) } / Math::Vector2 { io.DisplayFramebufferScale };

        io.AddMousePosEvent(pos.x, pos.y);

        // LOG(io.MousePos.x << ", " << io.MousePos.y);

        // LOG(arg.scrollWheel);

        return io.WantCaptureMouse;
    }

    bool ClientImRoot::injectAxisEvent(const Platform::Input::AxisEvent &arg)
    {
        ImGuiIO &io = ImGui::GetIO();
        switch (arg.mAxisType) {
        case Platform::Input::AxisEvent::WHEEL:
            io.AddMouseWheelEvent(0.0f, arg.mAxis1);
            break;
        case Platform::Input::AxisEvent::Z:
            mZAxis = arg.mAxis1;
            break;
        case Platform::Input::AxisEvent::LEFT:
            mLeftControllerStick = { arg.mAxis1, arg.mAxis2 };
            break;
        case Platform::Input::AxisEvent::RIGHT:
            mRightControllerStick = { arg.mAxis1, arg.mAxis2 };
            break;
        case Platform::Input::AxisEvent::DPAD:
            mDPadState = arg.mAxis1;
            break;
        }

        return io.WantCaptureMouse;
    }

    bool ClientImRoot::wantsSoftwareKeyboard() const
    {
        ImGuiIO &io = ImGui::GetIO();

        return io.WantTextInput;
    }

    void ClientImRoot::setCentralNode()
    {
        ImGuiIO &io = ImGui::GetIO();

        Math::Vector2 oldSize = mAreaSize;
        Math::Vector2 oldPos = mAreaPos;

        ImGuiDockNode *node = ImGui::DockBuilderGetCentralNode(mGameDockSpaceId);

        if (node) {
            mAreaPos = Math::Vector2 { node->Pos } * Math::Vector2 { io.DisplayFramebufferScale };
            mAreaSize = Math::Vector2 { node->Size } * Math::Vector2 { io.DisplayFramebufferScale };
        } else {
            mAreaPos = Math::Vector2::ZERO;
            mAreaSize = Math::Vector2::ZERO;
        }

        if (mAreaSize != oldSize || mAreaPos != oldPos)
            mWindow.applyClientSpaceResize(this);
    }

    Platform::Filesystem::Path ClientImRoot::findDataFile(std::string_view name) const
    {
        return Resources::ResourceManager::getSingleton().findResourceFile(name);
    }

    Threading::TaskQueue *ClientImRoot::taskQueue() const
    {
        return mWindow.taskQueue();
    }

    void ClientImRoot::Image(Render::ConstTexturePtr tex, Math::Vector2i image_size, const Math::Vector2 &uv0, const Math::Vector2 &uv1)
    {
        ImGui::Image((void *)tex.get(), image_size, uv0, uv1);

        mTextureCache.push_back(std::move(tex));        
    }

    void ClientImRoot::Image(const Platform::Filesystem::Path &path, Math::Vector2i image_size)
    {
        std::string_view name = path.stem();

        CachedImage &image = mImageCache[path];
        if (!image.mTexture && !image.mHandle) {
            image.mHandle.create(name, path);
        }

        if (!image.mTexture && image.mHandle.available()) {
            image.mTexture = Render::RenderContext::getSingleton().createTexture(Render::TextureType_2D, Render::FORMAT_RGBA8_SRGB, image.mHandle->mSize, image.mHandle->mBuffer);
        }

        if (image.mTexture) {
            const Render::Texture &tex = *image.mTexture;

            if (image_size.x == -1 || image_size.y == -1) {
                image_size = tex.size();
            }

            Image(image.mTexture, image_size);
        } else {
            ImGui::Spinner(path.stem().data(), 15, 6, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
        }
    }

    void ClientImRoot::DrawImage(const Platform::Filesystem::Path &path, Math::Vector2i pos, Math::Vector2i image_size, float spinnerRadius)
    {
        std::string_view name = path.stem();

        CachedImage &image = mImageCache[path];
        if (!image.mTexture && !image.mHandle) {
            image.mHandle.create(name, path);
        }

        if (!image.mTexture && image.mHandle.available()) {
            image.mTexture = Render::RenderContext::getSingleton().createTexture(Render::TextureType_2D, Render::FORMAT_RGBA8_SRGB, image.mHandle->mSize, image.mHandle->mBuffer);
        }

        if (image.mTexture) {
            const Render::Texture &tex = *image.mTexture;
            if (image_size.x == -1 || image_size.y == -1) {
                image_size = tex.size();
            } else {
                float ratio = static_cast<float>(tex.size().x) / tex.size().y;
                if (ratio < 1.0f) {
                    pos.x += (image_size.x - image_size.y * ratio) / 2;
                    image_size.x = image_size.y * ratio;
                } else {
                    pos.y += (image_size.y - image_size.x / ratio) / 2;
                    image_size.y = image_size.x / ratio;
                }
            }

            ImGui::GetWindowDrawList()->AddImage((void *)&tex, pos, pos + image_size);
        } else {
            ImGui::DrawSpinner(pos, pos + image_size, spinnerRadius, 6, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
        }
    }

    ImGuiRenderData::ImGuiRenderData(ClientImRoot &root)
        : mRoot(root)
    {
    }

    Threading::ImmediateTask<Render::RenderFuture> ImGuiRenderData::render(Render::RenderContext *context)
    {
        ImGuiIO &io = ImGui::GetIO();

        Math::Vector2i size = mRoot.window().getScreenSpace().mSize;

        io.DisplaySize = ImVec2(size.x / io.DisplayFramebufferScale.x, size.y / io.DisplayFramebufferScale.y);

        io.DeltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(mFrameClock.tick(std::chrono::steady_clock::now())).count();

        io.BackendPlatformUserData = &mRoot.window();

        mRoot.mTextureCache.clear();

        if (mRoot.ImRoot::render())
            mRoot.setCentralNode();

        io.BackendPlatformUserData = nullptr;

        co_return {};
    }

}
}

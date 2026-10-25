#include "../clientlib.h"

#include "readbackmainwindowcomponent.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

NAMED_UNIQUECOMPONENT(ReadbackMainWindowComponent, Engine::Core::ReadbackMainWindowComponent)

METATABLE_BEGIN(Engine::Core::ReadbackMainWindowComponent)
METATABLE_END(Engine::Core::ReadbackMainWindowComponent)

SERIALIZETABLE_BEGIN(Engine::Core::ReadbackMainWindowComponent)
SERIALIZETABLE_END(Engine::Core::ReadbackMainWindowComponent)

namespace Engine {
namespace Core {

    ReadbackMainWindowComponent::ReadbackMainWindowComponent(MainWindow &window)
        : MainWindowComponent(window, 1000)
        , mPass(1000)
    {
    }

    ReadbackMainWindowComponent::~ReadbackMainWindowComponent() = default;

    void ReadbackMainWindowComponent::render(Render::RenderTarget *target, size_t iteration)
    {
        mPass.render(target, iteration);
    }

    void ReadbackMainWindowComponent::setup(Render::RenderTarget *target)
    {
        mPass.setup(target);
    }

    void ReadbackMainWindowComponent::shutdown(Render::RenderTarget *target)
    {
        mPass.shutdown(target);
    }

    Render::ReadBackPass &ReadbackMainWindowComponent::pass()
    {
        return mPass;
    }

}
}
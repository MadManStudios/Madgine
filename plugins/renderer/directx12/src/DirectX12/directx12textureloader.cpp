#include "directx12lib.h"

#include "directx12textureloader.h"

#include "util/directx12texture.h"

#include "directx12rendercontext.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

RESOURCELOADER(Engine::Render::DirectX12TextureLoader);

namespace Engine {
namespace Render {

    DirectX12TextureLoader::DirectX12TextureLoader()
    {
    }

    bool DirectX12TextureLoader::loadImpl(DirectX12Texture &tex, ResourceDataInfo &info)
    {
        throw 0;
    }

    void DirectX12TextureLoader::unloadImpl(DirectX12Texture &tex)
    {
        tex.reset();
    }

    bool DirectX12TextureLoader::create(Texture &tex, TextureType type, TextureFormat format)
    {
        static_cast<DirectX12Texture &>(tex) = DirectX12Texture { type, false, format };

        return true;
    }

    void DirectX12TextureLoader::setData(Texture &tex, Vector2i size, const ByteBuffer &data)
    {
        static_cast<DirectX12Texture &>(tex).setData(size, data);
    }

    void DirectX12TextureLoader::setSubData(Texture &tex, Vector2i offset, Vector2i size, const ByteBuffer &data)
    {
        static_cast<DirectX12Texture &>(tex).setSubData(offset, size, data);
    }

    Threading::TaskQueue *DirectX12TextureLoader::loadingTaskQueue() const
    {
        return DirectX12RenderContext::renderQueue();
    }

}
}

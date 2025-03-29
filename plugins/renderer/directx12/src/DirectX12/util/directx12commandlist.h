#pragma once

#include "Generic/any.h"

#include "Madgine/render/future.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12CommandList {
        DirectX12CommandList() = default;
        DirectX12CommandList(DirectX12CommandAllocator *manager, ReleasePtr<ID3D12GraphicsCommandList> list, ReleasePtr<ID3D12CommandAllocator> allocator);
        ~DirectX12CommandList();

        DirectX12CommandList &operator=(DirectX12CommandList &&);

        RenderFuture execute();

        operator ID3D12GraphicsCommandList *();
        ID3D12GraphicsCommandList *operator->();

        template <typename T>
        void attachResource(T resource) {
            attachResource(Any { std::move(resource) });
        }
        void attachResource(Any resource);

        void Transition(ID3D12Resource *res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) const;

    private:
        DirectX12CommandAllocator *mManager;
        ReleasePtr<ID3D12GraphicsCommandList> mList;
        ReleasePtr<ID3D12CommandAllocator> mAllocator;
        std::vector<Any> mAttachedResources;
    };

}
}
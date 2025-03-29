#include "../directx12lib.h"

#include "directx12commandlist.h"
#include "directx12commandallocator.h"

namespace Engine {
namespace Render {

    DirectX12CommandList::DirectX12CommandList(DirectX12CommandAllocator *manager, ReleasePtr<ID3D12GraphicsCommandList> list, ReleasePtr<ID3D12CommandAllocator> allocator)
        : mManager(manager)
        , mList(std::move(list))
        , mAllocator(std::move(allocator))
    {
    }

    DirectX12CommandList::~DirectX12CommandList()
    {
        assert(!mList);
    }

    DirectX12CommandList &DirectX12CommandList::operator=(DirectX12CommandList &&other)
    {
        assert(!mList);
        mManager = other.mManager;
        mList = std::move(other.mList);
        mAllocator = std::move(other.mAllocator);
        mAttachedResources = std::move(other.mAttachedResources);
        return *this;
    }

    RenderFuture DirectX12CommandList::execute()
    {
        if (mList) {
            return mManager->ExecuteCommandList(std::move(mList), std::move(mAllocator), std::move(mAttachedResources));
        }
        return {};
    }

    DirectX12CommandList::operator ID3D12GraphicsCommandList *()
    {
        return mList;
    }

    ID3D12GraphicsCommandList *DirectX12CommandList::operator->()
    {
        return mList;
    }

    void DirectX12CommandList::attachResource(Any resource)
    {
        mAttachedResources.push_back(std::move(resource));
    }

    void DirectX12CommandList::Transition(ID3D12Resource *res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) const
    {
        D3D12_RESOURCE_BARRIER barrierDesc;
        ZeroMemory(&barrierDesc, sizeof(D3D12_RESOURCE_BARRIER));

        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrierDesc.Transition.pResource = res;
        barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrierDesc.Transition.StateBefore = from;
        barrierDesc.Transition.StateAfter = to;
        barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        mList->ResourceBarrier(1, &barrierDesc);
        DX12_CHECK();

        wchar_t name[128] = {};
        UINT size = sizeof(name);
        res->GetPrivateData(WKPDID_D3DDebugObjectNameW, &size, name);
        LOG_DEBUG("Transitioning " << StringUtil::fromWString(name) << ": " << from << " -> " << to);
    }

}
}
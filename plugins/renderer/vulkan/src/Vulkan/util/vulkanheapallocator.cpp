#include "../vulkanlib.h"

#include "vulkanheapallocator.h"

#include "../vulkanrendercontext.h"

namespace Engine {
namespace Render {

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(GetPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw 0;
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory)
    {
        VkBufferCreateInfo bufferInfo {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(GetDevice(), &bufferInfo, nullptr, buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(GetDevice(), *buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(GetDevice(), &allocInfo, nullptr, bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(GetDevice(), *buffer, *bufferMemory, 0);
    }

    VulkanHeapAllocator::VulkanHeapAllocator(std::string_view name)
        : mName { name }
    {
    }

    void VulkanHeapAllocator::setup(size_t count)
    {
    }

    Block VulkanHeapAllocator::allocate(size_t size, size_t alignment)
    {
        Heap &heap = mHeaps.emplace_back();

        createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &heap.mBuffer, &heap.mMemory);
        VkDebugUtilsObjectNameInfoEXT nameInfo {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        nameInfo.objectHandle = reinterpret_cast<uintptr_t>(heap.mBuffer.get());
        nameInfo.pObjectName = mName.c_str();
        VkResult result = vkSetDebugUtilsObjectNameEXT(GetDevice(), &nameInfo);
        VK_CHECK(result);

        return { reinterpret_cast<void *>(mHeaps.size() << 32), size };
    }

    void VulkanHeapAllocator::deallocate(Block block)
    {
        uint32_t offset = reinterpret_cast<uintptr_t>(block.mAddress);
        uint32_t buffer = reinterpret_cast<uintptr_t>(block.mAddress) >> 32;        
        assert(offset == 0 && buffer != 0);

        size_t index = buffer - 1;
        mHeaps[index].mBuffer.reset();
        mHeaps[index].mMemory.reset();
    }

    std::tuple<VkBuffer, VkDeviceMemory, size_t> VulkanHeapAllocator::resolve(void *ptr)
    {
        uint32_t offset = reinterpret_cast<uintptr_t>(ptr);
        uint32_t buffer = reinterpret_cast<uintptr_t>(ptr) >> 32;        
        
        return { mHeaps[buffer - 1].mBuffer, mHeaps[buffer - 1].mMemory, offset };
    }

    VulkanMappedHeapAllocator::VulkanMappedHeapAllocator(std::string_view name)
        : mName { name }
    {
    }

    Block VulkanMappedHeapAllocator::allocate(size_t size, size_t alignment)
    {
        Page &page = mPages.emplace_back();

        createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &page.mBuffer, &page.mMemory);
        VkDebugUtilsObjectNameInfoEXT nameInfo {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        nameInfo.objectHandle = reinterpret_cast<uintptr_t>(page.mBuffer.get());
        nameInfo.pObjectName = mName.c_str();
        VkResult result = vkSetDebugUtilsObjectNameEXT(GetDevice(), &nameInfo);
        VK_CHECK(result);

        page.mSize = size;

        result = vkMapMemory(GetDevice(), page.mMemory, 0, size, 0, &page.mMappedAddress);
        VK_CHECK(result);

        return { page.mMappedAddress, size };
    }

    void VulkanMappedHeapAllocator::deallocate(Block block)
    {
        auto it = std::ranges::find(mPages, block.mAddress, &Page::mMappedAddress);
        assert(it != mPages.end());

        vkUnmapMemory(GetDevice(), it->mMemory);

        mPages.erase(it);
    }

    std::pair<VkBuffer, size_t> VulkanMappedHeapAllocator::resolve(void *ptr)
    {
        auto it = std::ranges::find_if(mPages, [=](const Page &page) {
            return page.mMappedAddress <= ptr && ptr < static_cast<std::byte *>(page.mMappedAddress) + page.mSize;
        });
        assert(it != mPages.end());
        return { it->mBuffer, static_cast<std::byte *>(ptr) - static_cast<std::byte *>(it->mMappedAddress) };
    }

}
}
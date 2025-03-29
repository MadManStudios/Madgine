#pragma once

namespace Engine {

namespace Render {
    struct VulkanRenderWindow;
    struct VulkanRenderTarget;

    struct VulkanPersistentMeshData;
    struct VulkanTempMeshData;

    struct VulkanFontData;

    struct VulkanRenderContext;

    struct VulkanPersistentBuffer;
    struct VulkanTempBuffer;

    struct VulkanConstantBufferHeap;

    struct VulkanDescriptorHeap;

    constexpr struct create_t {
    } create;
}

}

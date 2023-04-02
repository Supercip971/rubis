#include <render/vulkan/buffer.h>
#include <render/vulkan/command.h>
#include <vulkan/vulkan_core.h>
uint32_t find_memory_type(VulkanCtx *ctx, uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t size)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(ctx->physical_device, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) != 0 &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            int heap = memProperties.memoryTypes[i].heapIndex;
            if (memProperties.memoryHeaps[heap].size >= size)
            {

                return i;
            }
        }
    }
    abort();

    return 0;
}

VkDeviceAddress vk_buffer_addr(VulkanCtx *ctx, VulkanBuffer buf)
{
    VkBufferDeviceAddressInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buf.buffer,
    };
    VkDeviceAddress addr = vkGetBufferDeviceAddress(ctx->logical_device, &info);

    return addr;
}

VulkanBuffer vk_buffer_alloc(VulkanCtx *ctx, size_t len, VkBufferUsageFlags flags, VkMemoryPropertyFlags properties)
{
    VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = len,
        .usage = flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VulkanBuffer buf = {
        .len = len,
    };
    vk_try$(vkCreateBuffer(ctx->logical_device, &info, NULL, &buf.buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ctx->logical_device, buf.buffer, &memRequirements);

    VkMemoryAllocateInfo allocate = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = find_memory_type(ctx, memRequirements.memoryTypeBits, properties, len),
    };

    VkMemoryAllocateFlagsInfo allocationFlags = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
    };
    if ((flags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        allocate.pNext = &allocationFlags;
    }

    vk_try$(vkAllocateMemory(ctx->logical_device, &allocate, NULL, &buf.raw_memory));

    vk_try$(vkBindBufferMemory(ctx->logical_device, buf.buffer, buf.raw_memory, 0));

    return buf;
}

void vk_buffer_free(VulkanCtx *ctx, VulkanBuffer buffer)
{
    vkDestroyBuffer(ctx->logical_device, buffer.buffer, NULL);
    vkFreeMemory(ctx->logical_device, buffer.raw_memory, NULL);
}

void vk_buffer_set(VulkanCtx *ctx, VulkanBuffer buf, uint32_t data)
{
    VkCommandBuffer command_buffer = vk_start_single_time_command(ctx);

    vkCmdFillBuffer(command_buffer, buf.buffer, 0, buf.len, data);

    vk_end_single_time_command(ctx, command_buffer);
}

void vk_buffer_copy(VulkanCtx *ctx, VulkanBuffer to, VulkanBuffer from)
{

    VkCommandBuffer command_buffer = vk_start_single_time_command(ctx);
    VkBufferCopy copy_region = {
        .size = to.len,
        .dstOffset = 0,
        .srcOffset = 0,
    };
    vkCmdCopyBuffer(command_buffer, from.buffer, to.buffer, 1, &copy_region);

    vk_end_single_time_command(ctx, command_buffer);
}
void *vk_buffer_map(VulkanCtx *ctx, VulkanBuffer with)
{
    void *data;
    vk_try$(vkMapMemory(ctx->logical_device, with.raw_memory, 0, with.len, 0, &data));
    return data;
}
void vk_buffer_unmap(VulkanCtx *ctx, VulkanBuffer with)
{
    vkUnmapMemory(ctx->logical_device, with.raw_memory);
}

#include <render/vulkan/buffer.h>
#include <render/vulkan/command.h>
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
        .memoryTypeIndex = find_memory_type(ctx, memRequirements.memoryTypeBits, properties),
    };

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
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = ctx->cmd_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(ctx->logical_device, &alloc_info, &command_buffer);
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(command_buffer, &begin_info);
    vkCmdFillBuffer(command_buffer, buf.buffer, 0, buf.len, data);
    vkEndCommandBuffer(command_buffer);
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };

    vk_try$(vkQueueSubmit(ctx->gfx_queue, 1, &submitInfo, VK_NULL_HANDLE));
    vkQueueWaitIdle(ctx->gfx_queue);

    vkFreeCommandBuffers(ctx->logical_device, ctx->cmd_pool, 1, &command_buffer);
}

void vk_buffer_copy(VulkanCtx *ctx, VulkanBuffer to, VulkanBuffer from)
{

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = ctx->cmd_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(ctx->logical_device, &alloc_info, &command_buffer);
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkBufferCopy copy_region = {
        .size = to.len,
        .dstOffset = 0,
        .srcOffset = 0,
    };
    vkCmdCopyBuffer(command_buffer, from.buffer, to.buffer, 1, &copy_region);
    vkEndCommandBuffer(command_buffer);
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };

    vk_try$(vkQueueSubmit(ctx->gfx_queue, 1, &submitInfo, VK_NULL_HANDLE));
    vkQueueWaitIdle(ctx->gfx_queue);

    vkFreeCommandBuffers(ctx->logical_device, ctx->cmd_pool, 1, &command_buffer);
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

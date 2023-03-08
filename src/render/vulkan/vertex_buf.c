#include <render/vulkan/vertex_buf.h>
#include <render/vulkan/buffer.h>
#include <vulkan/vulkan_core.h>
void vulkan_vertex_buffer_init(VulkanCtx* self)
{
    size_t len = self->scene.data.length * sizeof(float) * 4;

    self->vertex_buffer = vk_buffer_alloc(self, len, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void * mapped_data = vk_buffer_map(self, self->vertex_buffer);

    memcpy(mapped_data, self->scene.data.data, len);

    vk_buffer_unmap(self, self->vertex_buffer);
}
void vulkan_vertex_buffer_deinit(VulkanCtx* self)
{
    vk_buffer_free(self, self->vertex_buffer);
}

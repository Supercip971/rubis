#include <render/vulkan/buffer.h>
#include <render/vulkan/command.h>
#include <render/vulkan/textures.h>
#include <vulkan/vulkan_core.h>
#include "render/vulkan/vulkan.h"

VulkanBuffer vulkan_scene_texture_data_init(VulkanCtx *ctx, uint32_t *final_sizex, uint32_t *final_sizey)
{
    TexLists texs = ctx->scene.data.textures;

    size_t tsize = 0;
    size_t maxw = 0;
    size_t maxh = 0;

    for (int i = 0; i < texs.length; i++)
    {
        maxw = fmax(texs.data[i].width, maxw);
        maxh = fmax(texs.data[i].height, maxh);
        printf("candidate[%i]: %u\n  / %u\n ", i, texs.data[i].width, texs.data[i].height);
    }

    tsize = maxw * maxh * sizeof(uint32_t) * texs.length;

    printf("textures sizes: %zu  \n", tsize);
    VulkanBuffer staging_buf = vk_buffer_alloc(ctx, tsize + 16, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data = vk_buffer_map(ctx, staging_buf);

    size_t off = 0;
    for (int i = 0; i < texs.length; i++)
    {
        Image c = texs.data[i];
        size_t dst_size = maxw * maxh * sizeof(uint32_t);

        size_t src_size = c.width * c.height * sizeof(uint32_t);
        if (src_size != 0)
        {
            for (size_t j = 0; j < dst_size; j += src_size)
            {

                memcpy(data + off + j, c.data, src_size);
            }
        }
        off += dst_size;

        image_unload(&c);
    }

    vk_buffer_unmap(ctx, staging_buf);

    *final_sizex = maxw;
    *final_sizey = maxh;
    return staging_buf;
}


static void vulkan_scene_texture_load_impl(VulkanCtx *ctx, VulkanTex *self, VulkanBuffer *buffer, int depth, int width, int height, bool sampler)
{

    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = width,
        .extent.height = height,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = depth,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT,
    };

    vk_try$(vkCreateImage(ctx->gfx.device, &image_info, NULL, &self->image));

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(ctx->gfx.device, self->image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = find_memory_type(ctx, mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mem_requirements.size),
    };

    vk_try$(vkAllocateMemory(ctx->gfx.device, &alloc_info, NULL, &self->mem));

    vkBindImageMemory(ctx->gfx.device, self->image, self->mem, 0);

    swap_image_layout(ctx, self->image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, depth, false, false);

    image_load_from_buffer(ctx, self->image, width, height, depth, buffer->buffer);
    swap_image_layout(ctx, self->image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, depth, false, false);

    self->desc_info.imageView = image_view_create(ctx, self->image, depth, false);
    self->desc_info.sampler = (sampler ? image_sampler_create(ctx) : VK_NULL_HANDLE);
    self->desc_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    self->width = width;
    self->height = height;
}

void vulkan_scene_texture_load(VulkanCtx *ctx, VulkanTex *self, VulkanBuffer *buffer, int depth, int width, int height)
{
    vulkan_scene_texture_load_impl(ctx, self, buffer, depth, width, height, true);
}
static void vulkan_scene_ressource_texture_init(VulkanCtx *ctx, VulkanTexArrays *result)
{
    vec_init(&result->textures);
    TexLists texs = ctx->scene.data.textures;

    if(texs.length == 0)
    {
       Image empty = (Image)
        {
            .data = malloc(16 * sizeof(uint32_t)),
            .width = 4,
            .height = 4,
        };
        
        vec_push(&ctx->scene.data.textures, empty);
        texs = ctx->scene.data.textures; 
    }
    VulkanBuffer staging_buf;
    for (int i = 0; i < texs.length; i++)
    {

        VulkanTex tex = {};
        Image* current = &texs.data[i];
       
        unsigned int tex_size = current->width * current->height * sizeof(uint8_t) * 4; // 4 channel
        staging_buf = vk_buffer_alloc(ctx, tex_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        volatile uint8_t *data = vk_buffer_map(ctx, staging_buf);
        printf("loading texture[%i] %ux%u (%u)\n", i, current->width, current->height, tex_size);
        for (size_t j = 0; j < tex_size; j++)
        {
            data[j] = current->data[j];
        }

        vk_buffer_unmap(ctx, staging_buf);
        //vulkan_scene_texture_load(ctx, &ctx->scene.skymap, &staging_buf, 1, ctx->scene.data.skymap.width, ctx->scene.data.skymap.height);
        vulkan_scene_texture_load_impl(ctx, &tex, &staging_buf, 1, current->width, current->height, false);

        vk_buffer_free(ctx, staging_buf);

        vec_push(&result->textures, tex);
    }

    vec_init(&result->final_info);
    for (int i = 0; i < result->textures.length; i++)
    {
        vec_push(&result->final_info, result->textures.data[i].desc_info);
    }
}

static void vulkan_scene_sampler_init(VulkanCtx *ctx)
{
    ctx->scene.sampler.desc_info = (VkDescriptorImageInfo){
        .sampler = image_sampler_create(ctx),
        .imageView = NULL,
        .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
}
void vulkan_scene_textures_init(VulkanCtx *ctx)
{
    VulkanBuffer staging_buf;

    printf("creating vulkan scene shader textures\n");
    vulkan_shader_shared_texture_init(ctx, &ctx->gfx.fragment_image, ctx->core.aligned_width, ctx->core.aligned_height, true);
 
    printf("creating vulkan scene textures\n");

    vulkan_scene_ressource_texture_init(ctx, &ctx->scene.combined_textures);

    if (ctx->scene.data.skymap.height != 0)
    {

        uint32_t skybox_data_len = ctx->scene.data.skymap.width * ctx->scene.data.skymap.height * sizeof(uint32_t);

        staging_buf = vk_buffer_alloc(ctx, skybox_data_len + 16, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        void *data = vk_buffer_map(ctx, staging_buf);

        memcpy(data, ctx->scene.data.skymap.data, skybox_data_len);

        vk_buffer_unmap(ctx, staging_buf);
        vulkan_scene_texture_load(ctx, &ctx->scene.skymap, &staging_buf, 1, ctx->scene.data.skymap.width, ctx->scene.data.skymap.height);
        vk_buffer_free(ctx, staging_buf);
    }

    vulkan_scene_sampler_init(ctx);
}
void vulkan_scene_textures_deinit(VulkanCtx *ctx)
{

    //  vkDestroyImageView(ctx->device, ctx->combined_textures.desc_info.imageView, NULL);
    //  vkDestroySampler(ctx->device, ctx->combined_textures.desc_info.sampler, NULL);
    //  vkDestroyImage(ctx->device, ctx->combined_textures.image, NULL);
    //  vkFreeMemory(ctx->device, ctx->combined_textures.mem, NULL);
    for(int i = 0; i < ctx->scene.combined_textures.textures.length; i++)
    {
        vkDestroyImageView(ctx->gfx.device, ctx->scene.combined_textures.textures.data[i].desc_info.imageView, NULL);
        vkDestroyImage(ctx->gfx.device, ctx->scene.combined_textures.textures.data[i].image, NULL);
        vkFreeMemory(ctx->gfx.device, ctx->scene.combined_textures.textures.data[i].mem, NULL);
    }
    vkDestroyImageView(ctx->gfx.device, ctx->scene.skymap.desc_info.imageView, NULL);
    vkDestroySampler(ctx->gfx.device, ctx->scene.skymap.desc_info.sampler, NULL);
    vkDestroyImage(ctx->gfx.device, ctx->scene.skymap.image, NULL);
    vkFreeMemory(ctx->gfx.device, ctx->scene.skymap.mem, NULL);

    vkDestroyImageView(ctx->gfx.device, ctx->gfx.fragment_image.desc_info.imageView, NULL);
    vkDestroySampler(ctx->gfx.device, ctx->gfx.fragment_image.desc_info.sampler, NULL);
    vkDestroyImage(ctx->gfx.device, ctx->gfx.fragment_image.image, NULL);
    vkFreeMemory(ctx->gfx.device, ctx->gfx.fragment_image.mem, NULL);

    vkDestroySampler(ctx->gfx.device, ctx->scene.sampler.desc_info.sampler, NULL);
    (void)ctx;
}

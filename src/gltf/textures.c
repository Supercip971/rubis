#include <gltf/accessor.h>
#include <gltf/textures.h>
#include <stdio.h>
#include "obj/img.h"

void gltf_textures_parse(GltfCtx *self)
{
    vec_init(&self->textures);
    cJSON *textures_array = (cJSON_GetObjectItem(self->root, "textures"));
    cJSON *img_array = (cJSON_GetObjectItem(self->root, "images"));

    int textures_count = cJSON_GetArraySize(textures_array);

    for (int i = 0; i < textures_count; i++)
    {
        cJSON *texture = cJSON_GetArrayItem(textures_array, i);
        cJSON *source = cJSON_GetObjectItem(texture, "source");
        int source_id = source->valueint;
        cJSON *img = cJSON_GetArrayItem(img_array, source_id);
        cJSON *view_id = cJSON_GetObjectItem(img, "bufferView");

        GltfBufView view;

        gltf_get_buffer_from_view(self, view_id->valueint, &view, 0, 0);

        Image img_data = image_load(view.data, view.len);
        imageID id = scene_push_texture(self->target, img_data);

        printf("added texture: %i (%ix%i)\n", id, img_data.width, img_data.height);
        vec_push(&self->textures, id);
    }
}

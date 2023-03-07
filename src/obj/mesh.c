#include <obj/mesh.h>
#include "render/vulkan/vertex.h"
int mesh_count_faces(Mesh *self)
{
    int base_len = self->vertices.end - self->vertices.start;

    return base_len / (TRIANGLE_PACKED_COUNT );
}

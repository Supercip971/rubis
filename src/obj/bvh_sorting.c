#include <obj/bvh.h>
#include "math/aabb.h"
#include "math/vec3.h"

#define create_sort(name, dim, sign)                                   \
    static int name(const void *a, const void *b)                      \
    {                                                                  \
        const ElementOnList *node_a = a;                               \
        const ElementOnList *node_b = b;                               \
        float pa = vec3_dim(aabb_centroid(&node_a->entry.box) , (dim)); \
        float pb = vec3_dim(aabb_centroid(&node_b->entry.box) , (dim)); \
       /* float pa = vec3_dim((sign) ? node_a->entry.box.max :  node_a->entry.box.min, (dim));*/ \
        /*float pb = vec3_dim((sign) ? node_b->entry.box.max :  node_b->entry.box.min, (dim));*/ \
        return ((sign) ? (pa - pb) : (pb - pa));                       \
    }

create_sort(compare_x_pos, VDIM_X, 1)
create_sort(compare_x_neg, VDIM_X, 0)

create_sort(compare_y_pos, VDIM_Y, 1)
create_sort(compare_y_neg, VDIM_Y, 0)

create_sort(compare_z_pos, VDIM_Z, 1)
create_sort(compare_z_neg, VDIM_Z, 0)

tempBvhList sort_on_axis(const tempBvhList *list, VecDimension axis, bool sign)
{
    tempBvhList result = {};
    vec_init(&result);

    for (int i = 0; i < list->length; i++)
    {
        ElementOnList *entry = &list->data[i];
        vec_push(&result, *entry);
    }

    switch (axis)
    {
    case VDIM_X:
    {
        if (sign)
        {
            qsort(result.data, result.length, sizeof(ElementOnList), compare_x_pos);
        }
        else
        {
            qsort(result.data, result.length, sizeof(ElementOnList), compare_x_neg);
        }
    }
    break;
    case VDIM_Y:
    {
        if (sign)
        {
            qsort(result.data, result.length, sizeof(ElementOnList), compare_y_pos);
        }
        else
        {
            qsort(result.data, result.length, sizeof(ElementOnList), compare_y_neg);
        }
    }
    break;
    case VDIM_Z:
    {
        if (sign)
        {
            qsort(result.data, result.length, sizeof(ElementOnList), compare_z_pos);
        }
        else
        {
            qsort(result.data, result.length, sizeof(ElementOnList), compare_z_neg);
        }
    }
    break;
    }


    return result;
}

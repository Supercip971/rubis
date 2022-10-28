#include <obj/bvh.h>
#include <stdio.h>
#include "obj/mesh.h"
#include "obj/scene.h"
typedef struct
{
    BvhEntry entry;
    void *next_l;
    void *next_r;

} ElementOnList;

ElementOnList *element_copy(ElementOnList *from)
{

    ElementOnList *d = malloc(sizeof(ElementOnList));
    *d = *from;
    return d;
}
float bvh_distance(ElementOnList *left, ElementOnList *right)
{
    AABB a, b;
    Vec3 a_center, b_center;
    a = left->entry.box;
    b = right->entry.box;

    a_center = vec3_add(a.min, vec3_mul_val(vec3_sub(a.max, a.min), 0.5));
    b_center = vec3_add(b.min, vec3_mul_val(vec3_sub(b.max, b.min), 0.5));

    return vec3_squared_length(vec3_sub(b_center, a_center));
}

Vec3 aabb_centroid(const AABB *a)
{
    Vec3 a_center = vec3_add(a->min, vec3_mul_val(vec3_sub(a->max, a->min), 0.5));

    return a_center;
}
AABB aabb_surrounding(const AABB *__restrict a, const AABB *__restrict b)
{
    return (AABB){
        .min = vec3_min(a->min, b->min),
        .max = vec3_max(a->max, b->max),
    };
}

typedef vec_t(ElementOnList) tempBvhList;

/*
void bvh_update_parents(BvhList *self, BvhEntry *entry, int parent_idx)
{
    if (entry->is_next_a_bvh)
    {

        // self->data[entry->l].parent = parent_idx; // child->left
        //  self->data[entry->r].parent = parent_idx; // child->right
    }
}
*/
/*static inline bool aabb_intersect(const AABB *a, const AABB *b)
{
    return (a->min.x <= b->max.x && a->max.x >= b->min.x) &&
           (a->min.y <= b->max.y && a->max.y >= b->min.y) &&
           (a->min.z <= b->max.z && a->max.z >= b->min.z);
}*/

BvhEntry bvh_make_mesh_fusion(BvhEntry left, BvhEntry right)
{

    BvhEntry curr = {
        .is_next_a_bvh = false,
        .l = left.l,
        .r = right.l,
        //    .parent = 0,
        .box = aabb_surrounding(&left.box, &right.box),
    };

    return curr;
}

BvhEntry bvh_make_from_temp_list(BvhList *self, ElementOnList *element)
{
    if (!element->entry.is_next_a_bvh)
    {
   //     element->entry.r = 0;
        return element->entry;
    }

    BvhEntry left = bvh_make_from_temp_list(self, element->next_l);
    BvhEntry right = bvh_make_from_temp_list(self, element->next_r);

  /*  if (!left.is_next_a_bvh && !right.is_next_a_bvh && left.r == 0 && right.r == 0)
    {
        if (aabb_intersect(&left.box, &right.box))
        {

            return bvh_make_mesh_fusion(left, right);
        }
    }*/

    int l = self->length;

    vec_push(self, left);

    int r = l + 1;

    vec_push(self, right);

    // bvh_update_parents(self, &self->data[l], l);
    // bvh_update_parents(self, &self->data[r], r);
    BvhEntry curr = {
        .is_next_a_bvh = true,
        .l = l,
        .r = r,
        //    .parent = 0,
        .box = aabb_surrounding(&left.box, &right.box),
    };

    return curr;
}

void bvh_dump(BvhList *self, BvhEntry *entry, int depth)
{
#define TAB()                       \
    for (int i = 0; i < depth; i++) \
    {                               \
        printf("\t");               \
    }
    TAB();
    printf("entry[%i] \n", depth);
    TAB();
    printf("{%f,%f,%f} {%f,%f,%f}\n", entry->box.min.x, entry->box.min.y, entry->box.min.z, entry->box.max.x, entry->box.max.y, entry->box.max.z);
    TAB();
    if (!entry->is_next_a_bvh)
    {
        printf("bvh: %i|%i\n", entry->l, entry->r);
    }
    else
    {

        printf("- l: %i \n", entry->l);
        bvh_dump(self, &self->data[entry->l], depth + 1);

        TAB();
        printf("- r: %i \n", entry->r);
        bvh_dump(self, &self->data[entry->r], depth + 1);
    }
}
void sbvh_init(BvhList *self, int entry_id, Scene *scene)
{
    BvhEntry *entry = &self->data[entry_id];
    if (!entry->is_next_a_bvh)
    {
        BvhEntry left = self->data[entry->l];

        BvhEntry right = self->data[entry->r];
        (void)left;
        (void)right;
    }
    else
    {
        sbvh_init(self, entry->l, scene);
        sbvh_init(self, entry->r, scene);
    }
}
// probably don't know what this does in 1 month
// this tries to create a bvh using a linear memory (usable for gpu) without any pointers
// this is probably really inefficient:
// x + x/2 + x/4 ...
// n + (n)
// but as we are using it once, I may not want to make it faster (for now)

typedef enum
{
    BDIM_X,
    BDIM_Y,
    BDIM_Z,
} DimensionBvh;

DimensionBvh pick_dim(Vec3 a, Vec3 b)
{
    float dx = fabs(a.x - b.x);
    float dy = fabs(a.y - b.y);
    float dz = fabs(a.z - b.z);

    if (dx >= dy && dx >= dz)
    {
        return BDIM_X;
    }
    else if (dy >= dx && dy >= dz)
    {
        return BDIM_Y;
    }
    return BDIM_Z;
}

bool is_vec3_dim_superior(Vec3 point, Vec3 maxc, Vec3 minc, DimensionBvh dim)
{

    if (dim == BDIM_X)
    {
        float center = (maxc.x + minc.x) / 2;
        return point.x >= center;
    }
    else if (dim == BDIM_Y)
    {
        float center = (maxc.y + minc.y) / 2;

        return point.y >= center;
    }
    float center = (maxc.z + minc.z) / 2;

    return point.z >= center;
}

ElementOnList bvh_init_rec(tempBvhList *list, BvhList *tlist, int depth)
{
    if (list->length == 0)
    {
        abort();
    }
    if (list->length == 1)
    {
        return list->data[0];
    }
    ElementOnList res = {};
    tempBvhList vl;
    tempBvhList vr;
    vec_init(&vr);
    vec_init(&vl);

    Vec3 max_p = (list->data[0].entry.box.max);
    Vec3 min_p = (list->data[0].entry.box.min);

    Vec3 min_c = aabb_centroid(&list->data->entry.box);
    Vec3 max_c = aabb_centroid(&list->data->entry.box);
    for (int i = 0; i < list->length; i++)
    {
        max_p = vec3_max(max_p, list->data[i].entry.box.max);
        min_p = vec3_min(min_p, list->data[i].entry.box.min);

        max_c = vec3_max(max_p, aabb_centroid(&list->data[i].entry.box));
        min_c = vec3_min(min_p, aabb_centroid(&list->data[i].entry.box));
    }

    DimensionBvh dim = pick_dim(min_c, max_c);

    for (int i = 0; i < list->length; i++)
    {
        ElementOnList curr = list->data[i];

        if (is_vec3_dim_superior(aabb_centroid(&curr.entry.box), max_c, min_c, dim))
        {
            vec_push(&vr, curr);
        }
        else
        {
            vec_push(&vl, curr);
        }
    }

    //  printf("step[%i] (%i) l: %i r: %i \n", depth, list->length, vl.length, vr.length);
    //  printf("%f %f %f - %f %f %f\n", min_p.x, min_p.y, min_p.z, max_p.x, max_p.y, max_p.z);
    // generally when we have a lot of point in the same space, just random push, maybe I'll do something more
    // intelligent but this is a really rare edge case
    if (vl.length == 0 || vr.length == 0)
    {
        vec_clear(&vr);
        vec_clear(&vl);
        for (int i = 0; i < list->length; i++)
        {

            ElementOnList curr = list->data[i];
            if (i % 2)
            {
                vec_push(&vr, curr);
            }
            else
            {
                vec_push(&vl, curr);
            }
        }
    }
    res.entry.box = (AABB){min_p, max_p};
    res.entry.is_next_a_bvh = true;
    ElementOnList l = bvh_init_rec(&vl, tlist, 1 + depth);

    res.next_l = element_copy(&l);

    vec_deinit(&vl);
    ElementOnList r = bvh_init_rec(&vr, tlist, 1 + depth);

    res.next_r = element_copy(&r);

    vec_deinit(&vr);

    return res;
}
void bvh_init(BvhList *self, Scene *target)
{
    vec_init(self);

    tempBvhList curr;
    vec_init(&curr);

    // first list creation
    for (int i = 0; i < target->meshes.length; i++)
    {
        Mesh m = target->meshes.data[i];

        if (m.type == MESH_TRIANGLES)
        {
            int count = mesh_count_faces(&m);
            for (int c = 0; c < count; c++)
            {
                Triangle t = scene_mesh_triangle(target, i, c);
                
                BvhEntry entry = {
                    .box = {
                        .min = vec3_min(vec3_min(t.pa, t.pb), t.pc),
                        .max = vec3_max(vec3_max(t.pa, t.pb), t.pc),
                    },
                    .is_next_a_bvh = false,
                    .l = i,
                    .r = c,
                };

                ElementOnList on_list = {
                    .entry = entry,
                };

                vec_push(&curr, on_list);
            }
        }
    }

    // int ii = 0;
    //  fusion really near shape
    // int last = 0;
    //  ii = 0;

    //  last = 0;

    /*
    while (curr.length > 1)
    {
        ii++;
        printf("merge %i / %i ...\n", curr.length, ii);

        ElementOnList *left = NULL;
        int left_idx;
        ElementOnList *right = NULL;
        int right_idx;
        float best_dist = 100000000;
        float dist;
        AABB l_aa;
        AABB r_aa;
        left_idx = last;

        left = &curr.data[last];
        l_aa = curr.data[last].entry.box;

        for (int j = 0; j < curr.length; j++)
        {
            if (last == j)
            {
                continue;
            }
            dist = bvh_distance(&curr.data[last], &curr.data[j]);
            if (dist < best_dist)
            {

                best_dist = dist;
                r_aa = curr.data[j].entry.box;
                right_idx = j;
                right = &curr.data[j];

                if (aabb_intersect(&left->entry.box, &right->entry.box))
                {
                    goto early_ret;
                }
            }
        }

    early_ret:
        if ((!left || !right) && last != 0)
        {
            last = 0;
            continue;
        }
        else if (!left || !right)
        {
            printf("fuck\n");
            break;
        }

        ElementOnList res = {
            .next_l = element_copy(left),
            .next_r = element_copy(right),
            .entry = {
                .box = aabb_surrounding(&l_aa, &r_aa),
                .is_next_a_bvh = true,
            }};

        last = left_idx;
        vec_splice(&curr, left_idx, 1);

        if (right_idx > left_idx)
        {
            right_idx--;
        }

        vec_splice(&curr, right_idx, 1);

        vec_insert(&curr, 0, res);
    }
    */
    BvhEntry start = {};
    vec_push(self, start); // reserve the entry n°0 as a start entry

    ElementOnList element = bvh_init_rec(&curr, self, 0);
    start = bvh_make_from_temp_list(self, &element);

    // bvh_update_parents(self, &start, 0);
    self->data[0] = start;

    printf("bvh size: %lu \n", self->length * sizeof(BvhEntry));

 //    bvh_dump(self, self->data, 0);
}

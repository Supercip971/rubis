#include <obj/bvh.h>
#include <stdio.h>
#include "math/aabb.h"
#include "math/plane.h"
#include "math/vec3.h"
#include "obj/mesh.h"
#include "obj/scene.h"
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


BvhEntry bvh_make_mesh_fusion(BvhEntry left, BvhEntry right)
{

    BvhEntry curr = {
        .is_next_a_bvh = false,
        .la = left.la,
        .lb = left.lb,
        .ra = right.la,
        .rb = right.lb,
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

    if (!left.is_next_a_bvh && !right.is_next_a_bvh && left.ra == 0 && right.ra == 0)
    {
        // if (aabb_intersect(&left.box, &right.box))
        //{
        //
        //    return bvh_make_mesh_fusion(left, right);
        //}
    }

    int l = self->length;

    vec_push(self, left);

    int r = l + 1;

    vec_push(self, right);

    // bvh_update_parents(self, &self->data[l], l);
    // bvh_update_parents(self, &self->data[r], r);
    BvhEntry curr = {
        .is_next_a_bvh = true,
        .la = l,
        .lb = 0,
        .ra = r,
        .rb = 0,
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
        printf("bvh: %i (%i) |%i (%i) \n", entry->la, entry->lb, entry->ra, entry->rb);
    }
    else
    {

        printf("- l: %i \n", entry->la);
        bvh_dump(self, &self->data[entry->la], depth + 1);

        TAB();
        printf("- r: %i \n", entry->ra);
        bvh_dump(self, &self->data[entry->ra], depth + 1);
    }
}
// probably don't know what this does in 1 month
// this tries to create a bvh using a linear memory (usable for gpu) without any pointers
// this is probably really inefficient:
// x + x/2 + x/4 ...
// n + (n)
// but as we are using it once, I may not want to make it faster (for now)

VecDimension pick_dim(Vec3 a, Vec3 b)
{
    float dx = fabs(a.x - b.x);
    float dy = fabs(a.y - b.y);
    float dz = fabs(a.z - b.z);

    if (dx >= dy && dx >= dz)
    {
        return VDIM_X;
    }
    else if (dy >= dx && dy >= dz)
    {
        return VDIM_Y;
    }
    return VDIM_Z;
}
bool is_vec3_dim_superior(Vec3 point, Vec3 maxc, Vec3 minc, VecDimension dim)
{

    if (dim == VDIM_X)
    {
        float center = (maxc.x + minc.x) / 2;
        return point.x >= center;
    }
    else if (dim == VDIM_Y)
    {
        float center = (maxc.y + minc.y) / 2;

        return point.y >= center;
    }
    float center = (maxc.z + minc.z) / 2;

    return point.z >= center;
}

bool triangle_bounding_with(Triangle tri, AAPlane plane, AABB *box)
{
    return triangle_in_plane_aabb(tri.pa, tri.pb, tri.pc, plane, box);
}

AAPlane center_plane_from_aabb(AABB box)
{
    Vec3 center = aabb_centroid(&box);
    VecDimension dim = pick_dim(box.min, box.max);
    AAPlane plane = {
        .dim = dim,
        .t = vec3_dim(center, dim),
        .sign = 0,
    };
    return plane;
}

bool sbvh_bounding_with(BvhList *self, int entry_id, Scene *scene, AAPlane plane, AABB *result)
{
    BvhEntry *entry = &self->data[entry_id];
    if (!entry->is_next_a_bvh)
    {
        Triangle tri = scene_mesh_triangle(scene, entry->la, entry->lb);

        return triangle_bounding_with(tri, plane, result);
    }

    AABB a;
    bool ta = (sbvh_bounding_with(self, entry->la, scene, plane, &a));
    AABB b;
    bool tb = sbvh_bounding_with(self, entry->ra, scene, plane, &b);

    if (ta && tb)
    {
        *result = aabb_surrounding(&a, &b);
        return true;
    }
    else if (ta)
    {
        //  printf("<unable> [L] %i %i \n", entry->la, entry->ra);

        *result = a;
        return true;
    }
    else if (tb)
    {
        //  printf("<unable> [R] %i %i \n", entry->la, entry->ra);

        *result = b;
        return true;
    }

    return false;
}
int bvh_depth(BvhList *self, int entry_id)
{
    BvhEntry *entry = &self->data[entry_id];
    if (!entry->is_next_a_bvh)
    {
        return 1;
    }
    else
    {
        int l = bvh_depth(self, entry->la);
        int r = bvh_depth(self, entry->ra);
        return 1 + (l > r ? l : r);
    }
}

void sbvh_init(BvhList *self, int entry_id, Scene *scene)
{
    BvhEntry *entry = &self->data[entry_id];

    if (!entry->is_next_a_bvh)
    {
        return;
    }

    BvhEntry *right = &self->data[entry->ra];
    BvhEntry *left = &self->data[entry->la];

    AABB lb = left->box;
    AABB rb = right->box;

    int ca = entry->ra;
    int cb = entry->la;

    if (aabb_intersect(&lb, &rb) && (!left->is_next_a_bvh || !right->is_next_a_bvh) && bvh_depth(self, entry_id) <= 2)
    {

        // We have 2 sides, 2 element, if for each element in each sides we nead to compute the AABB
        // of that side element and then push 2 new object for each side

        AAPlane plane_l = center_plane_from_aabb(entry->box);
        plane_l.sign = 1;

        AAPlane plane_r = plane_l;
        plane_r.sign = -1;

        AABB new_lb;
        AABB new_rb;

        AABB new_la;
        AABB new_ra;
        bool valid_la = sbvh_bounding_with(self, entry->la, scene, plane_l, &new_la);
        bool valid_ra = sbvh_bounding_with(self, entry->ra, scene, plane_l, &new_ra);
        bool valid_lb = sbvh_bounding_with(self, entry->la, scene, plane_r, &new_lb);
        bool valid_rb = sbvh_bounding_with(self, entry->ra, scene, plane_r, &new_rb);

        AABB final_a = aabb_surrounding(&new_la, &new_ra);
        AABB final_b = aabb_surrounding(&new_lb, &new_rb);

        // bool valid_diff = aabb_near_same(&final_a, &final_b);
        if (valid_la && valid_ra && valid_lb && valid_rb)
        {

            BvhEntry new_l_entry = {
                .box = final_a,
                .is_next_a_bvh = true,
                .la = entry->la,
                .lb = entry->lb,
                .ra = entry->ra,
                .rb = entry->rb,
            };
            BvhEntry new_r_entry = {
                .box = final_b,

                .is_next_a_bvh = true,
                .la = entry->la,
                .lb = entry->lb,
                .ra = entry->ra,
                .rb = entry->rb,
            };

            int base_id = self->length;
            vec_push(self, new_l_entry);
            vec_push(self, new_r_entry);

            self->data[entry_id].la = base_id;
            self->data[entry_id].ra = base_id + 1;
        }
    }

    sbvh_init(self, ca, scene);
    sbvh_init(self, cb, scene);
}

float sah_cost_entry(ElementOnList *entry)
{
    if (!entry->entry.is_next_a_bvh)
    {
        return 1;
    }
    else
    {

        ElementOnList *left = entry->next_l;
        ElementOnList *right = entry->next_r;
        printf("what ?\n");
        return (sah_cost_entry(left) + sah_cost_entry(right));
    }
}

float sah_cost_list(tempBvhList *list, int split_at)
{
    AABB left = {};
    int cost_left_count = 0;
    AABB right = {};
    int cost_right_count = 0;

    for (int l = 0; l <= split_at; l++)
    {

        ElementOnList *entry = &list->data[l];
        if (cost_left_count == 0)
        {
            left = entry->entry.box;
        }

        left = aabb_surrounding(&left, &entry->entry.box);

        cost_left_count += sah_cost_entry(entry);
    }
    for (int r = split_at + 1; r < list->length; r++)
    {

        ElementOnList *entry = &list->data[r];
        if (cost_right_count == 0)
        {
            right = entry->entry.box;
        }

        right = aabb_surrounding(&right, &entry->entry.box);

        cost_right_count += sah_cost_entry(entry);
    }

    float ccost =
 0.125f + (cost_left_count * aabb_surface_area(left) + cost_right_count * aabb_surface_area(right)) / aabb_surface_area(aabb_surrounding(&left, &right));

    return ccost;
}
// Normaly, a cost function would be using 2 list
// But instead, it's more efficient to use a single list and a plane to make the difference
// between list 1 and list 2
// So if we call sah_cost_list with a different split, we don't need to recreate 2 different list
// WE EXPECT THE LIST TO BE SORTED
int best_sah_cost_list(tempBvhList *list, float *rcost)
{
    float cost = 10000000000.0f;
    int best_index = 0;

    // Sah is overkill for little list
    if (list->length <= 4)
    {
        *rcost = 10;
        return list->length / 2;
    }

    int inc = 1;
    if (list->length > 32)
    {
        inc = (int)((list->length) / 32);
    }

    for (int split_at = 0; split_at < list->length; split_at += inc)
    {
        float ccost = sah_cost_list(list, split_at);

        if (ccost < cost)
        {
            cost = ccost;
            best_index = split_at;

            //    printf("cost %f at %d/%d\n", ccost, split_at, list->length);
        }
    }

    *rcost = cost;

    // cost =
    return best_index;
}

/*

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}
*/
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

        max_c = vec3_max(max_c, aabb_centroid(&list->data[i].entry.box));                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
        min_c = vec3_min(min_c, aabb_centroid(&list->data[i].entry.box));
    }

    VecDimension dim = pick_dim(min_c, max_c);

    AABB lbox = {
        .min = min_p,
        .max = min_p,
    };
    AABB rbox = {
        .min = max_p,
        .max = max_p,
    };

    /*
    bool has_best = false;
    float min_cost = 0.0;
    tempBvhList best;
    int best_id = 0;
    for (int i = 0; i < 3; i++)
    {

        tempBvhList temp = sort_on_axis(list, i, true);
        float result_cost = 0;
        int id = best_sah_cost_list(&temp, &result_cost);

        if (!has_best || result_cost < min_cost)
        {
            if (has_best)
            {
                vec_deinit(&best);
            }
            min_cost = result_cost;
            has_best = true;
            best_id = id;
            best = temp;
        }
    }

 //   printf("[%i] len: %i %i \n", depth, list->length, best_id);

    lbox = best.data[0].entry.box;
    rbox = best.data[best.length - 1].entry.box;
    for (int i = 0; i < best.length; i++)
    {
        if (i <= best_id)
        {
            lbox = aabb_surrounding(&lbox, &best.data[i].entry.box);
            vec_push(&vl, best.data[i]);
        }
        else
        {
            rbox = aabb_surrounding(&rbox, &best.data[i].entry.box);
            vec_push(&vr, best.data[i]);
        }
    }

    //  printf("step[%i] (%i) l: %i r: %i \n", depth, list->length, vl.length, vr.length);
    //  printf("%f %f %f - %f %f %f\n", min_p.x, min_p.y, min_p.z, max_p.x, max_p.y, max_p.z);
    // generally when we have a lot of point in the same space, just random push, maybe I'll do something more
    // intelligent but this is a really rare edge case

    if(list->length > 4 && min_cost > list->length)
    {
        
        printf("cost %f > %f\n", min_cost, (float)list->length);
        abort();

    }*/
   // if ( min_cost > aabb_surface_area(aabb_surrounding(&lbox, &rbox)) * (list->length - 0.125f) || vl.length == 0 || vr.length == 0 
   // || vec3_min_comp((vec3_sub(max_c, min_c))) < 0.1f)
    {
        vec_clear(&vr);
        vec_clear(&vl);

        bool has_l = false;
        bool has_r = false;

        for (int i = 0; i < list->length; i++)
        {
            ElementOnList curr = list->data[i];

            if (is_vec3_dim_superior(aabb_centroid(&curr.entry.box), max_c, min_c, dim))
            {
                if (!has_r)
                {
                    rbox = curr.entry.box;
                    has_r = true;
                }
                else
                {
                    rbox = aabb_surrounding(&rbox, &curr.entry.box);
                }
                vec_push(&vr, curr);
            }
            else
            {
                if (!has_l)
                {
                    lbox = curr.entry.box;
                    has_l = true;
                }
                else
                {
                    lbox = aabb_surrounding(&lbox, &curr.entry.box);
                }
                vec_push(&vl, curr);
            }
        }
    }
    if (vl.length == 0 || vr.length == 0)
    {

        vec_clear(&vr);
        vec_clear(&vl);

        if (list->length == 2)
        {
            vec_push(&vr, list->data[1]);
            vec_push(&vl, list->data[0]);
        }
        else
        {

            printf("<!> doing random push %i %i \n", list->length, depth);
            for (int i = 0; i < list->length; i++)
            {

                ElementOnList curr = list->data[i];
                if (i < list->length / 2)
                {
                    vec_push(&vr, curr);
                }
                else
                {
                    vec_push(&vl, curr);
                }
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
                    .box = 
                        aabb_create_triangle(t.pa,  t.pb,  t.pc),
                    .is_next_a_bvh = false,
                    .la = i,
                    .lb = c,
                    .ra = 0,
                    .rb = 0,
                };

                ElementOnList on_list = {
                    .entry = entry,
                };

                vec_push(&curr, on_list);
            }
        }
    }

    BvhEntry start = {};
    vec_push(self, start); // reserve the entry n°0 as a start entry

    ElementOnList element = bvh_init_rec(&curr, self, 0);
    start = bvh_make_from_temp_list(self, &element);

    // bvh_update_parents(self, &start, 0);
    self->data[0] = start;
    printf("loading sbvh....\n");
    sbvh_init(self, 0, target);
    printf("bvh size: %lu \n", self->length * sizeof(BvhEntry));
    printf("bvh depth: %i \n", bvh_depth(self, 0));
    //    bvh_dump(self, self->data, 0);
}

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
    return triangle_in_plane_aabb(tri.a.pos, tri.b.pos, tri.c.pos, plane, box);
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

AAPlane subdiv_plane_from_aabb(AABB box, float factor)
{
    // Vec3 center = aabb_centroid(&box);
    VecDimension dim = pick_dim(box.min, box.max);
    float min_v = vec3_dim(box.min, dim);
    float max_v = vec3_dim(box.max, dim);
    AAPlane plane = {
        .dim = dim,
        .t = min_v + (max_v - min_v) * factor,
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

        AABB best_a;
        AABB best_b;
        bool is_valid = false;
        float best_sah = 10000000;

        float msah = aabb_surface_area(aabb_surrounding(&lb, &rb));

        for (int i = 0; i < 4; i++)
        {

            AAPlane plane_l = subdiv_plane_from_aabb(entry->box, i * 0.25f);
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
            if (valid_la && valid_ra && valid_lb && valid_rb)
            {
                float sah = aabb_surface_area(final_a) + aabb_surface_area(final_b);
                if (sah < best_sah)
                {
                    best_sah = sah;
                    best_a = final_a;
                    best_b = final_b;
                    is_valid = true;
                }
            }
        }
        // bool valid_diff = aabb_near_same(&final_a, &final_b);
        if (is_valid && best_sah < msah)
        {

            BvhEntry new_l_entry = {
                .box = best_a,
                .is_next_a_bvh = true,
                .la = entry->la,
                .lb = entry->lb,
                .ra = entry->ra,
                .rb = entry->rb,
            };
            BvhEntry new_r_entry = {
                .box = best_b,

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
        return 2;
    }
    else
    {

        ElementOnList *left = entry->next_l;
        ElementOnList *right = entry->next_r;
        return (sah_cost_entry(left) + sah_cost_entry(right));
    }
}
float sah_cost_entry_bounded(ElementOnList *entry, AABB aabb)
{
    if (!aabb_intersect(&aabb, &entry->entry.box))
    {
        return 0;
    }
    if (!entry->entry.is_next_a_bvh)
    {
        return 2;
    }
    else
    {

        ElementOnList *left = entry->next_l;
        ElementOnList *right = entry->next_r;
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
        0.125f + (cost_left_count * aabb_surface_area(left) + cost_right_count * aabb_surface_area(right));

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

int bvh_element_primitive_count(ElementOnList *elt)
{
    int count = 0;
    if (elt->entry.is_next_a_bvh)
    {
        count += bvh_element_primitive_count(elt->next_l);
        count += bvh_element_primitive_count(elt->next_r);
    }
    else
    {
        count += 1;
        count += 1;
    }

    return count;
}
int bvh_list_primitive_count(tempBvhList *list)
{
    int count = 0;
    for (int i = 0; i < list->length; i++)
    {
        count += bvh_element_primitive_count(&list->data[i]);
    }
    return count;
}
float best_cut_impl(tempBvhList *list, VecDimension *dim, float t, bool split)
{

    AABB total = {};
    Vec3 min_c = aabb_centroid(&list->data[0].entry.box);
    Vec3 max_c = aabb_centroid(&list->data[0].entry.box);

    for (int i = 0; i < list->length; i++)
    {
        total = aabb_surrounding(&total, &list->data[i].entry.box);

        Vec3 c = aabb_centroid(&list->data[i].entry.box);
        min_c = vec3_min(min_c, c);
        max_c = vec3_max(max_c, c);
    }

    float from = vec3_dim(min_c, *dim);
    float to = vec3_dim(max_c, *dim);

    float delta = map(t, 0.0, 1.0, from, to);

    AABB left_aabb;
    float left_count = 0;
    float right_count = 0;
    AABB right_aabb;
    AAPlane plane = {};
    plane.dim = *dim;
    plane.t = delta;
    plane.sign = -1;

    for (int i = 0; i < list->length; i++)
    {
        ElementOnList *entry = &list->data[i];
        float c = vec3_dim(aabb_centroid(&entry->entry.box), *dim);
        AABB left = entry->entry.box;
        AABB right = entry->entry.box;

        // if the box is in the part we want to split, we see if we can split it.
        if (aabb_plane_intersection(entry->entry.box, plane, &left, &right) && split)
        {
            if (left_count == 0)
            {
                left_aabb = left;
            }
            if (right_count == 0)
            {
                right_aabb = right;
            }

            left_count += sah_cost_entry_bounded(entry, left);
            right_count += sah_cost_entry_bounded(entry, right);

            left_aabb = aabb_surrounding(&left_aabb, &left);
            right_aabb = aabb_surrounding(&right_aabb, &right);

            continue;
        }
        if (c < delta)
        {
            if (left_count == 0)
            {
                left_aabb = entry->entry.box;
            }
            left_count += sah_cost_entry(entry);

            left_aabb = aabb_surrounding(&left_aabb, &entry->entry.box);
        }
        else
        {

            if (right_count == 0)
            {
                right_aabb = entry->entry.box;
            }
            right_count += sah_cost_entry(entry);

            right_aabb = aabb_surrounding(&right_aabb, &entry->entry.box);
        }
    }

    float invsize = 1.0f / aabb_surface_area(aabb_surrounding(&left_aabb, &right_aabb));

    float r2 = (aabb_surface_area(left_aabb) * left_count + aabb_surface_area(right_aabb) * right_count) * invsize;

   // float alpha = 1e-5f;

   // if (aabb_intersect(&left_aabb, &right_aabb))
   // {
   //     float childs_size = aabb_surface_area(aabb_inter(&left_aabb, &right_aabb));
   //     float root = aabb_surface_area(total);
   //     if (childs_size / root < alpha && split)
   //     {
   //         return 1000000000;
   //     }
   // }

    return r2;
}

float best_cut(tempBvhList *list, VecDimension *dim, bool *split)
{
    float cost = 10000000000.0f;
    float best_cut = 1000000.f;

    // Sah is overkill for little list
    if (list->length <= 4)
    {
        return 0;
    }

    //    AABB total = {};
    for (int i = 0; i < 3; i++)
    {
        VecDimension dim2 = (VecDimension)i;

        for (int t = 0; t < 12; t++)
        {
            float tf = (float)t / 12.0f;
            float tccost = best_cut_impl(list, &dim2, tf, true);
            float fccost = best_cut_impl(list, &dim2, tf, false);

            if (tccost < cost)
            {
                cost = tccost;
                *split = true;
                best_cut = tf;
                *dim = (VecDimension)i;
            }

            if (fccost < cost)
            {
                cost = fccost;
                *split = false;
                best_cut = tf;
                *dim = (VecDimension)i;
            }
        }
    }

    return best_cut;
}
/*

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}
*/

ElementOnList element_simplify(BvhList *list, ElementOnList base, AABB new_box)
{

    (void)list;
    ElementOnList *left = base.next_l;
    ElementOnList *right = base.next_r;

    if (!base.entry.is_next_a_bvh)
    {
        return base;
    }
    if(!aabb_intersect(&left->entry.box, &new_box) && !aabb_intersect(&right->entry.box, &new_box))
    {
        base.entry.box = (AABB){
            .max = left->entry.box.max,
            .min = left->entry.box.max,
        };
        base.entry.is_next_a_bvh = -1;
        return base;
    }
    if (!aabb_intersect(&left->entry.box, &new_box))
    {
          base.entry.box = aabb_inter(&right->entry.box, &new_box);
       
        return element_simplify(list, *right, new_box);
    }
    if (!aabb_intersect(&right->entry.box, &new_box))
    {
        base.entry.box = aabb_inter(&left->entry.box, &new_box);
        return element_simplify(list, *left, new_box);
    }

    ElementOnList ca = element_simplify(list, *left, new_box);
    ElementOnList cb = element_simplify(list, *right, new_box);

    base.next_l = element_copy(&ca);

    base.next_r = element_copy(&cb);
    return base;
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

    AABB tbox = {
        .min = min_c,
        .max = max_c,
    };

    vec_clear(&vr);
    vec_clear(&vl);

    bool has_l = false;
    bool has_r = false;

    float c = vec3_dim(vec3_add(max_c, min_c), dim) / 2.0f;
    bool split = false;
    if (list->length > 8)
    {
        c = best_cut(list, &dim, &split);
        float from = vec3_dim(tbox.min, dim);
        float to = vec3_dim(tbox.max, dim);

        c = map(c, 0.0, 1.0, from, to);
    }

    // printf("SPLIT! %i/%i, c: %f, dim: %d\n", list->length, split, c, dim);
    AAPlane plane = {};
    plane.dim = dim;
    plane.t = c;
    plane.sign = -1;
    for (int i = 0; i < list->length; i++)
    {
        ElementOnList curr = list->data[i];

        if (split)
        {
            AABB nl;
            AABB nr;
            if (aabb_plane_intersection(curr.entry.box, plane, &nl, &nr))
            {

                ElementOnList l = curr;
                ElementOnList r = curr;

                l.entry.box = nl;
                r.entry.box = nr;
                if (!has_l)
                {
                    lbox = nl;
                    has_l = true;
                }
                else
                {
                    lbox = aabb_surrounding(&lbox, &nl);
                }
                l.entry.box = nl;
                vec_push(&vl,  element_simplify(tlist, l, nl));
                if (!has_r)
                {
                    rbox = nr;
                    has_r = true;
                }
                else
                {
                    rbox = aabb_surrounding(&rbox, &nr);
                }
                r.entry.box = nr;
                vec_push(&vr,  element_simplify(tlist, r, nr));
                continue;
            }
        }
        if (vec3_dim(aabb_centroid(&curr.entry.box), dim) > c)
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

    if (vl.length == 0 || vr.length == 0)
    {

        vec_clear(&vr);
        vec_clear(&vl);
        *list = sort_on_axis(list, dim, 1);
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
                if (i > list->length / 2)
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
                        aabb_create_triangle(t.a.pos, t.b.pos, t.c.pos),
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
    // abort();
    //     bvh_dump(self, self->data, 0);
}

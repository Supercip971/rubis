#include <obj/bvh.h>
#include <stdio.h>
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
static float bvh_distance(ElementOnList *left, ElementOnList *right)
{
    AABB a, b;
    Vec3 a_center, b_center;
    a = left->entry.box;
    b = right->entry.box;

    a_center = vec3_add(a.min, vec3_mul_val(vec3_sub(a.max, a.min), 0.5));
    b_center = vec3_add(b.min, vec3_mul_val(vec3_sub(b.max, b.min), 0.5));

    return vec3_squared_length(vec3_sub(b_center, a_center));
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
static inline bool aabb_intersect(const AABB *a, const AABB *b)
{
    return (a->min.x <= b->max.x && a->max.x >= b->min.x) &&
           (a->min.y <= b->max.y && a->max.y >= b->min.y) &&
           (a->min.z <= b->max.z && a->max.z >= b->min.z);
}

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
        element->entry.r = 0;
        return element->entry;
    }

    BvhEntry left = bvh_make_from_temp_list(self, element->next_l);
    BvhEntry right = bvh_make_from_temp_list(self, element->next_r);

    if (!left.is_next_a_bvh && !right.is_next_a_bvh && left.r == 0 && right.r == 0)
    {
        if (aabb_intersect(&left.box, &right.box))
        {

            return bvh_make_mesh_fusion(left, right);
        }
    }

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
        printf("bvh: %i\n", entry->l);
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
    if (!entry->is_next_a_bvh && entry->r != 0)
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

void bvh_init(BvhList *self, Scene *target)
{
    vec_init(self);

    tempBvhList curr;
    vec_init(&curr);

    // first list creation
    for (int i = 0; i < target->meshes.length; i++)
    {
        Mesh m = target->meshes.data[i];
        BvhEntry entry = {
            .box = m.aabb,
            .is_next_a_bvh = false,
            .l = i,
        };

        ElementOnList on_list = {
            .entry = entry,
        };

        vec_push(&curr, on_list);
    }

    int ii = 0;
    // fusion really near shape
    int last = 0;
    ii = 0;

    last = 0;
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

        for (int i = last; i < curr.length; i++)
        {
            for (int j = 0; j < curr.length; j++)
            {
                if (i == j)
                {
                    continue;
                }
                dist = bvh_distance(&curr.data[i], &curr.data[j]);
                if (dist < best_dist)
                {

                    best_dist = dist;
                    left_idx = i;

                    left = &curr.data[i];
                    l_aa = curr.data[i].entry.box;

                    r_aa = curr.data[j].entry.box;
                    right_idx = j;
                    right = &curr.data[j];

                    if (aabb_intersect(&left->entry.box, &right->entry.box))
                    {
                        goto early_ret;
                    }
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

        vec_push(&curr, res);
    }
    BvhEntry start = {};
    vec_push(self, start); // reserve the entry n°0 as a start entry
    start = bvh_make_from_temp_list(self, &curr.data[0]);

    // bvh_update_parents(self, &start, 0);
    self->data[0] = start;

    printf("bvh size: %lu \n", self->length * sizeof(BvhEntry));

    bvh_dump(self, self->data, 0);
}

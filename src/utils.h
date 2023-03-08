#pragma once

#define MAYBE_UNUSED [[maybe_unused]]

#define umax(x, y) ((x) > (y) ? (x) : (y))
#define umin(x, y) ((x) < (y) ? (x) : (y))
#define umax_s(x, y) ({                     \
    __auto_type __x_val = (x);              \
    __auto_type __y_val = (y);              \
    __auto_type r = umax(__x_val, __y_val); \
    r;                                      \
})

#define umin_s(x, y) ({                     \
    __auto_type __x_val = (x);              \
    __auto_type __y_val = (y);              \
    __auto_type r = umin(__x_val, __y_val); \
    r;                                      \
})

#define clamp(x, min, max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define clamp_s(x, min, max) ({                           \
    __auto_type __x_val = (x);                            \
    __auto_type __min_val = (min);                        \
    __auto_type __max_val = (max);                        \
    __auto_type r = clamp(__x_val, __min_val, __max_val); \
    r;                                                    \
})

#define RAD2DEG(x) (x) * (180.0f / M_PI)
#define DEG2RAD(x) (x) * (M_PI / 180.0f)

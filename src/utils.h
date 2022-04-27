#pragma once

#define MAYBE_UNUSED [[maybe_unused]]

#define clamp(x, min, max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define clamp_s(x, min, max) ({                           \
    __auto_type __x_val = (x);                            \
    __auto_type __min_val = (min);                        \
    __auto_type __max_val = (max);                        \
    __auto_type r = clamp(__x_val, __min_val, __max_val); \
    r;                                                    \
})

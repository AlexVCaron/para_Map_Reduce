#ifndef HOUSE_TYPES_H
#define HOUSE_TYPES_H

#include <chrono>

using timer = std::chrono::high_resolution_clock;
using time_length = timer::duration;
using time_stamp = timer::time_point;
using time_unit = std::chrono::milliseconds;

struct exec {};
struct parallele_exec : exec {};
struct sequentiel_exec : exec {};

template <typename exe> 
struct exec_traits
{
    typedef exec exec_category;
};

template<>
struct exec_traits<parallele_exec>
{
    typedef parallele_exec exec_category;
};

template<>
struct exec_traits<sequentiel_exec>
{
    typedef sequentiel_exec exec_category;
};

#endif
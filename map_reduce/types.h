#ifndef HOUSE_TYPES_H
#define HOUSE_TYPES_H

#include "ProtectedT.h"
#include "MRUtilities.h"

#include <string>
#include <chrono>


using p_usubber = protectedT<subber<unsigned>>;
using p_files_data = protectedT<files_data>;

using timer = std::chrono::high_resolution_clock;
using time_length = timer::duration;
using time_stamp = timer::time_point;
using time_unit = std::chrono::milliseconds;

typedef bool(*rule)(std::string);

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
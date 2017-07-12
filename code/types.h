#ifndef HOUSE_TYPES_H
#define HOUSE_TYPES_H

#include <chrono>

using timer = std::chrono::high_resolution_clock;
using time_length = timer::duration;
using time_stamp = timer::time_point;
using time_unit = std::chrono::milliseconds;

struct exec_tag {};
struct parallele_exec_tag : exec_tag {};
struct sequentiel_exec_tag : exec_tag {};

#endif
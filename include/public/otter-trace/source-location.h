/**
 * @file source-location.h
 * @author Ada, Tuft
 * @brief Given file & function names, return a corresponding otter_src_ref_t.
 * @version 0.1
 * @date 2023-05-24
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "public/otter-common.h"

otter_src_ref_t get_source_location_ref(otter_src_location_t location);

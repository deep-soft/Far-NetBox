/* functable.h -- Struct containing function pointers to optimized functions
 * Copyright (C) 2017 Hans Kristian Rosbach
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ifndef FUNCTABLE_H_
#define FUNCTABLE_H_

#include "deflate.h"

struct functable_s {
    void     (* fill_window)    (deflate_state *s);
    Pos      (* insert_string)  (deflate_state *const s, const Pos str, uint32_t count);
    uint32_t (* adler32)        (uint32_t adler, const uint8_t *buf, uint32_t len);
    uint32_t (* crc32)          (uint32_t crc, const uint8_t *buf, uint32_t len);
};

ZLIB_INTERNAL extern __thread struct functable_s functable;


#endif
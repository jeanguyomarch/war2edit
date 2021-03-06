/*
 * Copyright (c) 2015-2016 Jean Guyomarc'h
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _TILE_H_
#define _TILE_H_

typedef enum
{
   TILE_NONE            = 0,
   TILE_TREES           = 1,
   TILE_GRASS_LIGHT     = 2,
   TILE_GROUND_LIGHT    = 3,
   TILE_WATER_LIGHT     = 4,
   TILE_WATER_DARK      = 5,
   TILE_GRASS_DARK      = 6,
   TILE_GROUND_DARK     = 7,
   TILE_ROCKS           = 8,
   TILE_HUMAN_WALL      = 9,
   TILE_ORC_WALL        = 10,

   __TILE_LAST,

   TILE_WALL_OPEN       = (1 << 5),
   TILE_WALL_CLOSED     = (1 << 6),
   TILE_WALL_MASK       = (TILE_WALL_OPEN | TILE_WALL_CLOSED),

   TILE_SPECIAL         = (1 << 7),

   /* Special value. Has nothing to do with the rest of the enum */
   TILE_RANDOMIZE       = 0x7f
} Tile;

typedef enum
{
   TILE_PROPAGATE_NONE  = 0,
   TILE_PROPAGATE_FULL  = 0xf,      /* 1111 */

   TILE_PROPAGATE_T     = (1 << 0), /* 0001 */
   TILE_PROPAGATE_B     = (1 << 1), /* 0010 */
   TILE_PROPAGATE_L     = (1 << 2), /* 0100 */
   TILE_PROPAGATE_R     = (1 << 3), /* 1000 */

   TILE_PROPAGATE_TL    = (TILE_PROPAGATE_T | TILE_PROPAGATE_L),
   TILE_PROPAGATE_TR    = (TILE_PROPAGATE_T | TILE_PROPAGATE_R),
   TILE_PROPAGATE_BL    = (TILE_PROPAGATE_B | TILE_PROPAGATE_L),
   TILE_PROPAGATE_BR    = (TILE_PROPAGATE_B | TILE_PROPAGATE_R)

} Tile_Propagate;

typedef struct
{
   int                  x;
   int                  y;
   Tile_Propagate       prop;
   uint8_t              tl;
   uint8_t              tr;
   uint8_t              bl;
   uint8_t              br;

   Eina_Bool            valid;
   Eina_Bool            conflict;

} Tile_Propagation;

static inline Eina_Bool
tile_solid_is(uint8_t tl,
              uint8_t tr,
              uint8_t bl,
              uint8_t br)
{
   return ((tl == tr) && (tl == bl) && (tl == br));
}

static inline Eina_Bool
tile_dark_water_is(uint8_t tl,
                   uint8_t tr,
                   uint8_t bl,
                   uint8_t br)
{
   return ((tl == TILE_WATER_DARK) && (tr == TILE_WATER_DARK) &&
           (bl == TILE_WATER_DARK) && (br == TILE_WATER_DARK));
}

static inline Eina_Bool
tile_water_is(uint8_t tl,
              uint8_t tr,
              uint8_t bl,
              uint8_t br)
{
   return ((tl == TILE_WATER_LIGHT) || (tl == TILE_WATER_DARK) ||
           (tr == TILE_WATER_LIGHT) || (tr == TILE_WATER_DARK) ||
           (bl == TILE_WATER_LIGHT) || (bl == TILE_WATER_DARK) ||
           (br == TILE_WATER_LIGHT) || (br == TILE_WATER_DARK));
}

static inline Eina_Bool
tile_coast_is(uint8_t tl,
              uint8_t tr,
              uint8_t bl,
              uint8_t br)
{
   return ((((tl == TILE_GROUND_LIGHT) || (tr == TILE_GROUND_LIGHT)) ||
            ((bl == TILE_GROUND_LIGHT) || (br == TILE_GROUND_LIGHT))) &&
           (((tl == TILE_WATER_LIGHT) || (tr == TILE_WATER_LIGHT)) ||
            ((bl == TILE_WATER_LIGHT) || (br == TILE_WATER_LIGHT))));
}

static inline Eina_Bool
tile_ground_is(uint8_t tl,
               uint8_t tr,
               uint8_t bl,
               uint8_t br)
{
   return (tile_solid_is(tl, tr, bl, br) &&
           ((tl == TILE_GROUND_LIGHT) || (tl == TILE_GROUND_DARK)));
}

static inline Eina_Bool
tile_dirt_is(uint8_t tl,
             uint8_t tr,
             uint8_t bl,
             uint8_t br)
{
   return ((tl == TILE_GROUND_LIGHT) || (tl == TILE_GROUND_DARK) ||
           (tr == TILE_GROUND_LIGHT) || (tr == TILE_GROUND_DARK) ||
           (bl == TILE_GROUND_LIGHT) || (bl == TILE_GROUND_DARK) ||
           (br == TILE_GROUND_LIGHT) || (br == TILE_GROUND_DARK));
}

static inline Eina_Bool
tile_coast_corner_is(uint8_t tl,
                     uint8_t tr,
                     uint8_t bl,
                     uint8_t br)
{
   unsigned int count = 0;

   if (tl == TILE_GROUND_LIGHT) count++;
   if (tr == TILE_GROUND_LIGHT) count++;
   if (bl == TILE_GROUND_LIGHT) count++;
   if (br == TILE_GROUND_LIGHT) count++;

   if (count == 3)
     {
        count = 0;
        if (tl == TILE_WATER_LIGHT) ++count;
        if (tr == TILE_WATER_LIGHT) ++count;
        if (bl == TILE_WATER_LIGHT) ++count;
        if (br == TILE_WATER_LIGHT) ++count;
        return (count == 1);
     }
   return EINA_FALSE;
}

static inline Eina_Bool
tile_grass_is(uint8_t tl,
              uint8_t tr,
              uint8_t bl,
              uint8_t br)
{
   return (((tl == TILE_GRASS_LIGHT) || (tl == TILE_GRASS_DARK)) &&
           ((tr == TILE_GRASS_LIGHT) || (tr == TILE_GRASS_DARK)) &&
           ((br == TILE_GRASS_LIGHT) || (br == TILE_GRASS_DARK)) &&
           ((bl == TILE_GRASS_LIGHT) || (bl == TILE_GRASS_DARK)));
}

static inline Eina_Bool
tile_light_grass_is(uint8_t tl,
                    uint8_t tr,
                    uint8_t bl,
                    uint8_t br)
{
   return ((tl == TILE_GRASS_LIGHT) && (tr == TILE_GRASS_LIGHT) &&
           (br == TILE_GRASS_LIGHT) && (bl == TILE_GRASS_LIGHT));
}

static inline Eina_Bool
tile_trees_is(uint8_t tl,
              uint8_t tr,
              uint8_t bl,
              uint8_t br)
{
   return ((tl == TILE_TREES) || (tr == TILE_TREES) ||
           (bl == TILE_TREES) || (br == TILE_TREES));
}

static inline Eina_Bool
tile_rocks_is(uint8_t tl,
              uint8_t tr,
              uint8_t bl,
              uint8_t br)
{
   return ((tl == TILE_ROCKS) || (tr == TILE_ROCKS) ||
           (bl == TILE_ROCKS) || (br == TILE_ROCKS));
}

static inline Eina_Bool
tile_wall_is(uint8_t tl,
             uint8_t tr EINA_UNUSED,
             uint8_t bl EINA_UNUSED,
             uint8_t br EINA_UNUSED)
{
   /* Wall tiles always contain 4 walls */
   return ((tl & TILE_WALL_OPEN) || (tl & TILE_WALL_CLOSED));
}

static inline Eina_Bool
tile_walkable_is(uint8_t tl,
                 uint8_t tr,
                 uint8_t bl,
                 uint8_t br)
{
   return (!tile_wall_is(tl, tr, bl, br) &&
           !tile_trees_is(tl, tr, bl, br) &&
           !tile_rocks_is(tl, tr, bl, br) &&
           !tile_water_is(tl, tr, bl, br) &&
           !tile_coast_is(tl, tr, bl, br));
}

static inline Eina_Bool
tile_deep_water_is(uint8_t tl,
                   uint8_t tr,
                   uint8_t bl,
                   uint8_t br)
{
   return (((tl == TILE_WATER_LIGHT) || (tl == TILE_WATER_DARK)) &&
           ((tr == TILE_WATER_LIGHT) || (tr == TILE_WATER_DARK)) &&
           ((bl == TILE_WATER_LIGHT) || (bl == TILE_WATER_DARK)) &&
           ((br == TILE_WATER_LIGHT) || (br == TILE_WATER_DARK)));
}

static inline Eina_Bool
tile_wall_has(uint8_t tl,
              uint8_t tr,
              uint8_t bl,
              uint8_t br)
{
   const uint8_t mask = TILE_WALL_OPEN | TILE_WALL_CLOSED;
   return ((tl & mask) || (tr & mask) || (bl & mask) || (br & mask));
}

uint16_t
tile_calculate(uint8_t tl,
               uint8_t tr,
               uint8_t bl,
               uint8_t br,
               uint8_t seed,
               Pud_Era era);

uint16_t
tile_mask_calculate(uint8_t tl,
                    uint8_t tr,
                    uint8_t bl,
                    uint8_t br);

void
tile_decompose(uint16_t  tile_code,
               uint8_t  *tl,
               uint8_t  *tr,
               uint8_t  *bl,
               uint8_t  *br,
               uint8_t  *seed);

Eina_Bool
tile_fragments_compatible_are(uint8_t t1,
                              uint8_t t2);

Eina_Bool
tile_compatible_is(uint8_t tl,
                   uint8_t tr,
                   uint8_t bl,
                   uint8_t br);

uint8_t tile_conflict_resolve_get(uint8_t imposed,
                                  uint8_t conflict);

/* Helpers */
#define TILE_SOLID_IS(cptr) \
   tile_solid_is((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

#define TILE_WATER_IS(cptr) \
   tile_water_is((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

#define TILE_DARK_WATER_IS(cptr) \
   tile_dark_water_is((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

#define TILE_GROUND_IS(cptr) \
   tile_ground_is((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

#define TILE_GRASS_IS(cptr) \
   tile_grass_is((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

#define TILE_LIGHT_GRASS_IS(cptr) \
   tile_light_grass_is((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

#define TILE_TREES_IS(cptr) \
   tile_trees_is((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

#define TILE_ROCKS_IS(cptr) \
   tile_rocks_is((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

#define TILE_WALKABLE_IS(cptr) \
   tile_walkable_is((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

#define TILE_WALL_IS(cptr) \
   tile_wall_is((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

uint16_t
tile_action_get(uint8_t tl,
                uint8_t tr,
                uint8_t bl,
                uint8_t br);

#define TILE_ACTION_GET(cptr) \
   tile_action_get((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

uint16_t
tile_movement_get(uint8_t tl,
                  uint8_t tr,
                  uint8_t bl,
                  uint8_t br);

#define TILE_MOVEMENT_GET(cptr) \
   tile_movement_get((cptr)->tile_tl, (cptr)->tile_tr, (cptr)->tile_bl, (cptr)->tile_br)

#endif /* ! _TILE_H_ */

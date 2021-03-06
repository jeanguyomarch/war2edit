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

#include "war2edit.h"

#define SELECTION_1x1 "sel/1x1"
#define SELECTION_2x2 "sel/2x2"
#define SELECTION_3x3 "sel/3x3"
#define SELECTION_4x4 "sel/4x4"

static Eet_File *_units_ef = NULL;
static Eet_File *_buildings[4] = { NULL, NULL, NULL, NULL };
static Eina_Hash *_sprites = NULL;
static cairo_surface_t *_sels[4] = { NULL, NULL, NULL, NULL };


static Sprite_Descriptor *
_sprite_descriptor_new(unsigned char *data,
                       unsigned int   w,
                       unsigned int   h)
{
   Sprite_Descriptor *d;

   d = malloc(sizeof(*d));
   if (EINA_UNLIKELY(!d))
     {
        CRI("Failed to allocate memory");
        return NULL;
     }
   d->data = data;
   d->w = w;
   d->h = h;
   d->color = PUD_PLAYER_RED;

   return d;
}

static void
_sprite_descriptor_free(Sprite_Descriptor *d)
{
   if (d)
     {
        free(d->data);
        free(d);
     }
}


static void *
_sprite_load(Eet_File     *src,
            const char   *key,
            unsigned int *w_ret,
            unsigned int *h_ret)
{
   unsigned char *mem;

   mem = eet_data_image_read(src, key, w_ret, h_ret, NULL, NULL, NULL, NULL);
   if (EINA_UNLIKELY(!mem))
     {
        CRI("Failed to load sprite for key \"%s\"", key);
        return NULL;
     }
   return mem;
}

Eet_File *
sprite_units_open(void)
{
   Eet_File *ef;
   char path[PATH_MAX];

   /* Don't open the units file twice */
   if  (_units_ef)
     return _units_ef;

   snprintf(path, sizeof(path),
            "%s/sprites/units/units.eet", elm_app_data_dir_get());
   ef = eet_open(path, EET_FILE_MODE_READ);
   if (EINA_UNLIKELY(ef == NULL))
     {
        CRI("Failed to open [%s]", path);
        return NULL;
     }
   DBG("Open units file [%s]", path);
   _units_ef = ef;

   return ef;
}

Eet_File *
sprite_buildings_open(Pud_Era era)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((era >= 0) && (era <= 3), NULL);

   Eet_File *ef;
   const char *file = NULL;
   char path[PATH_MAX];

   /* Don't load buildings file twice */
   if (_buildings[era])
     return _buildings[era];

   switch (era)
     {
      case PUD_ERA_FOREST:    file = "sprites/buildings/forest.eet";    break;
      case PUD_ERA_WINTER:    file = "sprites/buildings/winter.eet";    break;
      case PUD_ERA_WASTELAND: file = "sprites/buildings/wasteland.eet"; break;
      case PUD_ERA_SWAMP:     file = "sprites/buildings/swamp.eet";     break;
     }

   snprintf(path, sizeof(path), "%s/%s", elm_app_data_dir_get(), file);
   ef = eet_open(path, EET_FILE_MODE_READ);
   if (EINA_UNLIKELY(ef == NULL))
     {
        CRI("Failed to open [%s]", path);
        return NULL;
     }
   DBG("Open buildings file [%s]", path);

   _buildings[era] = ef;

   return ef;
}

Sprite_Descriptor *
sprite_get(Pud_Unit       unit,
           Pud_Era        era,
           Sprite_Info    info,
           Eina_Bool     *flip_me)
{
   unsigned char *data;
   char key[64];
   Eet_File *ef;
   Eina_Bool chk;
   int orient;
   Eina_Bool flip;
   Sprite_Descriptor *d;
   unsigned int w, h;

   if (pud_unit_building_is(unit))
     {
        ef = _buildings[era];
        snprintf(key, sizeof(key), "%s/%s",
                 pud_era_to_string(era), pud_unit_to_string(unit, PUD_FALSE));
        flip = EINA_FALSE;
     }
   else
     {
        ef = _units_ef;

        if (info != SPRITE_INFO_ICON)
          {
             switch (info)
               {
                case SPRITE_INFO_SOUTH_WEST:
                   orient = SPRITE_INFO_SOUTH_EAST;
                   flip = EINA_TRUE;
                   break;

                case SPRITE_INFO_WEST:
                   orient = SPRITE_INFO_EAST;
                   flip = EINA_TRUE;
                   break;

                case SPRITE_INFO_NORTH_WEST:
                   orient = SPRITE_INFO_NORTH_EAST;
                   flip = EINA_TRUE;
                   break;

                default:
                   orient = info;
                   flip = EINA_FALSE;
                   break;
               }

             switch (unit)
               {
                case PUD_UNIT_GNOMISH_SUBMARINE:
                case PUD_UNIT_GIANT_TURTLE:
                case PUD_UNIT_CRITTER:
                   snprintf(key, sizeof(key), "%s/%s/%i",
                            pud_unit_to_string(unit, PUD_FALSE), pud_era_to_string(era), orient);
                   break;

                case PUD_UNIT_HUMAN_START:
                case PUD_UNIT_ORC_START:
                   snprintf(key, sizeof(key), "%s/0", pud_unit_to_string(unit, PUD_FALSE));
                   break;

                default:
                   snprintf(key, sizeof(key), "%s/%i",
                            pud_unit_to_string(unit, PUD_FALSE), orient);
                   break;
               }
          }
        else
          {
             CRI("ICONS not implemented!");
             return NULL;
          }
     }
   if (flip_me) *flip_me = flip;

   key[sizeof(key) - 1] = '\0';
   d = eina_hash_find(_sprites, key);
   if (d == NULL)
     {
        data = _sprite_load(ef, key, &w, &h);
        if (EINA_UNLIKELY(data == NULL))
          {
             ERR("Failed to load sprite for key [%s]", key);
             return NULL;
          }

        d = _sprite_descriptor_new(data, w, h);
        if (EINA_UNLIKELY(!d))
          {
             CRI("Failed to create sprite descriptor");
             free(data);
             return NULL;
          }
        chk = eina_hash_add(_sprites, key, d);
        if (EINA_UNLIKELY(chk == EINA_FALSE))
          {
             ERR("Failed to add sprite <%p> to hash", data);
             _sprite_descriptor_free(d);
             return NULL;
          }
        //DBG("Access key [%s] (not yet registered). SRT = <%p>", key, data);
        return d;
     }
   else
     {
        //DBG("Access key [%s] (already registered). SRT = <%p>", key, data);
        return d;
     }
}

Sprite_Info
sprite_info_random_get(void)
{
   /* Does not return 4 */
   return rand() % (SPRITE_INFO_NORTH_WEST - SPRITE_INFO_NORTH) + SPRITE_INFO_NORTH;
}


static void
_free_cb(void *data)
{
   free(data);
}

Eina_Bool
sprite_init(void)
{
   char path[PATH_MAX];
   const char *sels[] = {
      "sel1x1.png",
      "sel2x2.png",
      "sel3x3.png",
      "sel4x4.png",
   };
   int i;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(
      EINA_C_ARRAY_LENGTH(sels) == EINA_C_ARRAY_LENGTH(_sels),
      EINA_FALSE);

   if (EINA_UNLIKELY(!sprite_units_open()))
     {
        CRI("Failed to open units file");
        goto fail;
     }

   _sprites = eina_hash_string_superfast_new(_free_cb);
   if (EINA_UNLIKELY(!_sprites))
     {
        CRI("Failed to create Hash for sprites");
        goto sprites_fail;
     }

   /* Load selection sprites */
   for (i = 0; i < (int)EINA_C_ARRAY_LENGTH(_sels); i++)
     {
        snprintf(path, sizeof(path),
                 "%s/sprites/misc/%s", elm_app_data_dir_get(), sels[i]);
        path[sizeof(path) - 1] = '\0';
        if (!ecore_file_exists(path))
          {
             CRI("File \"%s\" does not exist", path);
             goto sel_fail;
          }
        _sels[i] = cairo_image_surface_create_from_png(path);
        if (EINA_UNLIKELY(!_sels[i]))
          {
             CRI("Failed to create cairo surface from %s", path);
             goto surf_fail;
          }
     }

   return EINA_TRUE;

surf_fail:
   for (i--; i >= 0; i--)
     {
        cairo_surface_destroy(_sels[i]);
        _sels[i] = NULL;
     }
sel_fail:
   eina_hash_free(_sprites);
sprites_fail:
   eet_close(_units_ef);
fail:
   return EINA_FALSE;
}

void
sprite_shutdown(void)
{
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(_buildings); ++i)
     {
        if (_buildings[i])
          {
             eet_close(_buildings[i]);
             _buildings[i] = NULL;
          }
     }

   for (i = 0; i < EINA_C_ARRAY_LENGTH(_sels); i++)
     {
        cairo_surface_destroy(_sels[i]);
        _sels[i] = NULL;
     }

   eet_close(_units_ef);
   _units_ef = NULL;
   eina_hash_free(_sprites);
   _sprites = NULL;
}

void
sprite_tile_size_get(Pud_Unit      unit,
                     unsigned int *sprite_w,
                     unsigned int *sprite_h)
{
   const unsigned int size = pud_unit_size_get(unit);
   if (sprite_w) *sprite_w = size;
   if (sprite_h) *sprite_h = size;
}

cairo_surface_t *
sprite_selection_get(unsigned int edge)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((edge <= 0) || (edge > 4), NULL);
   return _sels[edge - 1];
}

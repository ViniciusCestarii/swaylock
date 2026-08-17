#ifndef _SWAYLOCK_ANIMATION_H
#define _SWAYLOCK_ANIMATION_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "cairo.h"

// A sequence of images played back one frame per typed character. Frames are
// ordered by the number that leads their file name, so a directory holding
// 1-nyancat.png, 2-nyancat.png, ... plays back in that order.
struct swaylock_animation {
	cairo_surface_t **frames;
	size_t frame_count;
};

// Loads every image in dir into anim, replacing anything already loaded.
// Returns false and leaves anim empty if the directory holds no usable image.
bool load_animation(struct swaylock_animation *anim, const char *dir);
void destroy_animation(struct swaylock_animation *anim);

// The frame for the given position in the sequence, wrapping in both
// directions so that backspacing past the first frame lands on the last one.
// Returns NULL when no frames are loaded.
cairo_surface_t *animation_frame(struct swaylock_animation *anim, int32_t pos);

#endif

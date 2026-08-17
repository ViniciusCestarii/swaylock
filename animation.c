#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "animation.h"
#include "background-image.h"
#include "log.h"

// The number leading a file name, or LONG_MAX for names that don't start with
// one, so that unnumbered files sort after the numbered sequence.
static long frame_number(const char *name) {
	char *end;
	long number = strtol(name, &end, 10);
	if (end == name) {
		return LONG_MAX;
	}
	return number;
}

static int compare_frames(const struct dirent **a, const struct dirent **b) {
	long na = frame_number((*a)->d_name);
	long nb = frame_number((*b)->d_name);
	if (na != nb) {
		return na < nb ? -1 : 1;
	}
	return strcmp((*a)->d_name, (*b)->d_name);
}

static int select_frames(const struct dirent *entry) {
	return entry->d_name[0] != '.';
}

bool load_animation(struct swaylock_animation *anim, const char *dir) {
	destroy_animation(anim);

	struct dirent **entries;
	int count = scandir(dir, &entries, select_frames, compare_frames);
	if (count < 0) {
		swaylock_log_errno(LOG_ERROR, "Failed to read animation directory %s",
				dir);
		return false;
	}

	anim->frames = calloc(count, sizeof(cairo_surface_t *));
	if (!anim->frames) {
		swaylock_log(LOG_ERROR, "Failed to allocate animation frames.");
		for (int i = 0; i < count; ++i) {
			free(entries[i]);
		}
		free(entries);
		return false;
	}

	for (int i = 0; i < count; ++i) {
		char path[PATH_MAX];
		if (snprintf(path, sizeof(path), "%s/%s", dir, entries[i]->d_name) <
				(int)sizeof(path)) {
			cairo_surface_t *frame = load_background_image(path);
			if (frame) {
				anim->frames[anim->frame_count++] = frame;
				swaylock_log(LOG_DEBUG, "Loaded animation frame %zu from %s",
						anim->frame_count, path);
			}
		} else {
			swaylock_log(LOG_ERROR, "Animation frame path is too long: %s/%s",
					dir, entries[i]->d_name);
		}
		free(entries[i]);
	}
	free(entries);

	if (anim->frame_count == 0) {
		swaylock_log(LOG_ERROR, "No usable images in animation directory %s",
				dir);
		destroy_animation(anim);
		return false;
	}
	return true;
}

void destroy_animation(struct swaylock_animation *anim) {
	for (size_t i = 0; i < anim->frame_count; ++i) {
		cairo_surface_destroy(anim->frames[i]);
	}
	free(anim->frames);
	anim->frames = NULL;
	anim->frame_count = 0;
}

cairo_surface_t *animation_frame(struct swaylock_animation *anim, int32_t pos) {
	if (anim->frame_count == 0) {
		return NULL;
	}
	// C's % keeps the sign of the dividend, so shift negatives back into range
	int32_t index = pos % (int32_t)anim->frame_count;
	if (index < 0) {
		index += anim->frame_count;
	}
	return anim->frames[index];
}

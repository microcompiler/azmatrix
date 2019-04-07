
#include "mx_font.h"

const uint8_t * mx_font_get_bitmap(const mx_font_t * font_p, uint32_t letter)
{
	const mx_font_t * font_i = font_p;
	while (font_i != NULL) {
		if (letter >= font_i->first_ascii && letter <= font_i->last_ascii) {
			uint32_t index = (letter - font_i->first_ascii);
			return &font_i->bitmap[font_i->map[index]];
		}
	}
	return NULL;
}

uint8_t mx_font_get_width(const mx_font_t * font_p, uint32_t letter)
{
	const mx_font_t * font_i = font_p;
	while (font_i != NULL) {
		if (letter >= font_i->first_ascii && letter <= font_i->last_ascii) {
			uint32_t index = (letter - font_i->first_ascii);
			return font_i->width[index];
		}
	}
	return 0;
}
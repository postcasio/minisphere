#include "minisphere.h"

#include "geometry.h"

rect_t
new_rect(int x1, int y1, int x2, int y2)
{
	rect_t rectangle;

	rectangle.x1 = x1;
	rectangle.y1 = y1;
	rectangle.x2 = x2;
	rectangle.y2 = y2;
	return rectangle;
}

bool
do_lines_intersect(rect_t a, rect_t b)
{
	float d, q, r, s;

	q = (a.y1 - b.y1) * (b.x2 - b.x1) - (a.x1 - b.x1) * (b.y2 - b.y1);
	d = (a.x2 - a.x1) * (b.y2 - b.y1) - (a.y2 - a.y1) * (b.x2 - b.x1);
	if (d == 0)
		return false;
	r = q / d;
	q = (a.y1 - b.y1) * (a.x2 - a.x1) - (a.x1 - b.x1) * (a.y2 - a.y1);
	s = q / d;
	return !(r < 0 || r > 1 || s < 0 || s > 1);
}

bool
do_rects_intersect(rect_t a, rect_t b)
{
	return !(a.x1 >= b.x2 || a.x2 <= b.x1 || a.y1 >= b.y2 || a.y2 <= b.y1);
}

bool
is_point_in_rect(int x, int y, rect_t bounds)
{
	return x >= bounds.x1 && x < bounds.x2
		&& y >= bounds.y1 && y < bounds.y2;
}

rect_t
translate_rect(rect_t rect, int x_offset, int y_offset)
{
	return new_rect(rect.x1 + x_offset, rect.y1 + y_offset, rect.x2 + x_offset, rect.y2 + y_offset);
}

rect_t
zoom_rect(rect_t rect, double scale_x, double scale_y)
{
	int old_width, old_height;
	int width, height;
	int x, y;
	
	if (scale_x == 1.0 && scale_y == 1.0)
		return rect;
	old_width = rect.x2 - rect.x1; old_height = rect.y2 - rect.y1;
	width = old_width * scale_x; height = old_height * scale_y;
	x = rect.x1 - (width - old_width) / 2;
	y = rect.y1 - (height - old_height) / 2;
	return new_rect(x, y, x + width, y + height);
}

bool
fread_rect_16(FILE* file, rect_t* out_rect)
{
	int16_t x1, y1, x2, y2;

	if (fread(&x1, 2, 1, file) != 1) return false;
	if (fread(&y1, 2, 1, file) != 1) return false;
	if (fread(&x2, 2, 1, file) != 1) return false;
	if (fread(&y2, 2, 1, file) != 1) return false;
	out_rect->x1 = x1; out_rect->y1 = y1;
	out_rect->x2 = x2; out_rect->y2 = y2;
	return true;
}

bool
fread_rect_32(FILE* file, rect_t* out_rect)
{
	int32_t x1, y1, x2, y2;

	if (fread(&x1, 4, 1, file) != 1) return false;
	if (fread(&y1, 4, 1, file) != 1) return false;
	if (fread(&x2, 4, 1, file) != 1) return false;
	if (fread(&y2, 4, 1, file) != 1) return false;
	out_rect->x1 = x1; out_rect->y1 = y1;
	out_rect->x2 = x2; out_rect->y2 = y2;
	return true;
}

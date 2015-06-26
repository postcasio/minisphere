#include "image.h"

void init_surface_api (void);

typedef
enum blend_mode
{
	BLEND_BLEND,
	BLEND_REPLACE,
	BLEND_RGB_ONLY,
	BLEND_ALPHA_ONLY,
	BLEND_ADD,
	BLEND_SUBTRACT,
	BLEND_MULTIPLY,
	BLEND_AVERAGE,
	BLEND_INVERT,
	BLEND_MAX
} blend_mode_t;

extern void     duk_push_sphere_surface    (duk_context* ctx, image_t* image);
extern image_t* duk_require_sphere_surface (duk_context* ctx, duk_idx_t index);

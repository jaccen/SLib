#include "image_stb.h"

#ifdef SLIB_GRAPHICS_IMAGE_USE_STB

#ifdef _MSC_VER
#pragma warning(disable : 4312)
#pragma warning(disable : 4996)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "../../../inc/thirdparty/stb/stbi_ext/stb_image.h"

#include "../../../inc/thirdparty/stb/stbi_ext/etc1_utils.c"

SLIB_GRAPHICS_NAMESPACE_BEGIN

Ref<Image> Image_STB::loadImage(const void* content, sl_size size)
{
	Ref<Image> ret;
	int width;
	int height;
	int channels;
	unsigned char *map = stbi_load_from_memory((stbi_uc*)content, (int)size, &width, &height, &channels, 4);
	if (map) {
		ret = Image::create(width, height);
		if (ret.isNotNull()) {
			Base::copyMemory(ret->getColors(), map, width * height * 4);
		}
		stbi_image_free(map);
	}
	return ret;
}

SLIB_GRAPHICS_NAMESPACE_END

#endif
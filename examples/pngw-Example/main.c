#include <png.h>

#define PNGW_IMPLEMENTATION
#include <png_wrapper.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	size_t width, height, file_depth;
	pngw_color file_color;

	pngw_error err = pngwFileInfo("dude.png", &width, &height, &file_depth, &file_color);

	if (err != PNGW_ERROR_NONE)
	{
		printf("an error has occured: %s\n", PNGW_ERROR_DESCRIPTIONS[err]);
		return 1;
	}

	printf("dude.png info: width=%zu height=%zu depth=%zu color=%s\n", width, height, file_depth, PNGW_COLOR_NAMES[file_color]);

	const size_t load_depth = 16;
	const pngw_color load_color = PNGW_COLOR_RGB;

	size_t size;
	err = pngwDataSize(width, height, load_depth, load_color, &size);

	if (err != PNGW_ERROR_NONE)
	{
		printf("an error has occured: %s\n", PNGW_ERROR_DESCRIPTIONS[err]);
		return 1;
	}

	printf("dude.png pixel data size: %zu bytes\n", size);

	pngwb_t* data = malloc(size);

	if (!data)
	{
		printf("an error has occured: out of memory\n");
		return 1;
	}

	printf("dude.png load configuration: width=%zu, height=%zu depth=%zu color=%s\n", width, height, load_depth, PNGW_COLOR_NAMES[load_color]);

	err = pngwReadFile("dude.png", data, PNGW_DEFAULT_ROW_OFFSET, width, height, load_depth, load_color);

	if (err != PNGW_ERROR_NONE)
	{
		printf("an error has occured: %s\n", PNGW_ERROR_DESCRIPTIONS[err]);
		return 1;
	}

	err = pngwWriteFile("new_dude.png", data, PNGW_DEFAULT_ROW_OFFSET, width, height, load_depth, load_color);

	if (err != PNGW_ERROR_NONE)
	{
		printf("an error has occured: %s\n", PNGW_ERROR_DESCRIPTIONS[err]);
		return 1;
	}

	return 0;
}
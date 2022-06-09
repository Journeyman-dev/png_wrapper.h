/*
    Copyright (c) 2022 Daniel Valcour
    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*

    png_wrapper.h version 1.1.0

 */

#ifndef PNGW_H
#define PNGW_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stddef.h>
#include <stdint.h>

typedef uint8_t pngwb_t;
typedef uint16_t pngws_t;

typedef enum pngw_error
{
    PNGW_ERROR_NONE = 0,
    PNGW_ERROR_FILE_NOT_FOUND = 1,
    PNGW_ERROR_FILE_CREATION_FAILURE = 2,
    PNGW_ERROR_OUT_OF_MEMORY = 3,
    PNGW_ERROR_INVALID_FILE_SIGNITURE = 4,
    PNGW_ERROR_JUMP_BUFFER_CALLED = 5,
    PNGW_ERROR_NULL_ARG = 6,
    PNGW_ERROR_INVALID_DEPTH = 7,
    PNGW_ERROR_INVALID_COLOR = 8,
    PNGW_ERROR_INVALID_DIMENSIONS = 9,
    PNGW_ERROR_COUNT = 10
} pngw_error;

// array of error descriptions, indexable by pngw_error enum values.
extern const char* const const PNGW_ERROR_DESCRIPTIONS[PNGW_ERROR_COUNT];

typedef enum pngw_color
{
    PNGW_COLOR_PALETTE = 0,
    PNGW_COLOR_G = 1,
    PNGW_COLOR_GA = 2,
    PNGW_COLOR_RGB = 3,
    PNGW_COLOR_RGBA = 4,
    PNGW_COLOR_COUNT = 5,
} pngw_color;

// array of color type names, indexable by pngw_color enum values.
extern const char* const const PNGW_COLOR_NAMES[PNGW_COLOR_COUNT];

#define PNGW_DEFAULT_ROW_OFFSET 0

// Get information about a png image file's format. Depth may be 1, 2, 4, 8, or 16. Color may be any type.
pngw_error pngwFileInfo(const char* const path, size_t* const width, size_t* const height, size_t* const depth, pngw_color* const color);

// Get the size of image data in bytes. Depth must be 8 or 16. Color may not be PNGW_COLOR_PALETTE. 
pngw_error pngwDataSize(const size_t width, const size_t height, const size_t depth, const pngw_color color, size_t* const size);

// Read png data from a file into a pixel byte array with the specified format. Data should be allocated before this function is called with enough space to contain the bytes. If the file is of a different format than specified in the arguments, the image will be converted on load. Depth must be 8 or 16. Color may not be PNGW_COLOR_PALETTE. Width and height must match the actual width and height of the image, which you can retrieve with pngwFileInfo() before loading. 
pngw_error pngwReadFile(const char* const path, pngwb_t* const data, const size_t row_offset, const size_t width, const size_t height, const size_t depth, const pngw_color color);

// Save png data to a file from a pixel byte array. The width, height, depth and color must be be the same as the format of data bytes.
pngw_error pngwWriteFile(const char* path, pngwb_t* const data, const size_t row_offset, const size_t width, const size_t height, const size_t depth, const pngw_color color);

// Convert an 8 bit depth RGB color to a grayscale value using libpng's default conversion equation.
pngwb_t pngGrayFromColor8(const pngwb_t r, const pngwb_t g, const pngwb_t b);

// Convert a 16 bit depth RGB color to a grayscale value using libpng's default conversion equation.
pngws_t pngGrayFromColor16(const pngws_t r, const pngws_t g, const pngws_t b);

// Get the libpng color macro of a pngw color type.
int pngwColorToPngColor(const pngw_color color);

// Get the pngw_color enum of a libpng color macro.
pngw_color pngwPngColorToColor(const int png_color);

#ifdef PNGW_IMPLEMENTATION
#ifndef PNGW_IMPLEMENTED
#define PNGW_IMPLEMENTED

#include <stdio.h>

#ifndef PNG_H
#error png.h must be included before png_wrapper.h can be implemented.
#endif

const char* const const PNGW_ERROR_DESCRIPTIONS[PNGW_ERROR_COUNT] =
{
    "no error has occured",
    "file not found at path",
    "failed to create file",
    "out of memory",
    "invalid file signiture",
    "jump buffer called",
    "NULL argument",
    "invalid bit depth",
    "invalid color type",
    "invalid pixel dimensions"
};

const char* const const PNGW_COLOR_NAMES[PNGW_COLOR_COUNT] =
{
    "Palette",
    "G",
    "GA",
    "RGB",
    "RGBA"
};

pngw_error pngwFileInfo(const char* const path, size_t* const width, size_t* const height, size_t* const depth, pngw_color* const color)
{
    if (path == NULL)
    {
        return PNGW_ERROR_NULL_ARG;
    }
    FILE* f = fopen(path, "rb");
    if (f == NULL)
    {
        return PNGW_ERROR_FILE_NOT_FOUND;
    }
    char signiture[8];
    fread(signiture, 1, 8, f);
    if (png_sig_cmp((png_const_bytep)&signiture[0], 0, 8))
    {
        fclose(f);
        return PNGW_ERROR_INVALID_FILE_SIGNITURE;
    }
    png_structp png_ptr;
    png_infop info_ptr;
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fclose(f);
        return PNGW_ERROR_OUT_OF_MEMORY;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fclose(f);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return PNGW_ERROR_OUT_OF_MEMORY;
    }
    png_init_io(png_ptr, f);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    png_uint_32 png_width, png_height;
    int png_width, png_height, png_bit_depth, png_color_type;
    png_get_IHDR(png_ptr, info_ptr, &png_width, &png_height, &png_bit_depth, &png_color_type, NULL, NULL, NULL);
    fclose(f);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    if (width != NULL)
    {
        *width = (size_t)png_width;
    }
    if (height != NULL)
    {
        *height = (size_t)png_height;
    }
    if (depth != NULL)
    {
        *depth = (size_t)png_bit_depth;
    }
    if (color != NULL)
    {
        *color = pngwPngColorToColor(png_color_type);
    }
    return PNGW_ERROR_NONE;
}

pngw_error pngwDataSize(const size_t width, const size_t height, const size_t depth, const pngw_color color, size_t* const size)
{
    if (!(color >= PNGW_COLOR_G && color <= PNGW_COLOR_RGBA))
    {
        return PNGW_ERROR_INVALID_COLOR;
    }
    if (depth != 8 && depth != 16)
    {
        return PNGW_ERROR_INVALID_DEPTH;
    }
    if (width == 0 || height == 0)
    {
        return PNGW_ERROR_INVALID_DIMENSIONS;
    }
    if (size != NULL)
    {
        *size = width * height * (depth / 8) * ((size_t)color);
    }
    return PNGW_ERROR_NONE;
}

pngw_error pngwReadFile(const char* const path, pngwb_t* const data, const size_t row_offset, const size_t width, const size_t height, const size_t depth, const pngw_color color)
{
    if (path == NULL || data == NULL)
    {
        return PNGW_ERROR_NULL_ARG;
    }
    /* Initial arg checks */
    if (!(color >= PNGW_COLOR_G && color <= PNGW_COLOR_RGBA))
    {
        return PNGW_ERROR_INVALID_COLOR;
    }
    if (depth != 8 && depth != 16)
    {
        return PNGW_ERROR_INVALID_DEPTH;
    }
    /* Open file */
    FILE* f = fopen(path, "rb");
    if (f == NULL)
    {
        return PNGW_ERROR_FILE_NOT_FOUND;
    }
    /* Check file signiture */
    char signiture[8];
    fread(signiture, 1, 8, f);
    if (png_sig_cmp((png_const_bytep)&signiture[0], 0, 8))
    {
        fclose(f);
        return PNGW_ERROR_INVALID_FILE_SIGNITURE;
    }
    /* Create libpng structs */
    png_structp png_ptr;
    png_infop info_ptr;
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fclose(f);
        return PNGW_ERROR_OUT_OF_MEMORY;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fclose(f);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return PNGW_ERROR_OUT_OF_MEMORY;
    }
    /* Create jump buffer to handle errors */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fclose(f);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return PNGW_ERROR_JUMP_BUFFER_CALLED;
    }
    /* Get png format from file */
    png_init_io(png_ptr, f);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    png_uint_32 png_width, png_height;
    int png_bit_depth, png_color_type;
    png_get_IHDR(png_ptr, info_ptr, &png_width, &png_height, &png_bit_depth, &png_color_type, NULL, NULL, NULL);
    if (width != (size_t)png_width || (size_t)png_height == 0)
    {
        fclose(f);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return PNGW_ERROR_INVALID_DIMENSIONS;
    }
    int load_png_color_type = pngwColorToPngColor(color);
    int load_png_bit_depth = (int)depth;
    // if alpha channel not wanted, strip it if the image has one.
    if ((png_color_type & PNG_COLOR_MASK_ALPHA) && !(load_png_color_type & PNG_COLOR_MASK_ALPHA))
    {
        png_set_strip_alpha(png_ptr);
    }
    // if expecting an alpha channel and none exists in the image, add a fully opaque alpha value to each pixel
    if ((load_png_color_type & PNG_COLOR_MASK_ALPHA) &&
        (png_color_type == PNG_COLOR_TYPE_GRAY || png_color_type == PNG_COLOR_TYPE_RGB || png_color_type == PNG_COLOR_TYPE_PALETTE))
    {
        png_set_filler(png_ptr, 0xffff, PNG_FILLER_AFTER);
    }
    // if multiple pixels are packed per byte on 8 bit depth, seperate them into seperate bytes cleanly
    if (png_bit_depth < 8 && load_png_bit_depth == 8)
    {
        png_set_packing(png_ptr);
    }
    // if the image has bit depth 16 and 8 is wanted, auto convert it to bit depth 8
    if (png_bit_depth == 16 && load_png_bit_depth == 8)
    {
#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
        png_set_scale_16(png_ptr);
#else
        png_set_strip_16(png_ptr); //if scaling not supported, strip off the excess byte instead
#endif
    }
    // if image has less than 16 bit depth and 16 is wanted, upscale it to 16
    if (png_bit_depth < 16 && load_png_bit_depth == 16)
    {
        png_set_expand_16(png_ptr);
    }
    // if gray and bit depth is less than 8, up it to 8
    if ((png_color_type == PNG_COLOR_TYPE_GRAY || png_color_type == PNG_COLOR_TYPE_GRAY_ALPHA) && png_bit_depth < 8)
    {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    // auto convert palette images into rgb or rgba
    if (png_color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png_ptr);
    }
    // set transparency to full alpha channels
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) != 0)
    {
        png_set_tRNS_to_alpha(png_ptr);
    }
    // set rgb image to gray output or vice versa
    if ((load_png_color_type == PNG_COLOR_TYPE_GRAY || load_png_color_type == PNG_COLOR_TYPE_GRAY_ALPHA) &&
        (png_color_type == PNG_COLOR_TYPE_RGB || png_color_type == PNG_COLOR_TYPE_RGB_ALPHA || png_color_type == PNG_COLOR_TYPE_PALETTE))
    {
        // negative weights causes default calculation to be used  ((6969 * R + 23434 * G + 2365 * B)/32768)
        png_set_rgb_to_gray_fixed(png_ptr, 1, -1.0, -1.0); // error action 1 causes no waring warning if image was not actually gray
    }
    if ((load_png_color_type == PNG_COLOR_TYPE_RGB || load_png_color_type == PNG_COLOR_TYPE_RGBA) &&
        (png_color_type == PNG_COLOR_TYPE_GRAY || png_color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
    {
        png_set_gray_to_rgb(png_ptr);
    }
    int actual_row_offset;
    if (row_offset == PNGW_DEFAULT_ROW_OFFSET)
    {
        actual_row_offset = width * (size_t)color * (depth / 8);
    }
    else
    {
        actual_row_offset = row_offset;
    }
    /* Load the pixels */
    for (size_t y = 0; y < height; y++)
    {
        png_bytep row_start = &data[y * actual_row_offset];
        png_read_row(png_ptr, row_start, NULL);
    }
    /* Cleanup */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(f);
    return PNGW_ERROR_NONE;
}

pngw_error pngwWriteFile(const char* path, pngwb_t* const data, const size_t row_offset, const size_t width, const size_t height, const size_t depth, const pngw_color color)
{
    if (path == NULL || data == NULL)
    {
        return PNGW_ERROR_NULL_ARG;
    }
    /* Create the file */
    FILE* f = fopen(path, "wb");
    if (!f)
    {
        return PNGW_ERROR_FILE_CREATION_FAILURE;
    }
    /* Create libpng structs */
    png_structp png_ptr;
    png_infop info_ptr;
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fclose(f);
        return PNGW_ERROR_OUT_OF_MEMORY;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fclose(f);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return PNGW_ERROR_OUT_OF_MEMORY;
    }
    /* Create jump buffer to handle errors */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fclose(f);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return PNGW_ERROR_JUMP_BUFFER_CALLED;
    }
    const int png_color_type = pngwColorToPngColor(color);
    /* Configure for writing */
    png_init_io(png_ptr, f);
    // Set the compression to a setup that will be good enough. No need for more complicated options in this library.
    png_set_IHDR(png_ptr, info_ptr, (uint32_t)width, (uint32_t)height, (int)depth, png_color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);
    int actual_row_offset;
    if (row_offset == PNGW_DEFAULT_ROW_OFFSET)
    {
        actual_row_offset = width * (size_t)color * (depth / 8);
    }
    else
    {
        actual_row_offset = row_offset;
    }
    for (size_t y = 0; y < height; y++)
    {
        png_bytep row_start = &data[y * actual_row_offset];
        png_write_row(png_ptr, row_start);
    }
    png_write_end(png_ptr, NULL);
    fclose(f);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return PNGW_ERROR_NONE;
}

pngwb_t pngGrayFromColor8(const pngwb_t r, const pngwb_t g, const pngwb_t b)
{
    return ((((6969 * ((uint32_t)r))) + (23434 * ((uint32_t)g)) + (2365 * ((uint32_t)b))) / 32768);
}

pngws_t pngGrayFromColor16(const pngws_t r, const pngws_t g, const pngws_t b)
{
    return ((((6969 * ((uint64_t)r))) + (23434 * ((uint64_t)g)) + (2365 * ((uint64_t)b))) / 32768);
}

int pngwColorToPngColor(const pngw_color color)
{
    switch (color)
    {
        case PNGW_COLOR_PALETTE:
            return PNG_COLOR_TYPE_PALETTE;
        case PNGW_COLOR_G:
            return PNG_COLOR_TYPE_GRAY;
        case PNGW_COLOR_GA:
            return PNG_COLOR_TYPE_GRAY_ALPHA;
        case PNGW_COLOR_RGB:
            return PNG_COLOR_TYPE_RGB;
        case PNGW_COLOR_RGBA:
            return PNG_COLOR_TYPE_RGBA;
        default:
            return PNG_COLOR_TYPE_GRAY;
    }
}

pngw_color pngwPngColorToColor(const int png_color)
{
    switch (png_color)
    {
        case PNG_COLOR_TYPE_PALETTE:
            return PNGW_COLOR_PALETTE;
        case PNG_COLOR_TYPE_GRAY:
            return PNGW_COLOR_G;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            return PNGW_COLOR_GA;
        case PNG_COLOR_TYPE_RGB:
            return PNGW_COLOR_RGB;
        case PNG_COLOR_TYPE_RGBA:
            return PNGW_COLOR_RGBA;
        default:
            return PNGW_COLOR_G;
    }
}

#endif
#endif
#ifdef __cplusplus
}
#endif
#endif
#include "global.h"

#include "lib/png/png.h"
#include "lib/jpeg8/jpeglib.h"

//todo: use int for different return values
bool LoadImageFile(Path filename, unsigned char **rgba_pic, int *width, int *height, bool showexterr)
{
    if(!rgba_pic) return false;

    char *ext = filename.ext();
    if(stricmp(ext,"jpg") && stricmp(ext,"png") && stricmp(ext,"tga"))
    {
        if(showexterr) syserror(const_cast<char *> ("LoadImageFile: File extension not recognized: %s"), (char*)filename);
        return false;
    }
    if(!filename.Exists())
    {
        syserror(const_cast<char *> ("LoadImageFile: File not found: %s"), (char*)filename);
        return false;
    }

    *rgba_pic=0;
    *width=*height=0;
    int filelen = 0;
    unsigned char *buffer=file_get_contents(filename, &filelen);
    if(!buffer)
    {
        syserror(const_cast<char *> ("LoadImageFile: Failed to load image: %s"), (char*)filename);
        return false;
    }

    if(!stricmp(ext,"jpg"))
        LoadJPGBuff(buffer,filelen,rgba_pic,width,height);
    else if(!stricmp(ext,"png"))
        LoadPNGBuff(buffer,rgba_pic,width,height);
    else if(!stricmp(ext,"tga"))
        LoadTGABuff(buffer,filelen,rgba_pic,width,height);
    delete[] buffer;
    return *rgba_pic != 0;
}

//
// TransformImageBytes
//
// change byte orders of 32bit image data
// TODO: this could take src/dest params for byte orders
//

unsigned char*TransformImageBytes(unsigned char*img,int width,int height, int mode, bool conv_inplace)
{
    int c = width*height;
    int *src = reinterpret_cast<int*>(img);
    int *dest = conv_inplace ? src : new int[c];
    if(mode == TMODE_CHANGE_ENDIAN)
    {
        for(int i = 0; i < c; i++)
            dest[i] = SWAPBYTES(src[i]);
    }
    else if(mode == TMODE_ARGB_ABGR)
    {
        for(int i = 0; i < c; i++)
        {
            int t = src[i];
            dest[i] = (t & 0xFF000000) |  ((t&0xFF)<<16) | ((t&0xFF0000)>>16) | ((t&0xFF00));
        }
    }
    return reinterpret_cast<unsigned char*>(dest);
}

/************************************ TGA ************************************/
// originated from fuh source.

#define	IMAGE_MAX_DIMENSIONS	4096

// Definitions for image types
#define TGA_MAPPED		1	// Uncompressed, color-mapped images
#define TGA_MAPPED_RLE	9	// Runlength encoded color-mapped images
#define TGA_RGB			2	// Uncompressed, RGB images
#define TGA_RGB_RLE		10	// Runlength encoded RGB images
#define TGA_MONO		3	// Uncompressed, black and white images
#define TGA_MONO_RLE	11	// Compressed, black and white images

// Custom definitions to simplify code
#define MYTGA_MAPPED	80
#define MYTGA_RGB15		81
#define MYTGA_RGB24		82
#define MYTGA_RGB32		83
#define MYTGA_MONO8		84
#define MYTGA_MONO16	85

typedef struct TGAHeader_s
{
    unsigned char	idLength, colormapType, imageType;
    unsigned short	colormapIndex, colormapLength;
    unsigned char	colormapSize;
    unsigned short	xOrigin, yOrigin, width, height;
    unsigned char	pixelSize, attributes;
} TGAHeader_t;


static void TGA_upsample15(unsigned char *dest, unsigned char *src, bool alpha)
{
    dest[2] = (unsigned char) ((src[0] & 0x1F) << 3);
    dest[1] = (unsigned char) ((((src[1] & 0x03) << 3) + ((src[0] & 0xE0) >> 5)) << 3);
    dest[0] = (unsigned char) (((src[1] & 0x7C) >> 2) << 3);
    dest[3] = (alpha && !(src[1] & 0x80)) ? 0 : 255;
}

static void TGA_upsample24(unsigned char *dest, unsigned char *src)
{
    dest[2] = src[0];
    dest[1] = src[1];
    dest[0] = src[2];
    dest[3] = 255;
}

static void TGA_upsample32(unsigned char *dest, unsigned char *src)
{
    dest[2] = src[0];
    dest[1] = src[1];
    dest[0] = src[2];
    dest[3] = src[3];
}

#define TGA_ERROR(msg)	{if (msg) {syserror(msg);} delete [] buffer; return;}

static unsigned short BuffLittleShort(const unsigned char *buffer)
{
    return (buffer[1] << 8) | buffer[0];
}

void LoadTGABuff(unsigned char *buffer, int buflen, unsigned char **pic, int *width, int *height)
{
    TGAHeader_t header;
    int image_width, image_height;
    int i, x, y, bpp, alphabits, compressed, mytype, row_inc, runlen, readpixelcount;
    unsigned char *in, *out, *data, *enddata, rgba[4], palette[256 * 4];

    *pic = 0;

    header.idLength = buffer[0];
    header.colormapType = buffer[1];
    header.imageType = buffer[2];

    header.colormapIndex = BuffLittleShort(buffer + 3);
    header.colormapLength = BuffLittleShort(buffer + 5);
    header.colormapSize = buffer[7];
    header.xOrigin = BuffLittleShort(buffer + 8);
    header.yOrigin = BuffLittleShort(buffer + 10);
    header.width = image_width = BuffLittleShort(buffer + 12);
    header.height = image_height = BuffLittleShort(buffer + 14);
    header.pixelSize = buffer[16];
    header.attributes = buffer[17];

    if (image_width > IMAGE_MAX_DIMENSIONS || image_height > IMAGE_MAX_DIMENSIONS || image_width <= 0 || image_height <= 0)
        TGA_ERROR(NULL);
    *width = image_width;
    *height = image_height;

    bpp = (header.pixelSize + 7) >> 3;
    alphabits = (header.attributes & 0x0F);
    compressed = (header.imageType & 0x08);

    in = buffer + 18 + header.idLength;
    enddata = buffer + buflen;

    // error check the image type's pixel size
    if (header.imageType == TGA_RGB || header.imageType == TGA_RGB_RLE)
    {
        if (!(header.pixelSize == 15 || header.pixelSize == 16 || header.pixelSize == 24 || header.pixelSize == 32))
            TGA_ERROR(const_cast<char *> ("Unsupported TGA image: Bad pixel size for RGB image\n"));
        mytype = (header.pixelSize == 24) ? MYTGA_RGB24 : (header.pixelSize == 32) ? MYTGA_RGB32 : MYTGA_RGB15;
    }
    else if (header.imageType == TGA_MAPPED || header.imageType == TGA_MAPPED_RLE)
    {
        if (header.pixelSize != 8)
            TGA_ERROR(const_cast<char *> ("Unsupported TGA image: Bad pixel size for color-mapped image.\n"));
        if (!(header.colormapSize == 15 || header.colormapSize == 16 || header.colormapSize == 24 || header.colormapSize == 32))
            TGA_ERROR(const_cast<char *> ("Unsupported TGA image: Bad colormap size.\n"));
        if (header.colormapType != 1 || header.colormapLength * 4 > sizeof(palette))
            TGA_ERROR(const_cast<char *> ("Unsupported TGA image: Bad colormap type and/or length for color-mapped image.\n"));

        // read in the palette
        if (header.colormapSize == 15 || header.colormapSize == 16)
        {
            for (i = 0, out = palette; i < header.colormapLength; i++, in += 2, out += 4)
                TGA_upsample15(out, in, alphabits == 1);
        }
        else if (header.colormapSize == 24)
        {
            for (i = 0, out = palette; i < header.colormapLength; i++, in += 3, out += 4)
                TGA_upsample24(out, in);
        }
        else if (header.colormapSize == 32)
        {
            for (i = 0, out = palette; i < header.colormapLength; i++, in += 4, out += 4)
                TGA_upsample32(out, in);
        }
        mytype = MYTGA_MAPPED;
    }
    else if (header.imageType == TGA_MONO || header.imageType == TGA_MONO_RLE)
    {
        if (!(header.pixelSize == 8 || (header.pixelSize == 16 && alphabits == 8)))
            TGA_ERROR(const_cast<char *> ("Unsupported TGA image: Bad pixel size for grayscale image.\n"));
        mytype = (header.pixelSize == 8) ? MYTGA_MONO8 : MYTGA_MONO16;
    }
    else
    {
        TGA_ERROR(const_cast<char *> ("Unsupported TGA image: Bad image type.\n"));
    }

    if (header.attributes & 0x10)
        TGA_ERROR(const_cast<char *> ("Unsupported TGA image: Pixel data spans right to left.\n"));

    data = new unsigned char[image_width * image_height * 4];
    memset(data,0,image_width * image_height * 4);
    *pic = data;

    // if bit 5 of attributes isn't set, the image has been stored from bottom to top
    if ((header.attributes & 0x20))
    {
        out = data;
        row_inc = 0;
    }
    else
    {
        out = data + (image_height - 1) * image_width * 4;
        row_inc = -image_width * 4 * 2;
    }
    x = y = 0;
    rgba[0] = rgba[1] = rgba[2] = rgba[3] = 255;

    while (y < image_height)
    {
        // decoder is mostly the same whether it's compressed or not
        readpixelcount = runlen = 0x7FFFFFFF;
        if (compressed && in < enddata)
        {
            runlen = *in++;
            // high bit indicates this is an RLE compressed run
            if (runlen & 0x80)
                readpixelcount = 1;
            runlen = 1 + (runlen & 0x7F);
        }

        while (runlen-- && y < image_height)
        {
            if (readpixelcount > 0)
            {
                readpixelcount--;
                rgba[0] = rgba[1] = rgba[2] = rgba[3] = 255;

                if (in + bpp <= enddata)
                {
                    switch(mytype)
                    {
                    case MYTGA_MAPPED:
                        for (i = 0; i < 4; i++)
                            rgba[i] = palette[in[0] * 4 + i];
                        break;
                    case MYTGA_RGB15:
                        TGA_upsample15(rgba, in, alphabits == 1);
                        break;
                    case MYTGA_RGB24:
                        TGA_upsample24(rgba, in);
                        break;
                    case MYTGA_RGB32:
                        TGA_upsample32(rgba, in);
                        break;
                    case MYTGA_MONO8:
                        rgba[0] = rgba[1] = rgba[2] = in[0];
                        break;
                    case MYTGA_MONO16:
                        rgba[0] = rgba[1] = rgba[2] = in[0];
                        rgba[3] = in[1];
                        break;
                    }
                    in += bpp;
                }
            }
            for (i = 0; i < 4; i++)
                *out++ = rgba[i];
            if (++x == image_width)
            {
                // end of line, advance to next
                x = 0;
                y++;
                out += row_inc;
            }
        }
    }
}

/************************************ PNG ************************************/
// originated from fuh source.

struct png_read_charbuf
{
    unsigned char *buffer;
    int cur;
};

static void PNG_IO_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_read_charbuf* src = (png_read_charbuf *) png_get_io_ptr(png_ptr);
    //fread(data, 1, length, f);
    memcpy(data, &src->buffer[src->cur], length);
    src->cur += length;
}

/*
static void PNG_IO_user_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	FILE *f = (FILE *) qpng_get_io_ptr(png_ptr);
	fwrite(data, 1, length, f);
}

static void PNG_IO_user_flush_data(png_structp png_ptr) {
	FILE *f = (FILE *) qpng_get_io_ptr(png_ptr);
	fflush(f);
}
*/

void LoadPNGBuff (unsigned char *buffer, unsigned char **pic, int *width, int *height)
{
    unsigned char **rowpointers;
    png_structp png_ptr;
    png_infop pnginfo;
    int y, bitdepth, colortype, interlace, compression, filter, bytesperpixel;
    unsigned long rowbytes;

    //if (!png_handle)
    //	return NULL;

    if (!buffer || !pic)
        return;
    *pic = 0;

    if (png_sig_cmp(buffer, 0, 8))
    {
        syserror (const_cast<char *> ("Invalid PNG image\n"));
        return;
    }

    if (!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
    {
        return;
    }

    if (!(pnginfo = png_create_info_struct(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &pnginfo, NULL);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &pnginfo, NULL);
        return;
    }

    png_read_charbuf src = {buffer, 8};
    png_set_read_fn(png_ptr,  (png_voidp) &src, PNG_IO_user_read_data);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, pnginfo);
    png_get_IHDR(png_ptr, pnginfo, (png_uint_32 *) width, (png_uint_32 *) height, &bitdepth,
                 &colortype, &interlace, &compression, &filter);

    if (*width > IMAGE_MAX_DIMENSIONS || *height > IMAGE_MAX_DIMENSIONS)
    {
        syserror (const_cast<char *> ("PNG image exceeds maximum supported dimensions\n"));
        png_destroy_read_struct(&png_ptr, &pnginfo, NULL);
        return;
    }

    if (colortype == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png_ptr);
        png_set_filler(png_ptr, 255, PNG_FILLER_AFTER);
    }

    if (colortype == PNG_COLOR_TYPE_GRAY && bitdepth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    if (png_get_valid(png_ptr, pnginfo, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if (colortype == PNG_COLOR_TYPE_GRAY || colortype == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    if (colortype != PNG_COLOR_TYPE_RGBA)
        png_set_filler(png_ptr, 255, PNG_FILLER_AFTER);

    if (bitdepth < 8)
        png_set_expand (png_ptr);
    else if (bitdepth == 16)
        png_set_strip_16(png_ptr);


    png_read_update_info(png_ptr, pnginfo);
    rowbytes = png_get_rowbytes(png_ptr, pnginfo);
    bytesperpixel = png_get_channels(png_ptr, pnginfo);
    bitdepth = png_get_bit_depth(png_ptr, pnginfo);

    if (bitdepth != 8 || bytesperpixel != 4)
    {
        syserror (const_cast<char *> ("Unsupported PNG image: Bad color depth and/or bpp\n"));
        png_destroy_read_struct(&png_ptr, &pnginfo, NULL);
        return;
    }

    *pic = new unsigned char[*height * rowbytes];
    rowpointers = new unsigned char*[*height * sizeof(*rowpointers)];

    for (y = 0; y < *height; y++)
        rowpointers[y] = *pic + y * rowbytes;

    png_read_image(png_ptr, rowpointers);
    png_read_end(png_ptr, NULL);

    png_destroy_read_struct(&png_ptr, &pnginfo, NULL);
    delete[] rowpointers;
}

/*
int Image_WritePNG (char *filename, int compression, byte *pixels, int width, int height) {
	char name[MAX_OSPATH];
	int i, bpp = 3, pngformat, width_sign;
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_byte **rowpointers;
	Q_snprintfz (name, sizeof(name), "%s/%s", com_basedir, filename);

	if (!png_handle)
		return false;

	width_sign = (width < 0) ? -1 : 1;
	width = abs(width);

	if (!(fp = fopen (name, "wb"))) {
		COM_CreatePath (name);
		if (!(fp = fopen (name, "wb")))
			return false;
	}

	if (!(png_ptr = qpng_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
		fclose(fp);
		return false;
	}

	if (!(info_ptr = qpng_create_info_struct(png_ptr))) {
		qpng_destroy_write_struct(&png_ptr, (png_infopp) NULL);
		fclose(fp);
		return false;
	}

	if (setjmp(png_ptr->jmpbuf)) {
		qpng_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

    qpng_set_write_fn(png_ptr, fp, PNG_IO_user_write_data, PNG_IO_user_flush_data);
	qpng_set_compression_level(png_ptr, bound(Z_NO_COMPRESSION, compression, Z_BEST_COMPRESSION));

	pngformat = (bpp == 4) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
	qpng_set_IHDR(png_ptr, info_ptr, width, height, 8, pngformat,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	qpng_write_info(png_ptr, info_ptr);

	rowpointers = Q_Malloc (height * sizeof(*rowpointers));
	for (i = 0; i < height; i++)
		rowpointers[i] = pixels + i * width_sign * width * bpp;
	qpng_write_image(png_ptr, rowpointers);
	qpng_write_end(png_ptr, info_ptr);
	free(rowpointers);
	qpng_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	return true;
}
*/

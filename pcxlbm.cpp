#include "global.hpp"

//////////////////////////////
/*
==============
Load256Image

Will load either an lbm or pcx, depending on extension.
Any of the return pointers can be NULL if you don't want them.
==============
*/
void Load256Image(char *name, unsigned char **pixels, unsigned char**palette, int *width, int *height)
{
	pcx_t	*pcx;
	int		x, y;
	int		len;
	int		dataByte, runLength;
	unsigned char	*out, *pix;
	unsigned char **pic;
	pic = pixels;

	if (!pic || !palette || !width || !height)
		return;
		
	*pic = NULL;
	*palette = NULL;
	*width = 0;
	*height = 0;

	//
	// load the file
	//
	pcx = (pcx_t*) file_get_contents(name, &len);

	//
	// parse the PCX file
	//
	unsigned char *raw = &pcx->data;

	pcx->xmin = pcx->xmin;
	pcx->ymin = pcx->ymin;
	pcx->xmax = pcx->xmax;
	pcx->ymax = pcx->ymax;
	pcx->hres = pcx->hres;
	pcx->vres = pcx->vres;
	pcx->bytes_per_line = pcx->bytes_per_line;
	pcx->palette_type = pcx->palette_type;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
		Error ("Bad pcx file %s", name);

	*palette = (unsigned char *)malloc(768);
	memcpy (*palette, (byte *)pcx + len - 768, 768);
	
	*width = pcx->xmax+1;
	*height = pcx->ymax+1;

	out = (unsigned char *)malloc ( (pcx->ymax+1) * (pcx->xmax+1) );
	if (!out)
		Error ("Skin_Cache: couldn't allocate");

	*pic = out;

	pix = out;

	for (y=0 ; y<=pcx->ymax ; y++, pix += pcx->xmax+1) {
		for (x=0 ; x<=pcx->xmax ; ) {
			dataByte = *raw++;
			
			runLength = 1;
			if((dataByte & 0xC0) == 0xC0) {
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}

			while(runLength-- > 0)
				pix[x++] = (unsigned char)dataByte;
		}
	}

	if ( raw - (byte *)pcx > len)
		Error ("PCX file %s was malformed", name);

	delete[] pcx;
}


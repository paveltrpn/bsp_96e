
//jpg
extern void LoadJPGBuff(unsigned char *fbuffer, int buflen, unsigned char **pic, int *width, int *height);

//load tga
extern void LoadTGABuff(unsigned char *buffer, int buflen, unsigned char **pic, int *width, int *height);

//load png
extern void LoadPNGBuff (unsigned char *buffer, unsigned char **pic, int *width, int *height);

//load known image formats from file
extern bool LoadImageFile(Path filename, unsigned char **rgba_pic, int *width, int *height, bool showexterr);

//swap endian of image bytes
#define TMODE_CHANGE_ENDIAN	0
#define TMODE_ARGB_ABGR 1
extern unsigned char*TransformImageBytes(unsigned char*img,int width,int height, int mode, bool conv_inplace);

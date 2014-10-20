/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000,2008 Silicon Motion, Inc.  All Rights Reserved.
Copyright (C) 2001 Corvin Zahn.  All Rights Reserved.
Copyright (C) 2008 Mandriva Linux.  All Rights Reserved.
Copyright (c) 2012 by Silicon Motion, Inc. (SMI)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the XFree86 Project and
Silicon Motion shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization from the XFree86 Project and silicon Motion.
*/


/*
  Corvin Zahn <zahn@zac.de>	Date:   2.11.2001
    - SAA7111 support
    - supports attributes: XV_ENCODING, XV_BRIGHTNESS, XV_CONTRAST,
      XV_SATURATION, XV_HUE, XV_COLORKEY, XV_INTERLACED
      XV_CAPTURE_BRIGHTNESS can be used to set brightness in the capture device
    - bug fixes
    - tries not to use acceleration functions
    - interlaced video for double vertical resolution
	XV_INTERLACED = 0: only one field of an interlaced video signal is
			   displayed:
			-> half vertical resolution, but no comb like artifacts
			   from moving vertical edges
	XV_INTERLACED = 1: both fields of an interlaced video signal are
			   displayed:
			-> full vertical resolution, but comb like artifacts from
			   moving vertical edges
	The default value can be set with the driver option Interlaced
*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include<sys/ioctl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<unistd.h>
#include<ctype.h>
#include<fcntl.h>
#include<sys/time.h>
#include<sys/select.h>

#ifdef __GLIBC__
#include<asm/types.h>
#endif
#include<sys/time.h>

#include	"smi_common.h"
#include	"smi_video.h"
#include 	"smi_dbg.h"
#include "xf86Crtc.h"

/*

   new attribute:

   XV_INTERLACED = 0: only one field of an interlaced video signal is displayed:
   -> half vertical resolution, but no comb like artifacts from
   moving vertical edges
   XV_INTERLACED = 1: both fields of an interlaced video signal are displayed:
   -> full vertical resolution, but comb like artifacts from
   moving vertical edges

   The default value can be set with the driver option Interlaced

 */


#define DWORD   unsigned int

#undef MIN
#undef ABS
#undef CLAMP
#undef ENTRIES

#define MIN(a, b) (((a) < (b)) ? (a) : (b)) 
#define ABS(n) (((n) < 0) ? -(n) : (n))
#define CLAMP(v, min, max) (((v) < (min)) ? (min) : MIN(v, max))

#define ENTRIES(array) (sizeof(array) / sizeof((array)[0]))
#define nElems(x)		(sizeof(x) / sizeof(x[0]))

#define MAKE_ATOM(a)	MakeAtom(a, sizeof(a) - 1, TRUE)

#if  SMI_USE_VIDEO
#include "dixstruct.h"

unsigned int	total_video_memory_k = 0;

static int SMI_AddEncoding(XF86VideoEncodingPtr enc, int i,
        int norm, int input, int channel);
static void SMI_BuildEncodings(SMI_PortPtr p);

static XF86VideoAdaptorPtr SMI_SetupVideo(ScreenPtr pScreen);
void SMI_ResetVideo(ScrnInfoPtr pScrn);

static void SMI_StopVideo(ScrnInfoPtr pScrn, pointer data, Bool shutdown);
static int SMI_SetPortAttribute(ScrnInfoPtr pScrn, Atom attribute,
        INT32 value, pointer data);
static int SMI_GetPortAttribute(ScrnInfoPtr pScrn, Atom attribute,
        INT32 *value, pointer data);
static void SMI_QueryBestSize(ScrnInfoPtr pScrn, Bool motion,
        short vid_w, short vid_h, short drw_w, short drw_h,
        unsigned int *p_w, unsigned int *p_h, pointer data);
static int SMI_PutImage_new(ScrnInfoPtr pScrn,
        short src_x, short src_y, short drw_x, short drw_y,
        short src_w, short src_h, short drw_w, short drw_h,
        int id, unsigned char *buf, short width, short height, Bool sync,
        RegionPtr clipBoxes, pointer data, DrawablePtr pDraw);
static int SMI_PutImage_old(ScrnInfoPtr pScrn,
        short src_x, short src_y, short drw_x, short drw_y,
        short src_w, short src_h, short drw_w, short drw_h,
        int id, unsigned char *buf, short width, short height, Bool sync,
        RegionPtr clipBoxes, pointer data, DrawablePtr pDraw);
static int SMI_QueryImageAttributes(ScrnInfoPtr pScrn,
        int id, unsigned short *width, unsigned short *height,
        int *picthes, int *offsets);
static void SMI_ClipNotify(ScrnInfoPtr pScrn, pointer data,
                                   WindowPtr window, int dx, int dy);

static Bool SMI_ClipVideo(ScrnInfoPtr pScrn, BoxPtr dst,
        INT32 *x1, INT32 *y1, INT32 *x2, INT32 *y2,
        RegionPtr reg, INT32 width, INT32 height);
static void SMI_DisplayVideo(ScrnInfoPtr pScrn, int id, int offset,
        short width, short height, int pitch, int x1, int y1, int x2, int y2,
        BoxPtr dstBox, short vid_w, short vid_h, short drw_w, short drw_h);
static void SMI_DisplayVideo0501(ScrnInfoPtr pScrn, int id, int offset,
        short width, short height, int pitch, int x1, int y1, int x2, int y2,
        BoxPtr dstBox, short vid_w, short vid_h, short drw_w, short drw_h);
static void SMI_DisplayVideo0750(ScrnInfoPtr pScrn, int id, int offset,
        short width, short height, int pitch, int x1, int y1, int x2, int y2,
        BoxPtr dstBox, short vid_w, short vid_h, short drw_w, short drw_h
#if (SMI_RANDR==1)
	,  xf86CrtcPtr crtc
#endif	
);
//mill add for CSC
static void SMI_CSC_Start(SMIPtr pSmi, CARD32 CSC_Control);
static void SMI_DisplayVideo0501_CSC(ScrnInfoPtr pScrn, int id, int offset,
        short width, short height, int pitch, int x1, int y1, int x2, int y2,
        BoxPtr dstBox, short vid_w, short vid_h, short drw_w, short drw_h, RegionPtr clipboxes
#if (SMI_RANDR == 1)
        , xf86CrtcPtr crtc
#endif        
        );
static void SMI_DisplayVideo0750_CSC(ScrnInfoPtr pScrn, int id, int offset,
        short width, short height, int pitch, int x1, int y1, int x2, int y2,
        BoxPtr dstBox, short vid_w, short vid_h, short drw_w, short drw_h, RegionPtr clipboxes);
static void SMI_BlockHandler(BLOCKHANDLER_ARGS_DECL);

static void SMI_InitOffscreenImages(ScreenPtr pScreen);
static FBAreaPtr SMI_AllocateMemory(ScrnInfoPtr pScrn, FBAreaPtr area,
        int numLines);

static uint32_t
CopyYV12Planar(unsigned char *src1, unsigned char *src2,
		   unsigned char *src3, unsigned char *dst,
		   int src1Pitch, int src23Pitch, int dstPitch,
		   int height, int width, Rotation rot);
int
SMI_AllocateMemory_1(ScrnInfoPtr pScrn, void **mem_struct, int size);

static uint32_t
CopyYV12ToPacked(unsigned char *src1, unsigned char *src2,
                    unsigned char *src3, unsigned char *dst,
                    int src1Pitch, int src23Pitch, int dstPitch,
                    int height, int width, Rotation rot);

static void CopyYV12ToVideoMem(unsigned char* src1,
								unsigned char* src2,
								unsigned char* src3,
							    unsigned char*dst,
								int	src1Pitch,
								int	src23Pitch,
								int dstPitch,
						        int	height,int width);

static int SMI_AllocSurface(ScrnInfoPtr pScrn,
        int id, unsigned short width, unsigned short height,
        XF86SurfacePtr surface);
static int SMI_FreeSurface(XF86SurfacePtr surface);
static int SMI_DisplaySurface(XF86SurfacePtr surface,
        short vid_x, short vid_y, short drw_x, short drw_y,
        short vid_w, short vid_h, short drw_w, short drw_h,
        RegionPtr clipBoxes);
static int SMI_StopSurface(XF86SurfacePtr surface);
static int SMI_GetSurfaceAttribute(ScrnInfoPtr pScrn, Atom attr, INT32 *value);
static int SMI_SetSurfaceAttribute(ScrnInfoPtr pScrn, Atom attr, INT32 value);

static int SetAttr(ScrnInfoPtr pScrn, int i, int value);
static int SetAttrSAA7110(ScrnInfoPtr pScrn, int i, int value);
static int SetAttrSAA7111(ScrnInfoPtr pScrn, int i, int value);
static void SetKeyReg(SMIPtr pSmi, int reg, int value);

//void DiableOverlay(SMIPtr pSmi);
#ifdef XF86_VERSION_NUMERIC
#if XF86_VERSION_CURRENT < XF86_VERSION_NUMERIC(4,3,99,0,0)
static Bool RegionsEqual(RegionPtr A, RegionPtr B);
#endif
#endif

/**
 * Atoms
 */

static Atom xvColorKey;
static Atom xvEncoding;
static Atom xvBrightness,xvCapBrightness, xvContrast, xvSaturation, xvHue;
static Atom xvInterlaced;

extern int entity_priv_index[MAX_ENTITIES];

/******************************************************************************\
 **																			  **
 **                           C A P A B I L I T I E S                          **
 **																			  **
 \******************************************************************************/


/**************************************************************************/
/* input channels */

#define N_COMPOSITE_CHANNELS 4
#define N_SVIDEO_CHANNELS 2

#define N_VIDEO_INPUTS 2
typedef enum _VideoInput { VID_COMPOSITE, VID_SVIDEO } VideoInput;


/**************************************************************************/
/* video input formats */

typedef struct _VideoInputDataRec {
    char* name;
} VideoInputDataRec;

static VideoInputDataRec VideoInputs[] = {
    { "composite" },
    { "svideo" }
};


/**************************************************************************/
/* video norms */

#define N_VIDEO_NORMS 3
typedef enum _VideoNorm { PAL, NTSC, SECAM } VideoNorm;

typedef struct _VideoNormDataRec {
    char* name;
    unsigned long Wt;
    unsigned long Wa;
    unsigned long Ht;
    unsigned long Ha;
    unsigned long HStart;
    unsigned long VStart;
    XvRationalRec rate;
} VideoNormDataRec;


static VideoNormDataRec VideoNorms[] =
{
    /* PAL-BDGHI */
    {"pal", 864, 704, 625, 576, 16, 16, { 1, 50 }},
    /* NTSC */
    {"ntsc", 858, 704, 525, 480, 21, 8, { 1001, 60000 }},
    /* SECAM (not tested) */
    {"secam", 864, 7040, 625, 576, 31, 16, { 1, 50 }},
};


/**************************************************************************/
/* number of (generated) XV_ENCODING vaulues */
#define N_ENCODINGS ((N_VIDEO_NORMS) * (N_COMPOSITE_CHANNELS + N_SVIDEO_CHANNELS))


/**************************************************************************/

static XF86VideoFormatRec SMI_VideoFormats[] =
{
#if 0
    { 15, TrueColor },					/* depth, class				*/
#endif
    { 16, TrueColor },					/* depth, class				*/
    { 24, TrueColor },					/* depth, class				*/
};


/**************************************************************************/

/**
 * Attributes
 */

#define XV_ENCODING_NAME        "XV_ENCODING"
#define XV_BRIGHTNESS_NAME      "XV_BRIGHTNESS"
#define XV_CAPTURE_BRIGHTNESS_NAME      "XV_CAPTURE_BRIGHTNESS"
#define XV_CONTRAST_NAME        "XV_CONTRAST"
#define XV_SATURATION_NAME      "XV_SATURATION"
#define XV_HUE_NAME             "XV_HUE"
#define XV_COLORKEY_NAME        "XV_COLORKEY"
#define XV_INTERLACED_NAME      "XV_INTERLACED"


/* fixed order! */
static XF86AttributeRec SMI_VideoAttributesSAA711x[N_ATTRS] = {
    {XvSettable | XvGettable,        0, N_ENCODINGS-1, XV_ENCODING_NAME},
    {XvSettable | XvGettable,        0,           255, XV_BRIGHTNESS_NAME},
    {XvSettable | XvGettable,        0,           255, XV_CAPTURE_BRIGHTNESS_NAME},
    {XvSettable | XvGettable,        0,           127, XV_CONTRAST_NAME},
    {XvSettable | XvGettable,        0,           127, XV_SATURATION_NAME},
    {XvSettable | XvGettable,     -128,           127, XV_HUE_NAME},
    {XvSettable | XvGettable, 0x000000,      0xFFFFFF, XV_COLORKEY_NAME},
    {XvSettable | XvGettable,        0,             1, XV_INTERLACED_NAME},
};

static XF86AttributeRec SMI_VideoAttributes[] = {
    {XvSettable | XvGettable,        0,           255, XV_BRIGHTNESS_NAME},
    {XvSettable | XvGettable, 0x000000,      0xFFFFFF, XV_COLORKEY_NAME},
};
/* monk: remove color key attribute */
static XF86AttributeRec SMI_VideoAttributes_CSC[] = {
    {XvSettable | XvGettable,        0,           255, XV_BRIGHTNESS_NAME},
};

/**************************************************************************/
static XF86ImageRec SMI_VideoImages[] =
{
    XVIMAGE_YUY2,
    XVIMAGE_YV12,
    XVIMAGE_I420,
    {
        FOURCC_RV15,					/* id						*/
        XvRGB,							/* type						*/
        LSBFirst,						/* byte_order				*/
        { 'R', 'V' ,'1', '5',
            0x00, '5',  0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00 },		/* guid						*/
        16,								/* bits_per_pixel			*/
        XvPacked,						/* format					*/
        1,								/* num_planes				*/
        15,								/* depth					*/
        0x001F, 0x03E0, 0x7C00,			/* red_mask, green, blue	*/
        0, 0, 0,						/* y_sample_bits, u, v		*/
        0, 0, 0,						/* horz_y_period, u, v		*/
        0, 0, 0,						/* vert_y_period, u, v		*/
        { 'R', 'V', 'B' },				/* component_order			*/
        XvTopToBottom					/* scaline_order			*/
    },
    {
        FOURCC_RV16,					/* id						*/
        XvRGB,							/* type						*/
        LSBFirst,						/* byte_order				*/
        { 'R', 'V' ,'1', '6',
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00 },		/* guid						*/
        16,								/* bits_per_pixel			*/
        XvPacked,						/* format					*/
        1,								/* num_planes				*/
        16,								/* depth					*/
        0x001F, 0x07E0, 0xF800,			/* red_mask, green, blue	*/
        0, 0, 0,						/* y_sample_bits, u, v		*/
        0, 0, 0,						/* horz_y_period, u, v		*/
        0, 0, 0,						/* vert_y_period, u, v		*/
        { 'R', 'V', 'B' },				/* component_order			*/
        XvTopToBottom					/* scaline_order			*/
    },
    {
        FOURCC_RV24,					/* id						*/
        XvRGB,							/* type						*/
        LSBFirst,						/* byte_order				*/
        { 'R', 'V' ,'2', '4',
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00 },		/* guid						*/
        24,								/* bits_per_pixel			*/
        XvPacked,						/* format					*/
        1,								/* num_planes				*/
        24,								/* depth					*/
        0x0000FF, 0x00FF00, 0xFF0000,	/* red_mask, green, blue	*/
        0, 0, 0,						/* y_sample_bits, u, v		*/
        0, 0, 0,						/* horz_y_period, u, v		*/
        0, 0, 0,						/* vert_y_period, u, v		*/
        { 'R', 'V', 'B' },				/* component_order			*/
        XvTopToBottom					/* scaline_order			*/
    },
    {
        FOURCC_RV32,					/* id						*/
        XvRGB,							/* type						*/
        LSBFirst,						/* byte_order				*/
        { 'R', 'V' ,'3', '2',
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00 },		/* guid						*/
        32,								/* bits_per_pixel			*/
        XvPacked,						/* format					*/
        1,								/* num_planes				*/
        24,								/* depth					*/
        0x0000FF, 0x00FF00, 0xFF0000,	/* red_mask, green, blue	*/
        0, 0, 0,						/* y_sample_bits, u, v		*/
        0, 0, 0,						/* horz_y_period, u, v		*/
        0, 0, 0,						/* vert_y_period, u, v		*/
        { 'R', 'V', 'B' },				/* component_order			*/
        XvTopToBottom					/* scaline_order			*/
    },
};

/**************************************************************************/
static XF86ImageRec SMI501_VideoImages[] =
{
    XVIMAGE_YUY2,
    XVIMAGE_YV12,
    XVIMAGE_I420,
    {
        FOURCC_RV16,					/* id						*/
        XvRGB,							/* type						*/
        LSBFirst,						/* byte_order				*/
        { 'R', 'V' ,'1', '6',
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00 },		/* guid						*/
        16,								/* bits_per_pixel			*/
        XvPacked,						/* format					*/
        1,								/* num_planes				*/
        16,								/* depth					*/
        0x001F, 0x07E0, 0xF800,			/* red_mask, green, blue	*/
        0, 0, 0,						/* y_sample_bits, u, v		*/
        0, 0, 0,						/* horz_y_period, u, v		*/
        0, 0, 0,						/* vert_y_period, u, v		*/
        { 'R', 'V', 'B' },				/* component_order			*/
        XvTopToBottom					/* scaline_order			*/
    },
    {
        FOURCC_RV32,					/* id						*/
        XvRGB,							/* type						*/
        LSBFirst,						/* byte_order				*/
        { 'R', 'V' ,'3', '2',
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00 },		/* guid						*/
        32,								/* bits_per_pixel			*/
        XvPacked,						/* format					*/
        1,								/* num_planes				*/
        24,								/* depth					*/
        0x0000FF, 0x00FF00, 0xFF0000,	/* red_mask, green, blue	*/
        0, 0, 0,						/* y_sample_bits, u, v		*/
        0, 0, 0,						/* horz_y_period, u, v		*/
        0, 0, 0,						/* vert_y_period, u, v		*/
        { 'R', 'V', 'B' },				/* component_order			*/
        XvTopToBottom					/* scaline_order			*/
    },
};





/**************************************************************************/

/**
 * SAA7111 video decoder register values
 */


/** SAA7111 control sequences for selecting one out of four
  composite input channels */
static I2CByte SAA7111CompositeChannelSelect[N_COMPOSITE_CHANNELS][4] = {
    { 0x02, 0xC0, 0x09, 0x4A}, /* CVBS AI11 */
    { 0x02, 0xC1, 0x09, 0x4A}, /* CVBS AI12 */
    { 0x02, 0xC2, 0x09, 0x4A}, /* CVBS AI21 */
    { 0x02, 0xC3, 0x09, 0x4A}, /* CVBS AI22 */
};


/** SAA7111 control sequences for selecting one out of two
  s-video input channels */
static I2CByte SAA7111SVideoChannelSelect[N_SVIDEO_CHANNELS][4] = {
    { 0x02, 0xC6, 0x09, 0xCA}, /* Y/C AI11/AI21 */
    { 0x02, 0xC7, 0x09, 0xCA}, /* Y/C AI12/AI22 */
};


/** SAA7111 control sequences for selecting one out of three
  video norms */
static I2CByte SAA7111VideoStd[3][8] = {
    {0x06, 108, 0x07, 108, 0x08, 0x09, 0x0E, 0x01}, /* PAL */
    {0x06, 107, 0x07, 107, 0x08, 0x49, 0x0E, 0x01}, /* NTSC */
    {0x06, 108, 0x07, 108, 0x08, 0x01, 0x0E, 0x51}  /* SECAM */
};

static I2CByte SAA7111InitData[] =
{
    0x11, 0x1D, /* 0D D0=1: automatic colour killer off
                   D1=0: DMSD data to YUV output
                   D2=1: output enable H/V sync on
                   D3=1: output enable YUV data on */
    0x02, 0xC0, /* Mode 0 */
    0x03, 0x23, /* automatic gain */
    0x04, 0x00, /*  */
    0x05, 0x00, /*  */
    0x06, 108,  /* hor sync begin */
    0x07, 108,  /* hor sync stop */
    0x08, 0x88, /* sync control:
                   D1-0=00: VNOI = normal mode
                   D2=0: PLL closed
                   D3=1: VTR mode
                   D7=1: automatic field detection */
    0x09, 0x41, /* 4A luminance control */
    0x0A, 0x80, /* brightness = 128 (CCIR level) */
    0x0B, 0x40, /* contrast = 1.0 */
    0x0C, 0x40, /* crominance = 1.0 (CCIR level) */
    0x0D, 0x00, /* hue = 0 */
    0x0E, 0x01, /* chroma bandwidth = nominal
                   fast colour time constant = nominal
                   chrom comp filter on
                   colour standard PAL BGHI, NTSC M */
    0x10, 0x48, /* luminance delay compensation = 0
                   VRLN = 1
                   fine pos of hs = 0
                   output format = YUV 422 */
    0x12, 0x00, /* 20 D5=1: VPO in tristate */
    0x13, 0x00,
    0x15, 0x00,
    0x16, 0x00,
    0x17, 0x00,

};

/*
int fd_vgxdma0;//vgxdma file description
static void * kbuffer;
static unsigned long phybuffer;
*/

/**************************************************************************/

/**
 * generates XF86VideoEncoding[i] with video norm norm, video input format
 * input and video input channel channel
 */
    static int
SMI_AddEncoding(XF86VideoEncodingPtr enc, int i,
        int norm, int input, int channel)
{
    char* norm_string;
    char* input_string;
    char channel_string[20];
    ENTER();

    norm_string = VideoNorms[norm].name;
    input_string = VideoInputs[input].name;
    sprintf(channel_string, "%d", channel);
    enc[i].id     = i;
    enc[i].name   = xalloc(strlen(norm_string) + 
            strlen(input_string) + 
            strlen(channel_string)+3);
    if (NULL == enc[i].name) {
	LEAVE(-1);
    }
    enc[i].width  = VideoNorms[norm].Wa;
    enc[i].height = VideoNorms[norm].Ha;
    enc[i].rate   = VideoNorms[norm].rate;
    sprintf(enc[i].name,"%s-%s-%s", norm_string, input_string, channel_string);

    LEAVE(0);
}


/**
 * builds XF86VideoEncodings with all legal combinations of video norm,
 * video input format and video input channel
 */
    static void
SMI_BuildEncodings(SMI_PortPtr p)
{
    int ch, n;
	ENTER();
    /* allocate memory for encoding array */
    p->enc = xalloc(sizeof(XF86VideoEncodingRec) * N_ENCODINGS);
    if (NULL == p->enc)
        goto fail;
    memset(p->enc,0,sizeof(XF86VideoEncodingRec) * N_ENCODINGS);
    /* allocate memory for video norm array */
    p->norm = xalloc(sizeof(int) * N_ENCODINGS);
    if (NULL == p->norm)
        goto fail;
    memset(p->norm,0,sizeof(int) * N_ENCODINGS);
    /* allocate memory for video input format array */
    p->input = xalloc(sizeof(int) * N_ENCODINGS);
    if (NULL == p->input)
        goto fail;
    memset(p->input,0,sizeof(int) * N_ENCODINGS);
    /* allocate memory for video channel number array */
    p->channel = xalloc(sizeof(int) * N_ENCODINGS);
    if (NULL == p->channel)
        goto fail;
    memset(p->channel,0,sizeof(int) * N_ENCODINGS);

    /* fill arrays */
    p->nenc = 0;
    for (ch = 0; ch < N_COMPOSITE_CHANNELS; ch++) {
        for (n = 0; n < N_VIDEO_NORMS; n++) {
            SMI_AddEncoding(p->enc, p->nenc, n, VID_COMPOSITE, ch);
            p->norm[p->nenc]  = n;
            p->input[p->nenc] = VID_COMPOSITE;
            p->channel[p->nenc] = ch;
            p->nenc++;
        }
    }
    for (ch = 0; ch < N_SVIDEO_CHANNELS; ch++) {
        for (n = 0; n < N_VIDEO_NORMS; n++) {
            SMI_AddEncoding(p->enc, p->nenc, n, VID_SVIDEO, ch);
            p->norm[p->nenc]  = n;
            p->input[p->nenc] = VID_SVIDEO;
            p->channel[p->nenc] = ch;
            p->nenc++;
        }
    }
    LEAVE();

fail:
    if (p->input) xfree(p->input);
    p->input = NULL;
    if (p->norm) xfree(p->norm);
    p->norm = NULL;
    if (p->channel) xfree(p->channel);
    p->channel = NULL;
    if (p->enc) xfree(p->enc);
    p->enc = NULL;
    p->nenc = 0;
	LEAVE();
}

/******************************************************************************\
 **                                                                            **
 **                  X V E X T E N S I O N   I N T E R F A C E                 **
 **                                                                            **
 \******************************************************************************/

Bool
SMI_Videoinit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SMIPtr pSmi = SMIPTR(pScrn);
    XF86VideoAdaptorPtr *ptrAdaptors = NULL, *newAdaptors = NULL;
    XF86VideoAdaptorPtr newAdaptor = NULL;
    int numAdaptors; 
	
    ENTER();

    int refByes = total_video_memory_k >> 1;	
    if(pSmi->pHardware->dual > 1)
 		refByes >>= 1;
		
	numAdaptors = xf86XVListGenericAdaptors(pScrn, &ptrAdaptors);
	newAdaptor = SMI_SetupVideo(pScreen);
	DEBUG("newAdaptor=%p, numAdaptors[%d]\n", newAdaptor, numAdaptors);
	SMI_InitOffscreenImages(pScreen);

    if(newAdaptor != NULL)
    {
        if(numAdaptors == 0)
        {
            numAdaptors = 1;
            ptrAdaptors = &newAdaptor;
        }
        else
        {
            newAdaptors = xalloc((numAdaptors + 1) *
                    sizeof(XF86VideoAdaptorPtr*));
            memcpy(newAdaptors, ptrAdaptors,
                        numAdaptors * sizeof(XF86VideoAdaptorPtr));
	        newAdaptors[numAdaptors++] = newAdaptor;
	        ptrAdaptors = newAdaptors;
        }
    }

    if (numAdaptors != 0)
    {
        xf86XVScreenInit(pScreen, ptrAdaptors, numAdaptors);
    }

    if (newAdaptors != NULL)
    {
        xfree(newAdaptors);
    }
    LEAVE(TRUE);
}


/*************************************************************************/

/**
 * sets video decoder attributes channel, encoding, brightness, contrast, saturation, hue
 */
    static int
SetAttr(ScrnInfoPtr pScrn, int i, int value)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    SMI_PortPtr pPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;

	ENTER();
    if (i < XV_ENCODING || i > XV_HUE)
        LEAVE (BadMatch);

    /* clamps value to attribute range */
    value = CLAMP(value, SMI_VideoAttributes[i].min_value,
            SMI_VideoAttributes[i].max_value);

    if (i == XV_BRIGHTNESS) {
        int my_value = (value <= 128? value + 128 : value - 128);
        SetKeyReg(pSmi, 0x5C, 0xEDEDED | (my_value << 24));
    } else if (pPort->I2CDev.SlaveAddr == SAA7110) {
        LEAVE(SetAttrSAA7110(pScrn, i, value));
    }
    else if (pPort->I2CDev.SlaveAddr == SAA7111) {
        LEAVE (SetAttrSAA7111(pScrn, i, value));
    }
#if 0
    else {
        return XvBadAlloc;
    }
#endif

    LEAVE (Success);
}


/**
 * sets SAA7110 video decoder attributes channel, encoding, brightness, contrast, saturation, hue
 */
    static int
SetAttrSAA7110(ScrnInfoPtr pScrn, int i, int value)
{
    ENTER();
    /* not supported */
    LEAVE( XvBadAlloc);
}


/**
 * sets SAA7111 video decoder attributes channel, encoding,
 * brightness, contrast, saturation, hue
 */
    static int
SetAttrSAA7111(ScrnInfoPtr pScrn, int i, int value)
{
    ENTER();
    SMIPtr pSmi = SMIPTR(pScrn);
    SMI_PortPtr pPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;

    if (i == XV_ENCODING) {
        int norm;
        int input;
        int channel;
        norm = pPort->norm[value];
        input = pPort->input[value];
        channel = pPort->channel[value];

	DEBUG("SetAttribute XV_ENCODING: %d. norm=%d input=%d channel=%d\n",
	       value, norm, input, channel);

        /* set video norm */
        if (!xf86I2CWriteVec(&(pPort->I2CDev), SAA7111VideoStd[norm],
                    ENTRIES(SAA7111VideoStd[norm]) / 2)) {
            LEAVE( XvBadAlloc);
        }
        /* set video input format and channel */
        if (input == VID_COMPOSITE) {
            if (!xf86I2CWriteVec(&(pPort->I2CDev),
                        SAA7111CompositeChannelSelect[channel],
                        ENTRIES(SAA7111CompositeChannelSelect[channel]) / 2)) {
                LEAVE( XvBadAlloc);
            }
        }
        else {
            if (!xf86I2CWriteVec(&(pPort->I2CDev),
                        SAA7111SVideoChannelSelect[channel],
                        ENTRIES(SAA7111SVideoChannelSelect[channel]) / 2)) {
                LEAVE( XvBadAlloc);
            }
        }
    }
    else if (i >= XV_CAPTURE_BRIGHTNESS && i <= XV_HUE) {
        int slave_adr = 0;

        switch (i) {
	case XV_CAPTURE_BRIGHTNESS:
	    DEBUG("SetAttribute XV_BRIGHTNESS: %d\n", value);
	    slave_adr = 0x0a;
	    break;
		
	case XV_CONTRAST:
	    DEBUG("SetAttribute XV_CONTRAST: %d\n", value);
	    slave_adr = 0x0b;
	    break;

	case XV_SATURATION:
	    DEBUG("SetAttribute XV_SATURATION: %d\n", value);
	    slave_adr = 0x0c;
	    break;

	case XV_HUE:
	    DEBUG("SetAttribute XV_HUE: %d\n", value);
	    slave_adr = 0x0d;
	    break;
        default:
            LEAVE( XvBadAlloc);
        }
        if (!xf86I2CWriteByte(&(pPort->I2CDev), slave_adr, (value & 0xff)))
            LEAVE( XvBadAlloc);
    }
    else {
        LEAVE( BadMatch);
    }

    /* debug: show registers */
    {
	I2CByte i2c_bytes[32];
	int i;
	xf86I2CReadBytes(&(pPort->I2CDev), 0, i2c_bytes, 32);
	DEBUG("SAA7111 Registers\n");
	for (i=0; i<32; i++) {
	    DEBUG("%02X=%02X ", i, i2c_bytes[i]);
	    if ((i&7) == 7) DEBUG("\n");
	}
    }

    LEAVE( Success);
}


/******************************************************************************\
 **																			  **
 **						 V I D E O   M A N A G E M E N T					  **
 **																			  **
 \******************************************************************************/

static XF86VideoAdaptorPtr
SMI_SetupVideo(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SMIPtr pSmi = SMIPTR(pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    SMI_PortPtr smiPortPtr;
    XF86VideoAdaptorPtr ptrAdaptor;

	ENTER();
    ptrAdaptor = xcalloc(1, sizeof(XF86VideoAdaptorRec) +
            sizeof(DevUnion) + sizeof(SMI_PortRec));
    if (ptrAdaptor == NULL)
    {
        LEAVE(NULL);
    }

    ptrAdaptor->type = XvInputMask
        | XvImageMask
        | XvWindowMask
        ;

    ptrAdaptor->flags = VIDEO_OVERLAID_IMAGES
        | VIDEO_CLIP_TO_VIEWPORT
        ;

    ptrAdaptor->name = "Silicon Motion XV Engine";
    ptrAdaptor->nPorts = 1;
    ptrAdaptor->pPortPrivates = (DevUnion*) &ptrAdaptor[1];
    ptrAdaptor->pPortPrivates[0].ptr = (pointer) &ptrAdaptor->pPortPrivates[1];
	/* lame trick,i know */
    smiPortPtr = (SMI_PortPtr) ptrAdaptor->pPortPrivates[0].ptr;
    SMI_BuildEncodings(smiPortPtr);
    ptrAdaptor->nEncodings = smiPortPtr->nenc;
    ptrAdaptor->pEncodings = smiPortPtr->enc;

    ptrAdaptor->nFormats = nElems(SMI_VideoFormats);
    ptrAdaptor->pFormats = SMI_VideoFormats;


	if(pSmi->IsSecondary && pSmi->IsCSCVideo){
		ptrAdaptor->nAttributes = nElems(SMI_VideoAttributes_CSC);
		ptrAdaptor->pAttributes = SMI_VideoAttributes_CSC;
	}else{
	    ptrAdaptor->nAttributes = nElems(SMI_VideoAttributes);
    	ptrAdaptor->pAttributes = SMI_VideoAttributes;
	}

   // if ((pSmi->Chipset != SMI_MSOC) && (pSmi->Chipset != SMI_MSOCE))
	if(SMI_OLDLYNX(pHw->devId))
    {
        ptrAdaptor->nImages = nElems(SMI_VideoImages);
        ptrAdaptor->pImages = SMI_VideoImages;
    }
    else
    {
        ptrAdaptor->nImages = nElems(SMI501_VideoImages);
        ptrAdaptor->pImages = SMI501_VideoImages;
    }    

    ptrAdaptor->PutVideo = NULL;
    ptrAdaptor->PutStill = NULL;
    ptrAdaptor->GetVideo = NULL;
    ptrAdaptor->GetStill = NULL;

    ptrAdaptor->StopVideo = SMI_StopVideo;
    ptrAdaptor->SetPortAttribute = SMI_SetPortAttribute;
    ptrAdaptor->GetPortAttribute = SMI_GetPortAttribute;
    ptrAdaptor->QueryBestSize = SMI_QueryBestSize;
    ptrAdaptor->PutImage = SMI_PutImage_old;
#if (SMI_RANDR)
    if(!SMI_OLDLYNX(pHw->devId))
        ptrAdaptor->PutImage = SMI_PutImage_new;
#endif    
    ptrAdaptor->QueryImageAttributes = SMI_QueryImageAttributes;

    smiPortPtr->Attribute[XV_COLORKEY] = pSmi->videoKey;
	DEBUG("monk:in %s:smiPortPtr->Attribute[XV_COLORKEY] = 0x%08x\n",__func__,
			smiPortPtr->Attribute[XV_COLORKEY]);
    smiPortPtr->Attribute[XV_INTERLACED] = pSmi->interlaced;
    smiPortPtr->videoStatus = 0;

#if defined(REGION_NULL)
    REGION_NULL(pScreen, &smiPortPtr->clip);
#else
    REGION_INIT(pScreen, &smiPortPtr->clip, NullBox, 0);
#endif
    ptrAdaptor->ClipNotify = SMI_ClipNotify;

    pSmi->ptrAdaptor      = ptrAdaptor;
    pSmi->BlockHandler    = pScreen->BlockHandler;
    pScreen->BlockHandler = SMI_BlockHandler;

    xvColorKey   = MAKE_ATOM(XV_COLORKEY_NAME);
    xvBrightness = MAKE_ATOM(XV_BRIGHTNESS_NAME);
    xvCapBrightness = MAKE_ATOM(XV_CAPTURE_BRIGHTNESS_NAME);

	/* write sm502 color key register with smiPort->attributie[colorkey] */
    SMI_ResetVideo(pScrn);
	if(pHw->devId == 0x718){
		int reg;
		reg = READ_SCR(pHw,0);	
		/* pci burst on
		 * pci master on
		 * latency timer off
		 * pci burst read on
		 * pci slave burst read size 8*/
		reg |= 0x23008030;
		WRITE_SCR(pHw,0,reg);
	}
	
    LEAVE(ptrAdaptor);
}





void
SMI_ResetVideo(
        ScrnInfoPtr	pScrn
        )
{
    SMIPtr pSmi = SMIPTR(pScrn);
    SMI_PortPtr pPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;
    int r, g, b;

	ENTER();
    SetAttr(pScrn, XV_ENCODING, 0);     /* Encoding = pal-composite-0 */
    SetAttr(pScrn, XV_BRIGHTNESS, 128); /* Brightness = 128 (CCIR level) */
    SetAttr(pScrn, XV_CAPTURE_BRIGHTNESS, 128); /* Brightness = 128 (CCIR level) */
    SetAttr(pScrn, XV_CONTRAST, 71);    /* Contrast = 71 (CCIR level) */
    SetAttr(pScrn, XV_SATURATION, 64);  /* Color saturation = 64 (CCIR level) */
    SetAttr(pScrn, XV_HUE, 0);          /* Hue = 0 */

    switch (pScrn->depth)
    {
        case 8:
            SetKeyReg(pSmi, FPR04, pPort->Attribute[XV_COLORKEY] & 0x00FF);
            SetKeyReg(pSmi, FPR08, 0);
            break;

        case 15:
        case 16:
            SetKeyReg(pSmi, FPR04, pPort->Attribute[XV_COLORKEY] & 0xFFFF);
            SetKeyReg(pSmi, FPR08, 0);
            break;

        default:
            r = (pPort->Attribute[XV_COLORKEY] & pScrn->mask.red) >> pScrn->offset.red;
            g = (pPort->Attribute[XV_COLORKEY] & pScrn->mask.green) >> pScrn->offset.green;
            b = (pPort->Attribute[XV_COLORKEY] & pScrn->mask.blue) >> pScrn->offset.blue;
            SetKeyReg(pSmi, FPR04, ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
            break;
    }

    SetKeyReg(pSmi, FPR5C, 0xEDEDED | (pPort->Attribute[XV_BRIGHTNESS] << 24));
	LEAVE();
}

static void
SMI_StopVideo(
        ScrnInfoPtr	pScrn,
        pointer		data,
        Bool		shutdown
        )
{
	ENTER();
    SMI_PortPtr pPort = (SMI_PortPtr) data;
    SMIPtr pSmi = SMIPTR(pScrn);
    SMIHWPtr pHw = pSmi->pHardware;

    REGION_EMPTY(pScrn->pScreen, &pPort->clip);

    if (shutdown)
    {
        if (pPort->videoStatus & CLIENT_VIDEO_ON)
        {
            
            if(!SMI_OLDLYNX(pHw->devId))
            {
                WRITE_DCR(pHw, DCR40, READ_DCR(pHw, DCR40) & ~0x00000004);
            }
            else
            {
                WRITE_VPR(pHw, 0x00, READ_VPR(pHw, 0x00) & ~0x01000008);
            }
#if SMI_USE_CAPTURE
            if ((pHw->devId != SMI_MSOC) && (pHw->devId != SMI_MSOCE))
            {
                WRITE_CPR(pHw, 0x00, READ_CPR(pHw, 0x00) & ~0x00000001);
                WRITE_VPR(pHw, 0x54, READ_VPR(pHw, 0x54) & ~0x00F00000);
            }
            /* #864		OUT_SEQ(pSmi, 0x21, IN_SEQ(pSmi, 0x21) | 0x04); */
#endif
        }
        if (pPort->area != NULL)
        {
            xf86FreeOffscreenArea(pPort->area);
            pPort->area = NULL;
        }
        pPort->videoStatus = 0;
        /* pPort->i2cDevice = 0;aaa*/
    }
    else
    {
        if (pPort->videoStatus & CLIENT_VIDEO_ON)
        {
            pPort->videoStatus |= OFF_TIMER;
            pPort->offTime = currentTime.milliseconds + OFF_DELAY;
        }
    }
	LEAVE();
}


static int
SMI_SetPortAttribute(
        ScrnInfoPtr	pScrn,
        Atom		attribute,
        INT32		value,
        pointer		data
        )
{
    int res;
    SMI_PortPtr pPort = (SMI_PortPtr) data;
    SMIPtr pSmi = SMIPTR(pScrn);
	ENTER();
    if (attribute == xvColorKey) {
        int r, g, b;
	if(SMI_NEWLYNX(pSmi->pHardware->devId))
            pPort->Attribute[XV_COLORKEY] = value+5;//who can tell me why+5
        else
            pPort->Attribute[XV_COLORKEY] = value;

        switch (pScrn->depth)
        {
            case 8:
                SetKeyReg(pSmi, FPR04, value & 0x00FF);
                break;

            case 15:
            case 16:
		if(SMI_NEWLYNX(pSmi->pHardware->devId))
                    SetKeyReg(pSmi, FPR04,(value+5) & 0xFFFF);
                else 
                    SetKeyReg(pSmi, FPR04, value & 0xFFFF);
                break;
			case 24:
                r = (value & pScrn->mask.red) >> pScrn->offset.red;
                g = (value & pScrn->mask.green) >> pScrn->offset.green;
                b = (value & pScrn->mask.blue) >> pScrn->offset.blue;
                SetKeyReg(pSmi, FPR04,((r >> 3) << 11)|((g >> 2) << 5)|(b >> 3));
                break;
        }
        res = Success;
    }
    else if (attribute == xvInterlaced) {
        pPort->Attribute[XV_INTERLACED] = (value != 0);
        res = Success;
    }
    else if (attribute == xvEncoding) {
        res = SetAttr(pScrn, XV_ENCODING, value);
    }
    else if (attribute == xvBrightness) {
        res = SetAttr(pScrn, XV_BRIGHTNESS, value);
    }
    else if (attribute == xvCapBrightness) {
        res = SetAttr(pScrn, XV_CAPTURE_BRIGHTNESS, value);
    }
    else if (attribute == xvContrast) {
        res = SetAttr(pScrn, XV_CONTRAST, value);
    }
    else if (attribute == xvSaturation) {
        res = SetAttr(pScrn, XV_SATURATION, value);
    }
    else if (attribute == xvHue) {
        res = SetAttr(pScrn, XV_HUE, value);
    }
    else {
        res = BadMatch;
    }

    LEAVE(res);
}


static int
SMI_GetPortAttribute(
        ScrnInfoPtr	pScrn,
        Atom		attribute,
        INT32		*value,
        pointer		data
        )
{
	ENTER();
    SMI_PortPtr pPort = (SMI_PortPtr) data;

    if (attribute == xvEncoding)
        *value = pPort->Attribute[XV_ENCODING];
    else if (attribute == xvBrightness)
        *value = pPort->Attribute[XV_BRIGHTNESS];
    else if (attribute == xvCapBrightness)
        *value = pPort->Attribute[XV_CAPTURE_BRIGHTNESS];
    else if (attribute == xvContrast)
        *value = pPort->Attribute[XV_CONTRAST];
    else if (attribute == xvSaturation)
        *value = pPort->Attribute[XV_SATURATION];
    else if (attribute == xvHue)
        *value = pPort->Attribute[XV_HUE];
    else if (attribute == xvColorKey)
	{
        *value = pPort->Attribute[XV_COLORKEY];
		DEBUG("monk:get color key:0x%08x\n",*value);
	}
    else
    {
        LEAVE(BadMatch);
    }
    LEAVE(Success);
}


static void
SMI_QueryBestSize(
        ScrnInfoPtr		pScrn,
        Bool			motion,
        short			vid_w,
        short			vid_h,
        short			drw_w,
        short			drw_h,
        unsigned int	*p_w,
        unsigned int	*p_h,
        pointer			data
        )
{
	ENTER();
    SMIPtr pSmi = SMIPTR(pScrn);
    *p_w = min(drw_w, pSmi->lcdWidth);
    *p_h = min(drw_h, pSmi->lcdHeight);
	LEAVE();
}

/*
 * this version driver is designed for thinNet system vgx multi-view project
 * For more efficient:
 * 1) remove all code that not related to vgx 
 * 2) remove code that used by OVERLAY 
 * chips. 
 * */
static int
SMI_PutImage_old(
        ScrnInfoPtr		pScrn,
        short			src_x,
        short			src_y,
        short			drw_x,
        short			drw_y,
        short			src_w,
        short			src_h,
        short			drw_w,
        short			drw_h,
        int				id,
        unsigned char	*buf,
        short			width,
        short			height,
        Bool			sync,
        RegionPtr		clipBoxes,
        pointer			data,
        DrawablePtr		pDraw
        )
{
	ENTER();
    SMIPtr pSmi = SMIPTR(pScrn);
    SMI_PortPtr pPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;
    INT32 x1, y1, x2, y2;
    int bpp = 0;
    int fbPitch, srcPitch, srcPitch2 = 0, dstPitch, areaHeight;
    BoxRec dstBox;
    CARD32 offset, offset2 = 0, offset3 = 0, tmp;
    int left, top, nPixels, nLines;
    unsigned char *dstStart;
	
	x1 = src_x;
    y1 = src_y;
    x2 = src_x + src_w;
    y2 = src_y + src_h;

    dstBox.x1 = drw_x;
    dstBox.y1 = drw_y;
    dstBox.x2 = drw_x + drw_w;
    dstBox.y2 = drw_y + drw_h;

	/* maybe we should use xf86ClipVideoHelper to replace below function */
    if (!SMI_ClipVideo(pScrn, &dstBox, &x1, &y1, &x2, &y2, clipBoxes, width,
                height))
    {
        LEAVE(Success);
    }

	/* after SMI_ClipVideo,dstBox and clipBoxes values are the same 
	 * they all stand for the finally visual partial coordinate
	 * and the x1,y1,x2,y2 stands for the coordinate that located in
	 * the original source data but with a complicated data format:
	 * hi 16bits integer data and low 16bits floatpoint data
	 * */

    dstBox.x1 -= pScrn->frameX0;
    dstBox.y1 -= pScrn->frameY0;
    dstBox.x2 -= pScrn->frameX0;
    dstBox.y2 -= pScrn->frameY0;

	fbPitch = pSmi->Stride * pSmi->Bpp;

    switch (id)
    {
        case FOURCC_YV12:
            srcPitch  = (width + 3) & ~3;
            offset2   = srcPitch * height;
            srcPitch2 = ((width >> 1) + 3) & ~3;
            offset3   = offset2 + (srcPitch2 * (height >> 1));
            	/* make sure dstPitch is aligned with 16 */
            dstPitch = ((width << 1)+15)&~15;
            break;
        case FOURCC_I420:
			srcPitch  = (width + 3) & ~3;
            offset2   = srcPitch * height;
            srcPitch2 = ((width >> 1) + 3) & ~3;
            offset3   = offset2 + (srcPitch2 * (height >> 1));
            dstPitch  = ((width << 1) + 15) & ~15;				
				/* for yv12 and i420
				 * each pixel takes 12bit,which means dstPitch should be
				 * width * 1.5f, and if width is a multiple of 2,then always
				 * we can assume width * 1.5f is a interger
				 * I think we can cost down the memory usage here
				 * but it not related to performance,so let it be
				 * */
            break;
        case FOURCC_RV24:
            bpp = 3;
            srcPitch = width * bpp;
            dstPitch = (srcPitch + 15) & ~15;
            break;
        case FOURCC_RV32:
            bpp = 4;
            srcPitch = width * bpp;
            dstPitch = (srcPitch + 15) & ~15;
            break;
        case FOURCC_YUY2:
        case FOURCC_RV15:
        case FOURCC_RV16:
        default:
            bpp = 2;
            srcPitch = width * bpp;
            dstPitch = (srcPitch + 15) & ~15;
            break;
    }

    switch(id)
    {
        case FOURCC_YV12:
        case FOURCC_I420:
			if(pSmi->IsSecondary)
				areaHeight = (dstPitch*height + fbPitch - 1) / fbPitch;
			else
				if(pSmi->IsCSCVideo)
					areaHeight = (dstPitch*height + fbPitch - 1) / fbPitch;
				else		
					areaHeight = ((dstPitch * height) + fbPitch - 1) / fbPitch;//4  //yuv 4:1:1

            break;
        default:
            areaHeight = ((dstPitch * height) + fbPitch - 1) / fbPitch;
            break;
    }
    pPort->area = SMI_AllocateMemory(pScrn, pPort->area, areaHeight);

    if (pPort->area == NULL)
    {
        LEAVE(BadAlloc);
    }

    top = y1 >> 16;
    left = (x1 >> 16) & ~1;
    nPixels = ((((x2 + 0xFFFF) >> 16) + 1) & ~1) - left;
	
#if 0
    {
        left *= bpp;
        offset = (pPort->area->box.y1 * fbPitch) + top*dstPitch;
    }
#else
    {
		/* why ~750 chip don't need + top*dstPitch */
        offset = (pPort->area->box.y1 * fbPitch);
    }
#endif
    dstStart = pSmi->pFB + offset;

    switch(id) {
        case FOURCC_YV12:
        case FOURCC_I420:
			top &= ~1;
			/* calculate the offset in U/V space of video*/
            //tmp = ((top >> 1) * srcPitch2) + (left >> 2);
  			if(1)
            {
                //offset2 += tmp;
                //offset3 += tmp;
                if(id == FOURCC_I420) {
                    tmp = offset2;
                    offset2 = offset3;
                    offset3 = tmp;
                }
            }

            {
                if(pSmi->IsSecondary)
                {
				{
					CopyYV12ToVideoMem(buf, 
							buf + offset2,
							buf + offset3,
							dstStart, 
							srcPitch, srcPitch2,
							dstPitch,
							height, width);
				}
                    break;
                }
                else
                {
                    if(pSmi->IsCSCVideo)
                    {
            				{
            					CopyYV12ToVideoMem(buf, 
            							buf + offset2,
            							buf + offset3,
            							dstStart, 
            							srcPitch, srcPitch2,
            							dstPitch,
            							height, width);
            				}
                        break;
                    }
                    else
                    {
                      xf86Msg(X_INFO, "ilena, offset2=[0x%x], offset3=[0x%x], srcPitch[%d], height[%d]\n", offset2, offset3, srcPitch, height);
                        nLines = ((((y2 + 0xffff) >> 16) + 1) & ~1) - top;
                     xf86XVCopyYUV12ToPacked(buf,
                                buf + offset2, buf + offset3, dstStart,
                                srcPitch, srcPitch2, dstPitch, height, width);  /* */
                        break;
                    }
                }
            }
        case FOURCC_UYVY:
        case FOURCC_YUY2:
        default:
            buf += (top * srcPitch) + left;
            nLines = ((y2 + 0xffff) >> 16) - top;
            xf86XVCopyPacked(buf, dstStart, srcPitch, dstPitch, nLines, nPixels);
            break;
    }

	REGION_COPY(pScrn->pScreen, &pPort->clip, clipBoxes);
	if((!pSmi->IsSecondary)&&(!pSmi->IsCSCVideo))
		xf86XVFillKeyHelper(pScrn->pScreen, pPort->Attribute[XV_COLORKEY],
				clipBoxes); 
    if(SMI_OLDLYNX(pSmi->pHardware->devId))
    {
        SMI_DisplayVideo( pScrn, id,
            offset, width, height,
            dstPitch,
            x1, 	y1,
            x2, 	y2,
            &dstBox,
            src_w, src_h,
            drw_w, drw_h );
    }
    else{
#if SMI_RANDR
                xf86CrtcPtr crtc; //ilena: defined an empty variable for compile error.
#endif         
	if(pSmi->IsSecondary)
        {	
            SMI_DisplayVideo0501_CSC(pScrn, id, 
									offset,
									width, height,
									dstPitch, 
									x1, y1,
									x2, y2,
				                    &dstBox, 
									src_w, src_h,
									drw_w, drw_h,
									clipBoxes
#if SMI_RANDR
                                        ,  crtc
#endif       
                                        );
        }
        else
        {
			if(!pSmi->IsCSCVideo)
				SMI_DisplayVideo0750(pScrn, id,
									offset, 
									width, height,
									dstPitch, 
									x1, y1, 
									x2, y2,
									&dstBox,
									src_w, src_h,
									drw_w, drw_h
#if SMI_RANDR
                                        ,  crtc
#endif       
                                        );
			else
				SMI_DisplayVideo0501_CSC(pScrn, id, 
									offset,
									width, height, 
									dstPitch, 
									x1, y1,
									x2, y2,
									&dstBox,
									src_w, src_h,
									drw_w, drw_h,
									clipBoxes
#if SMI_RANDR
                                        ,  crtc
#endif       
                                        );
		}
    }

    pPort->videoStatus = CLIENT_VIDEO_ON;
    LEAVE(Success);
}

static void SMI_ClipNotify(ScrnInfoPtr pScrn, pointer data,
                                   WindowPtr window, int dx, int dy)
{
    ENTER();
   xf86Msg(X_INFO, "ilena, DX=[%d], DY=[%d]\n", dx, dy);
                       
    LEAVE();

}


/* return 0 means no enough video memory for allcation */
int
SMI_AllocateMemory_1(ScrnInfoPtr pScrn, void **mem_struct, int size)
{
    ScreenPtr	pScreen = screenInfo.screens[pScrn->scrnIndex];
    SMIPtr	pSmi = SMIPTR(pScrn);
    int		offset = 0;

    ENTER();

    	FBLinearPtr	linear = *mem_struct;

    	/*  XAA allocates in units of pixels at the screen bpp,
        	 *  so adjust size appropriately.
        	 */
    	size = (size + pSmi->Bpp - 1) / pSmi->Bpp;

    	if (linear)
    	{
    	    if (linear->size >= size)
    		LEAVE(linear->offset * pSmi->Bpp);

    	    if (xf86ResizeOffscreenLinear(linear, size))
    		LEAVE(linear->offset * pSmi->Bpp);

    	    xf86FreeOffscreenLinear(linear);
    	}
    	else
    	{
    	    int max_size;

    	    xf86QueryLargestOffscreenLinear(pScreen, &max_size, 16,
    					    PRIORITY_EXTREME);
    	    if (max_size < size)
    		LEAVE(0);   		

    	    xf86PurgeUnlockedOffscreenAreas(pScreen);
    	}

    	linear = xf86AllocateOffscreenLinear(pScreen, size, 16,
					     NULL, NULL, NULL);
    	if ((*mem_struct = linear) != NULL)
    	    offset = linear->offset * pSmi->Bpp;

    	DEBUG("offset = %p\n", offset);

    LEAVE(offset);
}





static int
SMI_PutImage_new(
	ScrnInfoPtr		pScrn,
	short			src_x,
	short			src_y,
	short			drw_x,
	short			drw_y,
	short			src_w,
	short			src_h,
	short			drw_w,
	short			drw_h,
	int			id,
	unsigned char		*buf,
	short			width,
	short			height,
	Bool			sync,
	RegionPtr		clipBoxes,
	pointer			data,
	DrawablePtr		pDraw
)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    //SMIEntPtr pEntPriv = pSmi->entityPrivate;
    SMI_PortPtr pPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;
    INT32 x1, y1, x2, y2;
    int bpp = 0;
    int srcPitch, srcPitch2 = 0, dstPitch, size, cscSz, crtcIdx;
    BoxRec dstBox;
    CARD32 offset, offset2 = 0, offset3 = 0, tmp;
    int left, top, nPixels, nLines;
    unsigned char *dstStart;
    xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);
    xf86CrtcPtr crtc, crtctmp;
    uint32_t usedVidMem;
    Bool expansion;
    Bool VideoUseCSC = pSmi->IsCSCVideo;
    Bool VideoIsMirror;


    ENTER();

    switch (pSmi->pHardware->devId)
    {
        case SMI_750:
        case SMI_718:
        case SMI_MSOC:
            expansion = 0;//pEntPriv->xlcd > 0 && pEntPriv->ylcd > 0; // ilena: i'm not sure.
        break;
        default:
            expansion = FALSE;
        break;        
    }    

       	crtcIdx = 0;
    	VideoIsMirror = ( (crtcConf->num_crtc>1)&&(crtcConf->crtc[crtcIdx+1]->x == crtcConf->crtc[crtcIdx]->x)&&(crtcConf->crtc[crtcIdx+1]->y == crtcConf->crtc[crtcIdx]->y));
	//VideoUseCSC |=  VideoIsMirror;// mirror mode do not support over lay video playing

    for (crtcIdx = 0, usedVidMem = 0; crtcIdx < crtcConf->num_crtc; crtcIdx++)
    {
        x1 = src_x;
        y1 = src_y;
        x2 = src_x + src_w;
        y2 = src_y + src_h;

        dstBox.x1 = drw_x;
        dstBox.y1 = drw_y;
        dstBox.x2 = drw_x + drw_w;
        dstBox.y2 = drw_y + drw_h;
		
        /*if (pSmi->CSCVideo || pSmi->DualView || expansion) 
		{
		if (!xf86XVClipVideoHelper(&dstBox, &x1, &x2, &y1, &y2, clipBoxes,width, height))
		LEAVE(Success);
		}
		else */
		
        if (!xf86_crtc_clip_video_helper(pScrn, &crtc, crtcConf->crtc[crtcIdx],
                    &dstBox, &x1, &x2, &y1, &y2, clipBoxes, width, height))
            LEAVE(Success);
        if (crtc != crtcConf->crtc[crtcIdx])
            continue;
        if (!crtc->enabled)
            continue;
	if(crtcIdx&&VideoIsMirror)
	     continue;


        top = y1 >> 16;
        //left = (x1 >> 16) & ~1;
        left = x1 >> 16;
        nPixels = ((((x2 + 0xFFFF) >> 16) + 1) & ~1) - (left & ~1);
        //nPixels = ((((x2 + 0xFFFF) >> 16) + 1) & ~1) - left;

        /* Transform dstBox to the CRTC coordinates */
        /*dstBox.x1 -= crtc->x;
	        dstBox.y1 -= crtc->y;
	        dstBox.x2 -= crtc->x;
	        dstBox.y2 -= crtc->y;*/

        switch (id) {
            case FOURCC_YV12:
            case FOURCC_I420:
                bpp = 2;
                srcPitch  = (width + 3) & ~3;
                offset2   = srcPitch * height;
                srcPitch2 = ((width >> 1) + 3) & ~3;
                offset3   = offset2 + (srcPitch2 * (height >> 1));
                if ((crtcIdx == 0) && (!VideoUseCSC)) {
                    //dstPitch  = ((width << 1) + 15) & ~15;
                    size = (width >= height) ? ((width * 2 + 31) * height) :
                            (width * (height * 2 + 31));
                    size *= bpp;
                }
                else {
                    //dstPitch  = (((width >> 1) + 15) & ~15) << 1;
                    size = (width >= height) ? ((width + 62) * height) : 
                            (width * (height + 62));
                    size *= bpp;
                }
                break;
            case FOURCC_RV24:
                bpp = 3;
                srcPitch = width * bpp;
                dstPitch = (srcPitch + 15) & ~15;
                break;
            case FOURCC_RV32:
                bpp = 4;
                srcPitch = width * bpp;
                dstPitch = (srcPitch + 15) & ~15;
                break;
            case FOURCC_YUY2:
            case FOURCC_RV15:
            case FOURCC_RV16:
            default:
                bpp = 2;
                srcPitch = width * bpp;
                dstPitch = (srcPitch + 15) & ~15;
                break;
        }

        //size = dstPitch * height * bpp;
        pPort->video_offset = SMI_AllocateMemory_1(pScrn, &pPort->video_memory, size);
        if (pPort->video_memory == NULL)
            LEAVE(BadAlloc);

        left *= bpp;

        offset = pPort->video_offset + usedVidMem/*+ (top * dstPitch)*/;
        DEBUG("left = %d offset = %08X\n", left, offset);
        dstStart = pSmi->pFB + offset /*+ left*/;

        switch(id) 
        {
            case FOURCC_YV12:
            case FOURCC_I420:
                top &= ~1;
                tmp = ((top >> 1) * srcPitch2) + (left >> 2);
                nLines = ((((y2 + 0xffff) >> 16) + 1) & ~1) - top;
                offset2 += tmp;
                offset3 += tmp;
                /*offset2 = nLines * srcPitch + tmp;
		                offset3   = offset2 + (srcPitch2 * (nLines >> 1)) + tmp;*/
                if (crtc->rotation & (RR_Rotate_0 | RR_Rotate_180)) {
                    if ((crtcIdx == 0) && (!VideoUseCSC)){
                        if (crtc->rotation & RR_Rotate_0)
                            dstPitch = ((width << 1)+15)&~15;//by ilena
                        else
                            dstPitch  = ((nPixels << 1) + 15) & ~15;
                    }
                    else
                        dstPitch  = (((nPixels >> 1) + 15) & ~15) << 1;
                } else {
                    if ((crtcIdx == 0) && (!VideoUseCSC))
                        dstPitch  = ((nLines << 1) + 15) & ~15;
                    else{
                      /*if((crtcIdx == 1) && (!VideoUseCSC)){
                        dstPitch = ((width << 1)+15)&~15;//by ilena
                      }
                      else*/
                        dstPitch  = (((nLines >> 1) + 15) & ~15) << 1;
                    }
                }

                if (VideoUseCSC || (crtcIdx == 1) || expansion) {
                    usedVidMem += CopyYV12Planar(buf + (top * srcPitch) + 
                                                        (left >> 1), 
                                                        buf+offset2, 
                                                        buf+offset3, dstStart, 
                                                        srcPitch, srcPitch2, 
                                                        dstPitch, nLines, 
                                                        nPixels, 
                                                        crtc->rotation);
                } else {
                    if (id == FOURCC_I420) {
                        tmp = offset2;
                        offset2 = offset3;
                        offset3 = tmp;
                    }
                    //xf86XVCopyYUV12ToPacked
                    /* 
                    				below function actually convert YV12(planer,12bits/pixel) 
                    				to YUY2(packed,16bits/pixel)		Monk.liu
                    			*/
                    usedVidMem += CopyYV12ToPacked(buf + (top * srcPitch) + 
                                                    (left >> 1), 
                                                    buf + offset2, 
                                                    buf + offset3, dstStart,
                                                    srcPitch, srcPitch2, 
                                                    dstPitch, nLines,
                                                    nPixels, 
                                                    crtc->rotation);
                }
                break;
            case FOURCC_UYVY:
            case FOURCC_YUY2:
            default:
                buf += (top * srcPitch) + left;
                nLines = ((y2 + 0xffff) >> 16) - top;
                xf86XVCopyPacked(buf, dstStart, srcPitch, dstPitch, nLines, nPixels);
                break;
        }

        if ((pSmi->pHardware->devId!= SMI_712 && pSmi->pHardware->devId != SMI_722)||
                !REGION_EQUAL(pScrn->pScreen, &pPort->clip, clipBoxes)) {
            REGION_COPY(pScrn->pScreen, &pPort->clip, clipBoxes);
			
            if (!(VideoUseCSC || crtcIdx == 1 || expansion)) {
                RegionRec tmp;
                REGION_INIT (NULL, &tmp, &dstBox, 1);
                REGION_INTERSECT (NULL, &tmp, &tmp, clipBoxes);
                xf86XVFillKeyHelper(pScrn->pScreen, pPort->Attribute[XV_COLORKEY],
                        (&tmp));
            }
        }
        if ((pSmi->pHardware->devId!= SMI_712 && pSmi->pHardware->devId != SMI_722)) {
       	  if (VideoUseCSC  || (crtcIdx == 1) || expansion) {
                SMI_DisplayVideo0501_CSC(pScrn, id, offset, nPixels, nLines, 
                        dstPitch, x1, y1, x2, y2, &dstBox,
                        src_w, src_h, drw_w, drw_h, clipBoxes
#if SMI_RANDR
                        , crtc
#endif
                        ); 
            } else {
                SMI_DisplayVideo0750(pScrn, id, offset, width, height, dstPitch, x1, y1, x2, y2,
                        &dstBox, src_w, src_h, drw_w, drw_h
#if (SMI_RANDR==1)
                        , crtc
#endif
                        ); 
            }
        }
        pPort->videoStatus = CLIENT_VIDEO_ON;
    }
    LEAVE(Success);
}

static int
SMI_QueryImageAttributes(
        ScrnInfoPtr		pScrn,
        int				id,
        unsigned short	*width,
        unsigned short	*height,
        int				*pitches,
        int				*offsets
        )
{
    SMIPtr pSmi = SMIPTR(pScrn);
    int size, tmp;
	ENTER();
    /*
       if (*width > pSmi->lcdWidth)
       {
     *width = pSmi->lcdWidth;
     }
     if (*height > pSmi->lcdHeight)
     {
     *height = pSmi->lcdHeight;
     }
     */

    *width = (*width + 1) & ~1;
    if (offsets != NULL)
    {
        offsets[0] = 0;
    }

    switch (id)
    {
        case FOURCC_YV12:
        case FOURCC_I420:
            *height = (*height + 1) & ~1;
            size = (*width + 3) & ~3;
            if (pitches != NULL)
            {
                pitches[0] = size;
            }
            size *= *height;
            if (offsets != NULL)
            {
                offsets[1] = size;
            }
            tmp = ((*width >> 1) + 3) & ~3;
            if (pitches != NULL)
            {
                pitches[1] = pitches[2] = tmp;
            }
            tmp *= (*height >> 1);
            size += tmp;
            if (offsets != NULL)
            {
                offsets[2] = size;
            }
            size += tmp;
            break;

        case FOURCC_YUY2:
        case FOURCC_RV15:
        case FOURCC_RV16:
        default:
            size = *width * 2;
            if (pitches != NULL)
            {
                pitches[0] = size;
            }
            size *= *height;
            break;

        case FOURCC_RV24:
            size = *width * 3;
            if (pitches != NULL)
            {
                pitches[0] = size;
            }
            size *= *height;
            break;

        case FOURCC_RV32:
            size = *width * 4;
            if (pitches != NULL)
            {
                pitches[0] = size;
            }
            size *= *height;
            break;
    }

    LEAVE(size);
}


/******************************************************************************\
 **																			  **
 **						S U P P O R T   F U N C T I O N S					  **
 **																			  **
 \******************************************************************************/

/* To allow this ddx to work on 4_3_0 and above, we need to include this */
#ifdef XF86_VERSION_NUMERIC
#if XF86_VERSION_CURRENT < XF86_VERSION_NUMERIC(4,3,99,0,0)

static Bool
RegionsEqual(
        RegionPtr	A,
        RegionPtr	B
        )
{
    int *dataA, *dataB;
    int num;
    ENTER();

    num = REGION_NUM_RECTS(A);
    if (num != REGION_NUM_RECTS(B))
    {
        return(FALSE);
    }

    if (   (A->extents.x1 != B->extents.x1)
            || (A->extents.y1 != B->extents.y1)
            || (A->extents.x2 != B->extents.x2)
            || (A->extents.y2 != B->extents.y2)
       )
    {
        return(FALSE);
    }

    dataA = (int*) REGION_RECTS(A);
    dataB = (int*) REGION_RECTS(B);

    while (num--)
    {
        if ((dataA[0] != dataB[0]) || (dataA[1] != dataB[1]))
        {
            return(FALSE);
        }
        dataA += 2;
        dataB += 2;
    }

    LEAVE(TRUE);
}

#endif
#endif

static Bool
SMI_ClipVideo(
        ScrnInfoPtr	pScrn,
        BoxPtr		dst,
        INT32		*x1,
        INT32		*y1,
        INT32		*x2,
        INT32		*y2,
        RegionPtr	reg,
        INT32		width,
        INT32		height
        )
{
	ENTER();
    ScreenPtr pScreen = pScrn->pScreen;
    INT32 vscale, hscale;
    BoxPtr extents = REGION_EXTENTS(pScreen, reg);
    int diff;
    SMIPtr pSmi = SMIPTR(pScrn);

    DEBUG("ClipVideo(%d): x1=%d y1=%d x2=%d y2=%d\n",  __LINE__, *x1 >> 16, *y1 >> 16, *x2 >> 16, *y2 >> 16);
    /* PDR#941 */
    extents->x1 = max(extents->x1, pScrn->frameX0);
    extents->y1 = max(extents->y1, pScrn->frameY0);

    hscale = ((*x2 - *x1) << 16) / (dst->x2 - dst->x1);
    vscale = ((*y2 - *y1) << 16) / (dst->y2 - dst->y1);
#if 1
    // below code make shrinked movie that dragged out of onscreen also	
    // looks alright ,or the video data will scrolled if it must be a description of un-alright	
    // The root caus is that no suspport of shrink on sm712.	
    if(pSmi->pHardware->devId == SMI_LYNXEMplus)	
    {
        if(hscale>65536)
            hscale = 65536;
        if(vscale > 65536)
            vscale = 65536;
    }
#endif		

    *x1 <<= 16; *y1 <<= 16;
    *x2 <<= 16; *y2 <<= 16;

    DEBUG("ClipVideo(%d): x1=%d y1=%d x2=%d y2=%d\n",  __LINE__, *x1 >> 16, *y1 >> 16, *x2 >> 16, *y2 >> 16);

    diff = extents->x1 - dst->x1;
    if (diff > 0)
    {
        dst->x1 = extents->x1;
        *x1 += diff * hscale;
    }

    diff = extents->y1 - dst->y1;
    if (diff > 0)
    {
        dst->y1 = extents->y1;
        *y1 += diff * vscale;
    }

    diff = dst->x2 - extents->x2;
    if (diff > 0)
    {
        dst->x2 = extents->x2; /* PDR#687 */
        *x2 -= diff * hscale;
    }

    diff = dst->y2 - extents->y2;
    if (diff > 0)
    {
        dst->y2 = extents->y2;
        *y2 -= diff * vscale;
    }

    DEBUG("ClipVideo(%d): x1=%d y1=%d x2=%d y2=%d\n",  __LINE__, *x1 >> 16, *y1 >> 16, *x2 >> 16, *y2 >> 16);

    if (*x1 < 0)
    {
        diff = (-*x1 + hscale - 1) / hscale;
        dst->x1 += diff;
        *x1 += diff * hscale;
    }

    if (*y1 < 0)
    {
        diff = (-*y1 + vscale - 1) / vscale;
        dst->y1 += diff;
        *y1 += diff * vscale;
    }

    DEBUG("ClipVideo(%d): x1=%d y1=%d x2=%d y2=%d\n",  __LINE__, *x1 >> 16, *y1 >> 16, *x2 >> 16, *y2 >> 16);

#if 0 /* aaa was macht dieser code? */
    delta = *x2 - (width << 16);
    if (delta > 0)
    {
        diff = (delta + hscale - 1) / hscale;
        dst->x2 -= diff;
        *x2 -= diff * hscale;
    }

    delta = *y2 - (height << 16);
    if (delta > 0)
    {
        diff = (delta + vscale - 1) / vscale;
        dst->y2 -= diff;
        *y2 -= diff * vscale;
    }
#endif

    DEBUG("ClipVideo(%d): x1=%d y1=%d x2=%d y2=%d\n",  __LINE__, *x1 >> 16, *y1 >> 16, *x2 >> 16, *y2 >> 16);

    if ((*x1 >= *x2) || (*y1 >= *y2))
    {
        return(FALSE);
    }

    if (   (dst->x1 != extents->x1) || (dst->y1 != extents->y1)
            || (dst->x2 != extents->x2) || (dst->y2 != extents->y2)
       )
    {
        RegionRec clipReg;
        REGION_INIT(pScreen, &clipReg, dst, 1);
        REGION_INTERSECT(pScreen, reg, reg, &clipReg);
        REGION_UNINIT(pScreen, &clipReg);
    }

    DEBUG("ClipVideo(%d): x1=%d y1=%d x2=%d y2=%d\n",  __LINE__, *x1 >> 16, *y1 >> 16, *x2 >> 16, *y2 >> 16);

    LEAVE(TRUE);
}

static void
SMI_DisplayVideo(
        ScrnInfoPtr	pScrn,
        int			id,
        int			offset,
        short		width,
        short		height,
        int			pitch,
        int			x1,
        int			y1,
        int			x2,
        int			y2,
        BoxPtr		dstBox,
        short		vid_w,
        short		vid_h,
        short		drw_w,
        short		drw_h
        )
{
	ENTER();
    SMIPtr pSmi = SMIPTR(pScrn);
    SMIHWPtr pHw = pSmi->pHardware;    
    CARD32 vpr00;
    int hstretch, vstretch;
    int hstretch_vpr68, vstretch_vpr68;

    hstretch = vstretch = hstretch_vpr68 = vstretch_vpr68 = 0;


    vpr00 = READ_VPR(pHw, 0x00) & ~0x0CB800FF;

    switch (id)
    {
        case FOURCC_YV12:
        case FOURCC_I420:
        case FOURCC_YUY2:
            vpr00 |= 0x6;
            break;

        case FOURCC_RV15:
            vpr00 |= 0x1;
            break;

        case FOURCC_RV16:
            vpr00 |= 0x2;
            break;

        case FOURCC_RV24:
            vpr00 |= 0x4;
            break;

        case FOURCC_RV32:
            vpr00 |= 0x3;
            break;
    }


    if (drw_w > vid_w)
    {
        hstretch = 256 * (vid_w - 1) / (drw_w - 1);
        hstretch_vpr68 = 65536 * (vid_w - 1) / (drw_w - 1) ;
    }
    else
    {
        hstretch = 0;
        hstretch_vpr68 = 0;
    }

    if (drw_h > vid_h)
    {
        vstretch = 256 * (vid_h - 1) / (drw_h - 1)  ;
        vstretch_vpr68 = 65536 * (vid_h - 1) / (drw_h - 1) ;
        vpr00 |= 1 << 21;
    }
    else
    {
        vstretch = 0;
        vstretch_vpr68 = 0;
    }
#if 0
    SMI_WaitForSync(pScrn);
#endif

    WRITE_VPR(pHw, 0x00, vpr00 | (1 << 3) | (1 << 20));
    WRITE_VPR(pHw, 0x14, (dstBox->x1) | (dstBox->y1 << 16));
    WRITE_VPR(pHw, 0x18, (dstBox->x2) | (dstBox->y2 << 16));
    WRITE_VPR(pHw, 0x1C, offset >> 3);
    WRITE_VPR(pHw, 0x20, (pitch >> 3) | ((pitch >> 3) << 16));
    WRITE_VPR(pHw, 0x24, ((hstretch & 0xff) << 8) | (vstretch & 0xff));

    WRITE_VPR(pHw, 0x68, ((hstretch_vpr68 & 0xff) << 8) | (vstretch_vpr68 & 0xff));

    vpr00 = READ_VPR(pHw, 0x00);
    LEAVE();
}

static void 
SMI_DisplayVideo0501_CSC(
        ScrnInfoPtr pScrn, 
        int id, 
        int offset, 
        short width, 
        short height, 
        int pitch, 
        int x1, 
        int y1, 
        int x2, 
        int y2, 
        BoxPtr dstBox, 
        short vid_w, 
        short vid_h, 
        short drw_w, 
        short drw_h,
        RegionPtr clipboxes
#if SMI_RANDR
        , xf86CrtcPtr crtc
#endif        
        )
{
	ENTER();
    DWORD origSrcDimX, origSrcDimY, origDestDimX, origDestDimY;
    DWORD ScaleXn, ScaleXd, ScaleYn, ScaleYd;
    DWORD SrcTn, SrcTd, SrcLn, SrcLd;
    DWORD SrcRn, SrcRd, SrcBn, SrcBd;
    DWORD SrcDimX, SrcDimY, DestDimX, DestDimY;
    DWORD SrcFormat, DstFormat, HFilter, VFilter, byOrder;
    DWORD SrcYBase, SrcUBase, SrcVBase, SrcYPitch, SrcUVPitch;
    DWORD DestBase, DestPitch, DestHeight, DestWidth, DestBpp;
    DWORD SrcBpp = 0, SrcLnAdd = 0;

    SMIPtr pSmi = SMIPTR(pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    CARD32 crt_fb;

    RegionRec lRegion;
    REGION_INIT (NULL, &lRegion, dstBox, 1);
    REGION_INTERSECT (NULL, &lRegion, &lRegion, clipboxes);
#if(SMI_RANDR)	
    BoxPtr pbox = REGION_RECTS(&lRegion);
    int i, nbox = REGION_NUM_RECTS(&lRegion);
#else
    BoxPtr pbox = REGION_RECTS(clipboxes);
    int i, nbox = REGION_NUM_RECTS(clipboxes);
#endif
    xRectangle *rects;
    CARD32 CSC_Control ;
    int32_t	csc;
    float	Hscale, Vscale;
    uint32_t dst, h, tmp1, tmp2;

    if(!pScrn->vtSema)
		LEAVE();

    if(!pSmi->IsSecondary)
    {
        crt_fb = READ_DCR(pHw, 0xC) & 0x03FFFFFF;
    }
    else
    {
        crt_fb = READ_DCR(pHw, 0x204) & 0x03FFFFFF;
    }

    SrcYBase = offset+crt_fb;
    SrcUBase = SrcYBase;
    SrcVBase = SrcYBase;

    SrcYPitch = pitch;
    SrcUVPitch = SrcYPitch;	 
#if (SMI_RANDR==0)	
    DestBase = crt_fb;
    if (pSmi->Bpp == 3)
    {
        DestPitch = pSmi->Stride;
    }
    else
    {
        DestPitch = pSmi->Stride * pSmi->Bpp;
    }

#else
    crt_fb = 0; //ilena: randr driver donot need the crt_fb value.
    if ( crtc->rotation & RR_Rotate_0) {
        DestBase = pScrn->fbOffset;
        DestPitch = (pScrn->displayWidth * pSmi->Bpp + 15) & ~15;
    } else {
        DestBase = (char*)crtc->rotatedData - (char*)pSmi->pFB;
        DestPitch = (crtc->mode.HDisplay * pSmi->Bpp + 15) & ~15;
    }
#endif

    DestBpp = pSmi->Bpp;

    SrcFormat = 0; 
    byOrder = 0;

    origSrcDimX = vid_w;
    origSrcDimY = vid_h;

    origDestDimX = drw_w;
    origDestDimY = drw_h;

#if (SMI_RANDR == 0)
    ScaleXn = (origSrcDimX-1)/(origDestDimX-1);
    ScaleXd = ((origSrcDimX-1)<<13)/(origDestDimX-1) - (ScaleXn<<13);

    ScaleYn = (origSrcDimY-1)/(origDestDimY-1);
    ScaleYd = ((origSrcDimY-1)<<13)/(origDestDimY-1) - (ScaleYn<<13);  
    h = height;
    Hscale = (vid_w - 1) / (float)(drw_w - 1);
    Vscale = (vid_h - 1) / (float)(drw_h - 1);
#else
    if (crtc->rotation & (RR_Rotate_0 | RR_Rotate_180)) {
        h = height;
        Hscale = (vid_w - 1) / (float)(drw_w - 1);
		if (Hscale <= 4.0) {	
            ScaleXn = Hscale;
            ScaleXd = ((vid_w - 1) << 13) / (drw_w - 1) - (ScaleXn << 13);
        }
        else {
            ScaleXn = 4.0;
            ScaleXd = (1 << 11) - (ScaleXn << 13);
        }

        Vscale = (vid_h - 1) / (float)(drw_h - 1);
		if (Vscale <= 4.0) {	
            ScaleYn = Vscale;
            ScaleYd = ((vid_h - 1) << 13) / (drw_h - 1) - (ScaleYn << 13);
        }
        else {
            ScaleYn = 4.0;
            ScaleYd = (1 << 11) - (ScaleYn << 13);
        }

    } else if (crtc->rotation & (RR_Rotate_90 | RR_Rotate_270)) {
        h = width;
        Hscale = (vid_h - 1) / (float)(drw_h - 1);
        if (Hscale <= 4.0) {
            ScaleXn = Hscale;
	        ScaleXd = ((vid_h - 1) << 13) / (drw_h - 1) - (ScaleXn << 13);
        }
        else {
            ScaleXn = 4.0;
            ScaleXd = (1 << 11) - (ScaleXn << 13);
        }

        Vscale = (vid_w - 1) / (float)(drw_w - 1);
        if (Vscale <= 4.0) {
            ScaleYn = Vscale;
        	ScaleYd = ((vid_w - 1) << 13) / (drw_w - 1) - (ScaleYn << 13);
        }
        else {
            ScaleYn = 4.0;
            ScaleYd = (1 << 11) - (ScaleYn << 13);
        }
    }

    /* CSC Interpolation */
    if (ScaleXn < 1.0 || ScaleYn < 1.0) {
    csc = (1<<25)|(1<<24);// FIELD_SET(0, CSC_CONTROL, HORIZONTAL_FILTER, ENABLE) |
            //            FIELD_SET(0, CSC_CONTROL, VERTICAL_FILTER, ENABLE);
    } else {
        csc = 0;//FIELD_SET(0, CSC_CONTROL, HORIZONTAL_FILTER, DISABLE) |
            //FIELD_SET(0, CSC_CONTROL, VERTICAL_FILTER, DISABLE);
    }
    /* CSC Destination Format */
    if (pScrn->bitsPerPixel > 16) {
        csc |= (1<<26);// FIELD_SET(0, CSC_CONTROL, DESTINATION_FORMAT, RGB8888);
    } 
#endif	
    rects = xalloc(nbox * sizeof(xRectangle));  //caesar modified
    BOOL flag;
    for(i = 0, flag = FALSE; i < nbox; i++, pbox++) {  
#if (SMI_RANDR == 0)
        rects[i].x = pbox->x1;
        rects[i].y = pbox->y1;
        rects[i].width = pbox->x2 - pbox->x1 ;
        rects[i].height = pbox->y2 - pbox->y1 ;
#else
        switch (crtc->rotation) {
            case RR_Rotate_0:
                rects[i].x = pbox->x1;
                rects[i].y = pbox->y1;
                rects[i].width = pbox->x2 - pbox->x1;
                rects[i].height = pbox->y2 - pbox->y1;
                break;
            case RR_Rotate_90:
                rects[i].x = pbox->y1 - crtc->y;
                rects[i].y = crtc->mode.VDisplay - (pbox->x2 - crtc->x) - 1;
                rects[i].width = pbox->y2 - pbox->y1;
                rects[i].height = pbox->x2 - pbox->x1;
                if (!flag) {
                    tmp1 = dstBox->x1 - crtc->x;
                    tmp2 = dstBox->x2 - crtc->x;
                    dstBox->x1 = dstBox->y1 - crtc->y;
                    dstBox->x2 = dstBox->y2 - crtc->y;
                    dstBox->y1 = crtc->mode.VDisplay - tmp2 - 1;
                    dstBox->y2 = crtc->mode.VDisplay - tmp1;
                    flag = TRUE;
                }
                break;
            case RR_Rotate_270:
                rects[i].x = crtc->mode.HDisplay - (pbox->y2 - crtc->y) - 1;
                rects[i].y = pbox->x1 - crtc->x;
                rects[i].width = pbox->y2 - pbox->y1;
                rects[i].height = pbox->x2 - pbox->x1;
                if (!flag) {
                    tmp1 = dstBox->y1 - crtc->y;
                    tmp2 = dstBox->y2 - crtc->y;
                    dstBox->y1 = dstBox->x1 - crtc->x;
                    dstBox->y2 = dstBox->x2 - crtc->x;
                    dstBox->x1 = crtc->mode.HDisplay - tmp2 - 1;
                    dstBox->x2 = crtc->mode.HDisplay - tmp1;
                    flag = TRUE;
                }
                break;
            case RR_Rotate_180:
                rects[i].x = crtc->mode.HDisplay - (pbox->x2 - crtc->x) - 1;
                rects[i].y = crtc->mode.VDisplay - (pbox->y2 - crtc->y) - 1;
                rects[i].height = pbox->y2 - pbox->y1;
                rects[i].width = pbox->x2 - pbox->x1;
                if (!flag) {
                    tmp1 = dstBox->x1 - crtc->x;
                    tmp2 = dstBox->y1 - crtc->y;
                    dstBox->x1 = crtc->mode.HDisplay - (dstBox->x2 - crtc->x) - 1;
                    dstBox->x2 = crtc->mode.HDisplay - tmp1;
                    dstBox->y1 = crtc->mode.VDisplay - (dstBox->y2 - crtc->y) - 1;
                    dstBox->y2 = crtc->mode.VDisplay - tmp2;
                    flag = TRUE;
                }
                break;
        }

#endif

        DestHeight = rects[i].height;
        DestWidth = rects[i].width;
        switch(id)
        {
            case FOURCC_YV12:
                SrcFormat = 2;
                byOrder = 0;
                SrcLnAdd = 0;

#if(SMI_RANDR)
                SrcYPitch = pitch;//
#else
                //ilena: align the YPitch so that UVPitch can be devided by 16.
                SrcYPitch = (width + 31) & ~31;
#endif
                SrcUVPitch = SrcYPitch/2;

                SrcYBase = offset+crt_fb;	
                SrcVBase = SrcYBase + SrcYPitch * h;
                SrcUBase = SrcVBase + SrcUVPitch*h/2;
                /* CSC Source Format */
                csc |= (2<<28);//FIELD_SET(0, CSC_CONTROL, SOURCE_FORMAT, YUV420);
                /* CSC Byte Order */
                csc |= 0;//FIELD_SET(0, CSC_CONTROL, BYTE_ORDER, YUYV);
                break;

            case FOURCC_I420:
                SrcFormat = 2;
                byOrder = 0;
                SrcLnAdd = 0;

                SrcYPitch = pitch;//width;
                SrcUVPitch = SrcYPitch/2;

                SrcYBase = offset+crt_fb;	
                SrcUBase = SrcYBase + SrcYPitch*h;
                SrcVBase = SrcUBase + SrcUVPitch*h/2;
                /* CSC Source Format */
                csc |= (2<<28);//FIELD_SET(0, CSC_CONTROL, SOURCE_FORMAT, YUV420);
                /* CSC Byte Order */
                csc |= 0;//FIELD_SET(0, CSC_CONTROL, BYTE_ORDER, YUYV);
                break;

            case FOURCC_YUY2:
            case FOURCC_RV16:
            case FOURCC_RV32: 
                SrcFormat = 0;
                byOrder = 0;
                SrcBpp = 16/8;
                SrcYBase = offset+crt_fb;
                SrcLnAdd = 0;
                /* CSC Source Format */
                csc |= (2<<28);//FIELD_SET(0, CSC_CONTROL, SOURCE_FORMAT, YUV420);
                /* CSC Byte Order */
                csc |= 0;//FIELD_SET(0, CSC_CONTROL, BYTE_ORDER, YUYV);
                break;
        }

        switch (DestBpp)
        {
            case 2:
                DstFormat = 0;
                break;
            case 4:
                DstFormat = 1;
                break;
            default: 
                LEAVE();
                break;		
        }

        HFilter = 1;
        VFilter = 1;

        SrcLn = (rects[i].x - dstBox->x1)*Hscale;
        SrcLd = ((rects[i].x -  dstBox->x1)<<13)*1.0*Hscale- (SrcLn<<13);

        SrcRn = (rects[i].x+rects[i].width  - dstBox->x1)*Hscale;
        SrcRd = ((rects[i].x+rects[i].width  - dstBox->x1)<<13)*1.0*Hscale- (SrcRn<<13);

        SrcTn = (rects[i].y-dstBox->y1)*Vscale;
        SrcTd = ((rects[i].y-dstBox->y1)<<13)*1.0*Vscale -(SrcTn<<13);

        SrcBn = (rects[i].y+rects[i].height  - dstBox->y1)*Vscale;
        SrcBd = ((rects[i].y+rects[i].height  - dstBox->y1)<<13)*1.0*Vscale- (SrcBn<<13);

        SrcDimX = (SrcRd == 0) ? (SrcRn - SrcLn + 1) : (SrcRn - SrcLn + 2);
        SrcDimY = (SrcBd == 0) ? (SrcBn - SrcTn + 1) : (SrcBn - SrcTn + 2);

        DestDimX = rects[i].width;
        DestDimY = rects[i].height;

        //WaitForNotBusy();

        WRITE_DPR (pHw, 0xCC, 0x0);
        WRITE_DPR (pHw, 0xD0, (SrcLn<<16 + SrcLnAdd<<16)|SrcLd);
        WRITE_DPR (pHw, 0xD4, SrcTn<<16 | SrcTd);
        WRITE_DPR (pHw, 0xE0, SrcDimX<<16 | SrcDimY);
        WRITE_DPR (pHw, 0xE4, (SrcYPitch >> 4)<<16 |(SrcUVPitch >> 4) );
        WRITE_DPR (pHw, 0xE8, (rects[i].x)<<16 | rects[i].y);	
        WRITE_DPR (pHw, 0xEC, DestDimX<<16 | DestDimY);	
        WRITE_DPR (pHw, 0xF0, (DestPitch >> 4)<<16 | DestHeight);
        WRITE_DPR (pHw, 0xF4,  (ScaleXn<<13 | ScaleXd)<<16 |(ScaleYn<<13 | ScaleYd));

#if (SMI_RANDR==0)         
        WRITE_DPR (pHw, 0xC8,  SrcYBase + ((x1 >> 16) << 0) + (y1 >> 16) * SrcYPitch);
        WRITE_DPR (pHw, 0xD8,  SrcUBase + (x1 >> (16 + 1)) + (y1 >> (16 + 1)) * SrcUVPitch);
        WRITE_DPR (pHw, 0xDC,  SrcVBase + (x1 >> (16 + 1)) + (y1 >> (16 + 1)) * SrcUVPitch);
#else
        WRITE_DPR (pHw, 0xC8,  SrcYBase);
        WRITE_DPR (pHw, 0xD8,  SrcUBase);
        WRITE_DPR (pHw, 0xDC,  SrcVBase);

#endif
        WRITE_DPR (pHw, 0xF8,  DestBase);

        //WRITE_DPR(pHw, 0xFC, csc);		//ilena: we should set video format for every type!
	
        CSC_Control = 1<<31 | SrcFormat<<28 | DstFormat<<26 | HFilter<<25 | VFilter<<24 |byOrder<<23;
        SMI_CSC_Start(pSmi, CSC_Control);

    }
    REGION_UNINIT(NULL, &lRegion);
    xfree(rects); //caesar modified
    LEAVE();
}

static void
SMI_DisplayVideo0501(
        ScrnInfoPtr	pScrn,
        int			id,
        int			offset,
        short		width,
        short		height,
        int			pitch,
        int			x1,
        int			y1,
        int			x2,
        int			y2,
        BoxPtr		dstBox,
        short		vid_w,
        short		vid_h,
        short		drw_w,
        short		drw_h
        )
{
	ENTER();
    SMIPtr pSmi = SMIPTR(pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    CARD32 dcr40;
    int hstretch, vstretch;


    dcr40 = READ_DCR(pHw, DCR40) & ~0x00003FFF;

    switch (id)
    {
        case FOURCC_YV12:
        case FOURCC_I420:
        case FOURCC_YUY2:
            dcr40 |= 0x3;  
            break;

        case FOURCC_RV16:
            dcr40 |= 0x1;
            break;

        case FOURCC_RV32:
            dcr40 |= 0x2;
            break;
    }


    if (drw_w > vid_w) 	/*  Horizontal Stretch */
    {
        hstretch = 4096  * (vid_w - 1)/ (drw_w -1);
        //	dcr40 |= 1 << 8;
    }
    else				/*  Horizontal Shrink */
    {
        hstretch = 4096 * (drw_w - 1)/ (vid_w - 1); 
        hstretch = hstretch | 0x8000;
    }

    if (drw_h > vid_h)	/* Vertical Stretch */
    {
        vstretch = 4096 * (vid_h - 1)/ (drw_h - 1);
        //	dcr40 |= 1 << 9; 
    }
    else				/* Vertical Shrink */
    {
        vstretch = 4096 * (drw_h - 1)/ (vid_h - 1);
        vstretch = vstretch |0x8000;
    }
#if 0
    SMI_WaitForSync(pScrn);
#endif

    /* Set Color Key Enable bit */

    WRITE_DCR(pHw, DCR00, READ_DCR(pHw, DCR00) | (1 << 9));
    /*
       if (!pSmi->IsSecondary){
       WRITE_DPR(pSmi, 0x40, 0);
       WRITE_DPR(pSmi, 0x44, 0);
       }else{
       WRITE_DPR(pSmi, 0x40, (pSmi->videoRAMBytes/16<<4));
       WRITE_DPR(pSmi, 0x44, (pSmi->videoRAMBytes/16<<4));
       }
     */
    //WRITE_DCR(pSmi, DCR40, dcr40 | (1 << 2));
    WRITE_DCR(pHw, DCR50, (dstBox->x1) | (dstBox->y1 << 16));
    WRITE_DCR(pHw, DCR54, (dstBox->x2) | (dstBox->y2 << 16));
    WRITE_DCR(pHw, DCR44, offset + ((x1 >> 16) << 1) + (y1 >> 16 ) * pitch);
    //WRITE_DCR(pHw, DCR44, offset + ((x1 >> 16) << 1));

    WRITE_DCR(pHw, DCR48, (pitch) | ((pitch) << 16));
    WRITE_DCR(pHw, DCR4C, (offset + (pitch * height))) ;
    //WRITE_DCR(pSmi, DCR58, ((vstretch & 0xffff) << 16) | (hstretch & 0xffff));
    WRITE_DCR(pHw, DCR58, ((vstretch & 0xfff) << 16) | (hstretch & 0xfff));
    WRITE_DCR(pHw, DCR5C, 0x00000000);
    WRITE_DCR(pHw, DCR60, 0x00EDEDED);

    WRITE_DCR(pHw, DCR40, dcr40 | (1 << 2) /*| ((videoInterpolation & 0x3)<< 8)*/);
    /*
       ErrorF("SMI 510 Video Debug Parameters\n");
       ErrorF("offset = %8X, pitch = %8X, width = %8X, height = %8X\n", offset, pitch, width, height);
       ErrorF("drw_w = %8X, vid_w = %8X, drw_h = %8X, vid_h = %8X\n", drw_w, vid_w, drw_h, vid_h);
       ErrorF("dstBox  x1 = %8X, y1 = %8X, x2 = %8X, y2 = %8X\n", dstBox->x1, dstBox->y1, dstBox->x2, dstBox->y2);
       ErrorF("DCR40 = %8X\n", READ_DCR(pSmi, DCR40));
       ErrorF("DCR50 = %8X\n", READ_DCR(pSmi, DCR50));
       ErrorF("DCR54 = %8X\n", READ_DCR(pSmi, DCR54));
       ErrorF("DCR44 = %8X\n", READ_DCR(pSmi, DCR44));
       ErrorF("DCR48 = %8X\n", READ_DCR(pSmi, DCR48));
       ErrorF("DCR4C = %8X\n", READ_DCR(pSmi, DCR4C));
       ErrorF("DCR58 = %8X\n", READ_DCR(pSmi, DCR58));
       ErrorF("DCR5C = %8X\n", READ_DCR(pSmi, DCR5C));
     */
     LEAVE();
}

static void 
SMI_DisplayVideo0750_CSC(
        ScrnInfoPtr pScrn, 
        int id, 
        int offset, 
        short width, 
        short height, 
        int pitch, 
        int x1, 
        int y1, 
        int x2, 
        int y2, 
        BoxPtr dstBox, 
        short vid_w, 
        short vid_h, 
        short drw_w, 
        short drw_h,
        RegionPtr clipboxes
        )
{
    //return;
    ENTER();
    DWORD origSrcDimX, origSrcDimY, origDestDimX, origDestDimY;
    DWORD ScaleXn, ScaleXd, ScaleYn, ScaleYd;
    DWORD SrcTn, SrcTd, SrcLn, SrcLd;
    DWORD SrcRn, SrcRd, SrcBn, SrcBd;
    DWORD SrcDimX, SrcDimY, DestDimX, DestDimY;
    DWORD SrcFormat, DstFormat, HFilter, VFilter, byOrder;
    DWORD SrcYBase, SrcUBase, SrcVBase, SrcYPitch, SrcUVPitch;
    DWORD DestBase, DestPitch, DestHeight, DestWidth, DestBpp;
    DWORD SrcBpp = 0, SrcLnAdd = 0;

    SMIPtr pSmi = SMIPTR(pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    CARD32 crt_fb;
    BoxPtr pbox = REGION_RECTS(clipboxes);
    int i, nbox = REGION_NUM_RECTS(clipboxes);
    xRectangle *rects;
    CARD32 CSC_Control ;



    if(!pScrn->vtSema) return;

    if(!pSmi->IsSecondary)
    {
        crt_fb = READ_DCR(pHw, 0xC) & 0x03FFFFFF;
    }
    else
    {
        crt_fb = READ_DCR(pHw, 0x204) & 0x03FFFFFF;
    }
#if 1
    SrcYBase = offset+crt_fb;
    SrcUBase = SrcYBase;
    SrcVBase = SrcYBase;
    //	DestBase = crt_fb;
    SrcYPitch = pitch;
    SrcUVPitch = SrcYPitch;	 
#endif

    DestBase = crt_fb;

    if (pSmi->Bpp == 3)
    {
        DestPitch = pSmi->Stride;
    }
    else
    {
        DestPitch = pSmi->Stride * pSmi->Bpp;

    }
    DestBpp = pSmi->Bpp;

    SrcFormat = 0; 
    byOrder = 0;

    origSrcDimX = vid_w;
    origSrcDimY = vid_h;

    origDestDimX = drw_w;
    origDestDimY = drw_h;

    ScaleXn = (origSrcDimX-1)/(origDestDimX-1);
    ScaleXd = ((origSrcDimX-1)<<13)/(origDestDimX-1) - (ScaleXn<<13);

    ScaleYn = (origSrcDimY-1)/(origDestDimY-1);
    ScaleYd = ((origSrcDimY-1)<<13)/(origDestDimY-1) - (ScaleYn<<13);  

#if 0
    /* fedora 10 said ALLOCATE_LOCAL is poisoned */
    rects = ALLOCATE_LOCAL(nbox * sizeof(xRectangle));
#else
    rects = xalloc(nbox * sizeof(xRectangle));	
#endif
    for(i = 0; i < nbox; i++, pbox++) {  
        rects[i].x = pbox->x1;
        rects[i].y = pbox->y1;
        rects[i].width = pbox->x2 - pbox->x1;
        rects[i].height = pbox->y2 - pbox->y1;

        DestHeight = rects[i].height;
        DestWidth = rects[i].width;

        switch(id)
        {
            case FOURCC_YV12:
                SrcFormat = 2;
                byOrder = 0;
                SrcLnAdd = 0;

                SrcYPitch = pitch;
                SrcUVPitch = SrcYPitch/2;

                SrcYBase = offset+crt_fb;	
                SrcVBase = SrcYBase + SrcYPitch*height;
                SrcUBase = SrcVBase + SrcUVPitch*height/2;
                DEBUG("SMI_DisplayVideo0750_CSC(%d)1:\n",  __LINE__);
                break;

            case FOURCC_I420:
                SrcFormat = 2;
                byOrder = 0;
                SrcLnAdd = 0;

                SrcYPitch = pitch;
                SrcUVPitch = SrcYPitch/2;

                SrcYBase = offset+crt_fb;	
                SrcUBase = SrcYBase + SrcYPitch*height;
                SrcVBase = SrcUBase + SrcUVPitch*height/2;
                DEBUG("SMI_DisplayVideo0750_CSC(%d)2:\n",  __LINE__);
                break;

            case FOURCC_YUY2:
            case FOURCC_RV16:
            case FOURCC_RV32: 
                SrcFormat = 0;
                byOrder = 0;
                SrcBpp = 16/8;
                SrcYBase = offset+crt_fb;
                SrcLnAdd = 0;
                DEBUG("SMI_DisplayVideo0750_CSC(%d)3:\n",  __LINE__);			
                break;
        }

        switch (DestBpp)
        {
            case 2:
                DstFormat = 0;
                break;
            case 4:
                DstFormat = 1;
                break;
            default: 
                return;
                break;		

        }

        HFilter = 1;
        VFilter = 1;
        SrcLn = (rects[i].x - dstBox->x1)*(vid_w-1)/(drw_w-1);
        SrcLd = ((rects[i].x -  dstBox->x1)<<13)*1.0*(vid_w-1)/(drw_w-1) - (SrcLn<<13);

        SrcRn = (rects[i].x+rects[i].width  - dstBox->x1)*(vid_w-1)/(drw_w-1);
        SrcRd = ((rects[i].x+rects[i].width  - dstBox->x1)<<13)*1.0*(vid_w-1)/(drw_w-1) - (SrcRn<<13);

        SrcTn = (rects[i].y-dstBox->y1)*(vid_h-1) / (drw_h-1);
        SrcTd = ((rects[i].y-dstBox->y1)<<13)*1.0*(vid_h-1) / (drw_h-1) -(SrcTn<<13);

        SrcBn = (rects[i].y+rects[i].height  - dstBox->y1)*(vid_h-1)/(drw_h-1);
        SrcBd = ((rects[i].y+rects[i].height  - dstBox->y1)<<13)*1.0*(vid_h-1)/(drw_h-1) - (SrcBn<<13);

        /*SrcLn = (rects[i].x - dstBox->x1)*(origSrcDimX-1)/(origDestDimX-1);
          SrcLd = ((rects[i].x -  dstBox->x1)<<13)*(origSrcDimX-1)/(origDestDimX-1) - (SrcLn<<13);

          SrcRn = (rects[i].x+rects[i].width - 1 - dstBox->x1)*(vid_w-1)/(drw_w-1);
          SrcRd = ((rects[i].x+rects[i].width - 1 - dstBox->x1)<<13)*(vid_w-1)/(drw_w-1) - (SrcRn<<13);

          SrcTn = (rects[i].y-dstBox->y1)*(vid_h-1) / (drw_h-1);
          SrcTd = ((rects[i].y-dstBox->y1)<<13)*(vid_h-1) / (drw_h-1) -(SrcTn<<13);

          SrcBn = (rects[i].y+rects[i].height - 1 - dstBox->y1)*(vid_h-1)/(drw_h-1);
          SrcBd = ((rects[i].y+rects[i].height - 1 - dstBox->y1)<<13)*(vid_h-1)/(drw_h-1) - (SrcBn<<13);*/

        SrcDimX = (SrcRd == 0) ? (SrcRn - SrcLn + 1) : (SrcRn - SrcLn + 2);
        SrcDimY = (SrcBd == 0) ? (SrcBn - SrcTn + 1) : (SrcBn - SrcTn + 2);

        DestDimX = rects[i].width;
        DestDimY = rects[i].height;

#if 0
        xf86DrvMsg("", X_INFO, "T-Bag:Test, SrcTn is %d, SrcTd is %d  SrcLn is %d SrcLd is %d SrcRn is %d vid_h is %d drw_w is %d drw_h is %d width is %hd height is %hd FUNC: %s\n", x1,y1,x2,y2,vid_w,vid_h,drw_w,drw_h,width,height,__FUNCTION__);
#endif


        //WaitForNotBusy();

        WRITE_DPR (pHw, 0xCC, 0x0);
        WRITE_DPR (pHw, 0xD0, (SrcLn<<16)|SrcLd);
        WRITE_DPR (pHw, 0xD4, SrcTn<<16 | SrcTd);
        WRITE_DPR (pHw, 0xE0, (SrcDimX)<<16 | SrcDimY);
        //	WRITE_DPR (pHw, 0xE0, (624)<<16 | SrcDimY);
        WRITE_DPR (pHw, 0xE4, (SrcYPitch >> 4)<<16 |(SrcUVPitch >> 4) );
        WRITE_DPR (pHw, 0xE8, (rects[i].x)<<16 | rects[i].y);	
        WRITE_DPR (pHw, 0xEC, DestDimX<<16 | DestDimY);	
        WRITE_DPR (pHw, 0xF0, (DestPitch >> 4)<<16 | DestHeight);
        WRITE_DPR (pHw, 0xF4,  (ScaleXn<<13 | ScaleXd)<<16 |(ScaleYn<<13 | ScaleYd));
        //WRITE_DPR(pSmi, 0xF4, ( ((origSrcDimX-1)<<13)/(origDestDimX-1))<<16 |((origSrcDimY-1)<<13)/(origDestDimY-1));


        /* WRITE_DPR(pSmi, 0xC8,  SrcYBase + ((((x1 >> 16) << 1)  + (y1 >> 16) * SrcYPitch) + 15) & ~15);
           WRITE_DPR(pSmi, 0xD8,  SrcUBase + ((((x1 >> (16 + 1)) << 1) + (y1 >> (16 + 1)) * SrcUVPitch) + 15) & ~15   );
           WRITE_DPR(pSmi, 0xDC,  SrcVBase + ((((x1 >> (16 + 1)) << 1) + (y1 >> (16 + 1)) * SrcUVPitch) + 15) & ~15 );*/
        WRITE_DPR (pHw, 0xC8,  SrcYBase);
        WRITE_DPR (pHw, 0xD8,  SrcUBase);
        WRITE_DPR (pHw, 0xDC,  SrcVBase);

        //	WRITE_DPR(pSmi, 0xD8,  SrcUBase );
        //        WRITE_DPR(pSmi, 0xDC,  SrcVBase );



        WRITE_DPR (pHw, 0xF8,  DestBase);

        CSC_Control = 1<<31 | SrcFormat<<28 | DstFormat<<26 | HFilter<<25 | VFilter<<24 |byOrder<<23;
        CSC_Control &= 0x8FFFFFFF;
        SMI_CSC_Start(pSmi, CSC_Control);

    }
#if 0
    DEALLOCATE_LOCAL(rects);	/* fedora10 said DEALLOCATE_LOCAL is poisoned */
#else
    xfree(rects);
#endif
    LEAVE();
}


static void
SMI_DisplayVideo0750(
        ScrnInfoPtr	pScrn,
        int			id,
        int			offset,
        short		width,
        short		height,
        int			pitch,
        int			x1,
        int			y1,
        int			x2,
        int			y2,
        BoxPtr		dstBox,
        short		vid_w,
        short		vid_h,
        short		drw_w,
        short		drw_h
#if (SMI_RANDR==1)
	,  xf86CrtcPtr crtc
#endif	
)
{
	ENTER();
    SMIPtr pSmi = SMIPTR(pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    CARD32 dcr40;
    int hstretch, vstretch;
    int scale_factor = 0;
    uint32_t tmp1, tmp2, lines;
	int offset_phy2vir = 0;
    xf86DrvMsg("", X_INFO, "vid_w is %d, drw_w is %d\n", vid_w, drw_w);

    dcr40 = READ_DCR(pHw, DCR40) & ~0x00003FFF;

    dcr40 |= (1<<2);
    switch (id)
    {
        case FOURCC_YV12:
        case FOURCC_I420:
        case FOURCC_YUY2:
            dcr40 |= 0x3;  
            dcr40 |= 1<<13;
            break;
        case FOURCC_RV16:
            dcr40 |= 0x1;
            break;
        case FOURCC_RV32:
            dcr40 |= 0x2;
            break;
    }
#if 0// (SMI_RANDR)
   if (crtc->rotation & (RR_Rotate_0 | RR_Rotate_180)) 
#endif
    {
        lines = height;
        if (drw_h == vid_h) {
            scale_factor = 0; 
            vstretch = 1 << 12;
            scale_factor |= (vstretch<<16);
        } else if (drw_h > vid_h) {
#if 0//(SMI_RANDR)        
            dcr40 |=(1<<8)|(1<<9); 
#endif
            scale_factor = 0; 
            vstretch = vid_h * (1 << 12) / drw_h;
            vstretch -= (drw_h + vid_h - 1) / vid_h;
            scale_factor|= (vstretch<<16);
        } else {
            scale_factor |= (1<<31);
            vstretch = drw_h * (1 << 12) / vid_h;
            vstretch = vstretch < (1 << 10) ? (1 << 10) : vstretch;
            scale_factor |= (vstretch<<16);
        }

        if (drw_w == vid_w) {
            scale_factor |=0;
            hstretch = 1 << 12;
            scale_factor |=hstretch;
        } else if (drw_w > vid_w) {
#if 0//(SMI_RANDR)
            dcr40 |=(1<<8)|(1<<9); 
#endif
            scale_factor |=0; 
            hstretch = vid_w * (1 << 12) / drw_w;
            hstretch -= (drw_w + vid_w - 1) / vid_w;
            scale_factor|=hstretch;
        } else {
            scale_factor |=(1<<15);
            hstretch = drw_w * (1 << 12) / vid_w;
            hstretch = hstretch < (1 << 10) ? (1 << 10) : hstretch;
            scale_factor |=hstretch;
        }
    }
#if 0//(SMI_RANDR)
    else if (crtc->rotation & (RR_Rotate_90 | RR_Rotate_270)) {
        lines = width;
        if (drw_w == vid_w) {
            scale_factor |= 0; 
            vstretch = 1 << 12;
            scale_factor |= (vstretch<<16);
        } else if (drw_w > vid_w) {
            scale_factor  |= 0;  
            vstretch = vid_w * (1 << 12) / drw_w;
            vstretch -= (drw_w + vid_w - 1) / vid_w;
            scale_factor|= (vstretch<<16);
        } else {
            scale_factor |= (1<<31); 
            vstretch = drw_w * (1 << 12) / vid_w;
            vstretch = vstretch < (1 << 10) ? (1 << 10) : vstretch;
            scale_factor|= (vstretch<<16);
        }

        if (drw_h == vid_h) {
            scale_factor |= 0; 
            hstretch = 1 << 12;
            scale_factor |= (hstretch);
        } else if (drw_h > vid_h) {
            scale_factor |= 0; 
            hstretch = vid_h * (1 << 12) / drw_h;
            hstretch -= (drw_h + vid_h - 1) / vid_h;
            scale_factor |= ((hstretch));
        } else {
            scale_factor |= (1<<15);
            hstretch = drw_h * (1 << 12) / vid_h;
            hstretch = hstretch < (1 << 10) ? (1 << 10) : hstretch;
            scale_factor |= (hstretch);
        }
    }

    switch (crtc->rotation) {
        case RR_Rotate_0:
            break;
        case RR_Rotate_90:
            tmp1 = dstBox->x1 - crtc->x;
            tmp2 = dstBox->x2 - crtc->x;
            dstBox->x1 = dstBox->y1 - crtc->y;
            dstBox->x2 = dstBox->y2 - crtc->y;
            dstBox->y1 = crtc->mode.VDisplay - tmp2 - 1;
            dstBox->y2 = crtc->mode.VDisplay - tmp1 - 1;
            dstBox->y1 = (dstBox->y1 < 0)?0:dstBox->y1;
            dstBox->y2 = (dstBox->y2 < 0)?0:dstBox->y2;
            break;
        case RR_Rotate_270:
            tmp1 = dstBox->y1 - crtc->y;
            tmp2 = dstBox->y2 - crtc->y;
            dstBox->y1 = dstBox->x1 - crtc->x;
            dstBox->y2 = dstBox->x2 - crtc->x;
            dstBox->x1 = crtc->mode.HDisplay - tmp2 - 1;
            dstBox->x2 = crtc->mode.HDisplay - tmp1 - 1;
            dstBox->x1 = (dstBox->x1 < 0)?0:dstBox->x1;
            dstBox->x2 = (dstBox->x2 < 0)?0:dstBox->x2;
            break;
        case RR_Rotate_180:
            tmp1 = dstBox->x1 - crtc->x;
            tmp2 = dstBox->y1 - crtc->y;
            dstBox->x1 = crtc->mode.HDisplay - (dstBox->x2 - crtc->x) - 1;
            dstBox->x2 = crtc->mode.HDisplay - tmp1 - 1;
            dstBox->y1 = crtc->mode.VDisplay - (dstBox->y2 - crtc->y) - 1;
            dstBox->y2 = crtc->mode.VDisplay - tmp2 - 1;
            dstBox->x1 = (dstBox->x1 < 0)?0:dstBox->x1;
            dstBox->x2 = (dstBox->x2 < 0)?0:dstBox->x2;
            dstBox->y1 = (dstBox->y1 < 0)?0:dstBox->y1;
            dstBox->y2 = (dstBox->y2 < 0)?0:dstBox->y2;
            break;
        default:
            break;
    }
#endif
    //wait_for_not_busy_502( pSmi);

xf86Msg(X_INFO,"ilena, dstBox->y1 =[%d], offset_phy2vir=[%d]\n", dstBox->y1, offset_phy2vir);
    /* Set Color Key Enable bit */
    WRITE_DCR(pHw, DCR00, READ_DCR(pHw, DCR00) | (1 << 9));
    WRITE_DCR(pHw, DCR50, (dstBox->x1) | (dstBox->y1 << 16));
    WRITE_DCR(pHw, DCR54, ( (dstBox->y2 - 1)<<16) |(dstBox->x2 - 1)); 

    WRITE_DCR(pHw, DCR44,((1<<31)|offset));
    WRITE_DCR(pHw, DCR48, (pitch<<16) |(pitch));
    WRITE_DCR(pHw, DCR4C, (offset + pitch*(lines - 1)));
    WRITE_DCR(pHw, DCR58, scale_factor);    //ilena: care of this . so that the screen can play a movie oversize.
#if 1
    WRITE_DCR(pHw, DCR5C, 0x00000000);
    WRITE_DCR(pHw, DCR60, 0x00EDEDED);
#endif    
    WRITE_DCR(pHw, DCR40, dcr40);
    LEAVE();
}

static void
SMI_BlockHandler(BLOCKHANDLER_ARGS_DECL)
{
//	ENTER();
//    ScreenPtr	pScreen = screenInfo.screens[i];
//    ScrnInfoPtr	pScrn	= xf86Screens[i];
    SCREEN_PTR(arg);
    ScrnInfoPtr	pScrn	= xf86ScreenToScrn(pScreen);
    SMIPtr		pSmi    = SMIPTR(pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    SMI_PortPtr pPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;

    pScreen->BlockHandler = pSmi->BlockHandler;
//  (*pScreen->BlockHandler)(i, blockData, pTimeout, pReadMask);
    (*pScreen->BlockHandler)(BLOCKHANDLER_ARGS);
    pScreen->BlockHandler = SMI_BlockHandler;

    if (pPort->videoStatus & TIMER_MASK)
    {
        UpdateCurrentTime();
        if (pPort->videoStatus & OFF_TIMER)
        {
            if (pPort->offTime < currentTime.milliseconds)
            {
                if(!SMI_OLDLYNX(pSmi->pHardware->devId))
                {
                    WRITE_DCR(pHw, DCR40, READ_DCR(pHw, DCR40) & ~0x00000004);
                }
                else
                {
                    WRITE_VPR(pHw, 0x00, READ_VPR(pHw, 0x00) & ~0x00000008);
                }
                pPort->videoStatus = FREE_TIMER;
                pPort->freeTime = currentTime.milliseconds + FREE_DELAY;
            }
        }
        else
        {
            if (pPort->freeTime < currentTime.milliseconds)
            {
                xf86FreeOffscreenArea(pPort->area);
                pPort->area = NULL;
            }
            pPort->videoStatus = 0;
        }
    }
//	LEAVE();
}


/******************************************************************************\
 **																			  **
 **				 O F F S C R E E N   M E M O R Y   M A N A G E R			  **
 **																			  **
 \******************************************************************************/

static void
SMI_InitOffscreenImages(
        ScreenPtr	pScreen
        )
{
	ENTER();
    XF86OffscreenImagePtr offscreenImages;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SMIPtr pSmi = SMIPTR(pScrn);
    SMI_PortPtr pPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;

    offscreenImages = xalloc(sizeof(XF86OffscreenImageRec));
    if (offscreenImages == NULL)
    {
        LEAVE();
    }

    offscreenImages->image = SMI_VideoImages;
    offscreenImages->flags = VIDEO_OVERLAID_IMAGES
        | VIDEO_CLIP_TO_VIEWPORT;
    offscreenImages->alloc_surface = SMI_AllocSurface;
    offscreenImages->free_surface = SMI_FreeSurface;
    offscreenImages->display = SMI_DisplaySurface;
    offscreenImages->stop = SMI_StopSurface;
    offscreenImages->getAttribute = SMI_GetSurfaceAttribute;
    offscreenImages->setAttribute = SMI_SetSurfaceAttribute;
    offscreenImages->max_width = pSmi->lcdWidth;
    offscreenImages->max_height = pSmi->lcdHeight;
    if (!pPort->I2CDev.SlaveAddr) {
        offscreenImages->num_attributes = nElems(SMI_VideoAttributes);
        offscreenImages->attributes = SMI_VideoAttributes; 
    } else {
        offscreenImages->num_attributes = 
            nElems(SMI_VideoAttributesSAA711x);
        offscreenImages->attributes = SMI_VideoAttributesSAA711x; 
    }
    xf86XVRegisterOffscreenImages(pScreen, offscreenImages, 1);
	LEAVE();
}

static FBAreaPtr
SMI_AllocateMemory(
        ScrnInfoPtr	pScrn,
        FBAreaPtr	area,
        int			numLines
        )
{
	ENTER();
    ScreenPtr pScreen = screenInfo.screens[pScrn->scrnIndex];


    if (area != NULL)
    {
        if ((area->box.y2 - area->box.y1) >= numLines)
        {
            LEAVE(area);
        }

        if (xf86ResizeOffscreenArea(area, pScrn->displayWidth, numLines))
        {
            LEAVE(area);
        }

        xf86FreeOffscreenArea(area);
    }

    area = xf86AllocateOffscreenArea(pScreen, pScrn->displayWidth, numLines, 0,
            NULL, NULL, NULL);

    if (area == NULL)
    {
        int maxW, maxH;

        xf86QueryLargestOffscreenArea(pScreen, &maxW, &maxH, 0,
                FAVOR_WIDTH_THEN_AREA, PRIORITY_EXTREME);

	DEBUG("QueryLargestOffscreenArea maxW=%d maxH=%d displayWidth=%d numlines=%d\n",
	       maxW, maxH, pScrn->displayWidth, numLines);
		if ((maxW < pScrn->displayWidth) || (maxH < numLines))
		{
			LEAVE(NULL);
		}
        xf86PurgeUnlockedOffscreenAreas(pScreen);
        area = xf86AllocateOffscreenArea(pScreen, pScrn->displayWidth, numLines,
                0, NULL, NULL, NULL);
    }

    DEBUG("area = %p\n", area);
    LEAVE(area);
}

static uint32_t
CopyYV12Planar(unsigned char *src1, unsigned char *src2,
		   unsigned char *src3, unsigned char *dst,
		   int src1Pitch, int src23Pitch, int dstPitch,
		   int height, int width, Rotation rot)
{
    int j = height;
    int i = width; 
    unsigned char *tmp;
    uint32_t sum = 0;
    unsigned char *sysbuffer;
    unsigned char *vram;

    ENTER();

    vram = dst;
    switch (rot) { 
        case RR_Rotate_0:
        /* copy 1 data */
            while (j -- > 0) {
                memcpy(dst, src1, width);
                src1 += src1Pitch;
                dst += dstPitch;
                sum += dstPitch;
            }
            /* copy 2 data */
            j = height / 2;
            while (j -- > 0) {
                memcpy(dst, src2, width / 2);
                src2 += src23Pitch;
                dst += dstPitch / 2;
                sum += dstPitch / 2;
            }
            /* copy 3 data */
            j = height / 2;
            while (j -- > 0) {
                memcpy(dst, src3, width / 2);
                src3 += src23Pitch;
                dst += dstPitch / 2;
                sum += dstPitch / 2;
            }
            LEAVE(sum);
        case RR_Rotate_90:
            sum = dstPitch * width * 2;
            sysbuffer = (unsigned char*)malloc(sum);
            dst = sysbuffer;
            for (i = 0; i < height; i++) {
                tmp = src1;
                for (j = 0; j < width; j++) {
                    dst[(i) + ((width - j - 1) * dstPitch)] = *tmp++;
                }
                src1 += src1Pitch;
            }
            dst += dstPitch * width;
            for (i = 0; i < (height/2); i++) {
                tmp = src2;
                for (j = 0; j < (width/2); j++) {
                    dst[(i) + (((width/2) - j - 1) * (dstPitch / 2))] = *tmp++;
                }
                src2 += src23Pitch;
            }
            dst += dstPitch / 2 * width / 2;
            for (i = 0; i < (height/2); i++) {
                tmp = src3;
                for (j = 0; j < (width/2); j++) {
                    dst[(i) + (((width/2) - j - 1) * (dstPitch / 2))] = *tmp++;
                }
                src3 += src23Pitch;
            }
            break;
        case RR_Rotate_270:
            sum = dstPitch * width * 2;
            sysbuffer = (unsigned char*)malloc(sum);
            dst = sysbuffer;
            for (i = 0; i < height; i++) {
                tmp = src1;
                for (j = 0; j < width; j++) {
                    dst[(height - i - 1) + j * dstPitch] = *tmp++;
                }
                src1 += src1Pitch;
            }
            dst += dstPitch * width;
            for (i = 0; i < (height/2); i++) {
                tmp = src2;
                for (j = 0; j < (width/2); j++) {
                    dst[(height / 2 - i - 1) + j * (dstPitch / 2)] = *tmp++;
                }
                src2 += src23Pitch;
            }
            dst += dstPitch / 2 * width / 2;
            for (i = 0; i < (height/2); i++) {
                tmp = src3;
                for (j = 0; j < (width/2); j++) {
                    dst[(height / 2 - i - 1) + j * (dstPitch / 2)] = *tmp++;
                }
                src3 += src23Pitch;
            }
            break;
        case RR_Rotate_180:
            sum = dstPitch * height * 2;
            sysbuffer = (unsigned char*)malloc(sum);
            dst = sysbuffer;
            for (i = 0; i < height; i++) {
                tmp = src1;
                for (j = 0; j < width; j++) {
                    dst[(width - j - 1) + 
                        (height - i - 1) * dstPitch] = *tmp++;
                }
                src1 += src1Pitch;
            }
            dst += dstPitch * height;
            for (i = 0; i < (height/2); i++) {
                tmp = src2;
                for (j = 0; j < (width/2); j++) {
                    dst[(width / 2 - j - 1) + 
                        (height / 2 - i - 1) * (dstPitch / 2)] = *tmp++;
                }
                src2 += src23Pitch;
            }
            dst += dstPitch / 2 * height / 2;
            for (i = 0; i < (height/2); i++) {
                tmp = src3;
                for (j = 0; j < (width/2); j++) {
                    dst[(width / 2 - j - 1) + 
                        (height / 2 - i - 1) * (dstPitch / 2)] = *tmp++;
                }
                src3 += src23Pitch;
            }
            break;
        default:
            DEBUG("Wrong RR rotation!\n");
            LEAVE(0);
    }
    memcpy(vram, sysbuffer, sum);
    free(sysbuffer);
    LEAVE(sum);
}

static uint32_t
CopyYV12ToPacked(unsigned char *src1, unsigned char *src2,
                    unsigned char *src3, unsigned char *dst,
                    int src1Pitch, int src23Pitch, int dstPitch,
                    int height, int width, Rotation rot)
{
    uint32_t sum = 0, pitch;
    unsigned char *Y, *U, *V;
    ENTER();
    if (rot & RR_Rotate_0) {
        xf86XVCopyYUV12ToPacked(src1, src2, src3, dst, src1Pitch, src23Pitch,
                                    dstPitch, height, width);
        LEAVE(dstPitch * height * 2);
    } else if (rot & RR_Rotate_180) {
        pitch = width;
        Y = (unsigned char *)malloc(pitch * height * 2);
        U = Y + height * pitch;
        V = U + (height / 2) * (pitch / 2);
        sum = dstPitch * height * 2;
    } else if (rot & (RR_Rotate_90 | RR_Rotate_270)) {
        pitch = height;
        Y = (unsigned char *)malloc(pitch * width * 2);
        U = Y + width * pitch;
        V = U + (width / 2) * (pitch / 2);
        sum = dstPitch * width * 2;
    }
    CopyYV12Planar(src1, src2, src3, Y, src1Pitch, src23Pitch, pitch,
                    height, width, rot);
    if (rot & RR_Rotate_180) 
        xf86XVCopyYUV12ToPacked(Y, U, V, dst, pitch, pitch / 2, dstPitch,
                height, width);
    else if (rot & (RR_Rotate_90 | RR_Rotate_270))
        xf86XVCopyYUV12ToPacked(Y, U, V, dst, pitch, pitch / 2, dstPitch,
                width, height);
    free(Y);
    LEAVE(sum);
}


static void
CopyYV12ToVideoMem(
        unsigned char	*src1,
        unsigned char	*src2,
        unsigned char	*src3,
        unsigned char	*dst,
        int				src1Pitch,
        int				src23Pitch,
        int				dstPitch,
        int				height,
        int				width
        )
{
	ENTER();

	int total_size;
    int adjustsrc1Pitch, adjustsrc23Pitch;
    int gap1, gap23;
    uint32_t sum = 0;

    if(!src23Pitch%16)
    {
      total_size = (height * width * 3)>>1;
      memcpy(dst,src1,total_size);
    }
    else
      {
        //ilena: align the YPitch so that UVPitch can be devided by 16.
        //add some empty value to align every line's end.
        adjustsrc1Pitch = (src1Pitch + 31) & ~31;
        adjustsrc23Pitch = adjustsrc1Pitch/2;
        gap1 = adjustsrc1Pitch-src1Pitch;
        gap23 = adjustsrc23Pitch - src23Pitch;
        int j = height;
        int i = width;        
        while(j-->0){
          memcpy(dst,src1,width);
          dst += width+gap1;
          src1 += src1Pitch;
          sum += width+gap1;
        }
        j = height/2;
        while(j-->0){
          memcpy(dst, src2, width/2);
          dst += width/2+gap23;
          src2 += src23Pitch;
          sum += width/2+gap23;
        }
        j = height/2;
        while(j-->0){
          memcpy(dst, src3, width/2);
          dst += width/2+gap23;
          src3 += src23Pitch;
          sum += width/2+gap23;
        }               
    }
	LEAVE();
}

static int
SMI_AllocSurface(
        ScrnInfoPtr		pScrn,
        int				id,
        unsigned short	width,
        unsigned short	height,
        XF86SurfacePtr	surface
        )
{
    SMIPtr pSmi = SMIPTR(pScrn);
    int numLines, pitch, fbPitch, bpp;
    SMI_OffscreenPtr ptrOffscreen;
    FBAreaPtr area;

	ENTER();

    if ((width > pSmi->lcdWidth) || (height > pSmi->lcdHeight))
    {
        return(BadAlloc);
    }

    if (pSmi->Bpp == 3)
    {
        fbPitch = pSmi->Stride;
    }
    else
    {
        fbPitch = pSmi->Stride * pSmi->Bpp;
    }

    width = (width + 1) & ~1;
    switch (id)
    {
        case FOURCC_YV12:
        case FOURCC_I420:
        case FOURCC_YUY2:
        case FOURCC_RV15:
        case FOURCC_RV16:
            bpp = 2;
            break;

        case FOURCC_RV24:
            bpp = 3;
            break;

        case FOURCC_RV32:
            bpp = 4;
            break;

        default:
            return(BadAlloc);
    }
    pitch = (width * bpp + 15) & ~15;

    numLines = ((height * pitch) + fbPitch - 1) / fbPitch;

    area = SMI_AllocateMemory(pScrn, NULL, numLines);
    if (area == NULL)
    {
        return(BadAlloc);
    }

    surface->pitches = xalloc(sizeof(int));
    if (surface->pitches == NULL)
    {
        xf86FreeOffscreenArea(area);
        return(BadAlloc);
    }
    surface->offsets = xalloc(sizeof(int));
    if (surface->offsets == NULL)
    {
        xfree(surface->pitches);
        xf86FreeOffscreenArea(area);
        return(BadAlloc);
    }

    ptrOffscreen = xalloc(sizeof(SMI_OffscreenRec));
    if (ptrOffscreen == NULL)
    {
        xfree(surface->offsets);
        xfree(surface->pitches);
        xf86FreeOffscreenArea(area);
        return(BadAlloc);
    }

    surface->pScrn = pScrn;
    surface->id = id;
    surface->width = width;
    surface->height = height;
    surface->pitches[0] = pitch;
    surface->offsets[0] = area->box.y1 * fbPitch;
    surface->devPrivate.ptr = (pointer) ptrOffscreen;

    ptrOffscreen->area = area;
    ptrOffscreen->isOn = FALSE;

    LEAVE(Success);
}

static int
SMI_FreeSurface(
        XF86SurfacePtr	surface
        )
{
    SMI_OffscreenPtr ptrOffscreen = (SMI_OffscreenPtr) surface->devPrivate.ptr;

	ENTER();

    if (ptrOffscreen->isOn)
    {
        SMI_StopSurface(surface);
    }

    xf86FreeOffscreenArea(ptrOffscreen->area);
    xfree(surface->pitches);
    xfree(surface->offsets);
    xfree(surface->devPrivate.ptr);

    LEAVE(Success);
}

static int
SMI_DisplaySurface(
        XF86SurfacePtr	surface,
        short			vid_x,
        short			vid_y,
        short			drw_x,
        short			drw_y,
        short			vid_w,
        short			vid_h,
        short			drw_w,
        short			drw_h,
        RegionPtr		clipBoxes
        )
{
    SMI_OffscreenPtr ptrOffscreen = (SMI_OffscreenPtr) surface->devPrivate.ptr;
    SMIPtr pSmi = SMIPTR(surface->pScrn);
    SMI_PortPtr pPort = pSmi->ptrAdaptor->pPortPrivates[0].ptr;
    INT32 x1, y1, x2, y2;
    BoxRec dstBox;
    xf86CrtcPtr crtc;
	ENTER();
    x1 = vid_x;
    x2 = vid_x + vid_w;
    y1 = vid_y;
    y2 = vid_y + vid_h;

    dstBox.x1 = drw_x;
    dstBox.x2 = drw_x + drw_w;
    dstBox.y1 = drw_y;
    dstBox.y2 = drw_y + drw_h;

    if (!SMI_ClipVideo(surface->pScrn, &dstBox, &x1, &y1, &x2, &y2, clipBoxes,
                surface->width, surface->height))
    {
        LEAVE(Success);
    }

    dstBox.x1 -= surface->pScrn->frameX0;
    dstBox.y1 -= surface->pScrn->frameY0;
    dstBox.x2 -= surface->pScrn->frameX0;
    dstBox.y2 -= surface->pScrn->frameY0;

    xf86XVFillKeyHelper(surface->pScrn->pScreen,
            pPort->Attribute[XV_COLORKEY], clipBoxes);

    if ((pSmi->pHardware->devId == SMI_MSOC)||(pSmi->pHardware->devId == SMI_MSOCE))
    {
        SMI_ResetVideo(surface->pScrn);
        SMI_DisplayVideo0750(surface->pScrn, surface->id, surface->offsets[0],
                surface->width, surface->height, surface->pitches[0], x1, y1, x2,
                y2, &dstBox, vid_w, vid_h, drw_w, drw_h
#if SMI_RANDR
                                        ,  crtc
#endif       
                                        );
    }	
    else
    {
        SMI_ResetVideo(surface->pScrn);
        SMI_DisplayVideo(surface->pScrn, surface->id, surface->offsets[0],
                surface->width, surface->height, surface->pitches[0], x1, y1, x2,
                y2, &dstBox, vid_w, vid_h, drw_w, drw_h);
    }


    ptrOffscreen->isOn = TRUE;
    if (pPort->videoStatus & CLIENT_VIDEO_ON)
    {
        REGION_EMPTY(surface->pScrn->pScreen, &pPort->clip);
        UpdateCurrentTime();
        pPort->videoStatus = FREE_TIMER;
        pPort->freeTime = currentTime.milliseconds + FREE_DELAY;
    }

    LEAVE(Success);
}

static int
SMI_StopSurface(
        XF86SurfacePtr	surface
        )
{
    SMI_OffscreenPtr ptrOffscreen = (SMI_OffscreenPtr) surface->devPrivate.ptr;

	ENTER();

    if (ptrOffscreen->isOn)
    {
        SMIPtr pSmi = SMIPTR(surface->pScrn);
        SMIHWPtr pHw = pSmi->pHardware;
        {
            WRITE_VPR(pHw, 0x00, READ_VPR(pHw, 0x00) & ~0x00000008);
        }

        ptrOffscreen->isOn = FALSE;
    }

    LEAVE(Success);
}

static int
SMI_GetSurfaceAttribute(
        ScrnInfoPtr	pScrn,
        Atom		attr,
        INT32		*value
        )
{
	ENTER();
    SMIPtr pSmi = SMIPTR(pScrn);

    LEAVE(SMI_GetPortAttribute(pScrn, attr, value,
                (pointer) pSmi->ptrAdaptor->pPortPrivates[0].ptr));
}

static int
SMI_SetSurfaceAttribute(
        ScrnInfoPtr	pScrn,
        Atom		attr,
        INT32		value
        )
{
	ENTER();
    SMIPtr pSmi = SMIPTR(pScrn);

    LEAVE(SMI_SetPortAttribute(pScrn, attr, value,
                (pointer) pSmi->ptrAdaptor->pPortPrivates[0].ptr));
}

    static void
SetKeyReg(SMIPtr pSmi, int reg, int value)
{
	ENTER();
	SMIHWPtr pHw = pSmi->pHardware;
	if(!SMI_OLDLYNX(pHw->devId))
    {
        /* We don't change the color mask, and we don't do brightness.  IF
         * they write to the colorkey register, we'll write the value to the
         * 501 colorkey register */
        if ((FPR04 == reg) && (pSmi->IsPrimary))	/* Only act on colorkey value writes */
        {
			/*
			 * now value format in cpu register view:
			 * MSB 0000 ABCD LSB
			 * if value written into sm502 by big-endian cpu,the value in 502
			 * video memory will be like below:
			 * HIGH CDAB 0000 LOW
			 * WRITE_DCR call mmio_out32,which will do swab32
			 * so the final data in sm502 vidmem will be just
			 * HIGH 0000 ABCD LOW
			 */
			DEBUG("monk:set DCR08 = %08x\n",value);
            WRITE_DCR(pHw, DCR08, value);	/* ColorKey register is DCR08 */
        }    
    }
    else
    {
        WRITE_VPR(pHw, reg, value);
    }
	LEAVE();
}


    static void
SMI_CSC_Start(SMIPtr pSmi, CARD32 CSC_Control)
{
	ENTER();
    CARD32 CSC_Control_Old; 
    SMIHWPtr pHw = pSmi->pHardware;
    while(1)
    {
        CSC_Control_Old =  READ_DPR(pHw, 0xFC);
        if(CSC_Control_Old & 0x80000000)
            continue;
        else
        {
            CSC_Control |= 1<<31;
            WRITE_DPR(pHw, 0xFC, CSC_Control);
            break;
        }
    }
    //CSC stop
    while(1)
    {
        CSC_Control_Old =  READ_DPR(pHw, 0xFC);
        if(!(CSC_Control_Old & 0x80000000))
            break;
    }
	LEAVE();
}


#else /* SMI_USE_VIDEO */
Bool SMI_Videoinit(ScreenPtr pScreen)
{
	ENTER();
	LEAVE(TRUE);}
#endif


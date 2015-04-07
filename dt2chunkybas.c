/*
 * dt2chunkybas
 *
 * Copyright (c) 2015 Chris Young
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <proto/datatypes.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#include <datatypes/pictureclass.h>


#define CHUNKGFX_WIDTH  64
#define CHUNKGFX_HEIGHT 48 /* or 44 */

ULONG *bitmap_from_datatype(char *filename)
{
	Object *dto;
	ULONG *bitmap = AllocVecTags(CHUNKGFX_WIDTH * CHUNKGFX_HEIGHT * 4, NULL);

	if((dto = NewDTObject(filename,
					DTA_GroupID, GID_PICTURE,
					PDTA_DestMode, PMODE_V43,
					PDTA_PromoteMask, TRUE,
					TAG_DONE))) {
		struct BitMapHeader *bmh;

		if(GetDTAttrs(dto, PDTA_BitMapHeader, &bmh, TAG_DONE))
		{
			IDoMethod(dto, PDTM_SCALE, CHUNKGFX_WIDTH, CHUNKGFX_HEIGHT, 0);

			if((DoDTMethod(dto, 0, 0, DTM_PROCLAYOUT, 0, 1)) == 0) {
				DisposeDTObject(dto); /*** FIX IN NS **/
				return NULL;
			}

			IDoMethod(dto, PDTM_READPIXELARRAY, bitmap,
				PBPAFMT_RGBA, CHUNKGFX_WIDTH * 4, 0, 0,
				bmh->bmh_Width, bmh->bmh_Height);
		}
		DisposeDTObject(dto);
	}

	return bitmap;
}

char check_char(ULONG *bitmap, int i, struct ColorMap *cm)
{
	ULONG pix_argb[4];
	LONG pix_pen[4];
	int p;

	/* The four pixels are:	*/
	pix_argb[0] = bitmap[i*2];
	pix_argb[1] = bitmap[(i*2)+1];
	pix_argb[2] = bitmap[(i*2)+CHUNKGFX_WIDTH];
	pix_argb[3] = bitmap[(i*2)+CHUNKGFX_WIDTH+1];

	for(p = 0; p < 4; p++) {
		pix_pen[p] = FindColor(cm,
						(pix_argb[p] & 0x00ff0000) << 8,
						(pix_argb[p] & 0x0000ff00) << 16,
						(pix_argb[p] & 0x000000ff) << 24,
						16);
	}

	printf("[%d][%d][%d][%d]\n", pix_pen[0], pix_pen[1], pix_pen[2], pix_pen[3]);
}

void bitmap_to_chunky(ULONG *bitmap, struct ColorMap *cm)
{
	int i;

	for(i = 0; i < ((CHUNKGFX_WIDTH / 2) * (CHUNKGFX_HEIGHT / 2)); i++) {
		check_char(bitmap, i, cm);
	}
}

struct ColorMap *alloc_colormap(void)
{
	struct ColorMap *cm = GetColorMap(16);
	if(cm == NULL) return NULL;

	/* Set up speccy palette - nicked this straight from PDHFIC code converted from E.
	 * Not sure why the ordering is strange. */

  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 15 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 0 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 8 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 ,(ULONG) 0xFFFFFFFF );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 9 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 10 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0x00000000 ,(ULONG) 0xFFFFFFFF );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 11 ,(ULONG) 0x00000000 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 12 ,(ULONG) 0x00000000 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0xFFFFFFFF );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 13 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0xFFFFFFFF ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 14 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0xFFFFFFFF ,(ULONG) 0xFFFFFFFF );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 1 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 ,(ULONG) 0xDDDDDDDD );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 2 ,(ULONG) 0xDDDDDDDD ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 3 ,(ULONG) 0xDDDDDDDD ,(ULONG) 0x00000000 ,(ULONG) 0xDDDDDDDD );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 4 ,(ULONG) 0x00000000 ,(ULONG) 0xDDDDDDDD ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 5 ,(ULONG) 0x00000000 ,(ULONG) 0xDDDDDDDD ,(ULONG) 0xDDDDDDDD );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 6 ,(ULONG) 0xDDDDDDDD ,(ULONG) 0xDDDDDDDD ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 7 ,(ULONG) 0xDDDDDDDD ,(ULONG) 0xDDDDDDDD ,(ULONG) 0xDDDDDDDD );

	return cm;
}

int main(void)
{
	ULONG *bitmap;
	struct ColorMap *cm;

	if(cm = alloc_colormap()) {
		if(bitmap = bitmap_from_datatype("RAM:test.png")) {
			bitmap_to_chunky(bitmap, cm);
			FreeVec(bitmap);
		}
		FreeColorMap(cm);
	}

	return 0;
}

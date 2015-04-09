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

#include <stdio.h>

#include <proto/datatypes.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#include <datatypes/pictureclass.h>

#define CHUNKGFX_WIDTH  64
#define CHUNKGFX_HEIGHT 48 /* or 44 */

char *blockgfx[] = { 	"  ", /* 0000 */
						" .", /* 0001 */
						". ", /* 0010 */
						"..", /* 0011 */
						" '", /* 0100 */
						" :", /* 0101 */
						".'", /* 0110 */
						".:", /* 0111 */
						"' ", /* 1000 */
						"'.", /* 1001 */
						": ", /* 1010 */
						":.", /* 1011 */
						"''", /* 1100 */
						"':", /* 1101 */
						":'", /* 1110 */
						"::", /* 1111 */
};

ULONG *bitmap_from_datatype(const char *filename)
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
				DisposeDTObject(dto);
				return NULL;
			}

			IDoMethod(dto, PDTM_READPIXELARRAY, bitmap,
				PBPAFMT_ARGB, CHUNKGFX_WIDTH * 4, 0, 0,
				bmh->bmh_Width, bmh->bmh_Height);
		}
		DisposeDTObject(dto);
	}

	return bitmap;
}

void check_char(ULONG *bitmap, int i, struct ColorMap *cm, int mono)
{
	ULONG pix_argb[4];
	LONG pix_pen[4];
	ULONG pix_colour[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int pix_bright = 0;
	int pix_notbright = 0;
	int first_colour = 0;
	int second_colour = 0;
	int p, a;
	int ink, paper, bright = 0;
	int gfx = 0;

	/* The four pixels are:	*/
	pix_argb[0] = bitmap[i];
	pix_argb[1] = bitmap[i + 1];
	pix_argb[2] = bitmap[i + CHUNKGFX_WIDTH];
	pix_argb[3] = bitmap[i + CHUNKGFX_WIDTH + 1];

	for(p = 0; p < 4; p++) {
		pix_pen[p] = FindColor(cm,
						(pix_argb[p] & 0x00ff0000) << 8,
						(pix_argb[p] & 0x0000ff00) << 16,
						(pix_argb[p] & 0x000000ff) << 24,
						16);

		if(pix_pen[p] < 8) {
			pix_colour[pix_pen[p]]++;
			if(pix_pen[p] != 0) pix_notbright++;
		} else {
			pix_pen[p] -= 8;
			pix_colour[pix_pen[p]]++;
			if(pix_pen[p] != 0) pix_bright++;
		}
	}

	for(a = 0; a < 8; a++) {
		if(pix_colour[a] > pix_colour[first_colour]) first_colour = a;
	}

	if(pix_colour[first_colour] == 4) {
		/* bit of a fudge to get plain blocks in either ink or paper */
		if (first_colour < 5) second_colour = 7;
			else second_colour = 0;
	} else {
		for(a = 0; a < 8; a++) {
			if((pix_colour[a] > pix_colour[second_colour]) && (first_colour != a))
				second_colour = a;
		}

		if((pix_colour[first_colour] + pix_colour[second_colour]) != 4) {
			for(a = 0; a < 8; a++) {
				if(a == first_colour) continue;
				if(a == second_colour) continue;

				if((pix_colour[a]) > 0) {
					for(p = 0; p < 4; p++) {
						if(pix_pen[p] == a) {
							if(abs(a - second_colour) > abs(a - first_colour)) {
								pix_pen[p] = first_colour;
							} else {
								pix_pen[p] = second_colour;
							}
						}
					}
				}
			}
		}
	}

	if(first_colour < second_colour) {
		paper = second_colour;
		ink = first_colour;
	} else {
		paper = first_colour;
		ink = second_colour;
	}

	for(p = 0; p < 4; p++) {
		if(pix_pen[p] == ink) {
			gfx |= 1;
		}
		if (p < 3) gfx = gfx << 1;
	}

	if(mono == 0) {
		if(pix_bright > pix_notbright) bright = 1;
		printf("\\{0x11}\\{%d}\\{0x10}\\{%d}\\{0x13}\\{%d}", paper, ink, bright);
	}

	printf("\\%s", blockgfx[gfx]);
}

void bitmap_to_chunky(ULONG *bitmap, struct ColorMap *cm, int mono)
{
	int x, y;

	for(y = 0; y < CHUNKGFX_HEIGHT; y += 2) {
		printf("DATA \"");
		for(x = 0; x < CHUNKGFX_WIDTH; x += 2) {
			check_char(bitmap, x + (y * CHUNKGFX_WIDTH), cm, mono);
		}
		printf("\"\n");
	}
}

struct ColorMap *alloc_colormap(void)
{
	struct ColorMap *cm = GetColorMap(16);
	if(cm == NULL) return NULL;

	/* Set up speccy palette - nicked this straight from PDHFIC code converted from E.
	 * Not sure why the ordering is strange. */

  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 8 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 0 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 9 ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 ,(ULONG) 0xFFFFFFFF );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 10 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0x00000000 ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 11 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0x00000000 ,(ULONG) 0xFFFFFFFF );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 12 ,(ULONG) 0x00000000 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 13 ,(ULONG) 0x00000000 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0xFFFFFFFF );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 14 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0xFFFFFFFF ,(ULONG) 0x00000000 );
  SetRGB32CM((struct ColorMap*) cm ,(ULONG) 15 ,(ULONG) 0xFFFFFFFF ,(ULONG) 0xFFFFFFFF ,(ULONG) 0xFFFFFFFF );
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
	LONG rarray[] = {0, 0};
	struct RDArgs *args;
	ULONG *bitmap;
	struct ColorMap *cm;
	STRPTR template = "INPUT/A,MONO/S";

	enum {
		A_INPUT,
		A_MONO
	};

	if(args = ReadArgs(template, rarray, NULL)) {
		if(rarray[A_INPUT]) {
			if(cm = alloc_colormap()) {
				if(bitmap = bitmap_from_datatype((const char *)rarray[A_INPUT])) {
					bitmap_to_chunky(bitmap, cm, rarray[A_MONO]);
					FreeVec(bitmap);
				}
				FreeColorMap(cm);
			}
		}
		FreeArgs(args);
	}

	return 0;
}

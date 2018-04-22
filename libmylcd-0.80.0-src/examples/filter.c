
// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.



#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mylcd.h"
#include "demos.h"

#define Min(a,b)	(((a)<(b))?(a):(b))
#define Max(a,b)	(((a)>(b))?(a):(b))


void filter (TFRAME *src, TFRAME *des, const float kernel[5][5], const int dx, const int dy)
{
		
	#define red(ox,oy,factor) (in[((y)+(oy))*(pitch)+(((x)<<2)+((ox)<<2)+2)]*(factor))
	#define grn(ox,oy,factor) (in[((y)+(oy))*(pitch)+(((x)<<2)+((ox)<<2)+1)]*(factor))
	#define blu(ox,oy,factor) (in[((y)+(oy))*(pitch)+(((x)<<2)+((ox)<<2)  )]*(factor))
	#define alp(ox,oy,factor) (in[((y)+(oy))*(pitch)+(((x)<<2)+((ox)<<2)+3)]*(factor))
	
	int r,g,b;
	//int a;
	int x,y;
	const int pitch = src->pitch;
	const int w = src->width;
	const int h = src->height;
	const unsigned char *in = lGetPixelAddress(src, 0, 0);
	int *out;

	for (y = 2; y < h-2; y++){
		out = lGetPixelAddress(des, 2+dx, y+dy);
		
		for (x = 2; x < w-2; x++){
			r = red(-2, -2, kernel[4][4]) + red(-1, -2, kernel[3][4]) +	red(0, -2, kernel[2][4]) + red(+1, -2, kernel[1][4]) + red(+2, -2, kernel[0][4]) + 
				red(-2, -1, kernel[4][3]) + red(-1, -1, kernel[3][3]) +	red(0, -1, kernel[2][3]) + red(+1, -1, kernel[1][3]) + red(+2, -1, kernel[0][3]) + 
				red(-2,  0, kernel[4][2]) + red(-1,  0, kernel[3][2]) +	red(0,  0, kernel[2][2]) + red(+1,  0, kernel[1][2]) + red(+2,  0, kernel[0][2]) + 
				red(-2,  1, kernel[4][1]) + red(-1,  1, kernel[3][1]) +	red(0,  1, kernel[2][1]) + red(+1,  1, kernel[1][1]) + red(+2,  1, kernel[0][1]) + 
				red(-2,  2, kernel[4][0]) + red(-1,  2, kernel[3][0]) +	red(0,  2, kernel[2][0]) + red(+1,  2, kernel[1][0]) + red(+2,  2, kernel[0][0]);
			  
			g = grn(-2, -2, kernel[4][4]) + grn(-1, -2, kernel[3][4]) +	grn(0, -2, kernel[2][4]) + grn(+1, -2, kernel[1][4]) + grn(+2, -2, kernel[0][4]) + 
				grn(-2, -1, kernel[4][3]) + grn(-1, -1, kernel[3][3]) +	grn(0, -1, kernel[2][3]) + grn(+1, -1, kernel[1][3]) + grn(+2, -1, kernel[0][3]) + 
				grn(-2,  0, kernel[4][2]) + grn(-1,  0, kernel[3][2]) +	grn(0,  0, kernel[2][2]) + grn(+1,  0, kernel[1][2]) + grn(+2,  0, kernel[0][2]) + 
				grn(-2,  1, kernel[4][1]) + grn(-1,  1, kernel[3][1]) +	grn(0,  1, kernel[2][1]) + grn(+1,  1, kernel[1][1]) + grn(+2,  1, kernel[0][1]) + 
				grn(-2,  2, kernel[4][0]) + grn(-1,  2, kernel[3][0]) +	grn(0,  2, kernel[2][0]) + grn(+1,  2, kernel[1][0]) + grn(+2,  2, kernel[0][0]);
			  
			b = blu(-2, -2, kernel[4][4]) + blu(-1, -2, kernel[3][4]) +	blu(0, -2, kernel[2][4]) + blu(+1, -2, kernel[1][4]) + blu(+2, -2, kernel[0][4]) + 
				blu(-2, -1, kernel[4][3]) + blu(-1, -1, kernel[3][3]) +	blu(0, -1, kernel[2][3]) + blu(+1, -1, kernel[1][3]) + blu(+2, -1, kernel[0][3]) + 
				blu(-2,  0, kernel[4][2]) + blu(-1,  0, kernel[3][2]) +	blu(0,  0, kernel[2][2]) + blu(+1,  0, kernel[1][2]) + blu(+2,  0, kernel[0][2]) + 
				blu(-2,  1, kernel[4][1]) + blu(-1,  1, kernel[3][1]) +	blu(0,  1, kernel[2][1]) + blu(+1,  1, kernel[1][1]) + blu(+2,  1, kernel[0][1]) + 
				blu(-2,  2, kernel[4][0]) + blu(-1,  2, kernel[3][0]) +	blu(0,  2, kernel[2][0]) + blu(+1,  2, kernel[1][0]) + blu(+2,  2, kernel[0][0]);
		  
			/*a = alp(-2, -2, kernel[4][4]) + alp(-1, -2, kernel[3][4]) +	alp(0, -2, kernel[2][4]) + alp(+1, -2, kernel[1][4]) + alp(+2, -2, kernel[0][4]) + 
				alp(-2, -1, kernel[4][3]) + alp(-1, -1, kernel[3][3]) +	alp(0, -1, kernel[2][3]) + alp(+1, -1, kernel[1][3]) + alp(+2, -1, kernel[0][3]) + 
				alp(-2,  0, kernel[4][2]) + alp(-1,  0, kernel[3][2]) +	alp(0,  0, kernel[2][2]) + alp(+1,  0, kernel[1][2]) + alp(+2,  0, kernel[0][2]) + 
				alp(-2,  1, kernel[4][1]) + alp(-1,  1, kernel[3][1]) +	alp(0,  1, kernel[2][1]) + alp(+1,  1, kernel[1][1]) + alp(+2,  1, kernel[0][1]) + 
				alp(-2,  2, kernel[4][0]) + alp(-1,  2, kernel[3][0]) +	alp(0,  2, kernel[2][0]) + alp(+1,  2, kernel[1][0]) + alp(+2,  2, kernel[0][0]);
			*/	
			b &= 0xFF;
			g &= 0xFF;
			r &= 0xFF;
			//a = 0xFF;
			*out = /*a<<24|*/r<<16|g<<8|b;
			out++;
			//lSetPixel(des, x+10, y+10, r<<16|g<<8|b);
		}
	}
}


int convolve2D (int *in, int *out, const int dataSizeX, const int dataSizeY, 
               float *kernel, const int kernelSizeX, const int kernelSizeY)
{
    int i, j, m, n;
    int *inPtr, *inPtr2, *outPtr;
    float *kPtr;
    int kCenterX, kCenterY;
    int rowMin, rowMax;                             // to check boundary of input array
    int colMin, colMax;                             //
    //float sum;                                      // temp accumulation buffer
    float sum[3];

    // check validity of params
    if (!in || !out || !kernel) return 0;
    if (dataSizeX <= 0 || kernelSizeX <= 0) return 0;

    // find center position of kernel (half of kernel size)
    kCenterX = kernelSizeX >> 1;
    kCenterY = kernelSizeY >> 1;

    // init working  pointers
    inPtr = inPtr2 = &in[dataSizeX * kCenterY + kCenterX];  // note that  it is shifted (kCenterX, kCenterY),
    outPtr = out;
    kPtr = kernel;

    // start convolution
    for(i= 0; i < dataSizeY; ++i)                   // number of rows
    {
        // compute the range of convolution, the current row of kernel should be between these
        rowMax = i + kCenterY;
        rowMin = i - dataSizeY + kCenterY;

		outPtr = &out[i * dataSizeX];

        for(j = 0; j < dataSizeX; ++j)              // number of columns
        {
            // compute the range of convolution, the current column of kernel should be between these
            colMax = j + kCenterX;
            colMin = j - dataSizeX + kCenterX;

            sum[0] = 0;                                // set to 0 before accumulate
            sum[1] = 0;
            sum[2] = 0;

            // flip the kernel and traverse all the kernel values
            // multiply each kernel value with underlying input data
            for(m = 0; m < kernelSizeY; ++m)        // kernel rows
            {
                // check if the index is out of bound of input array
                if(m <= rowMax && m > rowMin)
                {
                    for(n = 0; n < kernelSizeX; ++n)
                    {
                        // check the boundary of array
                        if(n <= colMax && n > colMin){
                            sum[0] += ((int)*(inPtr - n)&0xFF0000) * *kPtr;
                            sum[1] += ((int)*(inPtr - n)&0x00FF00) * *kPtr;
                            sum[2] += ((int)*(inPtr - n)&0x0000FF) * *kPtr;
                          }

                        ++kPtr;                     // next kernel
                    }
                }
                else
                    kPtr += kernelSizeX;            // out of bound, move to next row of kernel

                inPtr -= dataSizeX;                 // move input data 1 raw up
            }

            *outPtr++ = 0xFF000000|(((int)sum[0]&0xFF0000) | ((int)sum[1]&0x00FF00) | ((int)sum[2]&0x0000FF));
            kPtr = kernel;                          // reset kernel to (0,0)
            inPtr = ++inPtr2;                       // next input
        }
    }

    return 1;
}


int convolve2DSeparable(int* in, int* out, int dataSizeX, int dataSizeY, 
                         float* kernelX, int kSizeX, float* kernelY, int kSizeY)
{
    int i, j, k, m, n;
    float *tmp[3], *sum[3];                               // intermediate data buffer
    int *inPtr, *outPtr;                    // working pointers
    float *tmpPtr[3], *tmpPtr2[3];                        // working pointers
    int kCenter, kOffset, endIndex;                 // kernel indice
    float ktmp;
    
    // check validity of params
    if(!in || !out || !kernelX || !kernelY) return 0;
    if(dataSizeX <= 0 || kSizeX <= 0) return 0;

    // allocate temp storage to keep intermediate result
    tmp[0] = malloc(dataSizeX * dataSizeY * sizeof(float));
    tmp[1] = malloc(dataSizeX * dataSizeY * sizeof(float));
    tmp[2] = malloc(dataSizeX * dataSizeY * sizeof(float));
    //if(!tmp) return 0;  // memory allocation error

    // store accumulated sum
    sum[0] = malloc(dataSizeX * sizeof(float));
    sum[1] = malloc(dataSizeX * sizeof(float));
    sum[2] = malloc(dataSizeX * sizeof(float));
   // if(!sum) return 0;  // memory allocation error

    // covolve horizontal direction ///////////////////////

    // find center position of kernel (half of kernel size)
    kCenter = kSizeX >> 1;                          // center index of kernel array
    endIndex = dataSizeX - kCenter;                 // index for full kernel convolution

    // init working pointers
    inPtr = in;
    tmpPtr[0] = tmp[0];                                   // store intermediate results from 1D horizontal convolution
    tmpPtr[1] = tmp[1];
    tmpPtr[2] = tmp[2];


    // start horizontal convolution (x-direction)
    for(i=0; i < dataSizeY; ++i)                    // number of rows
    {

        kOffset = 0;                                // starting index of partial kernel varies for each sample

        // COLUMN FROM index=0 TO index=kCenter-1
        for(j=0; j < kCenter; ++j)
        {
            *tmpPtr[0] = 0;                            // init to 0 before accumulation
			*tmpPtr[1] = 0;
			*tmpPtr[2] = 0;
			
            for(k = kCenter + kOffset, m = 0; k >= 0; --k, ++m) // convolve with partial of kernel
            {
                *tmpPtr[0] += (*(inPtr + m)&0x0000FF) * kernelX[k];
                *tmpPtr[1] += (*(inPtr + m)&0x00FF00) * kernelX[k];
                *tmpPtr[2] += (*(inPtr + m)&0xFF0000) * kernelX[k];
            }
            ++tmpPtr[0];                               // next output
            ++tmpPtr[1];
            ++tmpPtr[2];
            ++kOffset;                              // increase starting index of kernel
        }

        // COLUMN FROM index=kCenter TO index=(dataSizeX-kCenter-1)
        for(j = kCenter; j < endIndex; ++j)
        {
            *tmpPtr[0] = 0;                            // init to 0 before accumulate
            *tmpPtr[1] = 0;
            *tmpPtr[2] = 0;
            for(k = kSizeX-1, m = 0; k >= 0; --k, ++m)  // full kernel
            {
                *tmpPtr[0] += ((inPtr[m] )&0x0000FF) * kernelX[k];
                *tmpPtr[1] += ((inPtr[m] )&0x00FF00) * kernelX[k];
                *tmpPtr[2] += ((inPtr[m] )&0xFF0000) * kernelX[k];
            }
            ++inPtr;                                // next input
            ++tmpPtr[0];                               // next output
            ++tmpPtr[1];
            ++tmpPtr[2];
        }

        kOffset = 1;                                // ending index of partial kernel varies for each sample

        // COLUMN FROM index=(dataSizeX-kCenter) TO index=(dataSizeX-1)
        for(j = endIndex; j < dataSizeX; ++j)
        {
            *tmpPtr[0] = 0;                            // init to 0 before accumulation
            *tmpPtr[1] = 0;
            *tmpPtr[2] = 0;

            for(k = kSizeX-1, m=0; k >= kOffset; --k, ++m)   // convolve with partial of kernel
            {
                *tmpPtr[0] += (*(inPtr + m)&0x0000FF) * kernelX[k];
                *tmpPtr[1] += (*(inPtr + m)&0x00FF00) * kernelX[k];
                *tmpPtr[2] += (*(inPtr + m)&0xFF0000) * kernelX[k];
            }
            ++inPtr;                                // next input
            ++tmpPtr[0];                               // next output
            ++tmpPtr[1];
            ++tmpPtr[2];
            ++kOffset;                              // increase ending index of partial kernel
        }

        inPtr += kCenter;                           // next row
    }
    // END OF HORIZONTAL CONVOLUTION //////////////////////

    // start vertical direction ///////////////////////////

    // find center position of kernel (half of kernel size)
    kCenter = kSizeY >> 1;                          // center index of vertical kernel
    endIndex = dataSizeY - kCenter;                 // index where full kernel convolution should stop

    // set working pointers
    outPtr = out;
    tmpPtr[0] = tmpPtr2[0] = tmp[0];
    tmpPtr[1] = tmpPtr2[1] = tmp[1];
    tmpPtr[2] = tmpPtr2[2] = tmp[2];
    
    // clear out array before accumulation
    for(i = 0; i < dataSizeX; ++i){
        sum[0][i] = 0;
        sum[1][i] = 0;
        sum[2][i] = 0;
	}

    // start to convolve vertical direction (y-direction)

    // ROW FROM index=0 TO index=(kCenter-1)
    kOffset = 0;                                    // starting index of partial kernel varies for each sample
    for(i=0; i < kCenter; ++i)
    {
        for(k = kCenter + kOffset; k >= 0; --k)     // convolve with partial kernel
        {
        	ktmp = kernelY[k];
            for(j=0; j < dataSizeX; ++j)
            {
                sum[0][j] += ((*tmpPtr[0]++)) * ktmp;
                sum[1][j] += ((*tmpPtr[1]++)) * ktmp;
                sum[2][j] += ((*tmpPtr[2]++)) * ktmp;
            }
        }

        for(n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
        {
 			*outPtr = ((int)sum[0][n]&0x0000FF) | ((int)sum[1][n]&0x00FF00) | ((int)sum[2][n]&0xFF0000);
            ++outPtr;                               // next element of output 			
            sum[0][n] = 0;                             // reset to zero for next summing
            sum[1][n] = 0;
            sum[2][n] = 0;
        }

        tmpPtr[0] = tmpPtr2[0];                           // reset input pointer
        tmpPtr[1] = tmpPtr2[1];
        tmpPtr[2] = tmpPtr2[2];
        ++kOffset;                                  // increase starting index of kernel
    }

    // ROW FROM index=kCenter TO index=(dataSizeY-kCenter-1)
    
    for(i = kCenter; i < endIndex; ++i)
    {
        for(k = kSizeY -1; k >= 0; --k)             // convolve with full kernel
        {
        	ktmp = kernelY[k];
            for(j = 0; j < dataSizeX; ++j)
            {
                sum[0][j] += (*tmpPtr[0]++) * ktmp;
                sum[1][j] += (*tmpPtr[1]++) * ktmp;
                sum[2][j] += (*tmpPtr[2]++) * ktmp;
            }
        }

        for(n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
        {
            *outPtr++ = ((int)sum[0][n]&0x0000FF)  | ((int)sum[1][n]&0x00FF00) | ((int)sum[2][n]&0xFF0000);
            sum[0][n] = 0;                             // reset to 0 before next summing
            sum[1][n] = 0;
            sum[2][n] = 0;
        }

        // move to next row
        tmpPtr2[0] += dataSizeX;
        tmpPtr2[1] += dataSizeX;
        tmpPtr2[2] += dataSizeX;
        tmpPtr[0] = tmpPtr2[0];
        tmpPtr[1] = tmpPtr2[1];
        tmpPtr[2] = tmpPtr2[2];

    }

    // ROW FROM index=(dataSizeY-kCenter) TO index=(dataSizeY-1)
    kOffset = 1;                                    // ending index of partial kernel varies for each sample
    for(i=endIndex; i < dataSizeY; ++i)
    {
        for(k = kSizeY-1; k >= kOffset; --k)        // convolve with partial kernel
        {
        	ktmp = kernelY[k];
            for(j=0; j < dataSizeX; ++j)
            {
                sum[0][j] += (*tmpPtr[0]++) * ktmp;
                sum[1][j] += (*tmpPtr[1]++) * ktmp;
                sum[2][j] += (*tmpPtr[2]++) * ktmp;
            }
        }

        for(n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
        {
            *outPtr = ((int)sum[0][n]&0x0000FF) | ((int)sum[1][n]&0x00FF00) | ((int)sum[2][n]&0xFF0000);
            sum[0][n] = 0;                             // reset before next summing
            sum[1][n] = 0;
            sum[2][n] = 0;
            ++outPtr;                               // next output
        }

        // move to next row
        tmpPtr2[0] += dataSizeX;
        tmpPtr2[1] += dataSizeX;
        tmpPtr2[2] += dataSizeX;
        tmpPtr[0] = tmpPtr2[0];                           // next input
        tmpPtr[1] = tmpPtr2[1];
        tmpPtr[2] = tmpPtr2[2];        
        ++kOffset;                                  // increase ending index of kernel
    }
    // END OF VERTICAL CONVOLUTION ////////////////////////

    // deallocate temp buffers
    free(tmp[0]);
    free(tmp[1]);
    free(tmp[2]);
    free(sum[0]);
    free(sum[1]);
    free(sum[2]);
    return 1;
}

void blurRegion (TFRAME *src, const float kernel[5][5], int x1, int y1, int x2, int y2)
{
		
	const int w = (x2-x1) + 1;
	const int h = (y2-y1) + 1;
	TFRAME *tmp = lNewFrame(src->hw, w+16, h+16, src->bpp);
	TFRAME *tmp2 = lNewFrame(src->hw, w+16, h+16, src->bpp);
	
	lCopyArea(src, tmp, 0, 0, x1-4, y1-4, x2+4, y2+4);
	filter(tmp, tmp2, kernel, 0, 0);
	//filter(tmp2, tmp, kernel, 0, 0);
	//filter(tmp, tmp2, kernel, 0, 0);
	lCopyArea(tmp2, src, x1, y1, 4, 4, tmp2->width-4, tmp2->height-4);

	lDeleteFrame(tmp);
	lDeleteFrame(tmp2);
}

/* Listing 2: Code to perform box filtering using the algorithm
given in the text.

First call DoPreComputation giving a source image and dimensions.
Pass the output of DoPreComputation (p) to DoBoxBlur to perform
the blur. See Listing 5 for an example */

void DoPreComputation (int *src, int src_w, int src_h, int *r, int *g, int *b)
{
	int tot[3];
	int x, y;
	
	for (y=0;y<src_h;y++){
		for (x=0;x<src_w;x++){
			tot[0] = src[0]&0xff0000;
			tot[1] = src[0]&0x00ff00;
			tot[2] = src[0]&0x0000ff;

			if (x>0){
				tot[0] += r[-1];
				tot[1] += g[-1];
				tot[2] += b[-1];
			}
			if (y>0){
				tot[0] += r[-src_w];
				tot[1] += g[-src_w];
				tot[2] += b[-src_w];
			}
			if (x>0 && y>0){
				tot[0] -= r[-src_w-1];
				tot[1] -= g[-src_w-1];
				tot[2] -= b[-src_w-1];
			}
			
			*r++ = tot[0];
			*g++ = tot[1];
			*b++ = tot[2];
			src++;
		}
	}
}

// this is a utility function used by DoBoxBlur below
static int ReadP (int *p, int w, int h, int x, int y)
{
	if (x<0)
		x=0;
	else if (x>=w)
		x=w-1;
		
	if (y<0)
		y=0;
	else if (y>=h)
		y=h-1;
		
	return p[x+y*w];
}

// the main meat of the algorithm lies here
void DoBoxBlur (int src_w, int src_h, int *dst, int *r, int *g, int *b, int boxw, int boxh)
{
	const float mul=1.0f/((boxw*2+1)*(boxh*2+1));
	int tot[3];
	int x, y;
	
	for (y=1;y<src_h;y++){
		for (x=0;x<src_w;x++){
			tot[0] = ReadP(r, src_w,src_h,x+boxw,y+boxh)
				 	+ReadP(r, src_w,src_h,x-boxw,y-boxh)
				 	-ReadP(r, src_w,src_h,x-boxw,y+boxh)
				 	-ReadP(r, src_w,src_h,x+boxw,y-boxh);
			
			tot[1] = ReadP(g, src_w,src_h,x+boxw,y+boxh)
				 	+ReadP(g, src_w,src_h,x-boxw,y-boxh)
				 	-ReadP(g, src_w,src_h,x-boxw,y+boxh)
				 	-ReadP(g, src_w,src_h,x+boxw,y-boxh);

			tot[2] = ReadP(b, src_w,src_h,x+boxw,y+boxh)
				 	+ReadP(b, src_w,src_h,x-boxw,y-boxh)
				 	-ReadP(b, src_w,src_h,x-boxw,y+boxh)
				 	-ReadP(b, src_w,src_h,x+boxw,y-boxh);

			*dst++ = ((int)(tot[0]*mul)&0xFF0000) | ((int)(tot[1]*mul)&0x00FF00) | ((int)(tot[2]*mul)&0x0000FF);
			//dst++;
			//src++;
		}
	}
	
}


// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
void fastbluralpha (TFRAME *src, const int x1, const int y1, const int x2, const int y2, const int radius)
{
    if (radius < 1)
        return;

    int *pix = (int*)src->pixels;
    const int w   = src->width;
    const int h   = src->height;
    const int wm  = w-1;
    const int hm  = h-1;
    const int wh  = w*h;
    const int div = radius+radius+1;

    int *r = malloc(sizeof(int)*wh);
    int *g = malloc(sizeof(int)*wh);
    int *b = malloc(sizeof(int)*wh);
    int *a = malloc(sizeof(int)*wh);
    
    int rsum, gsum, bsum, asum, x, y, i, yp, yi, yw;
    int p;
    int *vmin = malloc(sizeof(int) * (w+h));

    int divsum = (div+1)>>1;
    divsum *= divsum;
    int *dv = malloc(sizeof(int)*256*divsum);
    for (i=0; i < 256*divsum; ++i) {
        dv[i] = (i/divsum);
    }

    yw = yi = 0;

    int **stack = malloc(sizeof(int)*div);
    for(int i = 0; i < div; ++i) {
        stack[i] = malloc(4 * sizeof(int));
    }

    int stackpointer;
    int stackstart;
    int *sir;
    int rbs;
    const int r1 = radius+1;
    int routsum, goutsum, boutsum, aoutsum;
    int rinsum, ginsum, binsum, ainsum;

    for (y = 0; y < h; ++y){
        rinsum = ginsum = binsum = ainsum
               = routsum = goutsum = boutsum = aoutsum
               = rsum = gsum = bsum = asum = 0;
               
        for(i =- radius; i <= radius; ++i) {
            p = pix[yi+Min(wm,Max(i,0))];
            sir = stack[i+radius];
            sir[0] = (p&0x00FF0000)>>16;
            sir[1] = (p&0x0000FF00)>>8;
            sir[2] = (p&0x000000FF);
            sir[3] = (p&0xFF000000)>>24;
            
            rbs = r1-abs(i);
            rsum += sir[0]*rbs;
            gsum += sir[1]*rbs;
            bsum += sir[2]*rbs;
            asum += sir[3]*rbs;
            
            if (i > 0){
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
                ainsum += sir[3];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
                aoutsum += sir[3];
            }
        }
        stackpointer = radius;

        for (x=0; x < w; ++x) {

            r[yi] = dv[rsum];
            g[yi] = dv[gsum];
            b[yi] = dv[bsum];
            a[yi] = dv[asum];

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;
            asum -= aoutsum;

            stackstart = stackpointer-radius+div;
            sir = stack[stackstart%div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];
            aoutsum -= sir[3];

            if (y == 0) {
                vmin[x] = Min(x+radius+1,wm);
            }
            p = pix[yw+vmin[x]];

            sir[0] = (p&0x00FF0000)>>16;
            sir[1] = (p&0x0000FF00)>>8;
            sir[2] = (p&0x000000FF);
            sir[3] = (p&0xFF000000)>>24;
            
            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];
            ainsum += sir[3];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;
            asum += ainsum;

            stackpointer = (stackpointer+1)%div;
            sir = stack[(stackpointer)%div];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];
            aoutsum += sir[3];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];
            ainsum -= sir[3];

            ++yi;
        }
        yw += w;
    }
    for (x=0; x < w; ++x){
        rinsum = ginsum = binsum = ainsum 
               = routsum = goutsum = boutsum = aoutsum 
               = rsum = gsum = bsum = asum = 0;
        
        yp =- radius * w;
        
        for(i=-radius; i <= radius; ++i) {
            yi=Max(0,yp)+x;

            sir = stack[i+radius];

            sir[0] = r[yi];
            sir[1] = g[yi];
            sir[2] = b[yi];
            sir[3] = a[yi];

            rbs = r1-abs(i);

            rsum += r[yi]*rbs;
            gsum += g[yi]*rbs;
            bsum += b[yi]*rbs;
            asum += a[yi]*rbs;

            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
                ainsum += sir[3];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
                aoutsum += sir[3];
            }

            if (i < hm)
                yp += w;
        }

        yi = x;
        stackpointer = radius;

        for (y = 0; y < h; ++y){
            pix[yi] = dv[asum]<<24|dv[rsum]<<16|dv[gsum]<<8|dv[bsum];

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;
            asum -= aoutsum;

            stackstart = stackpointer-radius+div;
            sir = stack[stackstart%div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];
            aoutsum -= sir[3];

            if (!x)
                vmin[y] = Min(y+r1,hm)*w;

            p = x+vmin[y];

            sir[0] = r[p];
            sir[1] = g[p];
            sir[2] = b[p];
            sir[3] = a[p];

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];
            ainsum += sir[3];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;
            asum += ainsum;

            stackpointer = (stackpointer+1)%div;
            sir = stack[stackpointer];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];
            aoutsum += sir[3];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];
            ainsum -= sir[3];

            yi += w;
        }
    }
    free(r);
    free(g);
    free(b);
    free(a);
    free(vmin);
    free(dv);

    for(int i = 0; i < div; ++i) {
        free(stack[i]);
    }
    free(stack);
}

// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
void fastblur (int *pix, const int w, const int h, const int x2, const int radius)
{
    if (radius < 1)
        return;

    const int wm  = w-1;
    const int hm  = h-1;
    const int wh  = w*h;
    const int div = radius+radius+1;

    int *r = malloc(sizeof(int)*wh);
    int *g = malloc(sizeof(int)*wh);
    int *b = malloc(sizeof(int)*wh);
    
    int rsum, gsum, bsum, x, y, i, yp, yi, yw;
    int p;
    int *vmin = malloc(sizeof(int) * (w+h));

    int divsum = (div+1)>>1;
    divsum *= divsum;
    int *dv = malloc((sizeof(int)<<8) *divsum);
    for (i=0; i < divsum<<8; ++i)
        dv[i] = (i/divsum);

    yw = yi = 0;

    int **stack = malloc(sizeof(int)*div);
    for (int i = 0; i < div; ++i)
        stack[i] = malloc(3 * sizeof(int));

    int stackpointer;
    int stackstart;
    int *sir;
    int rbs;
    const int r1 = radius+1;
    int routsum, goutsum, boutsum;
    int rinsum, ginsum, binsum;

	
    for (y = 0; y < h; ++y){
        rinsum = ginsum = binsum
               = routsum = goutsum = boutsum
               = rsum = gsum = bsum = 0;
               
        for(i =- radius; i <= radius; ++i) {
            p = pix[yi+Min(wm,Max(i,0))];
            rbs = r1-abs(i);
            sir = stack[i+radius];
                        
            sir[0] = (p&0x00FF0000)>>16;
            rsum += sir[0]*rbs;
            sir[1] = (p&0x0000FF00)>>8;
            gsum += sir[1]*rbs;
            sir[2] = (p&0x000000FF);
            bsum += sir[2]*rbs;
            
            if (i > 0){
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
            }
        }
        stackpointer = radius;

        for (x = 0; x < w; ++x) {
            if (!y)
                vmin[x] = Min(x+radius+1,wm);
            p = pix[yw+vmin[x]];
            
            r[yi] = dv[rsum];
            rsum -= routsum;
            g[yi] = dv[gsum];
            gsum -= goutsum;
            b[yi] = dv[bsum];
            bsum -= boutsum;

            stackstart = stackpointer-radius+div;
            sir = stack[stackstart%div];

            routsum -= sir[0];
            sir[0] = (p&0x00FF0000)>>16;
            rinsum += sir[0];
            rsum += rinsum;
            
            goutsum -= sir[1];
            sir[1] = (p&0x0000FF00)>>8;
            ginsum += sir[1];
            gsum += ginsum;
            
            boutsum -= sir[2];
            sir[2] = (p&0x000000FF);
            binsum += sir[2];
            bsum += binsum;

            stackpointer = (stackpointer+1)%div;
            sir = stack[(stackpointer)%div];

            routsum += sir[0];
            rinsum -= sir[0];
            goutsum += sir[1];
            ginsum -= sir[1];
            boutsum += sir[2];
            binsum -= sir[2];

            ++yi;
        }
        yw += w;
    }
    for (x=0; x < w && x <= x2; ++x){
        rinsum = ginsum = binsum 
               = routsum = goutsum = boutsum 
               = rsum = gsum = bsum = 0;
        
        yp =- radius * w;
        
        for(i=-radius; i <= radius; ++i) {
            yi = Max(0,yp)+x;
            rbs = r1-abs(i);
            sir = stack[i+radius];

            sir[0] = r[yi];
            rsum += r[yi]*rbs;
            sir[1] = g[yi];
            gsum += g[yi]*rbs;
            sir[2] = b[yi];
            bsum += b[yi]*rbs;

            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
            }

            if (i < hm)
                yp += w;
        }

        yi = x;
        stackpointer = radius;

        for (y = 0; y < h; ++y){
            pix[yi] = dv[rsum]<<16|dv[gsum]<<8|dv[bsum];
            if (!x)
                vmin[y] = Min(y+r1,hm)*w;
            p = x+vmin[y];
            
            stackstart = stackpointer-radius+div;
            sir = stack[stackstart%div];

			rsum -= routsum;
            routsum -= sir[0];
            sir[0] = r[p];
            rinsum += sir[0];
            rsum += rinsum;
            
            gsum -= goutsum;
            goutsum -= sir[1];
            sir[1] = g[p];
            ginsum += sir[1];
            gsum += ginsum;
            
            bsum -= boutsum;
            boutsum -= sir[2];
            sir[2] = b[p];
            binsum += sir[2];
            bsum += binsum;

            stackpointer = (stackpointer+1)%div;
            sir = stack[stackpointer];

            routsum += sir[0];
            rinsum -= sir[0];
            goutsum += sir[1];
            ginsum -= sir[1];
            boutsum += sir[2];
            binsum -= sir[2];

            yi += w;
        }
    }
    free(r);
    free(g);
    free(b);
    free(vmin);
    free(dv);

    for(int i = 0; i < div; ++i) {
        free(stack[i]);
    }
    free(stack);
}

void fastblur2 (int *pix, const int w, const int h, const int radius){

  if (radius<1){
    return;
  }

  const int wm=w-1;
	const   int hm=h-1;
  const int wh=w*h;
  const int div = radius+radius+1;
    int *r = malloc(sizeof(int)*wh);
    int *g = malloc(sizeof(int)*wh);
    int *b = malloc(sizeof(int)*wh);
  int rsum,gsum,bsum,x,y,i,p,p1,p2,yp,yi,yw;
  
  int *vmin = malloc(sizeof(int)* Max(w,h));
  int *vmax = malloc(sizeof(int)* Max(w,h));
  int *dv = malloc((sizeof(int)<<8) *div);
  
  for (i=0;i<256*div;i++){
     dv[i]=(i/div); 
  }
  
  yw=yi=0;
 
  for (y=0;y<h;y++){
    rsum=gsum=bsum=0;
    for(i=-radius;i<=radius;i++){
      p=pix[yi+min(wm,max(i,0))];
      rsum+=(p & 0xff0000)>>16;
      gsum+=(p & 0x00ff00)>>8;
      bsum+= p & 0x0000ff;
   }
   for (x=0;x<w;x++){
    
      r[yi]=dv[rsum];
      g[yi]=dv[gsum];
      b[yi]=dv[bsum];

      if (!y){
        vmin[x]=min(x+radius+1,wm);
        vmax[x]=max(x-radius,0);
       } 
       p1=pix[yw+vmin[x]];
       p2=pix[yw+vmax[x]];

      rsum+=((p1 & 0xff0000)-(p2 & 0xff0000))>>16;
      gsum+=((p1 & 0x00ff00)-(p2 & 0x00ff00))>>8;
      bsum+= (p1 & 0x0000ff)-(p2 & 0x0000ff);
      yi++;
    }
    yw += w;
  }
  
  for (x=0;x<w;x++){
    rsum=gsum=bsum=0;
    yp=-radius*w;
    for(i=-radius;i<=radius;i++){
      yi=max(0,yp)+x;
      rsum+=r[yi];
      gsum+=g[yi];
      bsum+=b[yi];
      yp+=w;
    }
    yi=x;
    for (y=0;y<h;y++){
      pix[yi]=0xff000000 | (dv[rsum]<<16) | (dv[gsum]<<8) | dv[bsum];
      if (!x){
        vmin[y]=Min(y+radius+1,hm)*w;
        vmax[y]=Max(y-radius,0)*w;
      } 
      p1=x+vmin[y];
      p2=x+vmax[y];

      rsum+=r[p1]-r[p2];
      gsum+=g[p1]-g[p2];
      bsum+=b[p1]-b[p2];

      yi+=w;
    }
  }

	free(r);
	free(g);
	free(b);
	
	free(vmin);
	free(vmax);
	free(dv);
}

static void javablur (int *srcPixels, int *dstPixels, const int width, const int height, const int radius)
{
	const int windowSize = radius * 2 + 1;
	const int radiusPlusOne = radius + 1;

	int sumAlpha;
	int sumRed;
	int sumGreen;
	int sumBlue;

	int srcIndex = 0;
	int dstIndex = 0;
	int pixel;

	const int sumLookupTableLen =  256 * windowSize;
	int *sumLookupTable = malloc(sizeof(int) * sumLookupTableLen);
        
	for (int i = 0; i < sumLookupTableLen; i++)
		sumLookupTable[i] = i / windowSize;

	const int indexLookupTableLen = radiusPlusOne;
	int *indexLookupTable = malloc(sizeof(int) * indexLookupTableLen);
        
	if (radius < width){
		for (int i = 0; i < indexLookupTableLen; i++)
			indexLookupTable[i] = i;
	}else{
		for (int i = 0; i < width; i++)
			indexLookupTable[i] = i;
		for (int i = width; i < indexLookupTableLen; i++)
			indexLookupTable[i] = width - 1;
	}

	for (int y = 0; y < height; y++){
		sumAlpha = sumRed = sumGreen = sumBlue = 0;
		dstIndex = y;

		pixel = srcPixels[srcIndex];
		// sumAlpha += radiusPlusOne * ((pixel >> 24) & 0xFF);
		sumRed   += radiusPlusOne * ((pixel >> 16) & 0xFF);
		sumGreen += radiusPlusOne * ((pixel >>  8) & 0xFF);
		sumBlue  += radiusPlusOne * ( pixel        & 0xFF);

		for (int i = 1; i <= radius; i++) {
			pixel = srcPixels[srcIndex + indexLookupTable[i]];
			// sumAlpha += (pixel >> 24) & 0xFF;
			sumRed   += (pixel >> 16) & 0xFF;
			sumGreen += (pixel >>  8) & 0xFF;
			sumBlue  +=  pixel        & 0xFF;
		}

		for (int x = 0; x < width; x++){
			dstPixels[dstIndex] = /*sumLookupTable[sumAlpha] << 24 |*/
									sumLookupTable[sumRed]   << 16 |
									sumLookupTable[sumGreen] <<  8 |
									sumLookupTable[sumBlue];
			dstIndex += height;

			int nextPixelIndex = x + radiusPlusOne;
			if (nextPixelIndex >= width) {
				nextPixelIndex = width - 1;
			}

			int previousPixelIndex = x - radius;
			if (previousPixelIndex < 0) {
				previousPixelIndex = 0;
			}

			int nextPixel = srcPixels[srcIndex + nextPixelIndex];
			int previousPixel = srcPixels[srcIndex + previousPixelIndex];

			//sumAlpha += (nextPixel     >> 24) & 0xFF;
			//sumAlpha -= (previousPixel >> 24) & 0xFF;
			sumRed += (nextPixel     >> 16) & 0xFF;
			sumRed -= (previousPixel >> 16) & 0xFF;
			sumGreen += (nextPixel     >> 8) & 0xFF;
			sumGreen -= (previousPixel >> 8) & 0xFF;
			sumBlue += nextPixel & 0xFF;
			sumBlue -= previousPixel & 0xFF;
		}
		srcIndex += width;
	}
	free(sumLookupTable);
    free(indexLookupTable);
}

typedef struct{
	ubyte r;
	ubyte g;
	ubyte b;
	ubyte a;
}__attribute__ ((packed))TRGBA;

void fadeArea1 (TFRAME *frame, const int x1, const int y1, const int x2, const int y2)
{
	int x,y;
	TRGBA *p;
	
	for (y = y1; y <= y2; y++){
		p = (TRGBA*)lGetPixelAddress(frame, 0, y);
		for (x = x1; x <= x2; x++){
			p[x].r >>= 1;
			p[x].g >>= 1;
			p[x].b >>= 1;
		}
	}
}

void fadeArea2 (TFRAME *frame, const int x1, const int y1, const int x2, const int y2)
{
	int x, y;
	int *ps;
	int *s = lGetPixelAddress(frame, x1, y1);
	const int pitch = frame->pitch>>2;

	for (y = y1; y <= y2; y++){
		ps = s;
		for (x = x1; x <= x2; x++, ps++)
			*ps = (*ps&0xFEFEFE)>>1;
		s += pitch;
	}
}

void copyImage (TFRAME *src, TFRAME *des, const int sw, const int sh, const int dx, const int dy, const int dw, const int dh)
{
	#define getPixelAddr32(f,x,y)	(f->pixels+(((y)*f->pitch)+((x)<<2)))
	
	const float scalex = (float)dw / (float)sw;
	const float scaley = (float)dh / (float)sh;
	int *sp, *dp;
	int x,y2,y,i;

	int xlookup[dw];
	for (i = 0; i  < dw; i++)
		xlookup[i] = i/scalex;
		
	for (y = dy; y < dy+dh; y++){
		y2 = (y-dy)/scaley;
		sp = (int*)getPixelAddr32(src, 0, y2);
		dp = (int*)getPixelAddr32(des, dx, y);
		
		for (x = 0; x < dw; x++)
			dp[x] = sp[xlookup[x]];
	}
}


#define qRed(n) ((n>>16)&0xFF)
#define qGreen(n) ((n>>8)&0xFF)
#define qBlue(n) ((n)&0xFF)

static int blendColor (int c1, int c2, int blend)
{
    int r = qRed(c1) * blend / 256 + qRed(c2) * (256 - blend) / 256;
    int g = qGreen(c1) * blend / 256 + qGreen(c2) * (256 - blend) / 256;
    int b = qBlue(c1) * blend / 256 + qBlue(c2) * (256 - blend) / 256;
    return blend<<24|r<<16|g<<8|b;
}

void huhtanenBlur (TFRAME *des, const int iterations)
{
/*	const int h = src->height;
	const int w = src->width;
	const int bgcolor = 0xFF000000;
	const int hs = h+(h/1.5);//h * 2;
	const int hofs = 0 ;
	const int ht = hs - h - hofs;
	const int hte = ht;
	int color;*/

/*	        
    for (int x = 0; x < w; x++){
        for (int y = 0; y < h; y++)
            //lSetPixel(des, x, hofs + y, lGetPixel(src, x, y));
            lSetPixel(des, hofs + y, x, lGetPixel(src, x, y));
	}
	for (int x = 0; x < w; x++){
		for (int y = 0; y < ht; y++){
			color = lGetPixel(src, x, h - y - 1);
			if (color&0xFF000000)
				//lSetPixel(des, x, h + hofs + y, blendColor(color, bgcolor, 200*(hte - y) / hte));
				lSetPixel(des, h + hofs + y, x, blendColor(color, bgcolor, 200*(hte - y) / hte));
		}
	}
	*/
	
           // blur the reflection everything first
            // Based on exponential blur algorithm by Jani Huhtanen
            //TRECT rect = {hs / 2, 0, hs / 2, w};
            //rect &= result->rect();

/*            int c1 = hs/2; //rect.left();
            int r1 = 0; //rect.top();
            int c2 = h+c1-1; // rect.right();
            int r2 = w+r1-1; //rect.bottom();
*/
			const int c1 = 0;
			const int c2 = des->width-1;
			const int r1 = 0;
			const int r2 = des->height-1;
            const int bpl = des->pitch;
            int rgba[4];
            unsigned char *p = NULL;

            for (int loop = 0; loop < iterations; loop++) {
                for (int col = c1; col < c2; col++) {
                    p = lGetPixelAddress(des, col, r1);
                    for (int i = 0; i < 3; i++)
                        rgba[i] = p[i] << 4;

                    for (int j = r1; j < r2; j++){
                    	p += bpl;
                        for (int i = 0; i < 3; i++)
                            p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
                    }
                }

                for (int row = r1; row <= r2; row++) {
                    p = lGetPixelAddress(des, c1, row);
                    for (int i = 0; i < 3; i++)
                        rgba[i] = p[i] << 4;

                    for (int j = c1; j < c2; j++){
                    	p += 4;
                        for (int i = 0; i < 3; i++)
                            p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
                    }
                }

                for (int col = c1; col <= c2; col++) {
                    p = lGetPixelAddress(des, col, r2);
                    for (int i = 0; i < 3; i++)
                        rgba[i] = p[i] << 4;

                    for (int j = r1; j < r2; j++){
                    	p -= bpl;
                        for (int i = 0; i < 3; i++)
                            p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
                    }
                }

                for (int row = r1; row <= r2; row++) {
                    p = lGetPixelAddress(des, c2, row);
                    for (int i = 0; i < 3; i++)
                        rgba[i] = p[i] << 4;

                    for (int j = c1; j < c2; j++){
                    	p -= 4;
                        for (int i = 0; i < 3; i++)
                            p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
                   }
                }
            }
/*
            // overdraw to leave only the reflection blurred (but not the actual image)
           	for (int x = 0; x < w; x++){
            	for (int y = 0; y < h; y++){
                    lSetPixel(des, hofs + y, x, lGetPixel(src, x, y));
				}
			}
	*/
}

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));


	TFRAME *i1 = lNewFrame(hw, DWIDTH, DHEIGHT, LFRM_BPP_32A);
	TFRAME *i2 = lCloneFrame(i1);
	
	//lLoadImageEx(i1, L"images/outline.png", 0, 0);
	//lLoadImageEx(i1, L"images/cspread.png", 0, 0);
	//lLoadImageEx(i2, L"images/cube.png", 0, 0);
	lLoadImageEx(i1, L"images/colours2.png", 0, 0);
	//lLoadImageEx(i2, L"images/colours2.png", 0, 0);
	//lLoadImage(i1, L"images/psp6.png");
	//lLoadImage(i1, L"images/111823.bmp");
	
	//TFRAME *i2 = lNewFrame(hw, i1->width, i1->height, i1->bpp);
	//TFRAME *i2 = lNewFrame(hw, DWIDTH, DHEIGHT, LFRM_BPP_32A);
	
	//TFRAME *i2 = lCloneFrame(i1);

	TFRAME *red = lNewFrame(hw, i1->width, i1->height, i1->bpp);
	TFRAME *green = lNewFrame(hw, i1->width, i1->height, i1->bpp);
	TFRAME *blue = lNewFrame(hw, i1->width, i1->height, i1->bpp);

	const float kernelX[5] = {1/16.0,  4/16.0,  6/16.0,  4/16.0, 1/16.0};
	const float kernelY[5] = {1/16.0,  4/16.0,  6/16.0,  4/16.0, 1/16.0};

	#define D(n) ((n)/256.0)
	const float kernel55[5][5] = {
		{D(1), D(4) , D(6),  D(4),  D(1)},
		{D(4), D(16), D(24), D(16), D(4)},
		{D(6), D(24), D(36), D(24), D(6)},
		{D(4), D(16), D(24), D(16), D(4)},
		{D(1), D(4),  D(6),  D(4),  D(1)}};
			
	int nn, n;
	nn = n = 1;
	
	timeBeginPeriod(1);
	unsigned int t0 = timeGetTime();
	while(nn--){
		//blurRegion(i1, kernel55, 0, 0, i1->width-1, i1->height-1);
		//filter(i1, i2, kernel55, 0, 0);
		//DoPreComputation(i1->pixels, i1->width, i1->height,(int*)red->pixels, (int*)green->pixels, (int*)blue->pixels);
		//DoBoxBlur(i1->width, i1->height, (int*)i1->pixels, (int*)red->pixels, (int*)green->pixels, (int*)blue->pixels, 5, 5);
		
		//fastblur(lGetPixelAddress(i1, 0, 0), i1->width, i1->height, i1->width-1, 5);
		//fastblur2(lGetPixelAddress(i1, 0, 0), i1->width, i1->height, 5);

		// 3 iterations of the huhtanen blur is roughly equivalent to 5 radials of the java blur
		// at 1 iteration this would be slightly faster than the java blur with a radius of 1
		// quality of this blur seems to be slightly better than that of the below java blur
		huhtanenBlur(i1, 2);

		// 2 stage blur and also the fastest blur tested here
		//javablur((int*)i1->pixels, (int*)i2->pixels, i1->width, i1->height, 5);
		//javablur((int*)i2->pixels, (int*)i1->pixels, i1->height, i1->width, 5);
		
		//fadeArea(i1, 0, 0, i1->width-1, i1->height-1);
		
		//copyImage(i1, i2, i1->width, i1->height, 50, 50, i2->width/2, i2->height/2);
		//lUpdate(i1->hw, lGetPixelAddress(i1, 0, 0), i1->frameSize);
		//lRefresh(i1);
		//lRefreshAsync(i1,1);
	
		//convolve2D((int*)i1->pixels, (int*)i2->pixels, i1->width, i1->height, (float*)kernel55, 5, 5);
		//convolve2DSeparable(i1->pixels, i2->pixels, i1->width, i1->height, kernelX, 5, kernelY, 5);
	}
	unsigned int t1 = timeGetTime();
	timeEndPeriod(1);
	
	printf("time: %.4fms \n", (float)((t1-t0)/(float)n));

	#if 1
	lRefresh(i1);
	#else
	lRefresh(i2);
	#endif
	
	lSaveImage(i1, L"filter0.png", IMG_PNG, 0, 0);
	//lSaveImage(i2, L"filter1.png", IMG_PNG, 0, 0);
	//lSaveImage(i3, L"filter2.png", IMG_PNG, 0, 0);
	
	lDeleteFrame(red);
	lDeleteFrame(green);
	lDeleteFrame(blue);
	lDeleteFrame(i1);
	lDeleteFrame(i2);

	demoCleanup();
	return 1;
}

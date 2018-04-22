
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>




#include "mylcd.h"
#include "demos.h"


int HSLtoRGB (float h, float s, float l)
{
  float r, g, b; //this function works with floats between 0 and 1
  float temp1, temp2, tempr, tempg, tempb;
  h = h / 256.0;
  s = s / 256.0;
  l = l / 256.0;

  //If saturation is 0, the color is a shade of grey
  if (s == 0){
  	r = g = b = l;
  //If saturation > 0, more complex calculations are needed
  }else{
    //set the temporary values
    if (l < 0.5)
    	temp2 = l * (1 + s);
    else
    	temp2 = (l + s) - (l * s);
    temp1 = 2 * l - temp2;
    tempr=h + 1.0 / 3.0;
    if (tempr > 1.0)
    	tempr--;
    tempg=h;
    tempb=h-1.0 / 3.0;
    if (tempb < 0.0)
    	tempb++;

    //red
    if(tempr < 1.0 / 6.0) r = temp1 + (temp2 - temp1) * 6.0 * tempr;
    else if(tempr < 0.5) r = temp2;
    else if(tempr < 2.0 / 3.0) r = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempr) * 6.0;
    else r = temp1;
    
     //green
    if(tempg < 1.0 / 6.0) g = temp1 + (temp2 - temp1) * 6.0 * tempg;
    else if(tempg < 0.5) g=temp2;
    else if(tempg < 2.0 / 3.0) g = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempg) * 6.0;
    else g = temp1;

    //blue
    if(tempb < 1.0 / 6.0) b = temp1 + (temp2 - temp1) * 6.0 * tempb;
    else if(tempb < 0.5) b = temp2;
    else if(tempb < 2.0 / 3.0) b = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempb) * 6.0;
    else b = temp1;
  }


	const ubyte rr = (int)(r * 255.0);
	const ubyte gg = (int)(g * 255.0);
	const ubyte bb = (int)(b * 255.0);
	return (rr << 16)|(gg << 8)|bb;
}

static int _min (int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}

int main (int argc, char *argv[])
{
  //set up the screen
	if (!initDemoConfig("config.cfg"))
		return 0;

	int BLACK = lGetRGBMask(frame, LMASK_BLACK);
	int WHITE = lGetRGBMask(frame, LMASK_WHITE);
	hw->render->backGround = BLACK;
	hw->render->foreGround = WHITE;
	lClearFrame(frame);

	const int w = frame->width;
	const int h = frame->height;	
	int fire[frame->width][frame->height];  //this buffer will contain the fire
	int palette[256]; //this will contain the colour palette

	int x,y;
  //make sure the fire buffer is zero in the beginning
	for(x = 0; x < frame->width; x++){
		for(y = 0; y < frame->height; y++)
			fire[x][y] = 0;
  	}
  
	//generate the palette
	int c,r,g,b;
	for (x = 0; x < 256; x++){
		//HSLtoRGB is used to generate colors:
		//Hue goes from 0 to 85: red to yellow
		//Saturation is always the maximum: 255
		//Lightness is 0..255 for x=0..128, and 255 for x=128..255
		//set the palette to the calculated RGB value
    	c = HSLtoRGB(x / 3, 255, _min(255, x * 2));
    	
    	if (frame->bpp == LFRM_BPP_16){
    		r = (c&0xF80000)>>8;
			g = (c&0x00FC00)>>5;
			b = (c&0x0000F8)>>3;
			palette[x] = r|g|b;
		}else if (frame->bpp == LFRM_BPP_12){
			r = (c&0xF00000)>>12;
			g = (c&0x00F000)>>8;
			b = (c&0x0000F0)>>4;
			palette[x] = r|g|b;
		}else if (frame->bpp == LFRM_BPP_8){	
			r = (c&0xE00000)>>16;
			g = (c&0x00E000)>>11;
			b = (c&0x000030)>>5;
			palette[x] = r|g|b;
		}else{
			palette[x] = 0xFF000000|c;
		}
	}
	
	int *buffer = lGetPixelAddress(frame, 0, 0);
	int startTime = GetTickCount();
	int tFrames = 0;
	
	while(!kbhit()) {
    	//randomize the bottom row of the fire buffer
		for(x = 0; x < w; x++)
   			fire[x][h - 1] = abs(32768 + rand())&255;
    		
		//do the fire calculations for every pixel, from top to bottom
		for (y = 0; y < h - 1; y++){
			for (x = 0; x < w; x++){
				fire[x][y] = ((fire[(x - 1 + w) % w][(y + 1) % h]
							 + fire[(x)			% w][(y + 1) % h]
							 + fire[(x + 1)		% w][(y + 1) % h]
							 + fire[(x)			% w][(y + 2) % h]) << 6) / 257;
			}
		}

		//set the drawing buffer to the fire buffer, using the palette colors
		if (frame->bpp == LFRM_BPP_16){
			for(y = 0; y < h; y++){
				for(x = 0; x < w; x++){
					if (fire[x][y]){
						((int16_t*)buffer)[y*w+x] = palette[fire[x][y]];
					}
				}
			}
		}else if (frame->bpp == LFRM_BPP_24 || frame->bpp == LFRM_BPP_32){
			for(y = 0; y < h; y++){
				for(x = 0; x < w; x++){
					if (fire[x][y]){
						buffer[y*w+x] = palette[fire[x][y]];
					}
				}
			}
		}else{
			for(y = 0; y < h; y++){
				for(x = 0; x < w; x++){
					lSetPixel_NB(frame, x, y, palette[fire[x][y]]);
				}
			}
		}
		lRefresh(frame);
		tFrames++;
		//lSleep(10);
	}
	printf("%.2f\n",tFrames/(float)((GetTickCount()-startTime)/1000.0));
   
  	demoCleanup();
	return 1;  
}


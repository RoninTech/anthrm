/****************************
* frame2h
* Written by Hung Ki Chan
****************************/

#include "mylcd.h"
#include "demos.h"


int frame2file (TFRAME *frame, const char *name, int outformat);

// 1bpp frame to 1bpp header
int frameArea2file1 (const char *name, TFRAME *frame, int x1, int y1, int x2, int y2);

// 1bpp frame to 8bpp header
int frameArea2file8 (const char *name, TFRAME *frame, int x1, int y1, int x2, int y2);



int frame2file (TFRAME *frame, const char *name, int outformat)
{
	if (outformat == 8)
    	return frameArea2file8(name, frame, 0, 0, frame->width-1, frame->height-1);
    else if (outformat == 1)
    	return frameArea2file1(name, frame, 0, 0, frame->width-1, frame->height-1);
    return 0;
}

static ubyte getPixByte (TFRAME *frame, int x, int y)
{
	return (ubyte)*(frame->pixels+(frame->pitch*y) + (x>>3));
}

int frameArea2file1 (const char *name, TFRAME *frame, int x1, int y1, int x2, int y2)
{
	if (!name || !frame) return 0;
    int x=0, y=0;
    FILE *fileptr;
    
    fileptr = fopen(name, "wt");
    
    if (fileptr == NULL){
          printf("Error opening file. \n");
    }else{
    	x2++, y2++;
		int tbytes = x2/8;
		if (tbytes < 1) tbytes = 1;
		
        fprintf(fileptr, "const static unsigned char frame%dx%d[%d][%d] = {\n", frame->width, frame->height, y2-y1, (x2-x1)/8);
        for (y = y1; y < y2; y++){
        	for (x = x1; x < tbytes; x++){
				if (x == x1){
					fprintf(fileptr, "{%d, ", getPixByte(frame,x<<3, y));
				}else if (x == tbytes-1){
					fprintf(fileptr, "%d", getPixByte(frame,x<<3,y));
				}else{
					fprintf(fileptr, "%d, ", getPixByte(frame,x<<3,y));
				}
			}
			if (y >= y2-1)
				fprintf(fileptr, "}\n};\n");
			else
				fprintf(fileptr, "},\n");
		}
	}
    //fputs("\n",fileptr);
    fclose(fileptr);
    return 1;
}

int frameArea2file8 (const char *name, TFRAME *frame, int x1, int y1, int x2, int y2)
{
	if (!name || !frame) return 0;
    int x=0, y=0;
    FILE *fileptr;
    
    fileptr = fopen(name, "wt");
    
    if (fileptr == NULL){
          printf("Error opening file. \n");
    }else{
    	x2++, y2++;

        fprintf(fileptr, "const static unsigned char frame[%d][%d] = {\n", y2-y1, x2-x1);
        for (y = y1; y < y2; y++){
        	for(x = x1; x < x2-1; x++){
				if (x == x1){
					fprintf(fileptr, "{%d,", lGetPixel(frame,x,y));
				}else{
					fprintf(fileptr, "%d,", lGetPixel(frame,x,y));
				}
			}
			if (y == y2-1)
				fprintf(fileptr, "%d}\n};\n", lGetPixel(frame,x,y));
			else
				fprintf(fileptr, "%d},\n", lGetPixel(frame,x,y));
		}
	}
    //fputs("\n",fileptr);
    fclose(fileptr);
    return 1;
}

int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;
       
	lResizeFrame(frame, 160, 43, 0);

	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	lPrintEx(frame, &rect, LFT_RODDY/*LFT_GOTHAMNIGHTS*//*XFILES12*/ /*F2*/ /*ARIAL_12*/, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY,\
		  "libmylcd %s\nmylcd.sourceforge.net", (char*)lVersion());
	
    frame2file(frame, "f2.h", 1);
    lRefresh(frame);
    demoCleanup();
}

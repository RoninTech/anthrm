/*

http://student.kuleuven.be/~m0216922/CG/raycasting.html
Copyright (c) 2004, Lode Vandevenne
All rights reserved.
*/

// modified for libmylcd by Michael McElligott


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "mylcd.h"
#include "demos.h"


#define RENDER_CEILING		1
#define RENDER_FLOOR		1
#define FLOOR_QUALITY_HIGH	1

#define mapWidth 24
#define mapHeight 24
#define texWidth 64
#define texHeight 64

const int worldMap[mapWidth][mapHeight]=
{
  {8 ,8,8,8,8,8,8,8,8,8,8,4,4,6,4,4,6,4,6,4,4,4,6,4},
  {8 ,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8 ,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,6},
  {8 ,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
  {8 ,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8 ,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,6,6,6,0,6,4,6},
  {8 ,8,8,8,0,8,8,8,8,8,8,4,4,4,4,4,4,6,0,0,0,0,0,6},
  {7 ,7,7,7,0,7,7,7,7,0,8,0,8,0,8,0,8,4,0,4,0,6,0,6},
  {7 ,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,0,0,0,0,0,6},
  {12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,0,0,0,0,4},
  {12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,6,0,6,0,6},
  {7 ,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,4,6,0,6,6,6},
  {7 ,7,7,7,0,7,7,7,7,8,8,4,0,6,8,4,8,3,3,3,0,3,3,3},
  {2 ,2,2,2,0,2,2,2,2,4,6,4,0,0,6,0,6,3,0,0,0,0,0,3},
  {2 ,2,0,0,0,0,0,2,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
  {2 ,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
  {1 ,0,0,0,0,0,0,0,1,4,4,4,4,4,6,0,6,3,3,0,0,0,3,3},
  {2 ,0,0,0,0,0,0,0,2,2,2,1,2,2,2,6,6,0,0,5,0,5,0,5},
  {2 ,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2 ,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {1 ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
  {2 ,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {2 ,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2 ,2,2,2,1,2,2,2,2,2,2,1,2,2,2,5,5,5,5,5,5,5,5,5}
};

#define numSprites 19

typedef struct Sprite{
  double x;
  double y;
  int texture;
}Sprite;


Sprite sprite[numSprites] =
{
  {20.5, 11.5, 10}, //green light in front of playerstart
  //green lights in every room
  {18.5,4.5, 10},
  {10.0,4.5, 10},
  {10.0,12.5,10},
  {3.5, 6.5, 10},
  {3.5, 20.5,10},
  {3.5, 14.5,10},
  {14.5,20.5,10},
  
  //row of pillars in front of wall: fisheye test
  {18.5, 10.5, 9},
  {18.5, 11.5, 9},
  {18.5, 12.5, 9},
  
  //some barrels around the map
  {21.5, 1.5, 8},
  {15.5, 1.5, 8},
  {16.0, 1.8, 8},
  {16.2, 1.2, 8},
  {3.5,  2.5, 8},
  {9.5, 15.5, 8},
  {10.0, 15.1,8},
  {10.5, 15.8,8},
};


//arrays used to sort the sprites
int spriteOrder[numSprites];
double spriteDistance[numSprites];

//function used to sort the sprites
void combSort(int* order, double* dist, int amount);


double posX = 22, posY = 11.5;  //x and y start position
double dirX = -1.0, dirY = 0.0; //initial direction vector
double planeX = 0.0, planeY = 0.66; //the 2d raycaster version of camera plane

double ftime = 0.0; //time of current frame
double oldTime = 0.0; //time of previous frame
int false = 0;
int ch;



int main(int argc, char *argv[])
{
	
	if (!initDemoConfig("config.cfg"))
		return 0;
		
	printf("use console for input\nkeys: a,w,s,d,z and x\n");
		
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLUE));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lClearFrame(frame);
	
	int frames=0;
	//int sleepcount=0;
	//uint64_t startTick = GetTickCount();
	ftime = (double)GetTickCount();
	
	double frameTime = 0.0;
	int w = DWIDTH;
	int h = DHEIGHT;
	int ttextures = 12;
	TFRAME *images[ttextures+1];
	//1D Zbuffer
	double ZBuffer[DWIDTH];

	//walls
	images[0] = lNewImage(hw, L"images/wolf3d/eagle.bmp", LFRM_BPP_24);
	images[1] = lNewImage(hw, L"images/wolf3d/redbrick.bmp", LFRM_BPP_24);
	images[2] = lNewImage(hw, L"images/wolf3d/purplestone.bmp", LFRM_BPP_24);
	images[3] = lNewImage(hw, L"images/wolf3d/greystone.bmp", LFRM_BPP_24);
	images[4] = lNewImage(hw, L"images/wolf3d/bluestone.bmp", LFRM_BPP_24);
	images[5] = lNewImage(hw, L"images/wolf3d/mossy.bmp", LFRM_BPP_24);
	images[6] = lNewImage(hw, L"images/wolf3d/wood.bmp", LFRM_BPP_24);
	images[7] = lNewImage(hw, L"images/wolf3d/colorstone.bmp", LFRM_BPP_24);
	images[11] = lNewImage(hw, L"images/wolf3d/wood_ss.bmp", LFRM_BPP_24);
	
	// sprites
	images[8] = lNewImage(hw, L"images/wolf3d/barrel.bmp", LFRM_BPP_24);
	images[9] = lNewImage(hw, L"images/wolf3d/pillar.bmp", LFRM_BPP_24);
	images[10] = lNewImage(hw, L"images/wolf3d/greenlight.bmp", LFRM_BPP_24);

	int i;
	for (i = 0; i < ttextures; i++){
		if (images[i] == NULL){
			printf("lNewImage() returned NULL\n");
			return 0;
		}
	}

	int x;
    while (ch != 27 && ch != 13){
		//speed modifiers
		for (x = 0; x < w; x++){           
            //calculate ray position and direction 
            double cameraX = 2.0 * x / (double)w - 1.0; //x-coordinate in camera space         
            double rayPosX = posX;
            double rayPosY = posY;
            double rayDirX = dirX + planeX * cameraX;
            double rayDirY = dirY + planeY * cameraX;
            //which box of the map we're in  
            int mapX = (int)(rayPosX);
            int mapY = (int)(rayPosY);
           
            //length of ray from current position to next x or y-side
            double sideDistX;
            double sideDistY;
           
             //length of ray from one x or y-side to next x or y-side
            double deltaDistX = sqrt(1.0 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
            double deltaDistY = sqrt(1.0 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
            double perpWallDist;
           
            //what direction to step in x or y-direction (either +1 or -1)
            int stepX;
            int stepY;
                      
            int hit = 0; //was there a wall hit?
            int side; //was a NS or a EW wall hit?
            //calculate step and initial sideDist                                 
            if (rayDirX < 0){
                stepX = -1;
                sideDistX = (rayPosX - mapX) * deltaDistX;
            }else{
                stepX = 1;
                sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
            }
                      
            if (rayDirY < 0){
                stepY = -1;
                sideDistY = (rayPosY - mapY) * deltaDistY;
            }else{
                stepY = 1;
                sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
            }
  
            //perform DDA       
            while (hit == 0){                  
                //jump to next map square, OR in x-direction, OR in y-direction          
                if (sideDistX < sideDistY){
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                }else{
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
         
                //Check if ray has hit a wall           
                if (worldMap[mapX][mapY] > 0) hit = 1;             
            }

            //Calculate distance projected on camera direction (oblique distance will give fisheye effect!)      
            if (side == 0)
            	perpWallDist = fabs((mapX - rayPosX + (1 - stepX) / 2) / rayDirX);
            else
            	perpWallDist = fabs((mapY - rayPosY + (1 - stepY) / 2) / rayDirY);
            
            //Calculate height of line to draw on screen           
            int lineHeight = abs((int)(h / perpWallDist));       
           
            //calculate lowest and highest pixel to fill in current stripe
            int drawStart = -lineHeight / 2 + h / 2;
            if (drawStart < 0)drawStart = 0;
            int drawEnd = lineHeight / 2 + h / 2;
            if (drawEnd >= h)drawEnd = h - 1;  
                
		    //texturing calculations
			int texNum = worldMap[mapX][mapY] - 1; //1 subtracted from it so that texture 0 can be used!
   
      		//calculate value of wallX
			double wallX; //where exactly the wall was hit
      		if (side == 1)
      			wallX = rayPosX + ((mapY - rayPosY + (1 - stepY) / 2) / rayDirY) * rayDirX;
      		else
      			wallX = rayPosY + ((mapX - rayPosX + (1 - stepX) / 2) / rayDirX) * rayDirY;
      		wallX -= floor(wallX);
       
      		//x coordinate on the texture
      		int texX = (int)(wallX * (double)texWidth);
      		if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
      		if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;
 
      		
 			//FLOOR CASTING
			double floorXWall, floorYWall; //x, y position of the floor texel at the bottom of the wall

			//4 different wall directions possible
			if(side == 0 && rayDirX > 0){
				floorXWall = mapX;
				floorYWall = mapY + wallX;
			}else if(side == 0 && rayDirX < 0){
				floorXWall = mapX + 1.0;
				floorYWall = mapY + wallX;
			}else if(side == 1 && rayDirY > 0){
        		floorXWall = mapX + wallX;
        		floorYWall = mapY;
			}else{
        		floorXWall = mapX + wallX;
        		floorYWall = mapY + 1.0;
      		} 

             //SET THE ZBUFFER FOR THE SPRITE CASTING
      		ZBuffer[x] = perpWallDist; //perpendicular distance is used
      		
      		double distWall, distPlayer, currentDist;  
			distWall = perpWallDist;
			distPlayer = 0.0;

			if (drawEnd < 0) drawEnd = h; //becomes < 0 when the integer overflows
    
      		int y, r, g, b;
      
     		//draw the floor from drawEnd to the bottom of the screen
      		for(y = drawEnd ; y < h; y++){
        		currentDist = h / (2.0 * y - h); //you could make a small lookup table for this instead

#if (RENDER_FLOOR || RENDER_CEILING)
        		double weight = (currentDist - distPlayer) / (distWall - distPlayer);
        		double currentFloorX = weight * floorXWall + (1.0 - weight) * posX;
        		double currentFloorY = weight * floorYWall + (1.0 - weight) * posY;
#endif        

#if (RENDER_FLOOR)
#if FLOOR_QUALITY_HIGH
        		int floorTexX = (int)(currentFloorX * texWidth) % texWidth;
        		int floorTexY = (int)(currentFloorY * texHeight) % texHeight;
#else
		        int floorTexX = (int)(currentFloorX * texWidth / 4.0) % texWidth;
        		int floorTexY = (int)(currentFloorY * texHeight / 4.0) % texHeight;        
#endif
#endif

#if (RENDER_FLOOR)
        		//floor
        		int colour = (lGetPixel(images[3], floorTexX, floorTexY) >> 1) & 0x7F7F7F /*0x777*/ /*0x6D*/;
#else
        		int colour = 0;
#endif

        		if (DBPP == LFRM_BPP_12){
					r = (colour&0xF00000)>>12;
					g = (colour&0x00F000)>>8;
					b = (colour&0x0000F0)>>4;
					lSetPixel(frame, x, y, r|g|b);
				}else if (DBPP == LFRM_BPP_16){
					r = (colour&0xF80000)>>8;
					g = (colour&0x00FC00)>>5;
					b = (colour&0x0000F8)>>3;
					lSetPixel(frame, x, y, r|g|b);
				}else if (DBPP == LFRM_BPP_15){	
					r = (colour&0xF80000)>>9;
					g = (colour&0x00F800)>>6;
					b = (colour&0x0000F8)>>3;
					lSetPixel(frame, x, y, r|g|b);
				}else{
					lSetPixel(frame, x, y, 0xFF000000|colour);
				}
         
      			 //ceiling (symmetrical!)
#if RENDER_CEILING
        		colour = lGetPixel(images[6], floorTexX, floorTexY);
#else
        		colour = 0xFF00FF; //0xE3 /*0x61*/;
#endif
				
				if (DBPP == LFRM_BPP_12){
					r = (colour&0xF00000)>>12;
					g = (colour&0x00F000)>>8;
					b = (colour&0x0000F0)>>4;
					lSetPixel(frame, x, (h - y)-1, r|g|b);
				}else if (DBPP == LFRM_BPP_16){
					r = (colour&0xF80000)>>8;
					g = (colour&0x00FC00)>>5;
					b = (colour&0x0000F8)>>3;
					lSetPixel(frame, x, (h - y)-1, r|g|b);
				}else if (DBPP == LFRM_BPP_15){	
					r = (colour&0xF80000)>>9;
					g = (colour&0x00F800)>>6;
					b = (colour&0x0000F8)>>3;
					lSetPixel(frame, x, (h - y)-1, r|g|b);
				}else{
					lSetPixel(frame, x, (h - y)-1, 0xFF000000|colour);
				}
      		}
      		 
			// WALLS
      		for (y = drawStart; y < drawEnd; y++){
        		int d = y * 256 - h * 128 + lineHeight * 128;  //256 and 128 factors to avoid floats
        		int texY = ((d * texHeight) / lineHeight) / 256;

				int colour = lGetPixel(images[texNum], texX, texY);
        		//make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
        		if (side == 1)
        			colour = (colour >> 1) &  0x7F7F7F /*0x777*//*0x6D*/;
        			
				if (DBPP == LFRM_BPP_12){
					r = (colour&0xF00000)>>12;
					g = (colour&0x00F000)>>8;
					b = (colour&0x0000F0)>>4;
					lSetPixel(frame, x, y, r|g|b);
				}else if (DBPP == LFRM_BPP_16){
					r = (colour&0xF80000)>>8;
					g = (colour&0x00FC00)>>5;
					b = (colour&0x0000F8)>>3;
					lSetPixel(frame, x, y, r|g|b);
				}else if (DBPP == LFRM_BPP_15){	
					r = (colour&0xF80000)>>9;	// 5
					g = (colour&0x00F800)>>6;	// 5
					b = (colour&0x0000F8)>>3;	// 5
					lSetPixel(frame, x, y, r|g|b);
				}else{
					lSetPixel(frame, x, y, 0xFF000000|colour);
				}
      		}
    	}


	    //SPRITE CASTING
    	//sort sprites from far to close
    	for(int i = 0; i < numSprites; i++){
      		spriteOrder[i] = i;
      		spriteDistance[i] = ((posX - sprite[i].x) * (posX - sprite[i].x) + (posY - sprite[i].y) * (posY - sprite[i].y)); //sqrt not taken, unneeded
    	}
    	combSort(spriteOrder, spriteDistance, numSprites);
     
    	//after sorting the sprites, do the projection and draw them
    	for (i = 0; i < numSprites; i++){
      		//translate sprite position to relative to camera
			double spriteX = sprite[spriteOrder[i]].x - posX;
			double spriteY = sprite[spriteOrder[i]].y - posY;
         
      		//transform sprite with the inverse camera matrix
      		// [ planeX   dirX ] -1                                       [ dirY      -dirX ]
			// [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
			// [ planeY   dirY ]                                          [ -planeY  planeX ]
      
			double invDet = 1.0 / (planeX * dirY - dirX * planeY); //required for correct matrix multiplication
			double transformX = invDet * (dirY * spriteX - dirX * spriteY);
			double transformY = invDet * (-planeY * spriteX + planeX * spriteY); //this is actually the depth inside the screen, that what Z is in 3D       
            
			int spriteScreenX = (int)((w / 2) * (1 + transformX / transformY));
      
			//calculate height of the sprite on screen
			int spriteHeight = abs((int)(h / (transformY))); //using "transformY" instead of the real distance prevents fisheye
			//calculate lowest and highest pixel to fill in current stripe
			int drawStartY = -spriteHeight / 2 + h / 2;
			if (drawStartY < 0) drawStartY = 0;
			int drawEndY = spriteHeight / 2 + h / 2;
			if (drawEndY >= h) drawEndY = h - 1;
      
			//calculate width of the sprite
			int spriteWidth = abs((int)(h / (transformY)));
			int drawStartX = -spriteWidth / 2 + spriteScreenX;
			if (drawStartX < 0) drawStartX = 0;
			int drawEndX = spriteWidth / 2 + spriteScreenX;
			if (drawEndX >= w) drawEndX = w - 1;
      
			//loop through every vertical stripe of the sprite on screen
			int r,g,b,y, stripe;
			for (stripe = drawStartX; stripe < drawEndX; stripe++){
				int texX = (int)(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * texWidth / spriteWidth) / 256;
				//the conditions in the if are:
				//1) it's in front of camera plane so you don't see things behind you
				//2) it's on the screen (left)
				//3) it's on the screen (right)
				//4) ZBuffer, with perpendicular distance
				if (transformY > 0 && stripe > 0 && stripe < w && transformY < ZBuffer[stripe]){
					for (y = drawStartY; y < drawEndY; y++){ //for every pixel of the current stripe
						int d = (y) * 256 - h * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats
						int texY = ((d * texHeight) / spriteHeight) / 256;
          				int colour = lGetPixel(images[sprite[spriteOrder[i]].texture], texX, texY);
						if (colour & 0x00FFFFFF){
							if (DBPP == LFRM_BPP_12){
								r = (colour&0xF00000)>>12;
								g = (colour&0x00F000)>>8;
								b = (colour&0x0000F0)>>4;
								lSetPixel(frame, stripe, y, r|g|b);
							}else if (DBPP == LFRM_BPP_16){
								r = (colour&0xF80000)>>8;
								g = (colour&0x00FC00)>>5;
								b = (colour&0x0000F8)>>3;
								lSetPixel(frame, stripe, y, r|g|b);
							}else if (DBPP == LFRM_BPP_15){
								r = (colour&0xF80000)>>9;	// 5
								g = (colour&0x00F800)>>6;	// 5
								b = (colour&0x0000F8)>>3;	// 5
								lSetPixel(frame, stripe, y, r|g|b);
							}else{
								lSetPixel(frame, stripe, y, 0xFF000000|colour);
							}
						}
					}
				}
			}
		}


        oldTime = ftime;
        ftime = (double)GetTickCount();
        frameTime = (ftime - oldTime) / 1000.0; //frameTime is the time this frame has taken, in seconds       
        //timing for input and FPS counter

        lRefreshAsync(frame, 1);
        frames++;
		//lClearFrame(frame);
		
        double moveSpeed = (double)frameTime * 5.0; //the constant value is in squares/second
        double rotSpeed = (double)frameTime * 3.0; //the constant value is in radians/second
        
		if (kbhit()){
			ch = getch();
        
        	//move forward if no wall in front of you     
			if (ch=='w'){
            	if(worldMap[(int)(posX + dirX * moveSpeed)][(int)(posY)] == false)
            		posX += dirX * moveSpeed;
            	if(worldMap[(int)(posX)][(int)(posY + dirY * moveSpeed)] == false)
            		posY += dirY * moveSpeed;
			}
			
        	//move backwards if no wall behind you       
			if (ch=='s'){
            	if (worldMap[(int)(posX - dirX * moveSpeed)][(int)(posY)] == false)
            		posX -= dirX * moveSpeed;
            	if (worldMap[(int)(posX)][(int)(posY - dirY * moveSpeed)] == false)
            		posY -= dirY * moveSpeed;
        	}   

			if (ch=='z'){
            	if (worldMap[(int)(posX - planeX * moveSpeed)][(int)(posY)] == false)
            		posX -= planeX * moveSpeed;
            	if (worldMap[(int)(posX)][(int)(posY - planeY * moveSpeed)] == false)
            		posY -= planeY * moveSpeed;
        	}   
        	
  			if (ch=='x'){
            	if(worldMap[(int)(posX + planeX * moveSpeed)][(int)(posY)] == false)
            		posX += planeX * moveSpeed;
            	if(worldMap[(int)(posX)][(int)(posY + planeY * moveSpeed)] == false)
            		posY += planeY * moveSpeed;
			}
			      	
       		 //rotate to the right   
        	if (ch=='d'){
            	//both camera direction and camera plane must be rotated
            	double oldDirX = dirX;
            	dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
            	dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
            	double oldPlaneX = planeX;
            	planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
            	planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
        	}
        	
        	//rotate to the left       
        	if (ch=='a'){
            	//both camera direction and camera plane must be rotated         
            	double oldDirX = dirX;
            	dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
            	dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
            	double oldPlaneX = planeX;
            	planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
            	planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
	        }
			lSleep(1);
		}else{
			lSleep(10);
			//sleepcount++;
		}
    }
	//u64 endTick = GetTickCount() - (sleepcount*30); //take away lSleep period
	//float totaltime = (float)(endTick - startTick)/1000.0;
	//int fps = frames / totaltime;
	//printf("%i %.2f %i\n",frames,totaltime,fps);
	
	for (i = 0; i < ttextures; i++)
		lDeleteFrame(images[i]);
	demoCleanup();
	return 1;
}  

static void swapD (double *a, double *b)
{
	double tmp = *a;
	*a = *b;
	*b = tmp;
}

static void swapI (int *a, int *b)
{
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}


//sort algorithm
void combSort (int *order, double *dist, int amount)
{
  int gap = amount;
  int swapped = 0;
  
  while (gap > 1 || swapped){
    //shrink factor 1.3
    gap = (gap * 10) / 13;
    if( gap == 9 || gap == 10) gap = 11;
    if (gap < 1) gap = 1;
    swapped = 0;
    int i;
    for (i = 0; i < amount - gap; i++){
      int j = i + gap;
      if (dist[i] < dist[j]){
        swapD(&dist[i], &dist[j]);
        swapI(&order[i], &order[j]);
        swapped = 1;
      }
    }
  }
}

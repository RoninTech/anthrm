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


#define mapWidth 24
#define mapHeight 24

const int worldMap[mapWidth][mapHeight]={
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1,
    1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1,
    1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

double posX = 22, posY = 12;  //x and y start position
double dirX = -1, dirY = 0; //initial direction vector
double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

double ftime = 0; //time of current frame
double oldTime = 0; //time of previous frame
int false = 0;
int ch;



int main (int argc, char *argv[])
{
	
	if (!initDemoConfig("config.cfg"))
		return 0;

	//lSetCapabilities(hw, CAP_BACKBUFFER, CAP_STATE_ON;
	
	if (DBPP != LFRM_BPP_1){
		DBPP = LFRM_BPP_1;
		lDeleteFrame(frame);
		frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
	}
		
	int frames=0;
	int sleepcount=0;
	uint64_t startTick = GetTickCount();
	ftime = (double)GetTickCount();

    while(ch != 27){
		lClearFrame(frame);
        int x;
        int w = DWIDTH;
        int h = DHEIGHT;
        
        for(x = 0; x < w; x++){           
            //calculate ray position and direction 
            double cameraX = 2 * x / (double)w - 1; //x-coordinate in camera space         
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
            double deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
            double deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
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
                
             /*
            //choose wall color                
         	int colour;
         	if (!side){
            	switch(worldMap[mapX][mapY]){
                	case 1:  colour = 0xF00;    break; //red
	                case 2:  colour = 0x0F0;  break; //green
                	case 3:  colour = 0x00F;   break; //blue
	                case 4:  colour = 0x0FF;  break; //white
                	default: colour = 0xFF0; break; //yellow
            	}
         	}else{
	            switch(worldMap[mapX][mapY]){
                	case 1:  colour = 0x700;    break; //red
                	case 2:  colour = 0x070;  break; //green
                	case 3:  colour = 0x007;   break; //blue
                	case 4:  colour = 0x077;  break; //white
                	default: colour = 0x770; break; //yellow
            	}
         	}
           
            //draw the pixels of the stripe as a vertical line                                                                       
            lDrawLine(frame, x, drawStart, x, drawEnd, colour);
            */
            if (worldMap[mapX][mapY]==1)
            	lDrawLine(frame, x, drawStart, x, drawEnd, LSP_SET);
            else if (worldMap[mapX][mapY]==2)
            	lDrawLine(frame, x, drawStart, x, drawEnd, LSP_SET);
            else if (worldMap[mapX][mapY]==3)
            	lDrawLineDotted(frame, x, drawStart, x, drawEnd, LSP_SET);
            else if (worldMap[mapX][mapY]==4)
            	lDrawLineDotted(frame, x, drawStart, x, drawEnd, LSP_SET);
            else
            	lDrawLineDotted(frame, x, drawStart, x, drawEnd, LSP_SET);
    	}
    
		
        //timing for input and FPS counter
        oldTime = ftime;
        ftime = (double)GetTickCount();
        double frameTime = (ftime - oldTime) / 1000.0; //frameTime is the time this frame has taken, in seconds       
        lRefresh(frame);

        //speed modifiers
        double moveSpeed = frameTime * 5.0; //the constant value is in squares/second
        double rotSpeed = frameTime * 3.0; //the constant value is in radians/second     

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
        
		}else{
			lSleep(30);
			sleepcount++;
		}
		frames++;
    }
     
	uint64_t endTick = GetTickCount() - (sleepcount*30); //take away lSleep period
	float totaltime = (float)(endTick - startTick)/1000.0;
	int fps = frames / totaltime;
	
	printf("%i %.2f %i\n",frames,totaltime,fps);
	
	demoCleanup();
	return 1;
}  



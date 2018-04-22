
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



#include "../common.h"
#include "stc.h"


const int blockw = 14;
const int blockh = 11;
const int posX = 50;
const int posY = 10;

	
static const int colourLookupTable[9] = {
	0xFF000000,
	0xFF00FFFF,
	0xFFFF0000,
	0xFF0000FF,
	0xFFFF6400,
	0xFF00FF00,
	0xFFFFFF00,
	0xFFFF00FF,
	0xFFFFFFFF
};


static int colourLookup (const int idx)
{
	return colourLookupTable[idx];
}

int platformInit (StcGame *game)
{
//	printf("platformInit\n");
	
	game->tetris = my_calloc(1, sizeof(StcPlatform));
	if (game->tetris)
		return ERROR_NONE;
	else
		return ERROR_NO_MEMORY;
}

/* Clear resources used by platform */
void platformEnd (StcGame *game)
{
	my_free(game->tetris);
	//printf("platformEnd\n");
}

/* Read input device and notify game */
void platformReadInput (StcGame *game)
{
	//printf("platformReadInput\n");
}

int tetrisInput (TVLCPLAYER *vp, TTETRIS *tetris, const int key)
{
	
	StcGame *game = tetris->game;

	switch (key){
	  case 'a':
	  case 'A':// printf("left\n");
	  	gameOnKeyDown(game, EVENT_MOVE_LEFT);
	  	gameOnKeyUp(game, EVENT_MOVE_LEFT);
	  	break;
	  	  
	  case 'd':	
	  case 'D':// printf("right\n");
	  	gameOnKeyDown(game, EVENT_MOVE_RIGHT);
	  	gameOnKeyUp(game, EVENT_MOVE_RIGHT);
	  	break;

	  case 'w':
	  case 'W':// printf("up\n");
		gameOnKeyDown(game, EVENT_ROTATE_CCW);
		gameOnKeyUp(game, EVENT_ROTATE_CCW);
		break;
	  	  
	  case 'e':		  	  	
	  case 'E':// printf("end\n");
		gameOnKeyDown(game, EVENT_ROTATE_CW);
		gameOnKeyUp(game, EVENT_ROTATE_CW);
		break;

	  case 's':
	  case 'S': //printf("dn\n");
		gameOnKeyDown(game, EVENT_MOVE_DOWN);
		gameOnKeyUp(game, EVENT_MOVE_DOWN);
		break;
	  	  			  	
	  case ' ' :// printf("drop\n");
		gameOnKeyDown(game, EVENT_DROP);
		gameOnKeyUp(game, EVENT_DROP);
		break;
	
	  case 'r': 
	  case 'R': 
		gameOnKeyDown(game, EVENT_RESTART);
		gameOnKeyUp(game, EVENT_RESTART);
		break;
			
	  case 'p': 
	  case 'P': 
		gameOnKeyDown(game, EVENT_PAUSE);
		gameOnKeyUp(game, EVENT_PAUSE);
		break;

	  case 13:
	  case 27:
	  case 'q':
	  case 'Q':
		setPageSec(vp, -1);
		gameOnKeyDown(game, EVENT_QUIT);
		break; 
	}

	return 1;
}

void drawCell (TFRAME *frame, int x, int y, const int idx)
{
	x = (x * blockw) + posX;
	y = (y * blockh) + posY;
	lDrawRectangleFilled(frame, x, y, x+blockw, y+blockh, colourLookup(idx));
}

void platformOnFilledRow (StcGame *game, int y)
{
	//drawCell(game->tetris->frame, 0, y, 8);
	//printf("row filled: %i\n", y);
}

void platformOnFilledRows (StcGame *game, int rowsFilled)
{
	//printf("total rows filled: %i\n", rowsFilled);
}

/* Render the state of the game */
void platformRenderGame (StcGame *game)
{
	//printf("platformRenderGame\n");
	

	TFRAME *frame = game->tetris->frame;

	for (int j = 0; j < BOARD_TILEMAP_HEIGHT; j++){
		for (int i = 0; i < BOARD_TILEMAP_WIDTH; i++){
			int cell = game->map[i][j];
			if (cell)
				drawCell(frame, i, j, cell);
		}
	}

	for (int j = 0; j < TETROMINO_SIZE; j++) {
		for (int i = 0; i < TETROMINO_SIZE; i++) {
			int cell = game->fallingBlock.cells[i][j];
			if (cell)
				drawCell(frame, game->fallingBlock.x + i, game->fallingBlock.y + j, cell);
		}
	}

	for (int j = 0; j < TETROMINO_SIZE; j++) {
		for (int i = 0; i < TETROMINO_SIZE; i++) {
			int cell = game->nextBlock.cells[i][j];
			if (cell)
				drawCell(frame, 13 + game->nextBlock.x + i, 9 + game->nextBlock.y + j, cell);
		}
	}

	lDrawLine(frame, posX-1, posY, posX-1, posY+(BOARD_TILEMAP_HEIGHT*blockh)+1, 0xFFFF0000);
	lDrawLine(frame, posX+(BOARD_TILEMAP_WIDTH*blockw)+1, posY, posX+(BOARD_TILEMAP_WIDTH*blockw)+1, posY+(BOARD_TILEMAP_HEIGHT*blockh)+1, 0xFFFF0000);
	lDrawLine(frame, posX-1, posY+(BOARD_TILEMAP_HEIGHT*blockh)+1, posX+(BOARD_TILEMAP_WIDTH*blockw)+1, posY+(BOARD_TILEMAP_HEIGHT*blockh)+1, 0xFFFF0000);

	lPrintf(frame, 224, 10, BFONT, LPRT_CPY, "Score: %i", game->stats.score);
	lPrintf(frame, 224, 30, BFONT, LPRT_CPY, "Lines: %i", game->stats.lines);
	lPrintf(frame, 224, 50, BFONT, LPRT_CPY, "Level: %i", game->stats.level);
}


/* Return the current system time in milliseconds */
long platformGetSystemTime (void)
{
	return timeGetTime(); //GetTickCount();
}

/* Initialize the random number generator */
void platformSeedRandom (long seed)
{
	srand(seed);
}

/* Return a random positive integer number */
int platformRandom (void)
{
	return rand();
}

int tetrisInit (TTETRIS *tetris, TFRAME *frame)
{
	tetris->game = my_calloc(1, sizeof(StcGame));
	if (!tetris->game) return 0;

	
	int err = gameInit(tetris->game);
	if (err != ERROR_NONE){
		printf("stc init failed: %i\n", err);
	}
	
	tetris->game->tetris->frame = frame;
	return 1;
}

void tetrisDraw (TTETRIS *tetris, TFRAME *frame)
{
	gameUpdate(tetris->game);
	//tetris->game->stateChanged = 0;
}

void tetrisClose (TTETRIS *tetris)
{	
	gameEnd(tetris->game);
	my_free(tetris->game);
	
}

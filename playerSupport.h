#ifndef PLAYER_SUPPORT_H
#define PLAYER_SUPPORT_H

#include "matrix.h"

unsigned short player1_col = 0xFFE7;
unsigned short player2_col = 0xE7FF;
unsigned char player1_row = 0x03;
unsigned char player2_row = 0xC0;

unsigned char player1_score = 0x00;
unsigned char player2_score = 0x00;
unsigned char gameOver = 0x00;

unsigned char greenPoints = 5;

enum PlayerColors{RED, BLUE};
enum Directions{UP, DOWN, LEFT, RIGHT};

void retrievePoint(enum PlayerColors playerColor, unsigned char player_row, unsigned short player_col){
	unsigned char i = 0x00;
	unsigned char j = 0x00;
	unsigned char rows[2];
	unsigned char cols[2];

	unsigned short temp_col = player_col;
	
	if(playerColor == BLUE){
		temp_col = temp_col >> 8;
	} 

	for(i = 0; i < 8; i++){
		if(GetBit(player_row, i)){
			rows[j] = i;
			j++;
		}
	}
	
	i = 0; j = 0;
	
	for(i = 0; i < 8; i++){
		if(!GetBit(temp_col, i)){
			cols[j] = i;
			j++;
		}
	}	
	
	i = 0; j = 0;
	
	unsigned char tempScore = 0x00;
	
	for(i = 0; i < 2; i++){
		for(j = 0; j < 2; j++){
			unsigned char row_pt = rows[i];
			unsigned char col_pt = cols[j];
			if(greenMatrix[row_pt][col_pt] == 0){
				tempScore++;
				greenMatrix[row_pt][col_pt] = 1;
				greenPoints--;
			}
		}
	}
	
	switch(playerColor){
		case RED:
			player1_score += tempScore;
			break;
		case BLUE:
			player2_score += tempScore;
			break;
	}
}

unsigned char isValidMove(enum PlayerColors playerColor, enum Directions direction){
	unsigned long temp = 0 | 0xFFFF;
	unsigned long check = 0 | 0x00FF;
	switch(playerColor){
		case RED:
			switch(direction){
				case UP:
					return (player1_row != 0xC0);
				case DOWN:
					return (player1_row != 0x03);
				case LEFT:
					return ((player1_col & temp) != 0xFF3F);
				case RIGHT:
					return ((player1_col & temp) != 0xFFFC);
			}
			break;
		case BLUE:
			switch(direction){
				case UP:
					return (player2_row != 0xC0);
				case DOWN:
					return (player2_row != 0x03);
				case LEFT:
					return ((player2_col & temp) != 0x3FFF);
				case RIGHT:
					return ((player2_col & temp) != 0xFCFF);
			}
			break;
	}
	
	return 0;
}

#endif
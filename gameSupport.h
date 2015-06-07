#ifndef GAME_SUPPORT_H
#define GAME_SUPPORT_H

void resetGameData(){
	gameOver = 0;
	
	player1_row = 0x03;
	player1_col = 0xFFE7;
	player1_score = 0;
	
	player2_row = 0xC0;
	player2_col = 0xE7FF;
	player2_score = 0;
}

void scoreBoard(char buf[]){
	unsigned char num = 1;
	sprintf(buf, "P1: %d P2: %d ", player1_score, player2_score);
	buf[15] = NULL;
}

#endif
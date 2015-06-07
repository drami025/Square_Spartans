#include <avr/io.h>
#include <avr/interrupt.h>
#include "timerTaskSupport.h"
#include "io.c"
#include "controller.h"
#include "gameSupport.h"

const unsigned char TASKNUM = 7;
task tasks[7];

void TimerISR(){
	TimerFlag=1;
	unsigned char i;
	for(i = 0; i < TASKNUM; i++){
		if(tasks[i].elapsedTime >= tasks[i].period){
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasks[i].period;
	}
}

enum LEDMatrix_States{LEDMatrix_Init, LEDMatrix_Red, LEDMatrix_Blue, LEDMatrix_Green, LEDMatrix_GameOver}LEDMatrix_State;
unsigned char green_row = 0x01;
unsigned char green_col = 0x00;
unsigned char green_index = 0x00;

int TickFct_LEDMatrix(int state){
	switch(state){
		case LEDMatrix_Init:
			state = LEDMatrix_Red;
			break;
		case LEDMatrix_Red:
			if(!gameOver){
				state = LEDMatrix_Blue;
			}	
			else{
				state = LEDMatrix_GameOver;
			}		
			break;
		case LEDMatrix_Blue:
			if(!gameOver){	
				state = LEDMatrix_Green;
			}
			else{
				state = LEDMatrix_GameOver;
			}				
			break;
		case LEDMatrix_Green:
			if(!gameOver){	
				state = LEDMatrix_Red;
			}
			else{
				state = LEDMatrix_GameOver;
			}				
			break;
		case LEDMatrix_GameOver:
			if(gameOver){
				state = LEDMatrix_GameOver;
			}
			else{
				state = LEDMatrix_Init;
			}
			break;
	}
	
	switch(state){
		case LEDMatrix_Init:
			break;
		case LEDMatrix_Red:
			transmit_data_green(0xFF);
			transmit_matrix_data(player1_row);
			transmit_data(player1_col);
			break;
		case LEDMatrix_Blue:
			transmit_data_green(0xFF);
			transmit_matrix_data(player2_row);
			transmit_data(player2_col);
			break;
		case LEDMatrix_Green:
			green_col = readGreenMatrixRow(green_index);
			transmit_data_green(green_col);
			transmit_matrix_data(green_row);
			transmit_data(0xFFFF);
			green_index++;
			green_row = green_row << 1;
			if(green_index > 7){
				green_index = 0;
				green_row = 0x01;
			}
			break;
		case LEDMatrix_GameOver:
			break;
	}
	return state;
}

const unsigned short controllerWaitLimit = 10;

enum Player1_States{Player1_Init, Player1_Up, Player1_Down, Player1_Left, Player1_Right, Player1_Wait} Player1_State;
unsigned short player1_index = 0;

int TickFct_Player1(int state){
	getControllerOneData(controllerOne);
	switch(state){
		case Player1_Init:
			if(controllerOne[5]){
				state = Player1_Up;
			}
			else if(controllerOne[6]){
				state = Player1_Down;
			}
			else if(controllerOne[7]){
				state = Player1_Left;
			}
			else if(controllerOne[8]){
				state = Player1_Right;
			}
			break;
		case Player1_Wait:
			if(player1_index < controllerWaitLimit){
				state = Player1_Wait;
			}
			else{
				state = Player1_Init;
			}
			break;
		default:
			state = Player1_Wait;
			break;
	}
	
	switch(state){
		case Player1_Init:
		player1_index = 0;
			break;
		case Player1_Up:
			if(isValidMove(RED, UP)){
				player1_row = player1_row << 1;
			}
			break;
		case Player1_Down:
			if(isValidMove(RED, DOWN)){
				player1_row = player1_row >> 1;
			}	
			break;
		case Player1_Left:
			if(isValidMove(RED, LEFT)){
				player1_col = ~player1_col;
				player1_col = player1_col << 1;
				player1_col = 0xFF00 | ~player1_col;
			}			
			break;
		case Player1_Right:
			if(isValidMove(RED, RIGHT)){
				player1_col = ~player1_col;
				player1_col = player1_col >> 1;
				player1_col = 0xFF00 | ~player1_col;
			}
		case Player1_Wait:
			player1_index++;
			break;
	}
	
	retrievePoint(RED, player1_row, player1_col);
	
	return state;
}

enum Player2_States{Player2_Init, Player2_Up, Player2_Down, Player2_Left, Player2_Right, Player2_Wait} Player2_State;
unsigned short player2_index = 0;

int TickFct_Player2(int state){
	getControllerTwoData(controllerTwo);
	switch(state){
		case Player2_Init:
			if(controllerTwo[5]){
				state = Player2_Up;
			}
			else if(controllerTwo[6]){
				state = Player2_Down;
			}
			else if(controllerTwo[7]){
				state = Player2_Left;
			}
			else if(controllerTwo[8]){
				state = Player2_Right;
			}
			break;
		case Player2_Wait:
			if(player2_index < controllerWaitLimit){
				state = Player2_Wait;
			}
			else{
				state = Player2_Init;
			}
			break;
		default:
			state = Player2_Wait;
			break;
	}
	
	switch(state){
		case Player2_Init:
			player2_index = 0;
			break;
		case Player2_Up:
			if(isValidMove(BLUE, UP)){
				player2_row = player2_row << 1;
			}
			break;
		case Player2_Down:
			if(isValidMove(BLUE, DOWN)){
				player2_row = player2_row >> 1;
			}	
			break;
		case Player2_Left:
			if(isValidMove(BLUE, LEFT)){
				player2_col = ~player2_col;
				player2_col = player2_col << 1;
				player2_col = 0x00FF | ~player2_col;
			}			
			break;
		case Player2_Right:
			if(isValidMove(BLUE, RIGHT)){
				player2_col = ~player2_col;
				player2_col = player2_col >> 1;
				player2_col = 0x00FF | ~player2_col;
			}
			break;
		case Player2_Wait:
			player2_index++;
			break;
	}
	
	retrievePoint(BLUE, player2_row, player2_col);
	
	return state;
}

enum GreenGenerator_States{GreenGenerator_Init, GreenGenerator_Wait, GreenGenerator_Subtract, GreenGenerator_Populate} GreenGenerator_State;
unsigned short generator_index = 0;
const unsigned short generatorTimeLimit = 250;

int TickFct_GreenGenerator(int state){
	
	switch(state){
		case GreenGenerator_Init:
			setRandomGreenPoints(0x05);
			state = GreenGenerator_Wait;
			break;
		case GreenGenerator_Wait:
			if(generator_index < generatorTimeLimit){
				state = GreenGenerator_Wait;
			}
			else if(!(generator_index < generatorTimeLimit) && !(greenPoints < 5)){
				state = GreenGenerator_Subtract;
			}
			else if(!(generator_index < generatorTimeLimit) && greenPoints < 5){
				state = GreenGenerator_Populate;
			}
			break;
		case GreenGenerator_Subtract:
			state = GreenGenerator_Wait;
			break;
		case GreenGenerator_Populate:
			state = GreenGenerator_Wait;
			break;
	}
	
	switch(state){
		case GreenGenerator_Init:
			break;
		case GreenGenerator_Wait:
			generator_index++;
			break;
		case GreenGenerator_Subtract:
			generator_index = 0;
			break;
		case GreenGenerator_Populate:
			setRandomGreenPoints(5 - greenPoints);
			greenPoints = 5;
			generator_index = 0;
			break;
	}
	return state;
}

enum GameOver_States{GameOver_Init, GameOver_Wait, GameOver_Red_Wins, GameOver_Red_Flash, GameOver_Red_Wait,
					GameOver_Blue_Wins, GameOver_Blue_Flash, GameOver_Blue_Wait, GameOver_Reset};

unsigned char flashRow = 0x01;
unsigned char gameOver_index = 0;
const unsigned char flash_pause = 10;

unsigned char scoreLimit = 20;

int TickFct_GameOver(int state){
	switch(state){
		case GameOver_Init:
			state = GameOver_Wait;
			break;
		case GameOver_Wait:
			if(player1_score < scoreLimit && player2_score < scoreLimit){
				state = GameOver_Wait;
			}
			else if(!(player1_score < scoreLimit) && player2_score < scoreLimit){
				state = GameOver_Red_Wins;
			}
			else if(player1_score < scoreLimit && !(player2_score < scoreLimit)){
				state = GameOver_Blue_Wins;
			}
			break;
		case GameOver_Red_Wins:
			state = GameOver_Red_Flash;
			break;
		case GameOver_Red_Flash:
			state = GameOver_Red_Wait;
			break;
		case GameOver_Red_Wait:
			if(gameOver && !(gameOver_index < flash_pause)){
				state = GameOver_Red_Flash;
			}
			else if(gameOver && gameOver_index < flash_pause){
				state = GameOver_Red_Wait;
			}
			else{
				state = GameOver_Wait;
			}
			break;
		case GameOver_Blue_Wins:
			state = GameOver_Blue_Flash;
			break;
		case GameOver_Blue_Flash:
			state = GameOver_Blue_Wait;
			break;
		case GameOver_Blue_Wait:
			if(gameOver && !(gameOver_index < flash_pause)){
				state = GameOver_Blue_Flash;
				break;
			}
			else if(gameOver && gameOver_index < flash_pause){
				state = GameOver_Blue_Wait;
			}
			else{
				state = GameOver_Wait;
			}
	}
	
	switch(state){
		case GameOver_Red_Wins:
			gameOver = 1;
			break;
		case GameOver_Blue_Wins:
			gameOver = 1;
			break;
		case GameOver_Red_Flash:
			transmit_matrix_data(flashRow);
			transmit_data_green(0xFF);
			transmit_data(0xFF00);
			flashRow = flashRow << 1;
			gameOver_index = 0;
			break;
		case GameOver_Red_Wait:
			if(!flashRow){
				flashRow = 0x01;
			}
			gameOver_index++;
			break;
		case GameOver_Blue_Flash:
			transmit_matrix_data(flashRow);
			transmit_data_green(0xFF);
			transmit_data(0x00FF);
			flashRow = flashRow << 1;
			gameOver_index = 0;
			break;
		case GameOver_Blue_Wait:
			if(!flashRow){
				flashRow = 0x01;
			}
			gameOver_index++;
			break;
		default:
			break;
	}
	
	return state;
}

enum GameReset_States{GameReset_Init, GameReset_Wait, GameReset_Reset};
	
int TickFct_GameReset(int state){
	unsigned char B0 = ~PINC & 0x80;
	
	switch(state){
		case GameReset_Init:
			if(B0){
				state = GameReset_Wait;
			}
			else{
				state = GameReset_Init;
			}
			break;
		case GameReset_Wait:
			if(!B0){
				state = GameReset_Reset;
			}
			else{
				state = GameReset_Wait;
			}
			break;
		case GameReset_Reset:
			state = GameReset_Init;
			break;
	}
	
	switch(state){
		case GameReset_Init:
			break;
		case GameReset_Wait:
			break;
		case GameReset_Reset:
			resetGameData();
			break;
	}
	return state;
}

enum LCD_States{LCD_Init, LCD_Display, LCD_Wait, LCD_Show_Winner, LCD_Wait_Reset};
unsigned char lcd_red_score = 0;
unsigned char lcd_blue_score = 0;

int TickFct_LCD(int state){
	char buf[16];
	
	switch(state){
		case LCD_Init:
			state = LCD_Display;
			break;
		case LCD_Display:
			state = LCD_Wait;
			break;
		case LCD_Wait:
			if(gameOver){
				state = LCD_Show_Winner;
			}
			else if(!gameOver && lcd_red_score == player1_score && lcd_blue_score == player2_score){
				state = LCD_Wait;
			}
			else{
				state = LCD_Display;
			}
			break;
		case LCD_Show_Winner:
			state = LCD_Wait_Reset;
			break;
		case LCD_Wait_Reset:
			if(gameOver){
				state = LCD_Wait_Reset;
			}
			else{
				state = LCD_Init;
			}
			break;
	}
	
	switch(state){
		case LCD_Init:
			lcd_red_score = 0;
			lcd_blue_score = 0;
			break;
		case LCD_Display:
			scoreBoard(buf);
			LCD_DisplayString(1, buf);
			lcd_red_score = player1_score;
			lcd_blue_score = player2_score;
			break;
		case LCD_Show_Winner:
			if(player1_score > player2_score){
				LCD_DisplayString(1, "Player 1 Wins!");
			}
			else{
				LCD_DisplayString(1, "Player 2 Wins!");
			}
			break;
		default:
			break;
	}
	
	return state;
}

int main(void)
{
	DDRA = 0x33; PORTA = 0xCC; // 33 CC
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0x7F; PORTC = 0x80;
	DDRB = 0xFF; PORTB = 0x00;
	
	LCD_init();
	
	initGreenMatrix();
	srand(1);
	
	unsigned long int matrix_period = 5;
	unsigned long int player1_period = 1000;
	unsigned long int player2_period = 1000;
	unsigned long int green_generate_period = 2000;
	unsigned long int gameover_period = 1000;
	unsigned long int gamereset_period = 1000;
	unsigned long int lcd_display_period = 800;
	
	unsigned long GCD = 1;
	
	GCD = findGCD(player1_period, matrix_period);
	GCD = findGCD(GCD, player2_period);
	GCD = findGCD(GCD, green_generate_period);
	GCD = findGCD(GCD, gameover_period);
	GCD = findGCD(GCD, gamereset_period);
	GCD = findGCD(GCD, lcd_display_period);
	
	unsigned long int matrix_calc = matrix_period/GCD;
	unsigned long int player1_calc = player1_period/GCD;
	unsigned long int player2_calc = player2_period/GCD;
	unsigned long int green_generate_calc = green_generate_period/GCD;
	unsigned long int gameover_calc = gameover_period/GCD;
	unsigned long int gamereset_calc = gamereset_period/GCD;
	unsigned long int lcd_display_calc = lcd_display_period/GCD;
	
	unsigned char i = 0x00;
	
	tasks[i].period = matrix_calc;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = LEDMatrix_Init;
	tasks[i].TickFct = &TickFct_LEDMatrix;
	
	i++;
	
	tasks[i].period = player1_calc;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = Player1_Init;
	tasks[i].TickFct = &TickFct_Player1;
	
	i++;
	
	tasks[i].period = player2_calc;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = Player2_Init;
	tasks[i].TickFct = &TickFct_Player2;
	
	i++;
	
	tasks[i].period = green_generate_calc;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = GreenGenerator_Init;
	tasks[i].TickFct = &TickFct_GreenGenerator;
	
	i++;
	
	tasks[i].period = gameover_calc;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = GameOver_Init;
	tasks[i].TickFct = &TickFct_GameOver;
	
	i++;
		
	tasks[i].period = gamereset_calc;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = GameReset_Init;
	tasks[i].TickFct = &TickFct_GameReset;
	
	i++;
		
	tasks[i].period = lcd_display_calc;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = LCD_Init;
	tasks[i].TickFct = &TickFct_LCD;

	TimerSet(GCD);
	TimerOn();
	
    while(1)
    {
		
    }
}
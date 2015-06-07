#ifndef MATRIX_H
#define MATRIX_H

unsigned char greenMatrix[8][8];

void initGreenMatrix(){
	unsigned char i;
	unsigned char j;
	
	for(i = 0; i < 8; i++){
		for(j = 0; j < 8; j++){
			greenMatrix[i][j] = 1;
		}
	}
}

void setRandomGreenPoints(unsigned char num){
	while(num > 0){
		unsigned char x = rand() % 8;
		unsigned char y = rand() % 8;
		
		if(greenMatrix[y][x]){
			greenMatrix[y][x] = 0;
			num--;
		}
	}
}

unsigned char readGreenMatrixRow(unsigned char row){
	unsigned char temp = 0;
	unsigned char col = 0;
	
	for(col = 0; col < 8; col++){
		if(greenMatrix[row][col]){
			temp |= (0x01 << col);
		}
	}
	
	return temp;
}

void transmit_matrix_data(unsigned char data){
	int i;
	for(i = 0; i < 8; ++i){
		PORTD = 0x08;
		PORTD |= ((data >> i) & 0x01);
		PORTD |= 0x02;
	}
	
	PORTD |= 0x04;
	PORTD = 0x00;
}

void transmit_data_green(unsigned char data){
	int i;
	for(i = 0; i < 8; i++){
		PORTD = 0x80;
		PORTD |= (((data >> i) & 0x01) << 4);
		PORTD |= 0x20;
	}
	
	PORTD |= 0x40;
	PORTD = 0x00;
}

void transmit_data(unsigned short data) {
	int i;
	unsigned char mask = 0xF0 & PORTC;
	for (i = 0; i < 16 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTC = 0x04 | mask;
		// set SER = next bit of data to be sent.
		PORTC |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x02;
		PORTC = 0x04 | mask;
		PORTC |= 0x08;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTC = 0x04 | mask;
	PORTC |= 0x02;
	PORTC = 0x04 | mask;
	PORTC |= 0x08;
	// clears all lines in preparation of a new transmission
	PORTC = 0x00 | mask;
}

#endif MATRIX_H
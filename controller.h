#ifndef CONTROLLER_H
#define	CONTROLLER_H
unsigned char controllerOne[16];
unsigned char controllerTwo[16];

void getControllerOneData(unsigned char controller[]){
	unsigned char data;
	int i = 0;
	PORTA = 0x13;
	
	for(i = 0; i < 16; i++){
		PORTA = 0x11;
		data = ((~PINA >> 2) & 0x01);
		controller[i] = data;
		PORTA = 0x10;
	}
	
	PORTA = 0x11;
}

void getControllerTwoData(unsigned char controller[]){
	unsigned char data;
	int i = 0;
	PORTA = 0x31;
	
	for(i = 0; i < 16; i++){
		PORTA = 0x11;
		data = ((~PINA >> 2) & 0x10);
		controller[i] = data;
		PORTA = 0x01;
	}
	
	PORTA = 0x11;
}
#endif
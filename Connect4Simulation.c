#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "TExaS.h"
#include "image.h"
#include "UART.h"
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

int const rows = 6, cols = 7;
int cellW = 8;
int cellH = 5;
int vlineW =  2;
int hlineH =  1 ;
int leftMargin = 6;
int marginTop = 9;
int i, j, k, turn = 0;
int playMode, kit_mode;
int AImode;
int colsScore[7];
int btlaps = 5;
unsigned int sendC;
unsigned int sendC2;

char temp ;
char temp2;
int cMode;
int cellPlayer[rows][cols];
int currentFreePos[cols];


unsigned int SW1, f1, SW1Pressed = 0;
unsigned int SW2, f2, SW2Pressed = 0;


//for the communication
const int allowedNumOftrialsToOut = 10;
const int allowedNumOftrialsToInput = 1000000;
int numOfTrialsOut;
int numOfTrialsIn;

unsigned char inputFromTheSecondDevice;
unsigned char outputToTheSecondDevice;

//the protocole
unsigned char handshake = 17;
unsigned char confirmation = 200;
unsigned char player1 = 31;
unsigned char player2 = 32;



// function prototypes 
void PortF_Init(void);
void welcomeScreen(void);
void Delay100ms(unsigned long count); // time delay in 0.1 seconds
void drawGrid(void);
int drawInCell(int);
int WhoIsWinner(void);
int selectMode(void);
int kitMode(void);
void Winner(int w);
int bt(int p, int lap);
int drawInCellAI(int p);
int connectionMode(void);
int twoKitsMode();
int main(void){
	
	UART1_Init();
	TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
  //TExaS_Init(NoLCD_NoScope);  // set system clock to 80 MHz
  Random_Init(1);
  Nokia5110_Init();
  PortF_Init();
 
  Nokia5110_Clear();
  welcomeScreen();
  Nokia5110_SetCursor(0, 0); // renders screen
  kit_mode = kitMode();
  // one kit mode
  if(!kit_mode){
		playMode = selectMode();
		// two players mode 
		if(playMode){
			Nokia5110_ClearBuffer();
			drawGrid();
			Nokia5110_DisplayBuffer();
			
			
			while(!WhoIsWinner() && turn < rows*cols){
				drawInCell(turn%2 + 1);
				turn++;
			}
		
			if(WhoIsWinner() == 1){
				//player 1 wins
				Winner(1);
			}
			
			else if(WhoIsWinner() == 2){
				//player 2 wins
				Winner(2);
			}
			
			else{
				//grid if full
				Winner(0);
			}
		}
		
		// Player VS AI 
		else{
				// Player VS AI 
			while(!WhoIsWinner() && turn < rows*cols){
				
				drawInCell(1);
				turn++;

				if(WhoIsWinner() || turn >= rows*cols) break;

				drawInCellAI(2);
				turn++;
			}

			if(WhoIsWinner() == 1){
				//player 1 wins
				Winner(1);
			}
			
			else if(WhoIsWinner() == 2){
				//player 2 wins
				Winner(2);
			}
			
			else{
				//grid if full
				Winner(0);
			}
		}
	}

	// two kits mode
	else{
		// two kits mode
		AImode = twoKitsMode();
		if(AImode){
			cMode = connectionMode();
			if(!cMode){
				
						// master mode
						//outputToTheScreen(2, 2, "Trying to connect..", 1);
						Nokia5110_Clear();
						Nokia5110_SetCursor(2,2);
						Nokia5110_OutString("Trying to connect..");
						GPIO_PORTF_DATA_R = 0x04; //blue led is on
						while(numOfTrialsIn < allowedNumOftrialsToInput){
							
							UART1_OutChar(handshake);
							inputFromTheSecondDevice = UART1_InCharNonBlocking(); // check for confirmation from the slave
							
							//if(codingMode) Delay100ms(5); // delay .5 second
							
							//if there was data, break the loop
							if(inputFromTheSecondDevice){
								/*GPIO_PORTF_DATA_R = 0x02;
								for(i = 0; i < (int)inputFromTheSecondDevice*2; i++){
									GPIO_PORTF_DATA_R ^= 0x04;
									if(!codingMode) Delay100ms(5);
								}*/
								break;
							}
							
							numOfTrialsIn++;
						}
						numOfTrialsIn = 0;
						
						if(inputFromTheSecondDevice == confirmation){
							Nokia5110_Clear();
							Nokia5110_SetCursor(2,2);
							Nokia5110_OutString("Connected");
							
							GPIO_PORTF_DATA_R = 0x08; //green led is on
							Nokia5110_ClearBuffer();
							drawGrid();
							Nokia5110_DisplayBuffer();
							while(!WhoIsWinner() && turn < rows*cols){
								
								sendC = drawInCell(1);
								temp = sendC + '0';
								UART1_OutChar(temp);
								//while( UART1_InCharNonBlocking() != 200);
								if(WhoIsWinner() == 1){
									//player 1 wins
									Winner(1);
									break;
								}
								
								else if(WhoIsWinner() == 2){
									//player 2 wins
									Winner(3);
									break;
								}
								
								temp = UART1_InChar();
								sendC = temp - '0';
								Nokia5110_Clear();
								cellPlayer[currentFreePos[sendC]][sendC] = 2;
								currentFreePos[sendC]++;
								
								
							}
							if(WhoIsWinner() == 1){
									//player 1 wins
									Winner(1);
								}
								
								else if(WhoIsWinner() == 2){
									//player 2 wins
									Winner(3);
								}
								
								else{
									//grid if full
									Winner(0);
								}
						}else{
							Nokia5110_SetCursor(2,2);
							Nokia5110_OutString("Connection Error! ");
							GPIO_PORTF_DATA_R = 0x02; //red led is on
							
							
							//if(!codingMode) Delay100ms(5);
							
						}
						
			}
			else{
				// slave mode
				//[slave] the handshake <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
						//the master should send 17
						while(numOfTrialsIn < allowedNumOftrialsToInput){
							Nokia5110_Clear();
							Nokia5110_SetCursor(2,2);
							Nokia5110_OutString("The handeshake.");
							GPIO_PORTF_DATA_R = 0x04; //blue led is on
							
							//if(codingMode) Delay100ms(5); // delay .5 second
							inputFromTheSecondDevice = UART1_InChar(); // see if there was data sent by the master
							
							//if there was data, break the loop
							if(inputFromTheSecondDevice){
								/*GPIO_PORTF_DATA_R = 0x02;
								for(i = 0; i < (int)inputFromTheSecondDevice*2; i++){
									GPIO_PORTF_DATA_R ^= 0x04;
									if(!codingMode) Delay100ms(5);
								}*/
								break;
							}
							
							numOfTrialsIn++;
						}
						numOfTrialsIn = 0;
						if(inputFromTheSecondDevice == handshake){
							//Nokia5110_Clear();
							//Nokia5110_SetCursor(0,0);
							
							//Nokia5110_OutString("Connected, confirmation code is sent.");
							//Delay100ms(100);
							//Nokia5110_Clear();
							GPIO_PORTF_DATA_R = 0x08; //green led is on
							
							//	isMenuMode = 0; // get out from the menu
							UART1_OutChar(confirmation);
							Nokia5110_ClearBuffer();
							drawGrid();
							Nokia5110_DisplayBuffer();
							
							
							//if(!codingMode) Delay100ms(5);
							//Nokia5110_ClearBuffer();
							//Nokia5110_SetCursor(1,2);
							//Nokia5110_OutString("Connected, confirmation code");
							//drawGrid();
							//Nokia5110_DisplayBuffer();
							while(!WhoIsWinner() && turn < rows*cols){
								/*//sendC = 100;
								temp =  UART1_InChar();
								sendC = temp - '0';
								//UART1_OutChar(200);
								cellPlayer[currentFreePos[sendC]][sendC] = 1;
								currentFreePos[sendC]++;
								Nokia5110_Clear();
								
								sendC = drawInCell(2);
								drawGrid();
								Nokia5110_DisplayBuffer();
								temp = sendC + '0';
								UART1_OutChar(temp);*/
								while(inputFromTheSecondDevice == (temp2 = UART1_InChar()) ){
									//temp2 = UART1_InChar();
								}
								
								
								sendC2 = temp2 - '0';
								Nokia5110_Clear();

								cellPlayer[currentFreePos[sendC2]][sendC2] = 1;
								currentFreePos[sendC2]++;
								if(WhoIsWinner() == 1){
									//player 1 wins
									Winner(1);
									break;
								}
								
								else if(WhoIsWinner() == 2){
									//player 2 wins
									Winner(3);
									break;
								}
								sendC2 = drawInCell(2);
								temp2 = sendC2 + '0';
								UART1_OutChar(temp2);
								
								
								
							}
							if(WhoIsWinner() == 1){
									//player 1 wins
									Winner(1);
								}
								
								else if(WhoIsWinner() == 2){
									//player 2 wins
									Winner(3);
								}
								
								else{
									//grid if full
									Winner(0);
								}
						}else{
							
							Nokia5110_SetCursor(2,2);
							Nokia5110_OutString("Connection Error!");
							GPIO_PORTF_DATA_R = 0x02; //red led is on
							//menuCursor = 0;
							//if(!codingMode) Delay100ms(5);
						}

			}
			
		}
		else{
				cMode = connectionMode();
			if(!cMode){
				
						// master mode
						//outputToTheScreen(2, 2, "Trying to connect..", 1);
						Nokia5110_Clear();
						Nokia5110_SetCursor(2,2);
						Nokia5110_OutString("Trying to connect..");
						GPIO_PORTF_DATA_R = 0x04; //blue led is on
						while(numOfTrialsIn < allowedNumOftrialsToInput){
							
							UART1_OutChar(handshake);
							inputFromTheSecondDevice = UART1_InCharNonBlocking(); // check for confirmation from the slave
							
							//if(codingMode) Delay100ms(5); // delay .5 second
							
							//if there was data, break the loop
							if(inputFromTheSecondDevice){
								/*GPIO_PORTF_DATA_R = 0x02;
								for(i = 0; i < (int)inputFromTheSecondDevice*2; i++){
									GPIO_PORTF_DATA_R ^= 0x04;
									if(!codingMode) Delay100ms(5);
								}*/
								break;
							}
							
							numOfTrialsIn++;
						}
						numOfTrialsIn = 0;
						
						if(inputFromTheSecondDevice == confirmation){
							Nokia5110_Clear();
							Nokia5110_SetCursor(2,2);
							Nokia5110_OutString("Connected");
							
							GPIO_PORTF_DATA_R = 0x08; //green led is on
							Nokia5110_ClearBuffer();
							drawGrid();
							Nokia5110_DisplayBuffer();
							while(!WhoIsWinner() && turn < rows*cols){
								
								sendC = drawInCell(1);
								temp = sendC + '0';
								UART1_OutChar(temp);
								//while( UART1_InCharNonBlocking() != 200);
								if(WhoIsWinner() == 1){
									//player 1 wins
									Winner(1);
									break;
								}
								
								else if(WhoIsWinner() == 2){
									//player 2 wins
									Winner(2);
									break;
								}
								
								temp = UART1_InChar();
								sendC = temp - '0';
								Nokia5110_Clear();
								cellPlayer[currentFreePos[sendC]][sendC] = 2;
								currentFreePos[sendC]++;
								
								
							}
							if(WhoIsWinner() == 1){
									//player 1 wins
									Winner(1);
								}
								
								else if(WhoIsWinner() == 2){
									//player 2 wins
									Winner(2);
								}
								
								else{
									//grid if full
									Winner(0);
								}
						}else{
							Nokia5110_SetCursor(2,2);
							Nokia5110_OutString("Connection Error! ");
							GPIO_PORTF_DATA_R = 0x02; //red led is on
							
							
							//if(!codingMode) Delay100ms(5);
							
						}
						
			}
			else{
				// AI mode
				
				//[slave] the handshake <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
						//the master should send 17
						while(numOfTrialsIn < allowedNumOftrialsToInput){
							Nokia5110_Clear();
							Nokia5110_SetCursor(2,2);
							Nokia5110_OutString("The handeshake.");
							GPIO_PORTF_DATA_R = 0x04; //blue led is on
							
							//if(codingMode) Delay100ms(5); // delay .5 second
							inputFromTheSecondDevice = UART1_InChar(); // see if there was data sent by the master
							
							//if there was data, break the loop
							if(inputFromTheSecondDevice){
								/*GPIO_PORTF_DATA_R = 0x02;
								for(i = 0; i < (int)inputFromTheSecondDevice*2; i++){
									GPIO_PORTF_DATA_R ^= 0x04;
									if(!codingMode) Delay100ms(5);
								}*/
								break;
							}
							
							numOfTrialsIn++;
						}
						numOfTrialsIn = 0;
						if(inputFromTheSecondDevice == handshake){
							//Nokia5110_Clear();
							//Nokia5110_SetCursor(0,0);
							
							//Nokia5110_OutString("Connected, confirmation code is sent.");
							//Delay100ms(100);
							//Nokia5110_Clear();
							GPIO_PORTF_DATA_R = 0x08; //green led is on
							
							//	isMenuMode = 0; // get out from the menu
							UART1_OutChar(confirmation);
							Nokia5110_ClearBuffer();
							drawGrid();
							Nokia5110_DisplayBuffer();
							
							
							//if(!codingMode) Delay100ms(5);
							//Nokia5110_ClearBuffer();
							//Nokia5110_SetCursor(1,2);
							//Nokia5110_OutString("Connected, confirmation code");
							//drawGrid();
							//Nokia5110_DisplayBuffer();
							while(!WhoIsWinner() && turn < rows*cols){
								/*//sendC = 100;
								temp =  UART1_InChar();
								sendC = temp - '0';
								//UART1_OutChar(200);
								cellPlayer[currentFreePos[sendC]][sendC] = 1;
								currentFreePos[sendC]++;
								Nokia5110_Clear();
								
								sendC = drawInCell(2);
								drawGrid();
								Nokia5110_DisplayBuffer();
								temp = sendC + '0';
								UART1_OutChar(temp);*/
								while(inputFromTheSecondDevice == (temp2 = UART1_InChar()) ){
									//temp2 = UART1_InChar();
								}
								
								
								sendC2 = temp2 - '0';
								Nokia5110_Clear();

								cellPlayer[currentFreePos[sendC2]][sendC2] = 1;
								currentFreePos[sendC2]++;
								if(WhoIsWinner() == 1){
									//player 1 wins
									Winner(1);
									break;
								}
								
								else if(WhoIsWinner() == 2){
									//player 2 wins
									Winner(2);
									break;
								}
								sendC2 = drawInCellAI(2);
								temp2 = sendC2 + '0';
								UART1_OutChar(temp2);
								
								
								
							}
							if(WhoIsWinner() == 1){
									//player 1 wins
									Winner(1);
								}
								
								else if(WhoIsWinner() == 2){
									//player 2 wins
									Winner(2);
								}
								
								else{
									//grid if full
									Winner(0);
								}
						}else{
							
							Nokia5110_SetCursor(2,2);
							Nokia5110_OutString("Connection Error!");
							GPIO_PORTF_DATA_R = 0x02; //red led is on
							//menuCursor = 0;
							//if(!codingMode) Delay100ms(5);
						}

			}

		}
	}
}

void Delay100ms(unsigned long count){unsigned long volatile time;
	
  while(count>0){
    time = 72724;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
// generate computer descision 
// backtrack (do recursion undo) function to predict 4 moves in advance  

int bt(int p, int lap){
	int ii;
	int ret, temp = 0, newp = (lap%2 ? (p == 1 ? 2 : 1) : p);
	
	for(ii = 0 ; ii < cols && !lap ; ii++){
		colsScore[ii] = - 1e9;
	}
	
	if(lap == btlaps)
		return 0;
	
	// do 
	for(ii = 0 ; ii < cols ; ii++){
		ret = -1;
		if(currentFreePos[ii] < rows){
			cellPlayer[currentFreePos[ii]][ii] = newp;
			currentFreePos[ii]++;
			if(WhoIsWinner() == p){
				if(lap)
					ret = (btlaps - lap);
				else
					ret = ii;
			}
			
			else if(WhoIsWinner()){
				ret = (lap - btlaps);
			}
			// recursion 
			else{
				if(lap)
					temp += bt(p , lap + 1);
				else
					colsScore[ii] = bt(p , lap + 1);
			}
			// undo
			currentFreePos[ii]--;
			cellPlayer[currentFreePos[ii]][ii] = 0;
		}
		if(ret != -1)
			return ret;
	}
	
	
	// getting the AI descision to play
	if(lap)
		return temp;
	else{
		ret = 0;
		for(ii = 1 ; ii < cols ; ii++){
			if(colsScore[ii] > colsScore[ret])
				ret = ii;
		}
		
		for(ii = 0 ; ii < 4 ; ii++)
			if(colsScore[3 - ii] == colsScore[ret])
				return (3-ii);
			else if(colsScore[3 + ii] == colsScore[ret])
				return (3+ii);
	}
	return ret;
}

// draw AI play in the grid 
int drawInCellAI(int p){
	int C ;
	int x = leftMargin + vlineW + 1 , y = marginTop; 

	Nokia5110_ClearBuffer();
	drawGrid();
	Nokia5110_PrintBMP(x , y , (p == 1 ? p1 : p2), 0);
	Nokia5110_PrintBMP(SCREENW - 15 ,y - 4 , (p == 1 ? PX : AI), 0);
	Nokia5110_DisplayBuffer();
	C = bt(p , 0);
	
	while(x/10 < C){
		Nokia5110_ClearBuffer();
		drawGrid();
		x += (cellW + vlineW);
		Nokia5110_PrintBMP(x , y , (p == 1 ? p1 : p2), 0);
		Nokia5110_PrintBMP(SCREENW - 15 ,y - 4 , (p == 1 ? PX : AI), 0);
		Nokia5110_DisplayBuffer();
		Delay100ms(10);
	}
	
	cellPlayer[currentFreePos[C]][C] = p;
	currentFreePos[C]++;
	
	Delay100ms(10);
	Nokia5110_ClearBuffer();
	drawGrid();
	Nokia5110_DisplayBuffer();
	return C;
}
// update the grid with any player move 
int drawInCell(int p){
	int C = 0;
	int x = leftMargin + vlineW + 1 , y = marginTop; 
	Nokia5110_ClearBuffer();
	Nokia5110_PrintBMP(x , y , (p == 1 ? p1 : p2), 0);
	Nokia5110_PrintBMP(SCREENW - 15 ,y - 4 , (p == 1 ? PX : Pplus), 0);

	drawGrid();
	Nokia5110_DisplayBuffer();
	while(1){
		SW1Pressed = 0;
		SW2Pressed = 0;
		SW1 = GPIO_PORTF_DATA_R&0x10;
		SW2 = GPIO_PORTF_DATA_R&0x01;
		//f1 = f2 = 0;
		while(!SW1){
			SW1Pressed = 1;
			SW1 = GPIO_PORTF_DATA_R&0x10;
		}
		
		if(SW1Pressed || currentFreePos[x/10] >= rows){
			do{
				Delay100ms(3);
				Nokia5110_ClearBuffer();
				drawGrid();

				if(x + cellW + vlineW < leftMargin + vlineW + cols*(cellW + vlineW))
					// x = 9 19 29 29 49 59 69 
					// c = x / 10;
					x += (cellW + vlineW)  ;
				else
					x = leftMargin + vlineW + 1;
				
				Nokia5110_PrintBMP(x , y , (p == 1 ? p1 : p2), 0);
				Nokia5110_PrintBMP(SCREENW - 15 ,y - 4 , (p == 1 ? PX : Pplus), 0);
				Nokia5110_DisplayBuffer();
				SW1Pressed = 0;
			}while(currentFreePos[x/10] >= rows);
		}
		
		while(!SW2){
			SW2Pressed = 1;
			SW2 = GPIO_PORTF_DATA_R&0x01;
		}
		
		C = x/10;
		
		if(SW2Pressed && currentFreePos[C] < rows){
			Nokia5110_ClearBuffer();
			cellPlayer[currentFreePos[C]][C] = p;
			currentFreePos[C]++;
			drawGrid();
			Nokia5110_DisplayBuffer();
			SW2Pressed = 0;
			return  C;
		}
	}
}

// drawing the initial grid  rows * cols ( 6 * 7 )

void drawGrid(){
	// vertical
	int x = 0;
	int y = SCREENH - 1;
	int c = 0;
	while(c <= cols){
		x = c * (cellW + vlineW);
		Nokia5110_PrintBMP(x + leftMargin, y , vline, 0);
		c++;
	} 
	// horizontal
	c = 0;
	x = leftMargin;
	y = 0;
	while(c <= rows){
		y = SCREENH - ( 1 +  c  * (cellH + hlineH));
		Nokia5110_PrintBMP(x, y, hline, 0);
		c++;
	}
	
	for(i = 0 ; i < rows ; i++)
		for(j = 0 ; j< cols ; j++)
			if(cellPlayer[i][j])
				Nokia5110_PrintBMP(leftMargin + 1 + j*(vlineW + cellW) + vlineW, SCREENH - (2 + i*(hlineH + cellH) + hlineH) , (cellPlayer[i][j] == 1 ? p1 : p2), 0);
}


// Welcome screen 
void welcomeScreen(){
	int i , j;
	Nokia5110_SetCursor(2, 1);
	Nokia5110_ClearBuffer();
	Nokia5110_OutString("Connect 4 ");
	Nokia5110_SetCursor(3, 3);
	Nokia5110_OutString("Welcome");
	Nokia5110_SetCursor(1, 5);
	Nokia5110_OutString("loading");

	for(j = 2 ; j < 9 ; j+= 2){
		Nokia5110_SetCursor( 6 + j   , 5);
		Nokia5110_OutString(".");
		Delay100ms(10);
	}

	for(i = 0 ; i < 7 ; i++){
		Nokia5110_SetCursor(0,i);
		Nokia5110_OutString("            ");
		Delay100ms(15);
	}
	Nokia5110_Clear();
}

// checking winning conditions vertical , horizontal or diagonal
int WhoIsWinner(){
	int c1 = 0, c2 = 0;

	//check horizontally
	for(i = 0; i < rows; i++){
		c1 = 0;
		c2 = 0;
		for(j = 0; j < cols; j++){
			if(cellPlayer[i][j] == 0){
				c1 = 0;
				c2 = 0;
			}
			
			else if(cellPlayer[i][j] == 1){
				c1++;
				c2 = 0;
			}
			
			else{
				c1 = 0;
				c2++;
			}
			
			if(c1 == 4)
				return 1;
			if(c2 == 4)
				return 2;
		}
	}
			
	//check vertically
	for(i = 0; i < cols ; i++){
		c1 = 0;
		c2 = 0;
		for(j = 0; j < rows; j++){
			if(cellPlayer[j][i] == 0){
				c1 = 0;
				c2 = 0;
			}
			
			else if(cellPlayer[j][i] == 1){
				c1++;
				c2 = 0;
			}
			
			else{
				c1 = 0;
				c2++;
			}
			
			if(c1 == 4)
				return 1;
			if(c2 == 4)
				return 2;
		}
	}
	
	//check diagonal right down
	for(i = 3, j = -2 ; i < 9 ; i++ , j++){
		int ii = min(i , rows-1);
		int jj = max(j , 0);
		c1 = 0;
		c2 = 0;
		for(k = 0 ; ii - k >= 0 && jj + k < cols ; k++){
			if(cellPlayer[ii - k][jj + k] == 0){
				c1 = 0;
				c2 = 0;
			}
			
			else if(cellPlayer[ii - k][jj + k] == 1){
				c1++;
				c2 = 0;
			}
			
			else{
				c1 = 0;
				c2++;
			}
			
			if(c1 == 4)
				return 1;
			if(c2 == 4)
				return 2;
		}
	}
	
	//check diagonal right up
	for(i = -3, j = 3 ; i < 3 ; i++ , j--){
		int ii = max(i , 0);
		int jj = max(j , 0);
		c1 = 0;
		c2 = 0;
		for(k = 0 ; ii + k < rows && jj + k < cols ; k++){
			if(cellPlayer[ii + k][jj + k] == 0){
				c1 = 0;
				c2 = 0;
			}
			
			else if(cellPlayer[ii + k][jj + k] == 1){
				c1++;
				c2 = 0;
			}
			
			else{
				c1 = 0;
				c2++;
			}
			
			if(c1 == 4)
				return 1;
			if(c2 == 4)
				return 2;
		}
	}
	
	return 0;
}

// selecting mode for playing 
// Player Vs Player or Player Vs AI
int selectMode(){
	
	int k = 0 ; 
	Nokia5110_SetCursor(1,0);
	Nokia5110_OutString("select mode");
	Nokia5110_SetCursor(2,2);
	Nokia5110_OutString("P1 VS AI");
	Nokia5110_SetCursor(2,4);
	Nokia5110_OutString("P1 VS P2");
	Nokia5110_SetCursor(0,k+2);	
	Nokia5110_OutString(" >");
	
	while(1)
	{
		SW1 = GPIO_PORTF_DATA_R&0x10;     // read PF4 into SW1
		//Delay100ms(1);
		if(!SW1){
				while (!SW1){
					SW1 = GPIO_PORTF_DATA_R&0x10;
				}
				
				Nokia5110_SetCursor(0,k+2);	
				Nokia5110_OutString("  ");				
				k+=2;
				if(k>2)
					k=0;
				Nokia5110_SetCursor(0,k+2);	
				Nokia5110_OutString(" >");
		 }
		SW2 = GPIO_PORTF_DATA_R&0x01;     // read PF4 into SW2
		//Delay100ms(1);
		if(!SW2){
			while (!SW2){
				SW2 = GPIO_PORTF_DATA_R&0x01;
			 }
			
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString("        ");	
			Delay100ms(3);			 
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString(k?"P1 VS P2":"P1 VS AI");	
			Delay100ms(3);			 
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString("        ");		
			Delay100ms(3);
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString(k?"P1 VS P2":"P1 VS AI");		
				break ;
		 }
		 
	 }
	// 0 P VS AI 
	// 2 P1 VS P2
	return k ; 
	
}

int twoKitsMode(){
	int k = 0 ; 
	Nokia5110_SetCursor(1,0);
	Nokia5110_OutString("select mode");
	Nokia5110_SetCursor(2,2);
	Nokia5110_OutString("P1 VS AI");
	Nokia5110_SetCursor(2,4);
	Nokia5110_OutString("P1 VS P2");
	Nokia5110_SetCursor(0,k+2);	
	Nokia5110_OutString(" >");
	
	while(1)
	{
		SW1 = GPIO_PORTF_DATA_R&0x10;     // read PF4 into SW1
		//Delay100ms(1);
		if(!SW1){
				while (!SW1){
					SW1 = GPIO_PORTF_DATA_R&0x10;
				}
				
				Nokia5110_SetCursor(0,k+2);	
				Nokia5110_OutString("  ");				
				k+=2;
				if(k>2)
					k=0;
				Nokia5110_SetCursor(0,k+2);	
				Nokia5110_OutString(" >");
		 }
		SW2 = GPIO_PORTF_DATA_R&0x01;     // read PF4 into SW2
		//Delay100ms(1);
		if(!SW2){
			while (!SW2){
				SW2 = GPIO_PORTF_DATA_R&0x01;
			 }
			
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString("        ");	
			Delay100ms(3);			 
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString(k?"P1 VS P2":"P1 VS AI");	
			Delay100ms(3);			 
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString("        ");		
			Delay100ms(3);
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString(k?"P1 VS P2":"P1 VS AI");		
				break ;
		 }
		 
	 }
	Nokia5110_Clear();
	// 0 P VS AI 
	// 2 P1 VS P2
	return k ; 
}
// select kit mode (1 kit or 2 kit)
int kitMode(){
	int k = 0 ; 
	Nokia5110_SetCursor(2,0);
	Nokia5110_OutString("Kit mode");
	Nokia5110_SetCursor(2,2);
	Nokia5110_OutString("One kit");
	Nokia5110_SetCursor(2,4);
	Nokia5110_OutString("Two kits");
	Nokia5110_SetCursor(0,k+2);	
	Nokia5110_OutString(" >");
	
	while(1)
	{
		SW1 = GPIO_PORTF_DATA_R&0x10;     // read PF4 into SW1
		//Delay100ms(1);
		if(!SW1){
				while (!SW1){
					SW1 = GPIO_PORTF_DATA_R&0x10;
				}
				
				Nokia5110_SetCursor(0,k+2);	
				Nokia5110_OutString("  ");				
				k+=2;
				if(k>2)
					k=0;
				Nokia5110_SetCursor(0,k+2);	
				Nokia5110_OutString(" >");
		 }
		SW2 = GPIO_PORTF_DATA_R&0x01;     // read PF4 into SW2
		//Delay100ms(1);
		if(!SW2){
			while (!SW2){
				SW2 = GPIO_PORTF_DATA_R&0x01;
			 }
			
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString("        ");	
			Delay100ms(3);			 
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString(k?"Two kits" : "One kit");	
			Delay100ms(3);			 
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString("        ");		
			Delay100ms(3);
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString(k?"Two kits" : "One kit");	
			 break ;
		 }
		 
	 }
	// 0 one Kit
	// 2 two kits
	Nokia5110_Clear();
	return k ; 
	
}
// master or slave mode
int connectionMode(){
	int k = 0 ; 
	Nokia5110_SetCursor(2,0);
	Nokia5110_OutString("Kit mode");
	Nokia5110_SetCursor(2,2);
	Nokia5110_OutString("Master");
	Nokia5110_SetCursor(2,4);
	Nokia5110_OutString("Slave");
	Nokia5110_SetCursor(0,k+2);	
	Nokia5110_OutString(" >");
	
	while(1)
	{
		SW1 = GPIO_PORTF_DATA_R&0x10;     // read PF4 into SW1
		//Delay100ms(1);
		if(!SW1){
				while (!SW1){
					SW1 = GPIO_PORTF_DATA_R&0x10;
				}
				
				Nokia5110_SetCursor(0,k+2);	
				Nokia5110_OutString("  ");				
				k+=2;
				if(k>2)
					k=0;
				Nokia5110_SetCursor(0,k+2);	
				Nokia5110_OutString(" >");
		 }
		SW2 = GPIO_PORTF_DATA_R&0x01;     // read PF4 into SW2
		//Delay100ms(1);
		if(!SW2){
			while (!SW2){
				SW2 = GPIO_PORTF_DATA_R&0x01;
			 }
			
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString("        ");	
			Delay100ms(3);			 
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString(!k?"Master" : "Slave");	
			Delay100ms(3);			 
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString("        ");		
			Delay100ms(3);
			Nokia5110_SetCursor(2,k+2);
			Nokia5110_OutString(!k?"Master" : "slave");	
			 break ;
		 }
		 
	 }
	// 0 master
	// 2 slave
	Nokia5110_Clear();
	return k ; 
	
}

// check who is the winner
// w 0 --> tie , 1 --> player one (X) wins , 2 --> player two (+) wins
void Winner(int w){
	
	Nokia5110_SetCursor(1,0);
	Nokia5110_OutString("Game Over!");
	Delay100ms(30);
	
	Nokia5110_SetCursor(1,0);
	Nokia5110_OutString("           ");
	Delay100ms(30);
	
	Nokia5110_SetCursor(1,0);
	Nokia5110_OutString("Game Over!");
	Delay100ms(30);
	
	Nokia5110_SetCursor(1,0);
	Nokia5110_OutString("          ");
	Delay100ms(40);
	Nokia5110_SetCursor(1,0);
	Nokia5110_OutString("Game Over!"); 
	
	Nokia5110_Clear();
	Nokia5110_SetCursor(2,1);
	if(w == 0){ // game is tie (no winner)
		Nokia5110_SetCursor(3,1);
		Nokia5110_OutString("No one ");
		Nokia5110_SetCursor(4,3);
		Nokia5110_OutString("Wins!");
		Nokia5110_SetCursor(2,5);
		Nokia5110_OutString("Game Over");

	}
	else if(w == 1){ // player 1 wins
		Nokia5110_OutString("Player X");
		Nokia5110_SetCursor(4,3);
		Nokia5110_OutString("Wins!");
		
	}
	else if(w == 3){
		Nokia5110_OutString("Player +");
		Nokia5110_SetCursor(4,3);
		Nokia5110_OutString("Wins!");
	}
	else{ // player 2 wins
		if(playMode && kitMode)
			Nokia5110_OutString("Player +");
		else{
			Nokia5110_SetCursor(5,1);
			Nokia5110_OutString("AI");
		}
		Nokia5110_SetCursor(4,3);
		Nokia5110_OutString("Wins!");
	}
}

void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}

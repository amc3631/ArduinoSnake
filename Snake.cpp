//Authors:

//Jingyuan Zhang
//Michael Boisvert


#include<Arduino.h>
#include<Adafruit_ILI9341.h>
#include<math.h>
// TFT display and SD card will share the hardware SPI interface.
// For the Adafruit shield, these are the default.
// The display uses hardware SPI, plus #9 & #10
// Mega2560 Wiring: connect TFT_CLK to 52, TFT_MISO to 50, and TFT_MOSI to 51
#define TFT_DC 9
#define TFT_CS 10
#define SD_CD 6

// height is 240, width is 320 when oriented horizontally
#define TFT_HEIGHT 240
#define TFT_WIDTH 320

#define PIXEL_HEIGHT 11
#define PIXEL_WIDTH 15

#define CURSOR_SIZE 20

#define JOY_VERT A1
#define JOY_HORIZ A0
#define JOY_SEL   2


#define JOY_VERT_A A9
#define JOY_HORIZ_A A8
#define JOY_SEL_A  13

#define JOY_CENTER   512
#define JOY_DEADZONE 64

// Use hardware SPI (on Mega2560, #52, #51, and #50) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// structure for a block on the screen
struct point{
	uint8_t x;
	uint8_t y;
};

//to initialize the map point
uint8_t block[16][12]= {0};

//To initialize the head position and the tail position of the snake
point head;
// to initialize another snake
point head2;
//The movement of tail is decided by all of the head movement
point tail[192];
point tail2[192];

//to initialize a point variable to store the food position.
point food;

//to define a counter to track the movement of the snake
//int track = 0;
int tailtrack = 0;
int guidetail = 0;
int tailtrack2 = 0;
int guidetail2 = 0;

//To guide the direction that snake moves, the
int xVal;
int yVal;
int xVal2;
int yVal2;

point prehead;
point prehead2;
int eat = 0;
int crash = 0;
int eat2 = 0;
int restart;

int length = 0;
int length2 = 0;

//to identify which mode to play, 0 means the single mode, and 1 means the double mode
int mode = 0;

//declare the function in advance
void processHead();
void processHead2();
void getFood();

void setup() {
	init();

	Serial.begin(9600);

	pinMode(JOY_SEL, INPUT_PULLUP);

	// Initialize the breakout board.
	// Required for both the display and the SD card.
	tft.begin();

	// because we hold it horizontally
	tft.setRotation(3);

	tft.fillScreen(ILI9341_BLACK);


}

// setup for 1 player mode
void singleSetup(){
	xVal = 1024;
	yVal = JOY_CENTER;
	tailtrack = 0;
	guidetail = 0;
	crash = 0;
	restart = 0;
	tft.fillScreen(ILI9341_BLACK);
	for(int i = 0; i < 16; i++){
		for(int j = 0; j < 12; j++){
			block[i][j] = 0;
		}
	}

	head.x = PIXEL_WIDTH;
	head.y = PIXEL_HEIGHT;

	processHead();

	prehead.x = head.x;
	prehead.y = head.y;
	head.x -= 1;

	processHead();

	getFood();
}

void processHead(){
	//draws the head and keeps track od the tail
	tft.fillRect((head.x*20), (head.y*20), CURSOR_SIZE, CURSOR_SIZE, ILI9341_WHITE);

	tail[tailtrack].x = head.x;
	tail[tailtrack].y = head.y;
	tailtrack += 1;
	if(tailtrack == 192){
		tailtrack = 0;
	}

	// check if head is in same spot as food
	if (block[head.x][head.y] == 2){
			eat = 1;
			return;
	}

	//check if snake has crashed into itself or the pther snake
	if(block[head.x][head.y] == 1|| block[head.x][head.y] == 3){
		Serial.println("case1");
		crash = 1;
		return;
	}

	//check if snake has crashed into edge of screen
	if(head.x > PIXEL_WIDTH){
		crash = 1;
		Serial.println("case2");
		return;
	}
	if(head.y > PIXEL_HEIGHT){
		crash = 1;
		Serial.println("case3");
		return;
	}
	block[head.x][head.y] = 1;
}


void processHead2(){
	//draws head and keeps track of tail fro second snake
	tft.fillRect((head2.x*20), (head2.y*20), CURSOR_SIZE, CURSOR_SIZE, ILI9341_BLUE);

	tail2[tailtrack2].x = head2.x;
	tail2[tailtrack2].y = head2.y;
	tailtrack2 += 1;
	if(tailtrack2 == 192){
		tailtrack2 = 0;
	}
	if (block[head2.x][head2.y] == 2){
			eat2 = 1;
			return;
	}
	if(block[head2.x][head2.y] == 1|| block[head2.x][head2.y] == 3){
		Serial.println("case1");
		crash = 1;
		return;
	}
	if(head2.x > PIXEL_WIDTH){
		crash = 1;
		Serial.println("case2");
		return;
	}
	if(head2.y > PIXEL_HEIGHT){
		crash = 1;
		Serial.println("case3");
		return;
	}
	block[head2.x][head2.y] = 3;
}


void processTail2(){
	// changes block containing snake to empty black block
	tft.fillRect(tail2[guidetail2].x*20, tail2[guidetail2].y*20, CURSOR_SIZE, CURSOR_SIZE, ILI9341_BLACK);
	block[tail2[guidetail2].x][tail2[guidetail2].y] = 0;

	guidetail2 += 1;
	if(guidetail2 == 192){
		guidetail2 = 0;
	}
}





void processTail(){
	// changes block containing snake to empty black block
	tft.fillRect(tail[guidetail].x*20, tail[guidetail].y*20, CURSOR_SIZE, CURSOR_SIZE, ILI9341_BLACK);
	block[tail[guidetail].x][tail[guidetail].y] = 0;

	guidetail += 1;
	if(guidetail == 192){
		guidetail = 0;
	}
}


void drawMenu(){
	//draws the initial menu
	tft.setTextSize(3);
	tft.setCursor(0,0);
	for(int i = 0; i < 2; i ++){
		if(i == mode){
			Serial.println(mode);
			tft.setTextColor(0x0000, 0xFFFF);
		}
		else{
			tft.setTextColor(0xFFFF, 0x0000);
		}
		tft.print(i+1);
		tft.println("PLAYER MODE");
	}
}

void menu(){
	// select mode from menu
	drawMenu();
	int premode = mode;
	if(analogRead(JOY_VERT) > JOY_CENTER+JOY_DEADZONE || analogRead(JOY_VERT) < JOY_CENTER - JOY_DEADZONE){
		mode = abs(mode - 1);
	}
	if(premode != mode){
		drawMenu();
	}
}

//Using the random function to generate the position of the food randomly
//in the position array, the food saved as 2
void getFood(){
	food.x = rand() % PIXEL_WIDTH + 1;
	food.y = rand() % PIXEL_HEIGHT + 1;
	while(block[food.x][food.y] != 0){
		food.x = rand() % PIXEL_WIDTH + 1;
		food.y = rand() % PIXEL_HEIGHT + 1;
	}
	//Serial.println(food.x);
	//Serial.println(food.y);
	tft.fillRect(food.x*20, food.y*20, CURSOR_SIZE, CURSOR_SIZE, ILI9341_RED);
	block[food.x][food.y] = 2;
}

//change position of head of second snake based on direction joystick is pushed
void headPosition2(){
	int y = analogRead(JOY_VERT_A);
	int x = analogRead(JOY_HORIZ_A);
	if(xVal2 != x && x > JOY_CENTER + JOY_DEADZONE){
		xVal2 = x;
		yVal2 = JOY_CENTER;
	}
	else if(xVal2 != x && x < JOY_CENTER - JOY_DEADZONE){
		xVal2 = x;
		yVal2 = JOY_CENTER;
	}
	else if(yVal2 != y && y > JOY_CENTER + JOY_DEADZONE){
		yVal2 = y;
		xVal2 = JOY_CENTER;
	}
	else if(yVal2 != y && y < JOY_CENTER - JOY_DEADZONE){
		yVal2 = y;
		xVal2 = JOY_CENTER;
	}


	if(yVal2 > JOY_CENTER + JOY_DEADZONE){
		if((head2.y + 1) != prehead2.y){
			prehead2.y = head2.y;
			prehead2.x = head2.x;
			head2.y += 1;
		}
		else{
			prehead2.y = head2.y;
			prehead2.x = head2.x;
			head2.y -= 1;
		}
	}
	else if(yVal2 < JOY_CENTER - JOY_DEADZONE){
		if((head2.y - 1) != prehead2.y){
			prehead2.y = head2.y;
			prehead2.x = head2.x;
			head2.y -= 1;
		}
		else{
			prehead2.y = head2.y;
			prehead2.x = head2.x;
			head2.y += 1;
		}
	}
	else if(xVal2 > JOY_CENTER + JOY_DEADZONE){
		if((head2.x - 1) != prehead2.x){
			prehead2.y = head2.y;
			prehead2.x = head2.x;
			head2.x -= 1;
		}
		else{
			prehead2.y = head2.y;
			prehead2.x = head2.x;
			head2.x += 1;
		}
	}
	else if(xVal2 < JOY_CENTER - JOY_DEADZONE){
		if((head2.x+1) != prehead2.x){
			prehead2.y = head2.y;
			prehead2.x = head2.x;
			head2.x += 1;
		}
		else{
			prehead2.y = head2.y;
			prehead2.x = head2.x;
			head2.x -= 1;
		}

	}

}

// game over screen after a snake crashes
void drawEnding(){
	tft.fillScreen(ILI9341_BLACK);
	tft.drawLine(0, 49, 320, 49, ILI9341_WHITE);
	tft.drawLine(0, 50, 320, 50,0xA8A8A8);
	tft.setCursor(45, 100);
	tft.setTextColor(ILI9341_RED);
	tft.setTextSize(5);
	tft.println("GAME OVER");
	tft.drawLine(0, 190, 320, 190,0xA8A8A8);
	tft.drawLine(0, 191, 320, 191, ILI9341_WHITE);

}

// writes on screen to tell players the game is paused
void drawPause(){
	tft.fillScreen(ILI9341_BLACK);
	tft.drawLine(0, 49, 320, 49, ILI9341_WHITE);
	tft.drawLine(0, 50, 320, 50,0xA8A8A8);
	tft.setCursor(45, 100);
	tft.setTextColor(ILI9341_RED);
	tft.setTextSize(5);
	tft.println("PAUSE");
	tft.drawLine(0, 190, 320, 190,0xA8A8A8);
	tft.drawLine(0, 191, 320, 191, ILI9341_WHITE);

}


//change position of head of first snake based on direction joystick is pushed
void headPosition(){
	int y = analogRead(JOY_VERT);
	int x = analogRead(JOY_HORIZ);
	if(xVal != x && x > JOY_CENTER + JOY_DEADZONE){
		xVal = x;
		yVal = JOY_CENTER;
	}
	else if(xVal != x && x < JOY_CENTER - JOY_DEADZONE){
		xVal = x;
		yVal = JOY_CENTER;
	}
	else if(yVal != y && y > JOY_CENTER + JOY_DEADZONE){
		yVal = y;
		xVal = JOY_CENTER;
	}
	else if(yVal != y && y < JOY_CENTER - JOY_DEADZONE){
		yVal = y;
		xVal = JOY_CENTER;
	}


	if(yVal > JOY_CENTER + JOY_DEADZONE){
		if((head.y + 1) != prehead.y){
			prehead.y = head.y;
			prehead.x = head.x;
			head.y += 1;
		}
		else{
			prehead.y = head.y;
			prehead.x = head.x;
			head.y -= 1;
		}
	}
	else if(yVal < JOY_CENTER - JOY_DEADZONE){
		if((head.y - 1) != prehead.y){
			prehead.y = head.y;
			prehead.x = head.x;
			head.y -= 1;
		}
		else{
			prehead.y = head.y;
			prehead.x = head.x;
			head.y += 1;
		}
	}
	else if(xVal > JOY_CENTER + JOY_DEADZONE){
		if((head.x - 1) != prehead.x){
			prehead.y = head.y;
			prehead.x = head.x;
			head.x -= 1;
		}
		else{
			prehead.y = head.y;
			prehead.x = head.x;
			head.x += 1;
		}
	}
	else if(xVal < JOY_CENTER - JOY_DEADZONE){
		if((head.x+1) != prehead.x){
			prehead.y = head.y;
			prehead.x = head.x;
			head.x += 1;
		}
		else{
			prehead.y = head.y;
			prehead.x = head.x;
			head.x -= 1;
		}

	}

}


// initialization for 2player mode
void doublesetup(){
		xVal = 1024;
		yVal = JOY_CENTER;
		tailtrack = 0;
		guidetail = 0;
		crash = 0;
		restart = 0;
		xVal2 = 0;
		yVal2 = JOY_CENTER;
		eat2 = 0;
		tailtrack2 = 0;
		guidetail2 = 0;
		length = 0;
		length2 = 0;
		tft.fillScreen(ILI9341_BLACK);
		for(int i = 0; i < 16; i++){
			for(int j = 0; j < 12; j++){
				block[i][j] = 0;
			}
		}

		head.x = PIXEL_WIDTH;
		head.y = PIXEL_HEIGHT;

		processHead();

		prehead.x = head.x;
		prehead.y = head.y;
		head.x -= 1;


		processHead();

		head2.x = 0;
		head2.y = 0;

		processHead2();

		prehead2.x = head2.x;
		prehead2.y = head2.y;
		head2.x += 1;

		processHead2();

		getFood();
	}




int main(){
	setup();
	while(true){
		tft.fillScreen(ILI9341_BLACK);
		delay(200);
		mode = 0;
		while(digitalRead(JOY_SEL) == HIGH){
			// draw menu to select mode
			menu();
			delay(20);
		}

		if(mode == 0){
			// start 1 player mode
			singleSetup();
			while(restart == 0){
				//draw and move head
				headPosition();
				processHead();
				// end when the snake crashes
				if(crash == 1){
					restart = 1;
					break;
				}
				if(eat == 1){
					// draw food somewhere else after it is eaten
					getFood();
					eat = 0;
					// change from block containing food to block containing snake
					block[head.x][head.y] = 1;
				}
				else{
					//clear end of tail, but not if food has just been eaten so that the snake grows
					processTail();
				}
				delay(500);
			}
				// after snake crashes go to end
				drawEnding();

				tft.setTextSize(1);
				tft.setCursor(0,230);
				tft.println("press joystick to restart");

				while(restart == 1){
					//wait for joystick to be pushed to restart game
					if(digitalRead(JOY_SEL) == LOW){
						mode = 3;
						break;
					}
				}
			}

			//if mode == 1, start the 2 player mode
			if(mode == 1){
				int status = 0;
				doublesetup();

					while(restart == 0){
						headPosition();
						headPosition2();
						processHead();
						processHead2();
						if(crash == 1){
							restart = 1;
							break;
						}
						// a variable for when each snake eats in order to determine which should grow
						// keep track of length of both snakes to determine winner of the game at the end
						if(eat == 1){
							getFood();
							eat = 0;
							block[head.x][head.y] = 1;
							processTail2();
							length+=1;
						}
						else if(eat2 == 1){
							getFood();
							eat2 = 0;
							block[head2.x][head2.y] = 3;
							processTail();
							length2+=1;
						}
						else{
							processTail();
							processTail2();
						}
						delay(500);
					// pause game if either joystick is pressed
					int v = digitalRead(JOY_SEL);
					int x = digitalRead(JOY_SEL_A);
					if(v == LOW || x == LOW){
						while(true){
							delay(500);
							tft.setCursor(0,0);
							tft.setTextSize(1);
							tft.setTextColor(ILI9341_WHITE);
							tft.print("pause! press joystick to return");
							v = digitalRead(JOY_SEL);
							x = digitalRead(JOY_SEL_A);

							if(v == LOW || x == LOW){
								// break and resume game if either joystick pressed again
								// write pause statement again in black to remove it
								v = 1;
								x = 1;
								tft.setCursor(0,0);
								tft.setTextSize(1);
								tft.setTextColor(ILI9341_BLACK);
								tft.print("pause! press joystick to return");
								break;
							}

						}

					}
					}

					if(crash == 1){
						drawEnding();
					}
					tft.setCursor(0,200);
					tft.setTextSize(2);
					tft.setTextColor(ILI9341_WHITE);
					// declare winner based on which snake was longer after game ends
					if(length > length2){
						tft.println("WINNER WHITE");
					}
					else if(length < length2){
						tft.println("WINNER BLUE");
					}
					else{
						tft.println("TIE");
					}
					tft.setTextSize(1);
					tft.setCursor(0,230);
					tft.println("press joystick to restart");


					while(restart == 1){
						if(digitalRead(JOY_SEL) == LOW || digitalRead(JOY_SEL_A) == LOW){
							mode = 3;
							break;
						}
				}
			}

	}





	Serial.flush();

	return 0;


}

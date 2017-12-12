/*
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// set the LCD address to 0x38 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x38,16,2);

String lcd_message;
int currentPlayer;
int currentScore;

long gameOverTime;

int buttonPin = 5;
boolean isPlaying = false;
boolean gameOver = false;

const int LCD_TIMEOUT = 5000; // set to 5 sec
*/
// 2-dimensional array of row pin numbers:
const int row[8] = {
  //2, 7, 19, 5, 13, 18, 12, 16
  6, 3, 8, 2, 10, 9, A3, 13
};

// 2-dimensional array of column pin numbers:
const int col[8] = {
  //6, 11, 10, 3, 17, 4, 8, 9
  A5, A4, 11, 7, 12, 5, 4, A2
};

// 2-dimensional array of pixels:
int pixels[8][8];

// cursor position:
int x = 5;
int y = 5;

void setup() {
/*
  currentScore = 0;
  currentPlayer = 1;
  pinMode(buttonPin, INPUT); 

  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight
  */
  // initialize the I/O pins as outputs iterate over the pins:
  for (int thisPin = 0; thisPin < 8; thisPin++) {
    // initialize the output pins:
    pinMode(col[thisPin], OUTPUT);
    pinMode(row[thisPin], OUTPUT);
    // take the col pins (i.e. the cathodes) high to ensure that the LEDS are off:
    digitalWrite(col[thisPin], HIGH);
  }

  // initialize the pixel matrix:
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      pixels[x][y] = HIGH;
    }
  }
}

void loop() {
/*
  // if the start button is pressed, set isPlaying
  if(digitalRead(buttonPin) == 1)
    isPlaying = true;
*/
  //if (isPlaying)
  //{
  // read input:
  readSensors();

  // draw the screen:
  refreshScreen();

  //}
/*
  if(!gameOver)
  {
    lcd_message = String(currentScore) + " points";
    updateDisplay();

    // update the matrix screen

    //gameOver = true;
    gameOverTime = millis(); // must be after gameOver = true;
  }
  else
  {
    // read the clock value
    // save the player, clock value, and score to the SD card
    
    if(millis() - gameOverTime > LCD_TIMEOUT)
    {
      gameOver = false;
      if (currentPlayer == 1)
        currentPlayer = 2;
      else
        currentPlayer = 1;

      lcd_message = "READY";
      updateDisplay();
      delay(3000);
    }
    else
    {
      lcd_message = "GAME OVER";
      updateDisplay();
    }
  }*/
}
/*
void updateDisplay()
{
  lcd.setCursor(0,0);
  lcd.print("Player: "+String(currentPlayer));
  lcd.setCursor(0,1);
  lcd.print(lcd_message);
  delay(100);
}*/

void readSensors() {
  // turn off the last position:
  pixels[x][y] = HIGH;
  // read the sensors for X and Y values:
  x = 7 - map(analogRead(A0), 0, 1023, 0, 7);
  y = map(analogRead(A1), 0, 1023, 0, 7);
  // set the new pixel position low so that the LED will turn on in the next
  // screen refresh:
  pixels[x][y] = LOW;

}

void refreshScreen() {
  // iterate over the rows (anodes):
  for (int thisRow = 0; thisRow < 8; thisRow++) {
    // take the row pin (anode) high:
    digitalWrite(row[thisRow], HIGH);
    // iterate over the cols (cathodes):
    for (int thisCol = 0; thisCol < 8; thisCol++) {
      // get the state of the current pixel;
      int thisPixel = pixels[thisRow][thisCol];
      // when the row is HIGH and the col is LOW,
      // the LED where they meet turns on:
      digitalWrite(col[thisCol], thisPixel);
      // turn the pixel off:
      if (thisPixel == LOW) {
        digitalWrite(col[thisCol], HIGH);
      }
    }
    // take the row pin low to turn off the whole row:
    digitalWrite(row[thisRow], LOW);
  }
}

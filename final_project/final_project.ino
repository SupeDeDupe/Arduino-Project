/*
  -----------------------------------
  -  9  10  11  12  13  14  15  16  -
  -                                 -
  -                                 -
  -  8  7    6   5   4   3   2   1  -
  -----------------------------------

  Matrix pin   Row   Col   Arduino pin #
  1             -     1     A5
  2             -     2     A4
  3             7     -     A3
  4             -     8     A2
  5             8     -     23
  6             -     5     25
  7             -     3     27
  8             5     -     29
  9             6     -     9
  10            3     -     8
  11            -     4     7
  12            1     -     6
  13            -     6     5
  14            -     7     4
  15            2     -     3
  16            4     -     2
*/


#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// set the LCD address to 0x38 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x38,16,2);

String lcd_message;
int currentPlayer;
int currentScore;

long gameOverTime;

int buttonPin = 22;
boolean isPlaying;
boolean gameOver;

const int LCD_TIMEOUT = 5000; // set to 5 sec


int maxItems = 3;
int lastItemDrawn = 0;
int multiplier = 1;

long startGameTime;

const int ROCK_SPEED_MAX = 50;
int rockSpeedMin = 350;
const int REDRAW_SPEED = 1;
const int AVATAR_SPEED = 170;
const int AVATAR_ROW = 7;
long lastMoveTime;
long lastDrawnTime;
long lastRockUpdate;
long lastRockCreated;
int avatarCol;
int lastAvatarCol;
boolean forward = true;

int rockCols[8];
int rockRows[8];
int rockSpeeds[8];
long rocksLastUpdated[8];

// define the pins that will be used 
int SWITCH_PIN = 52;
int Jx_PIN = A0;
int Jy_PIN = A1;
// define storage variables int 
int j_x = 0;
int j_y = 0;
String s = "";

// 2-dimensional array of row pin numbers:
const int row[8] = {
  //2, 7, 19, 5, 13, 18, 12, 16
  6, 3, 8, 2, 23, 9, A3, 29
};

// 2-dimensional array of column pin numbers:
const int col[8] = {
  //6, 11, 10, 3, 17, 4, 8, 9
  A5, A4, 25, 7, 27, 5, 4, A2
};

// cursor position:
int x = 5;
int y = 5;

void setup() {

  currentScore = 0;
  currentPlayer = 1;
  pinMode(buttonPin, INPUT);

  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight
  
  // initialize the I/O pins as outputs iterate over the pins:
  for (int thisPin = 0; thisPin < 8; thisPin++) {
    // initialize the output pins:
    pinMode(col[thisPin], OUTPUT);
    pinMode(row[thisPin], OUTPUT);
    // take the col pins (i.e. the cathodes) high to ensure that the LEDS are off:
    digitalWrite(col[thisPin], HIGH);
    // set the switch to have a default 20k pullup 
  }
  
  Serial.begin(9600);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  delay(300);

  isPlaying = false;
  gameOver = false;

  clearDisplay();
  lcd_message = "READY";
  updateDisplay();
  delay(200);
  
}

void loop() {
  
  // if the start button is pressed, set isPlaying
  if(digitalRead(buttonPin) == 1)
  {
    clearDisplay();
    lcd_message = "GO";
    updateDisplay();
    delay(200);
    
    isPlaying = true;
    gameOver = false;

    for (int i = 0; i < 8; i++) {
    rockCols[i] = i;
    rockRows[i] = -1;
    rocksLastUpdated[i] = millis();
    rockSpeeds[i] = 0;
    }
  
    avatarCol = 0;
    lastAvatarCol = 1;
    lastMoveTime = millis();
    lastDrawnTime = millis();
    lastRockUpdate = millis();
    lastRockCreated = millis();
    startGameTime = millis();
  
    for (int i = 0; i < maxItems; i++)
    {
      createRockSingleCol();
    }
  }

  //Serial.println(String(digitalRead(buttonPin)));
  if (isPlaying)
  {

  //int thisPixel = pixels[AVATAR_ROW][avatarCol];
  //digitalWrite(row[AVATAR_ROW], HIGH);
  //digitalWrite(col[avatarCol], LOW);
  
  // read input:
  //readSensors();

  checkCollision();

  j_x = analogRead(Jx_PIN);
  j_y = analogRead(Jy_PIN);
  if (digitalRead(SWITCH_PIN) == HIGH)
    s = "Switch: HIGH"; 
  else
    s = "Switch: LOW";

  if (millis() - lastRockCreated > 3000 / multiplier)
  {
    if(maxItems < 9)
      maxItems++;
    for(int i = 0; i < maxItems; i++)
      createRockSingleCol();
    lastRockCreated = millis();
  }

  if (rockSpeedMin - ROCK_SPEED_MAX > 35)
  {
    if (millis() - startGameTime > 6000 * multiplier)
    {
      
      rockSpeedMin = rockSpeedMin - 30;
      multiplier++;
    }
  }

  // draw the screen:
  refreshScreen();

  }
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
    if(gameOver)
    {
      clearMatrix();
      
    }
}

void checkCollision()
{
  for (int i = 0; i < 8; i++) 
  {
    if(rockRows[i] == AVATAR_ROW)
    {
      if (rockCols[i] == avatarCol)
      {
        lcd_message = "GAME OVER";
        clearDisplay();
        updateDisplay();
        delay(200);
        
        gameOver = true;
        isPlaying = false;
        gameOverGridUpdate();
      }
    }
  }
}

void gameOverGridUpdate()
{
  clearMatrix();

  for (int c = 0; c < 8; c++)
  {
    digitalWrite(col[c], LOW);
  }
  
  for (int r = 0; r < 8; r++)
  {
    digitalWrite(row[r], HIGH);
    delay(200);
  }
}

void clearDisplay()
{
  lcd.setCursor(0,0);
  lcd.print("                 ");
  lcd.setCursor(0,1);
  lcd.print("                 ");
  delay(100);
}

void updateDisplay()
{
  lcd.setCursor(0,0);
  lcd.print("Player: "+String(currentPlayer));
  lcd.setCursor(0,1);
  lcd.print(lcd_message);
  delay(100);
}

void refreshScreen() {

   if(millis() - lastMoveTime > AVATAR_SPEED)
   {
    updateAvatarPosition();
    updateRocks();
    
   }

   if(millis() - lastDrawnTime > REDRAW_SPEED)
   {
    //Serial.println(String(lastItemDrawn));
    clearMatrix();
    switch(lastItemDrawn)
    {
      
      case 0: //rock
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
       if(rockRows[lastItemDrawn] > -1)
       {
         digitalWrite(row[rockRows[lastItemDrawn]], HIGH);
         digitalWrite(col[rockCols[lastItemDrawn]], LOW);
       }
       break;
      case 8:
       digitalWrite(col[lastAvatarCol], HIGH); // sets the LED the avatar was last at to off
       digitalWrite(row[AVATAR_ROW], HIGH);
       digitalWrite(col[avatarCol], LOW); // sets the LED for the new location of the avatar to on
       break;
      default:
       break;
    }

    if(lastItemDrawn < 9)
      lastItemDrawn++;
    else
      lastItemDrawn = 0;
    lastDrawnTime = millis();
   }
}

void clearMatrix()
{
  for (int r = 0; r < 8; r++)
  {
    for(int c = 0; c < 8; c++)
    {
      digitalWrite(col[c], HIGH); 
      digitalWrite(row[r], LOW);
    }
  }
}

void createRockSingleCol()
{
  int newRockIndex;
  boolean allRocksDrawn = true;
  for(int i = 0; i < 8; i++)
  {
    if (rockRows[newRockIndex] < 0 || rockRows[newRockIndex] > 7)
      allRocksDrawn = false;
  }
  if(!allRocksDrawn)
  {
    do
    {
      newRockIndex = random(0, 8);
    }
    while (rockRows[newRockIndex] > -1 && rockRows[newRockIndex] < 8);
    rockSpeeds[newRockIndex] = random(ROCK_SPEED_MAX, rockSpeedMin);
    rockRows[newRockIndex] = 0;
  }
}

void updateRocks()
{
   if (millis() - lastRockUpdate > ROCK_SPEED_MAX )//&& rockRows[0] < 8)
   {

    for (int i = 0; i < 8; i++)
    {
      if(millis() - rocksLastUpdated[i] > rockSpeeds[i])
      {
        if(rockRows[i] > -1)
          rockRows[i]++;
        if(rockRows[i] > 7)
        {
          rockRows[i] = -1;
          createRockSingleCol();
        }
        rocksLastUpdated[i] = millis();
      }
    }

    lastRockUpdate = millis();
   }
}

void updateAvatarPosition()
{
  
  if (avatarCol < 7 && j_y < 400)
  {
    // turn off current light
    // turn on the new light
    lastAvatarCol = avatarCol;
    avatarCol++;
    lastMoveTime = millis();//moveRight
    //Serial.println(s + " - " + j_x + ":" + j_y);
    //Serial.println("1 " + String(avatarCol));
  }
  else if (avatarCol > 0 && j_y > 600)
  {
    // turn off current light
    // turn on the new light
    lastAvatarCol = avatarCol;
    avatarCol--;
    lastMoveTime = millis();// moveLeft
    //Serial.println(s + " - " + j_x + ":" + j_y);
    //Serial.println("2 " + String(avatarCol));
  }
  else
    forward = !forward;
}


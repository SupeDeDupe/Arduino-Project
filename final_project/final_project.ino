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
#include <MFRC522.h> // Include of the RC522 Library
#include <SPI.h> // Used for communication via SPI with the Module

// Define a few of the registers that we will be accessing on the MMA8452
// These hexadecimal numbers define memory locations inside the IMU that allow us to
// set its operating parameters, and how we get data from it.

#define OUT_X_MSB 0x01
#define XYZ_DATA_CFG  0x0E
#define WHO_AM_I_REG   0x0D
#define CTRL_REG1  0x2A

#define SDAPIN 53 // RFID Module SDA Pin is connected to the UNO 10 Pin 
#define RESETPIN 11 // RFID Module RST Pin is connected to the UNO 9 Pin

byte FoundTag; // Variable used to check if Tag was found
byte ReadTag; // Variable used to store anti-collision value to read Tag information 
byte TagData[MAX_LEN]; // Variable used to store Full Tag Data
byte TagSerialNumber[5]; // Variable used to store only Tag Serial Number

MFRC522 nfc(SDAPIN, RESETPIN); // Init of the library using the UNO pins declared above



// set the LCD address to 0x38 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x38,16,2);

// message for the botton half of the LCD display
String lcd_message; // for "READY", "GO", and "GAME OVER"
int currentPlayer;
int player1Score;
int player2Score;

long gameOverTime; // time of collision and gameOver is set to true

int buttonPin = 22; // button to start game
boolean isPlaying;
boolean gameOver;
String winningResult; // String to contain winning player result for LCD display

// timeout for LCD display before changing
const int LCD_TIMEOUT = 3000; // set to 3 sec

int maxItems = 3; // maxRocks to start with

// index of the last item (rock or avatar) drawn for fast matrix led switching
int lastItemDrawn = 0; 
int multiplier = 1; // used to increase game difficulty

long startGameTime; 

const int ROCK_SPEED_MAX = 50; // will never change
int rockSpeedMin = 350; // will decrease as the game is not over, based on the multiplier
const int REDRAW_SPEED = 1; // millisec delay between alternating lighting leds in the matrix
const int AVATAR_SPEED = 170; // time delay of drawing the avatar, which determines speed
const int AVATAR_ROW = 7; // constant row position of avatar (always bottom row)
const String blueTagName = "SARAH";
const String whiteTagName = "JASON";

const byte whiteTag[4] = {68, 180, 227, 89}; // the unique serial number for my personal RFID tag
const byte blueTag[4] = {190, 213, 3, 137}; // the unique serial number for my personal blue RFID tag

boolean blueRecognized = false;
boolean whiteRecognized = false;

String player1Name;
String player2Name;

long lastMoveTime;
long lastDrawnTime;
long lastRockUpdate;
long lastRockCreated;
int avatarCol; // current columns position of avatar
int lastAvatarCol; 

int rockCols[8];
int rockRows[8];
int rockSpeeds[8];
long rocksLastUpdated[8];

// define the pins that will be used 
int SWITCH_PIN = 46;
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

  delay(10);
  
  SPI.begin(); 
  Serial.begin(115200);
  Serial.println("Hi");
    
  // Start to find an RFID Module
  nfc.begin();
  byte version = nfc.getFirmwareVersion(); // Variable to store Firmware version of the Module
    
  // If can't find an RFID Module 
  if (!version)
  {
      while (1); //Wait until a RFID Module is found 
  }

  currentPlayer = 1;
  player1Name = "PLAYER 1";
  player2Name = "PLAYER 2";
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
  
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  delay(300);

  isPlaying = false;
  gameOver = false;

  clearDisplay();
  lcd_message = "GET READY";
  updateDisplay();
  delay(200);
  
}

void loop() {

  if (!isPlaying)
  {
   // Check to see if a Tag was detected 
  // If yes, then the variable FoundTag will contain "MI_OK"
    FoundTag = nfc.requestTag(MF1_REQIDL, TagData);
    if (FoundTag == MI_OK)
    {
        delay(200);
        
        // Get anti-collision value to properly read information from the Tag
        ReadTag = nfc.antiCollision(TagData);
        memcpy(TagSerialNumber, TagData, 4); // Write the Tag information in the TagSerialNumber variable

        if(TagSerialNumber[0] == whiteTag[0] && TagSerialNumber[1] == whiteTag[1] &&
            TagSerialNumber[2] == whiteTag[2] && TagSerialNumber[3] == whiteTag[3])
          {
            whiteRecognized = true;
            if (currentPlayer == 1)
              player1Name = whiteTagName;
            else
              player2Name = whiteTagName;
            clearDisplay();
            updateDisplay();
          }

        if(TagSerialNumber[0] == blueTag[0] && TagSerialNumber[1] == blueTag[1] &&
            TagSerialNumber[2] == blueTag[2] && TagSerialNumber[3] == blueTag[3])
          {
            blueRecognized = true;
            if (currentPlayer == 1)
              player1Name = blueTagName;
            else
              player2Name = blueTagName;
            clearDisplay();
            updateDisplay();
          }
    }
  }
  
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
  
  if(gameOver)
  {
      clearMatrix();
      
      if(millis() - gameOverTime > LCD_TIMEOUT)
      {
        if(currentPlayer == 1)
        {
          player1Score = (gameOverTime - startGameTime) / 1000;
          currentPlayer = 2;
          lcd_message = "GET READY";
          clearDisplay();
          updateDisplay();
          gameOver = false;
        }
        else
        { 
          player2Score = (gameOverTime - startGameTime) / 1000;

          // compare score and declare winner
          if (player1Score > player2Score)
            winningResult = player1Name + " WINS!";
          else if (player1Score < player2Score)
            winningResult = player2Name + " WINS!";
          else
            winningResult = "TIE";

          displayScores();
          
          gameOver = false;

          currentPlayer = 1;
          clearDisplay();
          lcd_message = "GET READY";
          updateDisplay();
          delay(200);
        }
      }
  }
}

void displayScores()
{
  clearDisplay();
  lcd.setCursor(0,0);
  lcd.print(player1Name + ": " + String(player1Score));
  lcd.setCursor(0,1);
  lcd.print( player2Name + ": " + String(player2Score));
  delay(5000);

  clearDisplay();
  lcd.setCursor(0,0);
  lcd.print(winningResult);
  
  delay(7000);
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
        gameOverTime = millis();
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
  if (currentPlayer == 1)
    lcd.print(player1Name);
  else
    lcd.print(player2Name);
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
}


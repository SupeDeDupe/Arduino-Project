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

int maxItems = 2;
int lastItemDrawn = 0;

const int ROCK_SPEED = 300;
const int REDRAW_SPEED = 1;
const int AVATAR_SPEED = 170;
const int AVATAR_ROW = 7;
long lastMoveTime;
long lastDrawnTime;
long lastRockUpdate;
int avatarCol;
int lastAvatarCol;
boolean forward = true;

int rockCols[8];
int rockRows[8];

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
    // set the switch to have a default 20k pullup 
  }
  
  Serial.begin(9600);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  delay(300);
  
  for (int i = 0; i < 8; i++) {
    rockCols[i] = i;
    rockRows[i] = -1;
  }

  avatarCol = 0;
  lastAvatarCol = 1;
  lastMoveTime = millis();
  lastDrawnTime = millis();
  lastRockUpdate = millis();
  createRock();
}

void loop() {
  /*
    // if the start button is pressed, set isPlaying
    if(digitalRead(buttonPin) == 1)
      isPlaying = true;
  */
  //if (isPlaying)
  //{

  //int thisPixel = pixels[AVATAR_ROW][avatarCol];
  //digitalWrite(row[AVATAR_ROW], HIGH);
  //digitalWrite(col[avatarCol], LOW);
  
  // read input:
  //readSensors();

  j_x = analogRead(Jx_PIN);
  j_y = analogRead(Jy_PIN);
  if (digitalRead(SWITCH_PIN) == HIGH)
    s = "Switch: HIGH"; 
  else
    s = "Switch: LOW";

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

void createRock()
{
  int newRockIndex = random(0, 8);
  rockRows[newRockIndex] = 0;
}

void updateRocks()
{
   if (millis() - lastRockUpdate > ROCK_SPEED )//&& rockRows[0] < 8)
   {
    
    for (int i = 0; i < 8; i++)
    {
      if(rockRows[i] > -1)
        rockRows[i]++;
      if(rockRows[i] > 7)
      {
        rockRows[i] = -1;
        createRock();
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
    Serial.println(s + " - " + j_x + ":" + j_y);
    //Serial.println("1 " + String(avatarCol));
  }
  else if (avatarCol > 0 && j_y > 600)
  {
    // turn off current light
    // turn on the new light
    lastAvatarCol = avatarCol;
    avatarCol--;
    lastMoveTime = millis();// moveLeft
    Serial.println(s + " - " + j_x + ":" + j_y);
    //Serial.println("2 " + String(avatarCol));
  }
  else
    forward = !forward;
}


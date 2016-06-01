/*
+-----+
+----[PWR]-------------------| USB |--+    The players are kicking a ball towards each other,
|                            +-----+  |    trying to score. You must push your button when the
|         GND/RST2  [ ][ ]            |    light is on your player. If you miss, other player
|       MOSI2/SCK2  [ ][ ]  A5/SCL[ ] |    scores a goal. If you press your button when the ball
|          5V/MISO2 [ ][ ]  A4/SDA[ ] |    isn't on your player, other player scores a goal.
|                             AREF[ ] |
|                              GND[ ] |
| [ ]N/C                    SCK/13[G] |    Imagine the goal of Player One here
| [ ]v.ref                 MISO/12[P] |    Imagine Player One standing here
| [ ]RST                   MOSI/11[ ]~|
| [ ]3V3    +---+               10[ ]~|
| [ ]5v     | A |                9[ ]~|
| [ ]GND   -| R |-               8[ ] |>--->4 TO 13 ARE FOR LEDS. SEE THE COMMENTS IN THE SETUP METHOD 
| [ ]GND   -| D |-                    |
| [ ]Vin   -| U |-               7[ ] |
|          -| I |-               6[ ]~|
| [ ]A0    -| N |-               5[P]~|    Imagine Player Two standing here
| [ ]A1    -| O |-               4[G] |    Imagine the goal of Player Two here
| [ ]A2     +---+           INT1/3[ ]~|----------------->PUSHBUTTON FOR PLAYER ONE
| [ ]A3                     INT0/2[ ] |----------------->PUSHBUTTON FOR PLAYER TWO
| [ ]A4/SDA  RST SCK MISO     TX>1[ ] |    Three goals win the game. Ball speed increases with
| [ ]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |    each hit.
|            [ ] [ ] [ ]              |
|  UNO_R3    GND MOSI 5V  ____________/
\_______________________/
http://busyducks.com/ascii-art-arduinos
*/

/* Created with Visual Studio 2015 using Visual Micro extension http://www.visualmicro.com/ */

// =================================================
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
// ==================================================
bool willTheBallGoTowardsPlayerTwo = true;
bool isInputAllowed = true;

const int playerOne = 12;
const int goalPlayerOne = 13;
const int buttonPlayerOne = 3;

const int playerTwo = 5;
const int goalPlayerTwo = 4;
const int buttonPlayerTwo = 2;

int scoreOfPlayerOne = 0;
int scoreOfPlayerTwo = 0;

unsigned long millisecondsPerLED = 400; //Initial game speed. Decrease this for faster initial speed. You should also change the last line of ShowScores() when you change this.
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

int currentPosition = playerOne;        //Player one starts the game.
int previousPosition = playerOne + 1;
int deltaPosition = 0;

int buttonStatePlayerOne = 0;
int lastButtonStatePlayerOne = 0;
int buttonStatePlayerTwo = 0;
int lastButtonStatePlayerTwo = 0;

int serialVal = 0;
int pauseButton = 14;
int pauseButtonState = 0;
int pauseButtonReading;
int prevPauseButtonState = 0;
long time = 0;
long debounce = 100;
int xxx = 0;

LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

void setup() {
  Serial.begin(9600);
  pinMode(14, INPUT);
  pinMode(4, 1);
  pinMode(5, 1);
  pinMode(6, 1);
  pinMode(7, 1);
  pinMode(8, 1);
  pinMode(9, 1);
  pinMode(10, 1);
  pinMode(11, 1);
  pinMode(12, 1);
  pinMode(13, 1);
  /*Connect pins 4 to 13 to 220ohm resistors, connect the LEDs' longer legs(+) to the resistors,
    connect the other legs(-) of the LEDs' to ground.
    I recommend red LEDs for 4 and 13(Goals), yellow LEDs for 5 and 12(Players), green LEDs for other pins(Field).
  */

  pinMode(2, 0);    //Pushbuttons for players. Pin 2 is for player two. Pin 3 is for player one.
  pinMode(3, 0);    //Connect pushbuttons following the instructions on this page: https://www.arduino.cc/en/Tutorial/DigitalReadSerial
  lcd.begin(16, 2);
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home (); // go home
  lcd.print("P1:      P2:");  
  lcd.setCursor(3, 0);
  lcd.print(0);
  lcd.setCursor(12, 0);
  lcd.print(0);
}

void loop()
{
  
  pauseButtonReading = digitalRead(pauseButton);
  
  if (pauseButtonReading == 1 && prevPauseButtonState == 0 && millis() - time > debounce) {
    Serial.println(xxx++);
    if (pauseButtonState == 1) {
      pauseButtonState = 0;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Playing...");
    }
      
    else {
      pauseButtonState = 1;
      lcd.setCursor(0, 1);
      lcd.print("Game paused");
    }
      
    time = millis();
    Serial.println(pauseButtonState);
  }
  
  
  if (pauseButtonState == 0) {
    
    ListenForInput();
    currentMillis = millis();
    if (currentMillis - previousMillis >= millisecondsPerLED) {   
      CheckGoalConditions();
      DetermineNextPosition();
      MoveBallToNextPosition();
      previousMillis = currentMillis;
    }
  }
  
  prevPauseButtonState = pauseButtonReading;

}
    
  


void ListenForInput()    //If you don't understand this method. See: https://www.arduino.cc/en/Tutorial/StateChangeDetection
{
  buttonStatePlayerOne = digitalRead(buttonPlayerOne);
  buttonStatePlayerTwo = digitalRead(buttonPlayerTwo);
  
  if (buttonStatePlayerOne != lastButtonStatePlayerOne && isInputAllowed && millis() - time > debounce)
  {
    if (buttonStatePlayerOne == 1)
    {
      if (currentPosition == playerOne)
      {
        ToggleBallDirection();
      }
      else
      {
        ScoreForPlayer(2);
      }
    }
    time = millis();
    lastButtonStatePlayerOne = buttonStatePlayerOne;
  }
  if (buttonStatePlayerTwo != lastButtonStatePlayerTwo && isInputAllowed && millis() - time > debounce)
  {
    if (buttonStatePlayerTwo == 1)
    {
      if (currentPosition == playerTwo)
      {
        ToggleBallDirection();
      }
      else
      {
        ScoreForPlayer(1);
      }
    }
    time = millis();
    lastButtonStatePlayerTwo = buttonStatePlayerTwo;
  } 
}

void ToggleBallDirection()
{
  if (willTheBallGoTowardsPlayerTwo == true)
  {
    willTheBallGoTowardsPlayerTwo = false;
  }
  else
  {
    willTheBallGoTowardsPlayerTwo = true;
  }
  isInputAllowed = false;   //Only one direction change per frame is allowed for consistent gameplay.
  millisecondsPerLED -= 50; //This increases the speed at every hit. Increase this amount for more speed. I think it would be better if the decrease amount got smaller with every hit. Maybe you can improve this and make a pull request?
}

void MoveBallToNextPosition()      //Moves the ball one spot.
{
  previousPosition = currentPosition;
  digitalWrite(previousPosition, 0);
  currentPosition = currentPosition + deltaPosition;
  digitalWrite(currentPosition, 1);
  isInputAllowed = true;            //Unlocking part of only one direction change per frame limitation.
}

void DetermineNextPosition()
{
  if (willTheBallGoTowardsPlayerTwo)
  {
    deltaPosition = -1;
  }
  else
  {
    deltaPosition = 1;
  }
}

void CheckGoalConditions()
{
  if (currentPosition == goalPlayerTwo)
  {
    ScoreForPlayer(1);
  }
  else if (currentPosition == goalPlayerOne)
  {
    ScoreForPlayer(2);
  }
}

void ScoreForPlayer(int playerWhoScored)
{
  isInputAllowed = false;
  scoringFlash(playerWhoScored);
  if (playerWhoScored == 1)
  {
    scoreOfPlayerOne++;
    updateScores(1);
    ShowScores(1);
  }
  else if (playerWhoScored == 2)
  {
    scoreOfPlayerTwo++;
    updateScores(2);
    ShowScores(2);
  } 
  
  CheckEndGame();
}

void scoringFlash(int player) {
  clearBoard();
  if(player == 1) {
    for (int i = 0; i < 5; i++) {
      digitalWrite(12, 1);
      digitalWrite(13, 1);
      delay(100);
      digitalWrite(12, 0);
      digitalWrite(13, 0);
      delay(100);
    }
  }
  if(player == 2) {
    for (int i = 0; i < 5; i++) {
      digitalWrite(4, 1);
      digitalWrite(5, 1);
      delay(100);
      digitalWrite(4, 0);
      digitalWrite(5, 0);
      delay(100);
    }
  }
 
}

void clearBoard() {
  for (int i = 4; i < 14; i++) {
    digitalWrite(i, LOW);
  }
}

void CheckEndGame() 
{ 
    if (scoreOfPlayerOne == 3)
    {
      EndGameCeremonyFor(1);
    }
    if(scoreOfPlayerTwo == 3)
    {
      EndGameCeremonyFor(2);
    } 
}

void updateScores(int player) {
  
  if (player == 1) {
    lcd.setCursor(12, 0);
    lcd.print(scoreOfPlayerOne);
  }
  if (player == 2) {
    lcd.setCursor(3, 0);
    lcd.print(scoreOfPlayerTwo);
  }
}

void ShowScores(int playerCurrentlyScored) //I am ashamed of this method.
{
  /*
  int i = 0;
  int j = 0;
  int s1 = scoreOfPlayerOne;
  int s2 = scoreOfPlayerTwo;
  int scoreboard[6];         //This array will contain the pin numbers to be lit to show the score. The unused array slots will contain the pin number of the player led who recently scored so her player led will be lit with the score.

  if (playerCurrentlyScored == 1)
  {
    for (int i = 0; i < 6; i++)
    {
      scoreboard[i] = 12; //This is to ensure LED PlayerOne will be lit with the score.
    }
  }
  else if (playerCurrentlyScored == 2)
  {
    for (int i = 0; i < 6; i++)
    {
      scoreboard[i] = 5; //This is to ensure LED PlayerTwo will be lit with the score.
    }
  }
  while (s1 > 0)             //These two while loops fill the array with the pin numbers to be lit to show the score.
  {
    scoreboard[i] = 11 - j;
    i++;
    j++;
    s1--;
  }
  j = 0;
  while (s2 > 0)
  {
    scoreboard[i] = 6 + j;
    i++;
    j++;
    s2--;
  }
  for (int i = 0; i < 6; i++) //This for loop lights up the leds for scoreboard display
  {
    digitalWrite(scoreboard[i], 1);
  }
  delay(3000);                //Are three seconds enough for players to process the score?
*/
  if (playerCurrentlyScored == 1) //Sets starting position for the next round.
  {
    currentPosition = playerOne;
    willTheBallGoTowardsPlayerTwo = true;
  }
  else if (playerCurrentlyScored == 2)
  {
    currentPosition = playerTwo;
    willTheBallGoTowardsPlayerTwo = false;
  }
  postScoreEffect();
  //FlashAllLEDs(4, 0);
  millisecondsPerLED = 400;  //Sets the speed to initial value at the beginning of each round.
}

void postScoreEffect() {
  clearBoard();
  for (int i = 0; i < 3; i++) {
    digitalWrite(8, 1);
    digitalWrite(9, 1);
    delay(700);
    digitalWrite(8, 0);
    digitalWrite(9, 0);
    delay(700);
    checkForPause();
  }
}

void checkForPause() {
  buttonStatePlayerOne = digitalRead(buttonPlayerOne);
  buttonStatePlayerTwo = digitalRead(buttonPlayerTwo);
  
  if (buttonStatePlayerOne == 1 && buttonStatePlayerTwo == 1)
  {
    digitalWrite(7, 1);
    digitalWrite(10, 1);
    delay(700);
    digitalWrite(7, 0);
    digitalWrite(10, 0);
    delay(700);
    
  }
}

void EndGameCeremonyFor(int winner)
{
  //FlashAllLEDs(25, winner);
  //TurnOffAllLEDsForPlayer(0);
  lcd.setCursor(0, 1);
  lcd.print("GAME OVER!!");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("             ");
  lcd.setCursor(0, 1);
  if (winner == 1) lcd.print("Player 1 Wins!");
  else lcd.print("Player 2 Wins!");
  clearBoard();
  if(winner == 2) {
    for(int i = 4; i < 9; i++) {
      digitalWrite(i, HIGH);
      delay(50);
      digitalWrite(i, LOW);
      delay(50);
    }
    for(int i = 8; i > 3; i--) {
      digitalWrite(i, HIGH);
      delay(50);
      digitalWrite(i, LOW);
      delay(50);
    }
  }
  if(winner == 1) {
    for(int i = 8; i < 14; i++) {
      digitalWrite(i, HIGH);
      delay(50);
      digitalWrite(i, LOW);
      delay(50);
    }
    for(int i = 13; i > 7; i--) {
      digitalWrite(i, HIGH);
      delay(50);
      digitalWrite(i, LOW);
      delay(50);
    }
  }
  clearBoard();
  scoreOfPlayerOne = 0;
  scoreOfPlayerTwo = 0;
  delay(3000);
  lcd.setCursor(3, 0);
  lcd.print(0);
  lcd.setCursor(12, 0);
  lcd.print(0);
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void TurnOnAllLEDsForPlayer(int player)
{
  if (player != 1)          //When this method is called with (2), only these pins(player two's) will turn on
  {
    digitalWrite(4, 1);
    digitalWrite(5, 1);
    digitalWrite(6, 1);
    digitalWrite(7, 1);
    digitalWrite(8, 1);
  }
  if (player != 2)          //When this method is called with (1), only these pins(player one's) will turn on
  {
    digitalWrite(9, 1); 
    digitalWrite(10, 1);
    digitalWrite(11, 1);
    digitalWrite(12, 1);
    digitalWrite(13, 1);
  }
}

void TurnOffAllLEDsForPlayer(int player)
{
  if (player != 1)          //When this method is called with (2), only these pins(player two's) will turn off
  {
    digitalWrite(4, 0);
    digitalWrite(5, 0);
    digitalWrite(6, 0);
    digitalWrite(7, 0);
    digitalWrite(8, 0);
  }
  if (player != 2)          //When this method is called with (1), only these pins(player one's) will turn off
  {
    digitalWrite(9, 0);
    digitalWrite(10, 0);
    digitalWrite(11, 0);
    digitalWrite(12, 0);
    digitalWrite(13, 0);
  }
}

void FlashAllLEDs(int blinkCount, int player) //Second parameter(int player) is for when you want to flash only one player's LEDs. Use 1 for player one, use 2 for player two. 
{
  for (int i = 0; i < blinkCount; i++)
  {
    TurnOnAllLEDsForPlayer(player);
    delay(200);
    TurnOffAllLEDsForPlayer(player);
    delay(200);
  }
}

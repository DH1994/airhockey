#define startFlag 1
#define stopFlag  1 << 1
#define writeTime 1 << 2
#define goalA 1 << 3
#define goalB 1 << 4

//#define DEBUG

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //RESET, ENABLE, D4, D5, D6, D7

uint16_t timer1_counter;
uint8_t brightness = 200;
uint8_t playerA = 0;
uint8_t playerB = 0;

unsigned long startTime = 0;
int16_t playTime = 0;

String inputStr;

uint8_t statusByte = 0;

uint8_t lastSeconds = 0;


void setup() {
  // put your setup code here, to run once:
    // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1_counter to the correct value for our interrupt interval
  timer1_counter = 65411;   // preload timer 65536-16MHz/256/500Hz
  

  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
  
  pinMode(6, OUTPUT); //Display brightness
  pinMode(7, OUTPUT); //FAN

  pinMode(8, INPUT_PULLUP); // Start button

  pinMode(A0, INPUT); //LDR for goala
  pinMode(A1, INPUT); //LDR for goalb
  
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  
  lcd.begin(16, 2);

  analogWrite(6, brightness);

  beginMessage();
}

void(* resetFunc) (void) = 0; //Reset function

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  TCNT1 = timer1_counter;   // preload timer
  if(statusByte & startFlag)
  {
    if(analogRead(A0) < 800)
    {
      statusByte |= goalA;
    } 
    
    if(analogRead(A1) < 800)
    {
      statusByte |= goalB;
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  #ifdef DEBUG
  inputStr = Serial.readString();

  if(inputStr == "goala")
    statusByte |= goalA;

  if(inputStr == "goalb")
    statusByte |= goalB;
  #endif
  
  if((inputStr == "start" || !digitalRead(8)) &&  !(statusByte & startFlag))
  {
    digitalWrite(7, HIGH);
    statusByte |= (startFlag);
    statusByte &= ~(stopFlag);
    startTime = millis();  
    #ifdef DEBUG
    Serial.println("Start");
    #endif
    lcd.clear();
    delay(1000);
  }
  
  if(playTime == 300 || inputStr == "stop" || playerA > 9 || playerB > 9)
  {
    statusByte &= ~(startFlag);
    statusByte |= stopFlag;
    startTime = 0;
  }

  if(statusByte & startFlag)
  {
    playTime = (millis() - startTime) / 1000;
    int minutes = playTime / 60;
    int seconds = playTime % 60;

    if(lastSeconds != seconds)
    {
      lastSeconds = seconds;
      lcd.setCursor(0, 0);
      lcd.print("player A " + String(playerA) + " " + timeString(minutes, seconds));
      lcd.setCursor(0, 1);
      lcd.print("player B " + String(playerB));
    } 

    if(statusByte & goalA)
    {
      playerA++;
      goal();
      statusByte &= ~(goalA);
    }
  
    if(statusByte & goalB)
    {
      playerB++;
      goal();
      statusByte &= ~(goalB);
    }

    if(!digitalRead(8))
    {
      statusByte &= ~(startFlag);
      statusByte |= stopFlag;
      startTime = 0;
    }
  }

  if(statusByte & stopFlag)
  {
    digitalWrite(7, LOW);
    endMessage();
    resetFunc();
  }
}

void endMessage()
{
  lcd.clear();
  
  lcd.setCursor(0, 0);
  if(playerA > playerB)
  {
    lcd.print(" Player A WON!! ");
  }

  if(playerB > playerA)
  {
    lcd.print(" Player B WON!! ");
  }
  
  lcd.setCursor(0, 1);
  lcd.print("*** END GAME ***");
  delay(5000);
}

void goal()
{
  lcd.clear();
  lcd.print(" *** GOAL!! *** ");
  for(int i = 0; i < 10; i++)
  {
    digitalWrite(6, i%2);
    delay(200);
  }
  analogWrite(6, brightness);
  lcd.clear();
}

String timeString(int minutes, int seconds)
{
  String strMin = String(minutes);
  if(minutes < 10)
  {
    strMin = "0" + strMin;
  }

  String strSec = String(seconds);
  if(seconds < 10)
  {
    strSec = "0" + strSec;
  }

  return strMin + ":" + strSec;
}

void beginMessage()
{ 
  lcd.setCursor(0, 0);
  lcd.print(" * AIR HOCKEY * ");
  
  lcd.setCursor(2, 1);
  lcd.print("Press Start!");  
}

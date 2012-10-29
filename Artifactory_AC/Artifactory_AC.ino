
/*********************************************************
*  Air Conditioning Timer Control                        *
*                                                        *
*  Connects to the existing control panel for the office *
*  air conditioning and implements a timer, preventing   *
*  the system from being inadvertently left on.          *
*                                                        *
*  1.  Relay connected in parallel with the momentary    *
*      SPST switch that toggles the A/C system on or off *
*  2.  Air conditioning state indicator LED anode        *
*      connected to the MCU ADC                          *
*  3.  16x2 LCD panel for status output                  *
*  4.  SPST switch connected to MCU for +10min timer adj *
*                                                        *
*  Daniel Harmsworth, 2012                               *
*********************************************************/

#include <LiquidCrystal.h>

#define acSwitchPin 13        // Pin driving the relay to control the A/C power switch
#define acStatePin 9          // Pin connected to the A/C Power LED optocoupler
#define uiButtonPin 8        // Pin connected to the input button
#define backlightPin 6

#define acStateThreshold 400  // Minimum ADC value for the acStatePin to be considered ON
#define timerValue 60         // Time (in minutes) for the default timeout
#define timerIncrement 10     // Time (in minutes) to increment the timer on button press
#define timerMax 120          // Maximum time (in minutes) allowed to be on the timer

#define buttonDebounce 1000     // Time (in milliseconds) for debouncing the input button
#define lcdUpdate 1000        // Time (in milliseconds) between LCD updates

boolean acIsOn = false;               // AC is turned ON
boolean acWaitingOff = false;         // AC has been sent the turn off signal
static unsigned long acStopTime = 0;  // Time the AC will timeout in millis epoch
static unsigned long acTimeLeft;      // Time left in millis until the timer expires

// Some helpful variables to store calculated values from those set in the defines
static unsigned long acTimeMax;
static unsigned long timerIncrementMillis;

static unsigned long uiButtonDeb = 0; // Used to store the expiry time in millis epoch for the button debounce timer

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
static unsigned long lcdNextUpdate;

void setup ()
{
  pinMode(acSwitchPin, OUTPUT);
  pinMode(acStatePin, INPUT);
  pinMode(uiButtonPin, INPUT);
  //pinMode(backlightPin, OUTPUT);
  Serial.begin(9600);
  
  acTimeMax = timerMax * 60000;
  timerIncrementMillis = timerIncrement * 60000;
  
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status: ");
  
  //digitalWrite(acSwitchPin, LOW);
}

void loop()
{
  
  if ( acIsOn )
  {
    acTimeLeft = (long)( acStopTime - millis() );
  }
  else
  {
    analogWrite(backlightPin, 0);
  }
  // If the air con has just been turned on
  if ( ( digitalRead(acStatePin) == HIGH ) && ( !acIsOn ) )
  {
    acStopTime = millis() + ( timerValue * 60000 );
    acIsOn = true;
    digitalWrite(backlightPin, HIGH);
  }
  
  // If the air con has just been turned off
  if ( ( digitalRead(acStatePin) == LOW)  && ( acIsOn ) )
  {
    acWaitingOff = false;
    acIsOn = false;
  }
  
  // If the timer has expired
  if ( ( (long)( millis() - acStopTime ) >= 0 ) && ( !acWaitingOff ) && ( acIsOn) )
  {
    acWaitingOff = true;
    digitalWrite(acSwitchPin, HIGH);
    delay(500);
    digitalWrite(acSwitchPin, LOW);
  }
  
  // If the button is pressed whilst the timer is running
  if ( ( (long)( millis() - uiButtonDeb ) >= 0 ) && ( digitalRead(uiButtonPin) == HIGH ) && ( acIsOn ) )
  {
    uiButtonDeb = millis() + buttonDebounce;
    Serial.println("Button Press");
    if ( ( acTimeLeft + timerIncrementMillis ) <= acTimeMax )
    {
      acStopTime += timerIncrementMillis;
    }
    else
    {
      acStopTime = millis() + ( acTimeMax );
    }
  }
  
  // LCD Stuff
  if ( (long)( millis() - lcdNextUpdate ) >= 0 )
  {
    char l2[16] = "               ";
    lcdNextUpdate += lcdUpdate;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System: ");
    if ( acIsOn ) { lcd.print("On "); } else { lcd.print("Off"); }
    
    lcd.setCursor(0, 1);
    
    if ( acIsOn )
    {
      //lcd.print(acTimeLeft);
      int mins = acTimeLeft / 60000;
      int secs = ( acTimeLeft / 1000 ) % 60;
      sprintf(l2,"%02d:%02d Left", mins, secs);
      lcd.print( l2 );
    }
    else
    {
      //lcd.print(acStateVal);
      lcd.print("System Is Off");
    }
  }
  
}


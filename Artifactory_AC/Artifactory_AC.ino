
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
*      connected to a 4n35 optocoupler                   *
*  3.  16x2 LCD panel for status output                  *
*  4.  SPST switch connected to MCU for +10min timer adj *
*                                                        *
*  Daniel Harmsworth, 2012                               *
*********************************************************/

#include <LiquidCrystal.h>

#define acSwitchPin 13          // Pin driving the relay to control the A/C power switch
#define acStatePin 9            // Pin connected to the A/C Power LED optocoupler
#define uiButtonPin 8           // Pin connected to the input button
#define backlightPin 6          // Pin connected to the backlight on the LCD

#define timerValue 60           // Time (in minutes) for the default timeout
#define timerIncrement 10       // Time (in minutes) to increment the timer on button press
#define timerMax 120            // Maximum time (in minutes) allowed to be on the timer

#define buttonDebounce 1000     // Time (in milliseconds) for debouncing the input button
#define lcdUpdate 1000          // Time (in milliseconds) between LCD updates

#define backlightFadeTime 10    // Time (in milliseconds) between level changes for backlight fade

boolean acIsOn = false;                 // AC is turned ON
boolean acWaitingOff = false;           // AC has been sent the turn off signal but has not yet shut down
static unsigned long acStopTime = 0;    // Time the AC will timeout in millis epoch
static unsigned long acTimeLeft = 0;    // Time left in millis until the timer expires

static unsigned long uiButtonDeb = 0;   // Time in millis epoch for button debounce expiry
static unsigned long backlightFadeEnd;  // Time in millis epoch for next backlight level shift (if needed)
int backlightTarget = 0;                // Target value for the backlight (0-255)
int backlightVal = 0;                   // Current value of the backlight setting
static unsigned long lcdNextUpdate;     // Time in millis epoch for next LCD refresh

// Calculated values derived from defines
static unsigned long timerMaxMillis       = timerMax * 60000;        // timerMax converted to millis
static unsigned long timerIncrementMillis = timerIncrement * 60000;  // timerIncrement converted to millis

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup ()
{
  pinMode(acSwitchPin, OUTPUT);
  pinMode(acStatePin, INPUT);
  pinMode(uiButtonPin, INPUT);
  pinMode(backlightPin, OUTPUT);
  
  Serial.begin(9600);
  
  lcd.begin(16, 2);
  lcd.clear();  
}

void loop()
{
  
  if ( acIsOn )
  {
    acTimeLeft = (long)( acStopTime - millis() );
  }

  // If the air con has just been turned on
  if ( ( digitalRead(acStatePin) == HIGH ) && ( !acIsOn ) )
  {
    acStopTime = millis() + ( timerValue * 60000 );
    acIsOn = true;
    backlightTarget = 255;
  }
  
  // If the air con has just been turned off
  if ( ( digitalRead(acStatePin) == LOW)  && ( acIsOn ) )
  {
    acWaitingOff = false;
    acIsOn = false;
    backlightTarget = 0;
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
    
    if ( ( acTimeLeft + timerIncrementMillis ) <= timerMaxMillis )
    {
      acStopTime += timerIncrementMillis;
    }
    else
    {
      acStopTime = millis() + ( timerMaxMillis );
    }
  }
  
  // LCD Refresh Function
  if ( (long)( millis() - lcdNextUpdate ) >= 0 )
  {
    char lineBuff[16] = "               ";
    lcdNextUpdate += lcdUpdate;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System: ");
    if ( acIsOn ) { lcd.print("On "); } else { lcd.print("Off"); }

    lcd.setCursor(0, 1);
    if ( acIsOn )
    {
      int hours = ( acTimeLeft / 60000 ) / 60;
      int mins =  ( acTimeLeft / 60000 ) % 60;
      int secs =  ( acTimeLeft / 1000  ) % 60;
      
      sprintf(lineBuff,"%01d:%02d:%02d Left", hours, mins, secs);
      lcd.print( lineBuff );
    }
    else
    {
      lcd.print("A/C is Off");
    }
  }
  
  // Fader function for the backlight
  if ( (long)( millis() - backlightFadeEnd ) >= 0 )
  {
    if       ( backlightVal > backlightTarget ) { backlightVal--; }
    else if  ( backlightVal < backlightTarget ) { backlightVal++; }
    backlightFadeEnd = millis() + backlightFadeTime;
  }
  analogWrite(backlightPin, backlightVal);
}


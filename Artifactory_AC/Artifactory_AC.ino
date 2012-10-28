
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

#define acSwitchPin 13        // Pin driving the relay to control the A/C power switch
#define acStatePin A0         // Pin connected to the A/C Power LED
#define uiButtonPin 10        // Pin connected to the input button

#define acStateThreshold 100  // Minimum ADC value for the acStatePin to be considered ON
#define timerValue 60         // Time (in minutes) for the default timeout
#define timerIncrement 10     // Time (in minutes) to increment the timer on button press
#define timerMax 120          // Maximum time (in minutes) allowed to be on the timer

#define buttonDebounce 10     // Time (in milliseconds) for debouncing the input button

boolean acIsOn = false;               // AC is turned ON
boolean acWaitingOff = false;         // AC has been sent the turn off signal
static unsigned long acStopTime = 0;  // Time the AC will timeout in millis epoch
static unsigned long acTimeLeft;      // Time left in millis until the timer expires

// Some helpful variables to store calculated values from those set in the defines
static unsigned long acTimeMax;
static unsigned long timerIncrementMillis;

static unsigned long uiButtonDeb = 0; // Used to store the expiry time in millis epoch for the button debounce timer


void setup ()
{
  pinMode(acSwitchPin, OUTPUT);
  pinMode(acStatePin, INPUT);
  pinMode(uiButtonPin, INPUT);
  acTimeMax = ( timerMax * 60 ) * 1000;
  timerIncrementMillis = ( timerIncrement * 60 ) * 1000;
}

void loop()
{
  int acStateVal = analogRead(acStatePin);
  
  if ( acIsOn )
  {
    acTimeLeft = (long)( acStopTime - millis() );
  }
  
  // If the air con has just been turned on
  if ( ( acStateVal >= acStateThreshold ) && ( !acIsOn ) )
  {
    acStopTime = millis() + ( ( timerValue * 60 ) * 1000 );
  }
  
  // If the air con has just been turned off
  if ( ( acStateVal < acStateThreshold ) && ( acIsOn ) )
  {
    acWaitingOff = false;
    acIsOn = false;
  }
  
  // If the timer has expired
  if ( ( (long)( millis() - acStopTime ) >= 0 ) && ( !acWaitingOff ))
  {
    acWaitingOff = true;
    digitalWrite(acSwitchPin, HIGH);
    delay(500);
    digitalWrite(acSwitchPin, LOW);
  }
  
  if ( ( (long)( millis() - uiButtonDeb ) >= 0 ) && ( digitalRead(uiButtonPin) == HIGH ) && ( acIsOn ) )
  {
    uiButtonDeb = millis();
    if ( ( acTimeLeft + timerIncrementMillis ) <= acTimeMax )
    {
      acStopTime += timerIncrementMillis;
    }
    else
    {
      acStopTime = millis() + ( acTimeMax );
    }
  }
}


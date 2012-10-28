
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
#define acStateThreshold 100  // Minimum ADC value for the acStatePin to be considered ON
#define timerValue 60         // Time (in minutes) for the default timeout

boolean acIsOn = false;
unsigned long acStartTime;
unsigned long acStopTime;

void setup ()
{
  pinMode(acSwitchPin, OUTPUT);
  pinMode(acStatePin, INPUT);
}

void loop()
{
  int acStateVal = analogRead(acStatePin);
  
  // If the air con has just been turned on
  if ( ( acStateVal >= acStateThreshold ) && ( !acIsOn ) )
  {
    acStartTime = millis();
    acStopTime = millis() + ( ( timerValue * 60 ) * 1000 );
  }
}


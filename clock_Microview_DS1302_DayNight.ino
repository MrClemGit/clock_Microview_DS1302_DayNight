// Example sketch for interfacing with the DS1302 timekeeping chip.
//
// Copyright (c) 2009, Matt Sparks
// All rights reserved.
//
// http://quadpoint.org/projects/arduino-ds1302
#include <stdio.h>
#include <DS1302.h>
#include <MicroView.h>
#include <Time.h>

//#define SET_DATE_TIME_JUST_ONCE
//#define SERIAL_OUTPUT
#define MICROVIEW_DIGITAL
//#define MICROVIEW_ANALOG

//Case RGB LED
//uncomment this line if using a Common Anode LED
//#define COMMON_ANODE
#define RGB_LED



#ifdef MICROVIEW_ANALOG
#define clocksize 24
#define SIZE_HOUR 0.4
#define SIZE_MINUTE 0.7
#define SIZE_SECOND 0.9
uint16_t 	onDelay = 5;		// this is the on delay in milliseconds, if there is no on delay, the erase will be too fast to clean up the screen.
#endif

struct _customtime {
  unsigned int uihour;
  unsigned int uiminute;
}
;

_customtime _Sleepy_time;
_customtime _Wakeup_time;
_customtime _Sleepy_time_W;
_customtime _Wakeup_time_W;
_customtime _Sleepy_time_WE;
_customtime _Wakeup_time_WE;

bool bIsItWeekEnd;
bool bSleepy_activated;
bool bTurnOnLED;
bool bDSTOn;

#ifdef RGB_LED
int redPin = 6;
int greenPin = 5;
int bluePin = 3;
#endif
namespace {

  // Set the appropriate digital I/O pin connections. These are the pin
  // assignments for the Arduino as well for as the DS1302 chip. See the DS1302
  // datasheet:
  //
  //   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
  const int kCePin   = /*5*/A2;  // Chip Enable
  const int kIoPin   = /*6*/A0;  // Input/Output
  const int kSclkPin = /*7*/A1;  // Serial Clock

  // Create a DS1302 object.
  DS1302 rtc(kCePin, kIoPin, kSclkPin);

  String dayAsString(const Time::Day day) {
    switch (day) {
      case Time::kSunday:
      bIsItWeekEnd = true;
      return "Sunday";
      case Time::kMonday:
      bIsItWeekEnd = false;
      return "Monday";
      case Time::kTuesday:
      bIsItWeekEnd = false;
      return "Tuesday";
      case Time::kWednesday:
      bIsItWeekEnd = false;
      return "Wednesday";
      case Time::kThursday:
      bIsItWeekEnd = false;
      return "Thursday";
      case Time::kFriday:
      bIsItWeekEnd = false;
      return "Friday";
      case Time::kSaturday:
      bIsItWeekEnd = true;
      return "Saturday";
    }
    return "(unknown day)";
  }
void SetRTCTime(Time t)
{
	// Initialize a new chip by turning off write protection and clearing the
  // clock halt flag. These methods needn't always be called. See the DS1302
  // datasheet for details.
  rtc.writeProtect(false);
  rtc.halt(false);

  // Make a new time object to set the date and time.
  // Sunday, September 22, 2013 at 01:38:50.
  //Time t(2014, 10, 19, 15, 57, 00, Time::kSunday);

  // Set the time and date on the chip.
  rtc.time(t);



}
  
  bool IsDst(Time t)
{
        if (t.mon < 3 || t.mon > 10)  return false; 
        if (t.mon > 3 && t.mon < 10)  return true; 

        

        if ((t.mon == 3)&&(t.day == Time::kSunday) &&(t.date > 25) &&(t.hr >= 3))
		{
          
		  return true;
         }else
		   {
          return false;
		  }
        if ((t.mon == 10)&&(t.day == Time::kSunday)&&(t.date > 24) &&(t.hr >= 3))
        {  
		#ifdef SERIAL_OUTPUT
		 Serial.println("DST OFF\n");
		#endif
		return false;
        }else
		{
          #ifdef SERIAL_OUTPUT
		 Serial.println("DST ON\n");
		#endif
		  return true;
}
        //return false; // this line never gonna happend
}


  void printTime() {
    // Get the current time and date from the chip.
    Time t = rtc.time();
	
	//check DST for Central european time if we have to add or remove a hour 
	//if last sunday of March add a hour at 3 and if you are not already in DST summer time
	if (((t.mon==3)&&(bDSTOn == false ))||((bDSTOn == true )&&(t.mon)==10))
	{
			
		if ((bDSTOn == true )&& ((t.mon)== 10)&&(t.day == Time::kSunday) &&(t.date > 24) &&(t.hr == 3)&&(t.min == 0)&&(t.sec == 0))
		{
			bDSTOn = false;
			//remove a hour
			t.hr -=1; 
			SetRTCTime(t);
			t = rtc.time();
			
		}
		
		if ((bDSTOn == false )&& ((t.mon)== 3)&&(t.day == Time::kSunday) &&(t.date > 25) &&(t.hr == 3)&&(t.min == 0)&&(t.sec == 0))
		{
			bDSTOn = true;
			//remove a hour
			t.hr +=1; 
			SetRTCTime(t);
			t = rtc.time();
			
		}
		
		
	}
	
	//if last sunday of October remove a hour at 3 and if you are not already in DST winter time
	//

    // Name the day of the week.
    const String day = dayAsString(t.day);

#ifdef SERIAL_OUTPUT
    // Format the time and date and insert into the temporary buffer.
    char buf[50];
    snprintf(buf, sizeof(buf), "%s %04d-%02d-%02d %02d:%02d:%02d",
    day.c_str(),
    t.yr, t.mon, t.date,
    t.hr, t.min, t.sec);

    // Print the formatted string to serial so we can see the time.
    Serial.println(buf);
    
#endif

#ifdef MICROVIEW_DIGITAL
      uView.setCursor(15,1);

    char time_str[50];
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d DST:%d",t.hr, t.min, t.sec,bDSTOn);
    uView.print(time_str);
    uView.display();        // display current page buffer
    //char date_str[50];
    //snprintf(date_str, sizeof(date_str), "%s \n%04d-%02d-%02d",day.c_str(),t.yr, t.mon, t.date);

#endif

  }

}  // namespace



bool SynchroTime()
{
  bool l_bStatus = false;

  Time t = rtc.time();

  if (abs(t.min-minute())>1)
  {
#ifdef SERIAL_OUTPUT
    Serial.println("################Need to resync\n");
#endif
    l_bStatus = true;
  }

  return l_bStatus;

}


void SetArduinoTime()
{
  Time t = rtc.time();
  setTime(t.hr, t.min, t.sec, t.date, t.mon, t.yr);
  bDSTOn = IsDst(t);
}
void dimColor()
{

}


void setColor(int red, int green, int blue)
{

#ifdef COMMON_ANODE
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
#endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);

}
void setLED(bool bSleepy_Mode)
{
  if (bSleepy_Mode == true)
  {
#ifdef RGB_LED
    setColor(0, 1, 10);
#else
    //LED blue On
    //LED yellow Off

#endif


  }
  else
  {
#ifdef RGB_LED
    if (bTurnOnLED == true)
    {
      setColor(255, 20, 20);
    }
    else
    {
      setColor(0,0, 0);
    }
#else
    //LED yellow On
    //LED blue Off

#endif


  }
}
void MAJSleepAndWakeHours()
{
#ifdef SERIAL_OUTPUT
  Serial.println("MAJSleepAndWakeHours\n");
#endif
  Time t = rtc.time();
  if ((t.day == Time::kFriday)||(t.day == Time::kSaturday))
  {
#ifdef MICROVIEW_DIGITAL
    uView.setCursor(1,1);

    char time_str[50];
    snprintf(time_str, sizeof(time_str), "WE");
    uView.print(time_str);
    uView.display();        // display current page buffer


#endif
    _Sleepy_time.uihour=_Sleepy_time_WE.uihour;
    _Sleepy_time.uiminute=_Sleepy_time_WE.uiminute;
    _Wakeup_time.uihour=_Wakeup_time_WE.uihour;
    _Wakeup_time.uiminute=_Wakeup_time_WE.uiminute;
  }
  else
  {
#ifdef MICROVIEW_DIGITAL
    uView.setCursor(1,1);

    char time_str[50];
    snprintf(time_str, sizeof(time_str), "W");
    uView.print(time_str);
    uView.display();        // display current page buffer


#endif
    _Sleepy_time.uihour=_Sleepy_time_W.uihour;
    _Sleepy_time.uiminute=_Sleepy_time_W.uiminute;
    _Wakeup_time.uihour=_Wakeup_time_W.uihour;
    _Wakeup_time.uiminute=_Wakeup_time_W.uiminute;

  }
}
void DisplayWakeup()
{

#ifdef SERIAL_OUTPUT
  Serial.println("DisplayWakeup\n");
#endif
  MAJSleepAndWakeHours();
  char Sleep_str[50];
  char Wakeup_str[50];

  snprintf(Sleep_str, sizeof(Sleep_str), "BedTime   %02d:%02d\n",_Sleepy_time.uihour, _Sleepy_time.uiminute);
  snprintf(Wakeup_str, sizeof(Wakeup_str), "Waking up %02d:%02d\n",_Wakeup_time.uihour, _Wakeup_time.uiminute);

  uView.setCursor(1,17);
  uView.print(Sleep_str);
  uView.setCursor(1,33);
  uView.print(Wakeup_str);
  uView.display();        // display current page buffer

}



void IsSleepyOrWakeUpTime()
{
  if ((bSleepy_activated==false) && ((hour() == _Sleepy_time.uihour) && (minute() >= _Sleepy_time.uiminute)))
  {
    //Allumer la lumière de nuit

    bSleepy_activated = true;
#ifdef SERIAL_OUTPUT
    Serial.println("Allumer la lumière de nuit");
#endif

  }
  else if ((bSleepy_activated==true) && ((hour() == _Wakeup_time.uihour) && (minute() >= _Wakeup_time.uiminute)))
  {
    //Allumer la lumière de jour

    bSleepy_activated = false;
    DisplayWakeup();
#ifdef SERIAL_OUTPUT
    Serial.println("Allumer la lumière de réveil");
#endif

  }

  /*if ((bSleepy_activated==false) && ((hour() >= _Wakeup_time.uihour+2) && ((hour() <= _Sleepy_time.uihour-2)&&(minute() >= _Sleepy_time.uiminute))))
  {
    bTurnOnLED = false;
    Serial.println("eteindre la LED");
  }
  else
  {
    bTurnOnLED = true;
    Serial.println("Allumer la LED");
  }*/




}

void setup()
{
  _Sleepy_time_W.uihour=21;
  _Sleepy_time_W.uiminute=0;
  _Wakeup_time_W.uihour=7;
  _Wakeup_time_W.uiminute=0;

  _Sleepy_time_WE.uihour=21;
  _Sleepy_time_WE.uiminute=15;
  _Wakeup_time_WE.uihour=8;
  _Wakeup_time_WE.uiminute=0;


  
#ifdef SERIAL_OUTPUT
Serial.begin(115200);
  while (!Serial) ;  
#endif


  bSleepy_activated = false;
  bTurnOnLED = true;

#ifdef SET_DATE_TIME_JUST_ONCE
 // Make a new time object to set the date and time.
  // Sunday, September 22, 2013 at 01:38:50.
  Time t(2014, 10, 26, 9, 38, 00, Time::kSunday);
  SetRTCTime(t);  
#endif
  bDSTOn= true;
  SetArduinoTime();
  
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);




#ifdef MICROVIEW_DIGITAL
  uView.begin();
  uView.clear(ALL);	// erase hardware memory inside the OLED controller
  uView.clear(PAGE);	// erase the memory buffer, when next uView.display() is called, the OLED will be cleared.
#endif

#ifdef MICROVIEW_ANALOG
  uView.begin();

  uView.clear(ALL);	// erase hardware memory inside the OLED controller
  uView.display();	// display the content in the buffer memory, by default it is the MicroView logo

  delay(700);
  uView.clear(PAGE);	// erase the memory buffer, when next uView.display() is called, the OLED will be cleared.
  uView.setFontType(0); 		// set font type 0, please see declaration in MicroView.cpp
  uView.setCursor(27, 0);		// points cursor to x=27 y=0
  uView.print(12);
  uView.setCursor(30, uView.getLCDHeight() - uView.getFontHeight());
  uView.print(6);
  uView.setCursor(0, uView.getLCDHeight() / 2 - (uView.getFontHeight() / 2));
  uView.print(9);
  uView.setCursor(uView.getLCDWidth() - uView.getFontWidth(), uView.getLCDHeight() / 2 - (uView.getFontHeight() / 2));
  uView.print(3);
  uView.display();			// display the memory buffer drawn
#endif


#ifdef MICROVIEW_DIGITAL
  DisplayWakeup();
#endif

  if ((hour() >= _Sleepy_time.uihour) || (hour() <= _Wakeup_time.uihour))
  {
    bSleepy_activated = true;

#ifdef SERIAL_OUTPUT
    Serial.println("Allumer la lumière de nuit");
#endif
  }
  else
  {
    bSleepy_activated = false;

#ifdef SERIAL_OUTPUT
    Serial.println("Allumer la lumière de réveil");
#endif
  }
  setLED(bSleepy_activated);


}
void DisplayAnalog()
{
#ifdef MICROVIEW_ANALOG
  static double counter = 99999;
  static unsigned long mSec = millis() + 1000;
  static uint8_t x0, y0, x1, y1;
  static float degresshour = -1, degressmin, degresssec, hourx, houry, minx, miny, secx, secy;
  static float sizeHour = clocksize * SIZE_HOUR;
  static float sizeMinute = clocksize * SIZE_MINUTE;
  static float sizeSecond = clocksize * SIZE_SECOND;

  if (mSec != (unsigned long)second())
  {
    if (degresshour != -1)
    {
      uView.line(32, 24, 32 + hourx, 24 + houry, WHITE, XOR);
      uView.line(32, 24, 32 + minx, 24 + miny, WHITE, XOR);
      uView.line(32, 24, 32 + secx, 24 + secy, WHITE, XOR);
    }

    degresshour = (((hour() * 360) / 12) + 270) * (PI / 180);
    degressmin = (((minute() * 360) / 60) + 270) * (PI / 180);
    degresssec = (((second() * 360) / 60) + 270) * (PI / 180);

    hourx = cos(degresshour) * sizeHour;
    houry = sin(degresshour) * sizeHour;

    minx = cos(degressmin) * sizeMinute;
    miny = sin(degressmin) * sizeMinute;

    secx = cos(degresssec) * sizeSecond;
    secy = sin(degresssec) * sizeSecond;


    uView.line(32, 24, 32 + hourx, 24 + houry, WHITE, XOR);
    uView.line(32, 24, 32 + minx, 24 + miny, WHITE, XOR);
    uView.line(32, 24, 32 + secx, 24 + secy, WHITE, XOR);
    uView.display();

    mSec = second();
    printTime();
  }
#endif
}
// Loop and print the time every second.
void loop() {

  if (SynchroTime() == true)
  {
    SetArduinoTime();
  }
#ifdef MICROVIEW_ANALOG
  DisplayAnalog();
#endif

  IsSleepyOrWakeUpTime();
  setLED(bSleepy_activated);

#ifndef MICROVIEW_ANALOG
  printTime();
  delay(1000);
#endif







}



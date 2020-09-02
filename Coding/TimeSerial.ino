/*
   TimeSerial.pde
   example code illustrating Time library set through serial port messages.

   Messages consist of the letter T followed by ten digit time (as seconds since Jan 1 1970)
   you can send the text on the next line using Serial Monitor to set the clock to noon Jan 1 2013
  T1357041600

   A Processing example sketch to automatically send the messages is included in the download
   On Linux, you can use "date +T%s\n > /dev/ttyACM0" (UTC time zone)
*/
#include <LiquidCrystal.h>
#include <TimeLib.h>

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

String weekdayN[7] = {"Sun", "Mon", "Tue", "Wed", "Thurs", "Fri", "Sat"};

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int dateP = 11;

void setup()  {
  Serial.begin(9600);
   //Button
  pinMode(dateP, INPUT);
  
  while (!Serial) ; // Needed for Leonardo only
  pinMode(13, OUTPUT);
  lcd.println("Waiting for sync message");
  setSyncProvider( requestSync);  //set function to call when sync required
 
}

void loop() {
  if (Serial.available()) {
    processSyncMessage();
  }
  if (timeStatus() != timeNotSet) {
    digitalClockDisplay();
  }
  if (timeStatus() == timeSet) {
    digitalWrite(13, HIGH); // LED on if synced
  } else {
    digitalWrite(13, LOW);  // LED off if needs refresh
  }
  delay(1000);
  lcd.clear();
}

void digitalClockDisplay() {
  // digital clock display of the time
  bool v = digitalRead(dateP);
  //https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week (Guass Method)
  int weekdayV = (1 + 5*((year()-1) % 4) + 4*((year()-1) % 100) + 6*((year()-1) % 400) % 7);
  
    lcd.print(day());
    lcd.print(", ");
    lcd.print(weekdayN[weekdayV]);
    lcd.print(month());
    lcd.print(" ");
    lcd.print(year());
    
    lcd.print(hourFormat12());
    printDigits(minute());
    printDigits(second());
//    if(isAM()){
//      lcd.print(" AM");
//    }
//    else{
//      lcd.print("PM");
//    }
//  }
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  lcd.print(":");
  if (digits < 10)
    lcd.print('0');
  lcd.print(digits);
}


void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    if ( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
      setTime(pctime); // Sync Arduino clock to the time received on the serial port
    }
  }
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);
  return 0; // the time will be sent later in response to serial mesg
}

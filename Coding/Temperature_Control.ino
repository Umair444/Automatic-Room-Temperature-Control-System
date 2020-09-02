#include <LiquidCrystal.h>
#include <TimeLib.h>

#define NUM_SAMPLES 10

int tempL_A = 30, tempL_H = 20;      //Temperature limits

int outputP_AC = 8;
int outputP_HT = 9;
int inputPA = 10;     //Output from LM335, to arduino
int timeP = 12;
//Temperature Output Value, 2.7315V per 273.15k
float tempOV = 0;      //No voltage at start

//___________________________________________________________
int sum = 0;                    // sum of samples taken
unsigned char sample_count = 0; // current sample number
float voltage = 0.0;            // calculated voltage

//____________________________LCD INIT_______________________
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//___________________________TIME INIT_______________________
#define TIME_HEADER "T"
#define TIME_REQUEST 7

//________________________Setup______________________________
void setup() {
  Serial.begin(9600);
  pinMode(outputP_AC, OUTPUT);
  pinMode(outputP_HT, OUTPUT);

  //____LCD_SETUP________________________________
  lcd.begin(16, 2);
  // Print Message to LCD
  lcd.print("____Welcome!____");
  delay(3000);
  lcd.clear();

  //______TIME_SETUP________________________________
  while (!Serial) ; // Needed for Leonardo only
  pinMode(13, OUTPUT);
  pinMode(12, INPUT);
  setSyncProvider( requestSync);
}

int TempM() {             //Temperature Measurement

  //First Find Voltages
  while (sample_count < NUM_SAMPLES) {
    sum += analogRead(A2);
    sample_count++;
    delay(10);
  }
  voltage = ((float)sum / (float)NUM_SAMPLES * 5.015) / 1024.0;
  Serial.print(voltage);
  Serial.println (" V");
  sample_count = 0;
  sum = 0;

  tempOV = voltage * 100;         //In Kelvins
  tempOV = tempOV - 273;          //In Celcius
  return tempOV;
}

int avg = 0;
void LCD_Scroll(int n) { //n = no. of chars
  //For single Line mutiply n by 2, in this case I use clear()
  delay(2000);
  for (int pos = 0; pos < n; pos++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    delay(500);
  }
  lcd.clear();
}

//You can Define more to print
String str = " ", str1 = "Temperature: ";
void LCD_Print() {
  String tempStr = String(avg , DEC);
  String str = String(str1 + tempStr);
  lcd.print(str);

  //ADD OTHER PRINTS

  LCD_Scroll(str.length());
}

//_____________LEAP____________________________
int leapV = 0;
int Leap(int y) {   //Input year
  if (y % 4 == 0 && y % 400 == 0) {
    //Year is leap
    leapV = 1;
    if (y % 100 == 0) {
      //Year is not leap
      leapV = 0;
    }
  }
  else {
    //Year is not leap
    leapV = 0;
  }
  return leapV;
}

//__________________TIME CALCULATIONS___________________________________________________________________________
int R(int A, int B) {
  return A % B;
}
int monthDays[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
String daysName[7] = {"Sun", "Mon", "Tue", "Wed", "Thurs", "Fri", "Sat"};
String monthsName[12] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"};

int SumMonth(int n) { //n = no. of times you want to sum = month()-2 (-1 for array)
  int sum = 0, store = 0;
  for (int i = 0; i < n; i++) { //A month less than given
    sum = store + monthDays[i];
    store = sum;
  }
  return sum;
}

int WEEKDAY(int d, int m, int y) {
  //1st January weekday is:
  //https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week (Guass Method)
  int weekdayV = R(1 + R(y - 1, 4) * 5 + R(y - 1, 100) * 4 + R(y - 1, 400) * 6 , 7);
  int L = Leap(y);

  if (L == 1) { //Year is leap
    weekdayV = R(weekdayV + R(SumMonth(m) + d + 1, 7), 7) - 1;
  }
  if (L == 0) {
    weekdayV = R(weekdayV + R(SumMonth(m) + d, 7), 7) - 1;
  }

  return weekdayV;
}

String MONTHDAY(int m) {
  return monthsName[m - 1];
}

//______TIME_PRINT_____________________________
void digitalClockDisplay() {
  // digital clock display of the time

  lcd.print(hour());
  printDigits(minute());
  printDigits(second());

  //  lcd.print(daysName[WEEKDAY(month(), year())]);
  lcd.setCursor(0, 1);
  Serial.print(daysName[WEEKDAY(day(), month(), year())]);
  Serial.println();
  lcd.print(day());
  lcd.print("/");
  lcd.print(MONTHDAY(month()));
  lcd.print("/");
  lcd.print(year() % 100);
  lcd.print(",");
  lcd.print(daysName[WEEKDAY(day(), month(), year())]);
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  lcd.print(":");
  if (digits < 10)
    lcd.print('0');
  lcd.print(digits);
}

//________TIME_DETERMINATION_________________________________
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

int count = 0;
void loop() {
  bool timeR = digitalRead(timeP);
  if (timeR == true) {
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
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 0);
  }

  else {
    //Average Temperature Loop
    tempOV = TempM();
    int store = tempOV, n = 10;    //'n' = no. of samples for avg
    for (int i = 0; i < n; i++) {
      tempOV = TempM();
      avg = (store + tempOV) / 2;
      store = avg;
      delay(100);   //10 * 100 = 1sec (FOR NOW)
    }

    if (tempOV > tempL_A) {
      digitalWrite(outputP_AC, HIGH);   //Turn on AC
    }
    if (tempOV < tempL_H) {
      digitalWrite(outputP_HT, HIGH);   //Turn on Heater
    }
    if (tempOV < tempL_A && tempOV > tempL_H) { //Turn off
      digitalWrite(outputP_AC, LOW);
      digitalWrite(outputP_HT, LOW);
    }

    //Turn on LCD
    lcd.display();
    //Print on LCD
    LCD_Print();
  }
}




//References:
/*
   https://startingelectronics.org/articles/arduino/measuring-voltage-with-arduino/

*/

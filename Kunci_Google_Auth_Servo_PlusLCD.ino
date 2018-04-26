#include <Key.h>
#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <sha1.h>
#include <TOTP.h>
#include <DS3231.h>
#include <LiquidCrystal_I2C.h>

DS3231 rtc(A4,A5);

LiquidCrystal_I2C lcd(0x3F, 16, 2); //mendefine LCD alamat I2C 0x3F, 16x2 LCD

// debug print, menggunakan #define DEBUG untuk enable Serial output
// referensi http://forum.arduino.cc/index.php?topic=46900.0


// servo configuration: PIN, door closed/opened position and speed
#define SERVO_PIN     9


uint8_t hmacKey[] = {0x65, 0x6d, 0x62, 0x64, 0x64, 0x73, 0x79, 0x73, 0x74, 0x6d};
//Password: embddsystm
TOTP totp = TOTP(hmacKey, 10);

// keypad configuration
const byte rows = 4;
const byte cols = 3;
char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[rows] = {2, 3, 4, 5};
byte colPins[cols] = {6, 7, 8};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

Servo doorServo;


char* totpCode;
char inputCode[7];
int inputCode_idx=0;
boolean doorOpen;
void wilkommen(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" TOTP DOORLOCK");
  lcd.setCursor(0, 1);
  lcd.print(" Masukkan Kode!");  
}


void setup() {
  lcd.begin();
  lcd.backlight();
  

  doorServo.attach(SERVO_PIN);
  lcd.setCursor(0, 0);
  lcd.print("Selamat Datang!");
  lcd.setCursor(0, 1);
  lcd.print("Servo OK");  
  delay(1000);

  // init software RTC with the current time

  rtc.begin();
  lcd.setCursor(0, 1);
  lcd.print("Clock OK");  
  delay(1000);
  
  // reset input buffer index
  inputCode_idx = 0;
  
  // close the door
  doorServo.write(0);
  doorOpen = false;
  wilkommen();

  
}


void loop() {
  
  char key = keypad.getKey();

  // a key was pressed
  if (key != NO_KEY) {

    // # resets the input buffer    
    if(key == '#') {
      lcd.setCursor(0, 0);
      lcd.print("# ditekan...");
      lcd.setCursor(0, 1);
      lcd.print("Mereset Buffer.");
      delay(1000);
      wilkommen();
      inputCode_idx = 0;      
    }
    
    // * closes the door
    else if(key == '*') {

      if(doorOpen == false){ 
      lcd.setCursor(0, 0);
      lcd.print("* ditekan, tapi");
      lcd.setCursor(0, 1);
      lcd.print("Pintu Tertutup");
      delay(1000);
      wilkommen();
      }

      else {
      lcd.setCursor(0, 0);
      lcd.print("* ditekan...");
      lcd.setCursor(0, 1);
      lcd.print("Menutup Pintu");
      delay(1000);
      wilkommen();
          doorServo.write(0);
        doorOpen = false;
      }
    }
    else {      
      // save key value in input buffer
      inputCode[inputCode_idx++] = key;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Kode masukan:");
      lcd.setCursor(0, 1);
      lcd.print(inputCode);
      // if the buffer is full, add string terminator, reset the index
      // get the actual TOTP code and compare to the buffer's content
      if(inputCode_idx == 6) {
        
        inputCode[inputCode_idx] = '\0';
        inputCode_idx = 0;
        delay(500);
        
        
        long GMT = rtc.getUnixTime(rtc.getTime()); //perubahan unix time berdasarkan waktu GMT
        totpCode = totp.getCode(GMT); //Ingat!!! Waktu yang ada pada RTC adalah waktu GMT
        
        // code is ok :)
        if(strcmp(inputCode, totpCode) == 0) {
          if(doorOpen == true) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Kode Benar, tapi");
            lcd.setCursor(0, 1);
            lcd.print("Pintu Terbuka");
            lcd.print(inputCode);
            delay(1000);
          }
          
          else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Kode Benar!");
            lcd.setCursor(0, 1);
            lcd.print("Membuka Pintu");
            delay(1000);
            wilkommen();
              doorServo.write(90);
            doorOpen = true;
          }
          
        // code is wrong :(  
        } else {
         //kode benar totpCode
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Kode Salah!");
          lcd.setCursor(0, 1);
          lcd.print("Hubungi Admin!!!");
          delay(1000); 
          wilkommen();
        }
      }      
    }
  }
}




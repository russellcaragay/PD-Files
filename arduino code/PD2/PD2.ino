#include <LiquidCrystal_PCF8574.h> //For LCD Library
#include <SoftwareSerial.h> //For GSM Library

//Variables
int counter = 0; //For 60 seconds counter
bool alarm = false; //Toggles alarm
bool call = false; //Toggles call
String func, cctvNum, number1, number2, time, location; //For sms

//For LCD
LiquidCrystal_PCF8574 lcd(0x27);

//For GSM
//TXD Pin 6
//RXD Pin 7
SoftwareSerial simPin(6, 7);

void setup()
{
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  
  simPin.begin(9600);
  Serial.begin(9600);
  while(!Serial);

  //Pins 8, 9, 10, 11
  //8 - Green LED
  //9 - Red Led
  //10 - Buzzer
  //11 - Buzzer Power
  DDRB = B1111;
}

void loop()
{
  //No accident
  if(!alarm)
  {
    PORTB = B0001;
    lcd.clear();
    delay(1000);
    lcd.setCursor(0,0);
    lcd.print("Monitoring...");
    delay(1000);
  }

  //Accident Detected (Buzzer Tone)
  if(alarm){
    tone(10,1000);
    delay(500);
    noTone(10);
    delay(500);
  }

  //GSM Call number1
  if(call){
    //60 Seconds Counter
    if(counter <= 60){
      simPin.println("ATD+63" + number1 + ";\r\n"); // Command to call the user
      counter++;
    }else{
      Serial.println("End Call");
      simPin.println("ATH"); // end call
    }
  }
    
  //Serial Communication
  while(Serial.available() > 0)
  {
    //Sample text for serial: 19218270084921827008412:00AM12345678901234567890123456789012
    String message = Serial.readString();
    message.trim(); //Remove spaces

    func = message.substring(0,1); //0 - reset, 1 to 4 - alarm
    number1 = message.substring(1,11); // 2nd to 11th -> First number (9*********)
    number2 = message.substring(11,21); // 12th to 21st -> Second number (9*********)
    time = message.substring(21,28); // 22nd to 28th -> time (12:00AM)

    if(message.length() <= 60) //Only 1 accident
    {
      cctvNum = message.substring(0,1); // 1st character in the serial must be number "0" to "4"
      location = "Location: " + message.substring(28,59); // 29th to 59th -> Location (31 characters max)
    }
    else if(message.length() <= 120) //2 accidents
    {
      cctvNum = message.substring(0,1); //Camera #1
      location = "Cam " + message.substring(0,1) + " Location: " + message.substring(28,59) + "\n"; //Location of Camera #1

      cctvNum += " & " + message.substring(60,61); //Camera #2
      location += "Cam " + message.substring(60,61)  + " Location: " + message.substring(88,119); //Location of Camera #2
      
    }
    else //3 accidents
    {
      cctvNum = message.substring(0,1); //Camera #1
      location = "Cam " + message.substring(0,1) + " Location: " + message.substring(28,59) + "\n"; //Location of Camera #1

      cctvNum += " & " + message.substring(60,61); //Camera #2
      location += "Cam " + message.substring(60,61) + " Location: " + message.substring(88,119) + "\n"; //Location of Camera #2

      cctvNum += " & " + message.substring(120,121); //Camera #3
      location += "Cam " + message.substring(120,121)  + " Location: " + message.substring(148,179); //Location of Camera #3
    }

    //Debugging
    Serial.println(message.length());
    Serial.println(message);
    Serial.println(cctvNum);
    Serial.println(number1);
    Serial.println(number2);
    Serial.println(time);
    Serial.println(location);

    if(func >= "1" && func <= "4") //Accident Detected - For SMS
    {
      alarm = true;
      call = true;
      counter = 0;
      PORTB = B1010;
      
      lcd.clear();
      lcd.print("Accident! Cam #");
      lcd.setCursor(0,1);
      lcd.print(cctvNum);

      //Call SendMessage Function
      alertResponder();
      alertResponder2();
      
    }
    else if(func == "0") //Resetting the device
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Resetting...");
      delay(1000);

      //Reset the value of counter
      counter = 0;
      alarm = false;
      call = false;
      number1 = "";
      number2 = "";
      time = "";
      cctvNum = "";
      location = "";
      
      simPin.println("ATH"); // end call
    }  
    else //Invalid Input
    {
      Serial.println("Invalid Input");
    }

  }
}

void alertResponder()
{
  simPin.println("AT+CMGF=1\r\n"); //AT command to set GSM module to Text Mode
  delay(10);

  simPin.println("AT+CMGS=\"+63" + number1 + "\"\r\n"); //AT command set first destination number
  delay(10);

  simPin.println("ACCIDENT DETECTED!");
  simPin.println("Cameras #: " + cctvNum);
  simPin.println(location);
  simPin.println("Time: " + time);
  simPin.println("Please Coordinate to the Command Center Immediately!");
  delay(50);

  simPin.println((char)26);
  delay(4300);
}

void alertResponder2()
{
  simPin.println("AT+CMGF=1\r\n"); //AT command to set GSM module to Text Mode
  delay(10);

  simPin.println("AT+CMGS=\"+63" + number2 + "\"\r\n"); //AT command set first destination number
  delay(10);

  simPin.println("ACCIDENT DETECTED!");
  simPin.println(cctvNum);
  simPin.println(location);
  simPin.println("Time: " + time);
  simPin.println("Please Coordinate to the Command Center Immediately!");
  delay(50);

  simPin.println((char)26);
  delay(4300);
}
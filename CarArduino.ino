#include <SoftwareSerial.h>
#include <Adafruit_INA219.h>
#include <Adafruit_SSD1306.h>
#include <TimerOne.h>
#include <Wire.h>
#include <SPI.h>


SoftwareSerial mySerial(10,11);

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_INA219 ina219;
#define bluetoothPin 2
#define statePin 6 //Pin 4 was occupied by OLED_RESET
#define motorPin 0


char state = 'g';
char state2= '3';
int count = 0, connection = 99;
unsigned long previousMillis = 0;
unsigned long interval = 500;
const int chipSelect = 10;
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float energy = 0;
String test = "";
void isr()
{
  count++;
}
void ina219values() {
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  energy = energy + loadvoltage * current_mA / 3600;
}
void decrypt(char info)
{

  test = "Connection Status :"; test += connection;
  test += "\nVoltage :"; test += loadvoltage; test += "V";
  test += "\nCurrent : "; test += current_mA; test += "mA";
  test += "\nmW : " ; test += (loadvoltage * current_mA); test += "mWh";
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(test);
  display.display();
}
void setup() {
  mySerial.begin(9600);
  Serial.begin(9600); // Default communication rate of the Bluetooth module
  pinMode(bluetoothPin, OUTPUT);
  digitalWrite(bluetoothPin, HIGH);
  pinMode(statePin, INPUT);
  Timer1.initialize(250000);         // initialize timer1, and set a 1/4 second period
  Timer1.attachInterrupt(isr); // attaches isrTimer() as a timer overflow interrupt
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  ina219.begin();
}
void loop() {

  int x = analogRead(motorPin); //Check if attached motor is moving
  double voltage = (x / 1023.0) * 5;
  connection = digitalRead(statePin);
 // Serial.println(state2); 
   //Serial.println(connection);
  //  Serial.println(voltage);
  if (voltage != 0) //if the motor is moving shut the bluetooth sensor down
    digitalWrite(bluetoothPin, LOW);
  else { //if the motor is not moving turn on the bluetooth sensor
    digitalWrite(bluetoothPin, HIGH);
    //Serial.println(Serial.available());
    if(mySerial.available() > 0)
      {
        state2 = mySerial.read();
        }
    
  }
  if (Serial.available() > 0) { // Checks whether data is comming from the serial port
      
        state = Serial.read(); // Reads the data from the serial port
      }
      if (state2 == 'W') {
        Serial.println('F');
        delay(100);
      }
      else
        Serial.println('Q');
    
    unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    ina219values();
   decrypt(state);
   String msg= "";
   msg += connection; msg +=","; msg += loadvoltage; msg +=","; msg += current_mA; msg += ","; msg +=(loadvoltage * current_mA);
   mySerial.println(msg);
    //messages are sent as string of bytes, separated by a comman for each data
   }
}

#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define RX  2 // TX of esp8266 in connected with Arduino pin 2
#define TX  3 // RX of esp8266 in connected with Arduino pin 3
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Measuring Current Using ACS712
int sensitivity = 66;  // use 100 for 20A Module and 66 for 30A Module
int adcvalue = 0;
int offsetvoltage = 2500;  //Sensor Callibration
double Voltage = 0;        //voltage measuring
double current = 0;        // Current measuring

String WIFI_SSID = "Redmi 12C";   // WIFI NAME
String WIFI_PASS = "thinkpad";    // WIFI PASSWORD
String API = "W5FNNAYF5FX40M5N";  // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
int countTrueCommand;
int countTimeCommand;
boolean found = false;
SoftwareSerial esp8266(RX, TX);
void setup() {
  lcd.init();
  lcd.backlight();
  lcd.print("REMOTE SOLAR");
  Serial.begin(115200);
  esp8266.begin(115200);
  lcd.setCursor(0, 1);
  lcd.print("POWER MONITORING");
  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_PASS + "\"", 20, "OK");  
  lcd.clear();
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi: ");
  lcd.print(WIFI_SSID);
  delay(3000);
}
void loop() {
  // Measure The Votage ***********************************
  // read the input on analog pin A0:
  int sensorValue = analogRead(A0);
  float vol = (sensorValue * 5.0) / 1023.0;
  float voltage = abs(vol);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Voltage = ");
  lcd.print(voltage);
  lcd.print("V");

  // Measure The Current ***********************************
  adcvalue = analogRead(A1);             //reading the value from the analog pin A0
  Voltage = (adcvalue / 1024.0) * 5000;  // Gets you mV
  current = ((Voltage - offsetvoltage) / sensitivity);
  current=abs(current);
  lcd.setCursor(0, 1);
  lcd.print("Current = ");
  lcd.print(current);
  lcd.print("A");  //unit for the current to be measured

  //To send the data to server
  String getData = "GET /update?api_key=" + API + "&field1=" + voltage + "&field2=" + current;
  sendCommand("AT+CIPMUX=1", 5, "OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
  sendCommand("AT+CIPSEND=0," + String(getData.length() + 4), 4, ">");
  esp8266.println(getData);
  delay(1500);
  countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0", 5, "OK");
  delay(3000);

  //Power and energy
  float power = voltage * current;
  float energy = power * 60;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Power = ");
  lcd.print(power);
  lcd.print("W");
  lcd.setCursor(0, 1);
  lcd.print("Energy = ");
  lcd.print(energy);
  lcd.print("Wh");
  delay(3000);
}
void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1)) {
    esp8266.println(command);      //at+cipsend
    delay(100);
    if (esp8266.find(readReplay))  //ok
    {
      found = true;
      break;
    }
    countTimeCommand++;
  }
  if (found == true) {
    Serial.println("OK");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  if (found == false) {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
}
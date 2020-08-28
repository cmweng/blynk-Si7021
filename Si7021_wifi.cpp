// ************************
// reference：
// BLynk
// https://examples.blynk.cc/?board=ESP8266&shield=ESP8266%20WiFi&example=More%2FDHT11
// ESP32 & Si7021
// http://www.esp32learning.com/code/esp32-and-si7021-temperature-sensor-example.php
//
// 此程式會將Si7021抓到的溫到傳到Vertual Pin (6), 溼度傳到Vertual Pin (5)，可自行修改
// Audunio IDE需先安裝Blynk (by Volodymyr Shymanskyy) library
// 需先下載Blynk app，註冊後，新增project，取得Auth Token
// 請將Auth Token及wifi SSID、PASSWORD輸入程式中
//**************************

#include "Wire.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = " ";
// Your WiFi credentials.
char ssid[] = " ";
char pass[] = " ";

// SI7021 I2C address is 0x40(64)
#define si7021Addr 0x40

// h濕度，t溫度
float h;
float t;

// deepsleep
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1800        /* Time ESP32 will go to sleep (in seconds) */

// core 0 led blink
TaskHandle_t countdown;    //ore0執行
boolean countdown_flag = false;
#define ledPin  2
int led = 1;

// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
void sendSensor()
{

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V5, h);  // Virtual Pin (5) 溼度
  Blynk.virtualWrite(V6, t);  // Virtual Pin (6) 溫度
}

void get_data(){
  unsigned int data[2];
   
  Wire.beginTransmission(si7021Addr);
  //Send humidity measurement command
  Wire.write(0xF5);
  Wire.endTransmission();
  delay(500);
     
  // Request 2 bytes of data
  Wire.requestFrom(si7021Addr, 2);
  // Read 2 bytes of data to get humidity
  if(Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }
     
  // Convert the data
  float humidity  = ((data[0] * 256.0) + data[1]);
  humidity = ((125 * humidity) / 65536.0) - 6;
 
  Wire.beginTransmission(si7021Addr);
  // Send temperature measurement command
  Wire.write(0xF3);
  Wire.endTransmission();
  delay(500);
     
  // Request 2 bytes of data
  Wire.requestFrom(si7021Addr, 2);
   
  // Read 2 bytes of data for temperature
  if(Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }
 
  // Convert the data
  float temp  = ((data[0] * 256.0) + data[1]);
  float celsTemp = ((175.72 * temp) / 65536.0) - 46.85;
  float fahrTemp = celsTemp * 1.8 + 32;

   h = humidity;
   t = celsTemp;
   
  // Output data to serial monitor
  Serial.print("Humidity : ");
  Serial.print(humidity);
  Serial.println(" % RH");
  Serial.print("Celsius : ");
  Serial.print(celsTemp);
  Serial.println(" C");
  Serial.print("Fahrenheit : ");
  Serial.print(fahrTemp);
  Serial.println(" F");

}

void countdown_code(void* parameter){
  for(;;){
        if(led==1){
          led = 0;
        }else if(led == 0){
          led = 1;
        }
        digitalWrite(ledPin, led);   
        delay(500);
  }
} 

void setup()
{
  setCpuFrequencyMhz(80); //Set CPU clock to 80MHz
  Wire.begin();
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, led);  
  // blink led while sending data
  xTaskCreatePinnedToCore(
        countdown_code, /* Function to implement the task */
        "countdown", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        1,  /* Priority of the task */
        &countdown,  /* Task handle. */
        0); /* Core where the task should run */
  delay(100);

  Wire.beginTransmission(si7021Addr);
  Wire.endTransmission();
  delay(300);

  get_data();
  
  Blynk.begin(auth, ssid, pass);
  countdown_flag = true;
  sendSensor();
  

  delay (1000);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
  
}
 
void loop()
{
  delay(100);
}

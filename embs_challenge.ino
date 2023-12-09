#include <FB_Const.h>
#include <FB_Error.h>
#include <FB_Network.h>
#include <FB_Utils.h>
#include <Firebase.h>
#include <FirebaseFS.h>
#include <Firebase_ESP_Client.h>


#include"addons/TokenHelper.h"
#include"addons/RTDBHelper.h"
#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
const int lm35Pin = 36;

const int modeEXButtonPin = 17;

#define REPORTING_PERIOD_MS 1000

bool mode_ex = false;
float BPM, SpO2;
#define WIFI_SSID "opporeno7"  
#define WIFI_PASSWORD "123456"
#define API_KEY "AIzaSyCQx6V7V-Oo3-Q2RMx-I8V7MdsFkonHrm0" 
#define DATABASE_URL "https://embs-challenge-c510d-default-rtdb.firebaseio.com/"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false ;
const int age = 25;

  // Firebase data object
PulseOximeter pox;  // PulseOximeter object for MAX30100 sensor
uint32_t tsLastReport = 0;

void onBeatDetected()
{
  Serial.println("Beat!");  // Callback function for beat detection
}

void setup() {
  Serial.begin(115200);
  
  pinMode(modeEXButtonPin, INPUT_PULLUP);



  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config,&auth,"","")) {
    Serial.println("signup ok");
    signupOK = true;
  }else{
    Serial.printf("%s\n",config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  

  Serial.print("Initializing pulse oximeter..");

  // Initialize the PulseOximeter
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
    pox.setOnBeatDetectedCallback(onBeatDetected);  // Set the beat detection callback
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);  // Set the IR LED current

   // Initialize Firebase connection
}

void loop() { // Send data to Firebase
  if (Firebase.ready() &&signupOK&& (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){     
    sendDataPrevMillis = millis();
   pox.update();  // Update the PulseOximeter data
  BPM = pox.getHeartRate();  // Get heart rate from the sensor
  SpO2 = pox.getSpO2();  // Get blood oxygen saturation from the sensor
float temperature = readLM35Temperature();
  
  if (Firebase.RTDB.pushFloat(&fbdo,"/sensorData/SpO2",SpO2)){
    Serial.print("SPO2: ");
  Serial.println(SpO2);
  
                if ((SpO2>=95)&&(SpO2<=100)) {Serial.print("normal and healthy oxygen saturation levels");} 
                else if ((SpO2>90)&&(SpO2<=95)){Serial.print("midly lower than normal consult a health caare professional if persitent");}
                else  if ((SpO2>=86)&&(SpO2<=90)) {Serial.print("moderate reduction in oxygrnr saturation .seek medical attentions");} 
              else  if (SpO2<86) {Serial.print("critical low oxygenn levels.urgent medical intervention required");
              }
              
  }else{Serial.println("failed to push spo2 data to firebase");
   }delay(300000);
  if (Firebase.RTDB.setFloat(&fbdo,"/sensorData/BPM",BPM)){
    Serial.print("BPM: ");
  Serial.println(BPM);
  if (mode_ex == false) {if ((BPM>40)&&(BPM<60)){
                  Serial.print("your resting heart rate is on the lower side , which can be normal for well-conditioned athletes .if you're not an athlete ,it's good to check with healthcare professional to ensure it's within a healthy range for you ");
                 }
                 else if ((BPM>=60)&&(BPM<=100)){Serial.print("your resting heart rate is withun the normal range.keep monitoring for consitency ,and if you notice significant deviations , consider discussing with a healthcare professionals");
                  }
                  else if (BPM<=40){Serial.print("severe bradycardia may lead to insufficient blood flow and related symptoms  .urgent medical attention is crucial to determine the cause and appropriate treatment .");
                    }
                    else if (BPM>=150){Serial.print("severe tachycardia can strain the heart and lead to complications .urgent medical attention is necessary to identify the cause and determine appropriate intervention .");
                      }
                      else if ((BPM>120)&&(BPM<150)){Serial.print("moderate tachycardia may be a sign of underlying issues like anxiety ,anemia , or heart conditions .consultaion with a healthcare professional for evaluation is recommended");
                        }
                        else if ((BPM>100)&&(BPM<=120)){Serial.print("mild tachycardia may result from stress , dehydration,or excessive caffeine intake .monitoring lifestyle factors and staying hydrated can help manage this.");
                          } 
    if (mode_ex == true ){if ((BPM>=(((220-age)/100))*50)&&(BPM<(((220-age)/100))*70)){Serial.print("exellent job job maintainig a moderate-intensity heart rate , this range is beneficial for overall cardiovascular health and endurance.keep it up.");
                  }
                 else if ((BPM>=(((220-age)/100))*70)&&(BPM<(((220-age)/100))*85)){Serial.print("great effort reaching a vigorous-intensity heartrate this range enhances cardiovascular fitness , helps burn calories , and contributes to improved endurance .keep challenging yourself. ");
                    }
                  else if ((BPM>=(((220-age)/100))*85)&&(BPM<=(((220-age)/100))*100)){Serial.print("exercising at or near your maximum heart rate should be approached cautiously.while short bursts of intensity can be part of interval training ,prolonged periods at this level may increase the risk of injury or overtraining.consult with a fitness professional for personalized guidance");
                    }
                   else if  (BPM>(((220-age)/100))*100){Serial.print("exercising above 100% of your maximum heart rate is not recommended ,as it can lead to overexertion and increased risk of injury .ensure your workout intensity stays within safe and sustainable limits for optimal health benefits ");
                    }
                     else if  (BPM<(((220-age)/100))*50){Serial.print("if your heart rate consistenly stays below the target zone ,you may not be challenging yourself enough to achiev optimal cardiovascular benefits .adjust your exercise intensity to ensure you're within the recommended target range . ");
                     }
  }else{Serial.println("failed to push bpm data to firebase");} delay(300000);
  if (Firebase.RTDB.setFloat(&fbdo,"/sensorData/temperature",temperature)){
   Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  if (temperature<36){Serial.print("if your temperature is consistly below the normal range,it might be advisible to consult with a healthcare professional to rule any underlying health conditions .");}
  else if ((temperature>=36)&&(temperature<38)){Serial.print("congratulations your body temperature is within the normal ranfe .this is a positive sign of good overall health");}
  else if ((temperature>38)&&(temperature<40)){Serial.print("a high fever may be a sign of illness.ensure you stay well-hydrated,rest,and consider seeking medical attetion if the fever persist or if other symptoms develop.");}
  else if (temperature>=40){Serial.print("a body temperatue this high can be dangerous .seek emergency medical attention immediatly .");}
  

  
  }else{Serial.println("failed to push temperature data to firebase");}delay(300000);
  }
  }
  }}
  float readLM35Temperature() {
  int sensorValue = analogRead(lm35Pin);
  
  // Convert the analog sensor value to temperature in Celsius
  float temperature = (sensorValue / 4095.0) * 3300.0 / 10.0;
  return temperature;
}

#include <Adafruit_ThinkInk.h>
#include <Arduino.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"
#include <ArduinoJson.h>
#include "prediction.hpp"

#define EPD_DC      33 // A9can be any pin, but required!
#define EPD_CS      15  // A8 can be any pin, but required!
#define EPD_BUSY    -1  // can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS     32  // can set to -1 to not use a pin (uses a lot of RAM!)
#define EPD_RESET   -1  // can set to -1 and share with chip Reset (can't deep sleep)

// 2.9" Grayscale Featherwing or Breakout:
ThinkInk_290_Grayscale4_T5 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

#define COLOR1 EPD_BLACK
#define COLOR2 EPD_LIGHT
#define COLOR3 EPD_DARK
// WiFi network name and password:
const char * networkName = "xxxx";
const char * networkPswd = "xxxx";

const char* mbta_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"PcCdHIbhN5c9\n" \
"-----END CERTIFICATE-----\n";


bool connected = false;

//Are we currently connected?
float voltage;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;
long StartTime;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  StartTime = millis();
  Serial.println("Begin program");
  connectToWiFi(networkName, networkPswd);
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  //display.begin(THINKINK_GRAYSCALE4);
  display.begin(THINKINK_MONO);
  
  if (!connected) {
      connectToWiFi(networkName, networkPswd);
  }
  // put your main code here, to run repeatedly:
  PrepDisplay();
  WriteBanner();
  String mbta_data = get_mbta_prediction_json(2579);
  parse_and_display(mbta_data);
  display.display();
  wifi_off();
  BeginSleep();
  sleep(60);

}

void loop() {

}

void WriteBanner() {
  char batt_str[30];
  read_batt(batt_str);
  display.print(batt_str);
  printLocalTime();
  display.setCursor(150, 1);
  display.print(&timeinfo, "%B %d %Y %H:%M");
  display.drawFastHLine(0, 14, display.width(), COLOR1);
}

void PrepDisplay() {
  display.clearBuffer();
  display.setTextSize(1);
  display.setCursor(0, 1);
  display.setTextColor(COLOR1);
  display.setTextWrap(true);

}

void ScreenInit() {
  display.clearBuffer();
  display.setTextSize(2);
  display.setCursor(5, 50);
  display.setTextColor(COLOR1);
  display.setTextWrap(true);
  display.print("System Booting");
  display.display();
}

void parse_and_display(String &payload) {
  DynamicJsonDocument doc(6144);

  DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
}
  Serial.println();
  //serializeJsonPretty(doc, Serial);
  int i = 0;
  StopPrediction predictions[15];
  int num_predictions = 0;
  display.setTextSize(2);


  for (JsonObject elem : doc["data"].as<JsonArray>()) {

    JsonObject attributes = elem["attributes"];
    const char* arrival_time = attributes["arrival_time"]; // "2021-03-07T09:30:51-05:00",
    JsonObject relationships = elem["relationships"];

    const char* route_num = relationships["route"]["data"]["id"]; // "87", "88"
    const char* stop_id = relationships["stop"]["data"]["id"]; // "2579",
    predictions[i] = StopPrediction(route_num, stop_id, arrival_time);
    if (String(route_num) == "88") {
      predictions[i].set_via("Highlnd");
    }
    else if (String(route_num) == "87") {
      predictions[i].set_via("Elm");
    }
    i++;
    Serial.printf("Adding prediction %d \n", i);
    num_predictions++;
  };
  i = 0;
  write_predictions(predictions, num_predictions);
}

void write_predictions(StopPrediction predictions[], int num_predictions) {
  // Write an array of predictions to the screen
  int line_number = 0;
  for (int s = 0; s <= num_predictions; s++) {
    auto pred = predictions[s];
    Serial.print("Writing prediction to screen");
    int cursor_pos_y = 16 * (line_number + 1);
    display.setCursor(1, cursor_pos_y);
    Serial.printf("set curser to y: %d \n", cursor_pos_y);
    Serial.println("write string to screen buff");
    int arriving_in = pred.get_countdown_min(timeinfo);
    Serial.printf("On print object %d \n", s);
    if (arriving_in < 0) {
      continue;
    }
    if (line_number > 6) {
      break;}
    if (String(pred.via()) == "0") {
      display.printf("Bus: %s %d Min", pred.route(), arriving_in);
    }
    else {
      display.printf("Bus: %s %d Min v %s", pred.route(), arriving_in, pred.via());
    }
    line_number++;
  }
}

String get_mbta_prediction_json(int stop) {
  while (!connected) {
    sleep(1);
  }
  HTTPClient http;

  Serial.print("[HTTP] begin...\n");
  String mbta_query_url = "https://api-v3.mbta.com/predictions?filter[stop]=";
  mbta_query_url += stop;
  http.begin(mbta_query_url, mbta_cert); //HTTP

  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  String payload;

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          payload = http.getString();
      }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      payload = "HTTP Error";
  }
  http.end();
  return payload;
}

void printLocalTime()
{
   while (!connected) {
    sleep(1);
  }
  while (!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    sleep(1);
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
  while (!connected) {
    sleep(1);
  }
}

void read_batt(char * fill_batt) {
  float voltage = analogRead(A13)*2*(3.3/4096);
  Serial.printf("Bat voltage: %f", voltage);
  sprintf(fill_batt, "Batt Voltage: %.3f", voltage);
  Serial.println();
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());

          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}

void wifi_off() {
  connected = false;
  WiFi.disconnect(true);
}

void BeginSleep() {
  // Function for deep sleep power savings - not implimented yet.
  long SleepDuration = 60; // Sleep time in seconds, aligned to the nearest minute boundary, so if 30 will always update at 00 or 30 past the hour
  int  WakeupTime    = 7;  // Don't wakeup until after 07:00 to save battery power
  int  SleepTime     = 23; // Sleep after (23+1) 00:00 to save battery power
  long SleepTimer = (SleepDuration ); //Some ESP32 are too fast to maintain accurate time
  esp_sleep_enable_timer_wakeup((SleepTimer) * 1000000LL); // Added +20 seconnds to cover ESP32 RTC timer source inaccuracies

  Serial.println("Entering " + String(SleepTimer) + "-secs of sleep time");
  Serial.println("Awake for : " + String((millis() - StartTime) / 1000.0, 3) + "-secs");
  Serial.println("Starting deep-sleep period...");
  esp_deep_sleep_start();  // Sleep for e.g. 30 minutes
}
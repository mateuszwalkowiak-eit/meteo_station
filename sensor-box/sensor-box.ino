#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "ESPAsyncWebServer.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <math.h> 

const char* ssid = "SSID";
const char* password = "pass";

// OTA
static const char* otaHostname = "ESP8266-WeatherStation";
static const char* otaPassword = ""; 

#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5
#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 600            //mv  (z dokumentacji: ~0.6V przy 0 mg/m3)
#define        SYS_VOLTAGE                     5000           
#define SEALEVELPRESSURE_HPA (1013.25)
const char* ntpServer = "pl.pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const long timeOffset_sec = gmtOffset_sec + daylightOffset_sec;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, timeOffset_sec, 60000);
/*
I/O define
*/
const int iled = 2;                                            //drive the led of sensor
const int vout = 0;                                            //analog input

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
float density, voltage;
int   adcvalue;
float temperature;
float humidity;
float pressure;
int quality;
int procent_density;
String airQualityText; // opis jakości powietrza do HTML/SSE
AsyncWebServer server(80);
AsyncEventSource events("/events");

unsigned long lastTime = 0;  
unsigned long timerDelay = 30000;  // send readings timer

void getBME280Readings(){
 
  temperature = bme.readTemperature();
  pressure = bme.readPressure() / 100.0F;
  humidity = bme.readHumidity();
}

static const char* getAirQualityText(float d) {
  if (d <= 0) return "Brak danych";
  if (d <= 35) return "Znakomita jakość powietrza";
  if (d <= 75) return "Dobra jakość powietrza";
  if (d <= 115) return "Lekkie zanieczyszczenie powietrza";
  if (d <= 150) return "Średnie zanieczyszczenie powietrza";
  if (d <= 250) return "Mocne zanieczyszczenie powietrza";
  if (d <= 500) return "Ogromne zanieczyszczenie powietrza";
  return "Ekstremalne zanieczyszczenie powietrza";
}

String processor(const String& var){
  getBME280Readings();
  if(var == "TEMPERATURE"){
    return String(temperature);
  }
  else if(var == "HUMIDITY"){
    return String(humidity);
  }
  else if(var == "PRESSURE"){
    return String(pressure, 1);
  }
  else if(var == "DUST"){
    return String(density);
  }
  else if(var == "PROCENT_DENSITY"){
    return String(procent_density);
  }
  else if(var == "AIR_QUALITY_TEXT"){
    return airQualityText;
  }
  return String();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
<meta charset="utf-8">
  <title>Stacja meteorologiczna</title>
</head>
<body>
  <div class="topnav">
    <h3>Stacja meteorologiczna</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> TEMPERATURA</h4><p><span class="reading"><span id="temp">%TEMPERATURE%</span> &deg;C</span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> WILGOTNOŚĆ</h4><p><span class="reading"><span id="hum">%HUMIDITY%</span> &percnt;</span></p>
      </div>
      <div class="card pressure">
        <h4><i class="fas fa-angle-double-down"></i> CIŚNIENIE</h4><p><span class="reading"><span id="pres">%PRESSURE%</span> hPa</span></p>
      </div>
      <div class="card dust">
        <h4> PYŁ PM2.5</h4><p><span class="reading"><span id="dust">%DUST%</span> µg/m3</span></p>
        </div>
        <div class="card quality">
        <h4> Jakość powietrza</h4>
        <p><span class="reading"><span id="quality">%PROCENT_DENSITY%</span>%</span></p>
        <p><span id="qualityText">%AIR_QUALITY_TEXT%</span></p>
        </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('temperature', function(e) {
  console.log("temperature", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);
 
 source.addEventListener('humidity', function(e) {
  console.log("humidity", e.data);
  document.getElementById("hum").innerHTML = e.data;
 }, false);
 
 source.addEventListener('pressure', function(e) {
  console.log("pressure", e.data);
  document.getElementById("pres").innerHTML = e.data;
 }, false);

 source.addEventListener('dust', function(e) {
  console.log("dust", e.data);
  document.getElementById("dust").innerHTML = e.data;
 }, false);

 source.addEventListener('quality', function(e) {
  console.log("quality", e.data);
  document.getElementById("quality").innerHTML = e.data;
 }, false);

 source.addEventListener('qualityText', function(e) {
  console.log("qualityText", e.data);
  document.getElementById("qualityText").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";
/*
private function
*/
int Filter(int m)
{
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;
    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

static const char* postUrl = "http://127.0.0.1/weather/post-data.php";
static const char* apiKeyValue = "apikey";

static bool postReadingsToServer() {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(4000);

  if (!http.begin(client, postUrl)) return false;
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String payload;
  payload.reserve(220);
  payload += "api_key="; payload += apiKeyValue;
  payload += "&value1="; payload += String(temperature, 1);
  payload += "&value2="; payload += String((int)(humidity + 0.5f));
  payload += "&value3="; payload += String(pressure, 1);
  payload += "&value4="; payload += String(density, 2);
  payload += "&value5="; payload += String(procent_density);
  payload += "&value6="; payload += String(WiFi.RSSI());

  int code = http.POST(payload);
  http.end();

  return (code > 0 && code < 400);
}

// GP2Y1010AU0F (Waveshare) - parametry pomiaru i konwersji
static constexpr float ADC_INTERNAL_VREF = 1.0f;   // V na pinie ADC ESP8266 
static constexpr float A0_SCALE          = 3.2f;   
static constexpr int   ADC_MAX           = 1023;

static constexpr uint32_t GP2Y_SAMPLE_PERIOD_US = 10000; // 10ms zgodnie z typowym przebiegiem
static constexpr int      GP2Y_SAMPLES          = 15;
static constexpr int      GP2Y_TRIM             = 2;

static constexpr uint8_t GP2Y_LED_ON_LEVEL  = HIGH;
static constexpr uint8_t GP2Y_LED_OFF_LEVEL = LOW;

static constexpr float GP2Y_V0_DEFAULT_V = 0.6f;   // ~0.6V przy 0 mg/m3
static constexpr float GP2Y_V_SAT_V      = 3.6f;   // ~3.6–3.7V plateau
static constexpr float GP2Y_MAX_MG_M3    = 0.5f;   // liniowo do 0.5 mg/m3
static constexpr float GP2Y_SLOPE_V_PER_MG_M3 = (GP2Y_V_SAT_V - GP2Y_V0_DEFAULT_V) / GP2Y_MAX_MG_M3; // 6.0 V/(mg/m3)

static constexpr float GP2Y_V0_FLOOR_V = (NO_DUST_VOLTAGE / 1000.0f);

static float gp2yV0_A0 = NAN;
static volatile bool gp2yCalibrateRequested = false;
// zrób pierwszą kalibrację chwilę po starcie (po „rozgrzaniu” czujnika)
static unsigned long gp2yInitialCalibDueMs = 0;
static bool gp2yInitialCalibDone = false;

static void gp2ySetBaselineV0(float vA0) {
  if (vA0 < GP2Y_V0_FLOOR_V) vA0 = GP2Y_V0_FLOOR_V;
  gp2yV0_A0 = vA0;
  Serial.print("GP2Y baseline V0 (A0) set to: ");
  Serial.print(gp2yV0_A0, 3);
  Serial.println(" V");
}

static void gp2yCalibrateBaseline(uint8_t repeats) {
  float sum = 0.0f;
  for (uint8_t i = 0; i < repeats; ++i) {
    sum += gp2yReadVoltageA0();
    ArduinoOTA.handle();
    yield();
  }
  gp2ySetBaselineV0(sum / (float)repeats);
}

static uint16_t gp2yReadAdcOnce() {
  digitalWrite(iled, GP2Y_LED_ON_LEVEL);
  delayMicroseconds(280);
  uint16_t a = (uint16_t)analogRead(vout);
  digitalWrite(iled, GP2Y_LED_OFF_LEVEL);
  return a;
}

static void sort_u16(uint16_t* a, int n) {
  for (int i = 0; i < n - 1; ++i) {
    for (int j = i + 1; j < n; ++j) {
      if (a[j] < a[i]) {
        uint16_t t = a[i]; a[i] = a[j]; a[j] = t;
      }
    }
  }
}

static float gp2yReadVoltageA0() {
  uint16_t s[GP2Y_SAMPLES];
  for (int i = 0; i < GP2Y_SAMPLES; ++i) {
    const uint32_t t0 = micros();
    s[i] = gp2yReadAdcOnce();

    while ((uint32_t)(micros() - t0) < GP2Y_SAMPLE_PERIOD_US) {
      ArduinoOTA.handle();
      yield();
    }
  }

  sort_u16(s, GP2Y_SAMPLES);

  // trimmed mean
  uint32_t sum = 0;
  const int from = GP2Y_TRIM;
  const int to = GP2Y_SAMPLES - GP2Y_TRIM;
  for (int i = from; i < to; ++i) sum += s[i];
  const float avgAdc = (float)sum / (float)(to - from);

  const float vAdc = (avgAdc / (float)ADC_MAX) * ADC_INTERNAL_VREF; // V na ADC (wewnętrzny)
  const float vA0  = vAdc * A0_SCALE;                               // V na pinie A0 (zewnętrzny)
  return vA0;
}

// Referencja do procentowego wskaźnika jakości (spójna z progami w getAirQualityText: 35 ug/m3 = "znakomita")
static constexpr float PM25_REF_UGM3 = 35.0f;

static float gp2yVoltageToUgM3(float vA0) {
  if (isnan(gp2yV0_A0)) {
    gp2ySetBaselineV0(GP2Y_V0_DEFAULT_V); // domyślnie 0.6V (z dokumentacji)
  }

  if (vA0 >= GP2Y_V_SAT_V) {
    // 0.5 mg/m3 = 500 ug/m3 (wg opisu charakterystyki)
    return GP2Y_MAX_MG_M3 * 1000.0f;
  }

  const float deltaV  = vA0 - gp2yV0_A0;
  const float deltaMv = deltaV * 1000.0f;

  float ugm3 = deltaMv * COV_RATIO;
  if (ugm3 < 0) ugm3 = 0;

  if (ugm3 < 15.0f && vA0 < gp2yV0_A0) {
    gp2yV0_A0 = 0.999f * gp2yV0_A0 + 0.001f * vA0;
    if (gp2yV0_A0 < GP2Y_V0_FLOOR_V) gp2yV0_A0 = GP2Y_V0_FLOOR_V;
  }

  return ugm3;
}

void setup() {
  pinMode(iled, OUTPUT);
  digitalWrite(iled, GP2Y_LED_OFF_LEVEL);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.print(rssi);
  Serial.print(" dBm \n");
  timeClient.begin();
  bool status;

  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not detect a BME280 sensor, Fix wiring Connections!");
    while (1);
  }
  
  // OTA init
  ArduinoOTA.setHostname(otaHostname);
  if (otaPassword && otaPassword[0] != '\0') {
    ArduinoOTA.setPassword(otaPassword);
  }
  ArduinoOTA.begin();

  // Pierwsza kalibracja baseline po ~3s od staru
  gp2yInitialCalibDueMs = millis() + 3000;

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Prosty endpoint do ponownej kalibracji baseline (uruchom w „czystym” powietrzu)
  server.on("/calibrate-dust", HTTP_GET, [](AsyncWebServerRequest *request){
    gp2yCalibrateRequested = true;
    request->send(200, "text/plain; charset=utf-8", "OK: dust baseline calibration scheduled\n");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
}

void loop() {
  ArduinoOTA.handle();

  static unsigned long lastDustMs = 0;

  // wysyłka wyrównana do 5 minut wg NTP
  static unsigned long lastSentSlot5m = 0;
  static bool lastSentSlot5mInit = false;
  const unsigned long slotSeconds = 300;          // 5 minut
  const unsigned long sendWindowSeconds = 5;      // tolerancja na start slotu

  const unsigned long dustIntervalMs = 1000;
  const unsigned long now = millis();

  // Wykonaj pierwszą kalibrację baseline po starcie
  if (!gp2yInitialCalibDone && (long)(now - gp2yInitialCalibDueMs) >= 0) {
    gp2yInitialCalibDone = true;
    gp2yCalibrateBaseline(6);
  }

  // Kalibracja baseline na żądanie z HTTP
  if (gp2yCalibrateRequested) {
    gp2yCalibrateRequested = false;
    gp2yCalibrateBaseline(6);
  }

  timeClient.update();

  if (now - lastDustMs >= dustIntervalMs) {
    lastDustMs = now;

    const float vA0 = gp2yReadVoltageA0();
    voltage = vA0;
    density = gp2yVoltageToUgM3(vA0);

    procent_density = (int)((density / PM25_REF_UGM3) * 100.0f + 0.5f);
    airQualityText = getAirQualityText(density);

    Serial.print("PM2.5: ");
    Serial.print(density, 1);
    Serial.print(" ug/m3, A0=");
    Serial.print(vA0, 3);
    Serial.print(" V, V0=");
    Serial.print(gp2yV0_A0, 3);
    Serial.print(" V, poziom=");
    Serial.print(procent_density);
    Serial.print("%\n");
    Serial.println(airQualityText);
  }

  // Wysyłka tylko na początku każdego 5-minutowego przedziału czasu (xx:00, xx:05, ...)
  const unsigned long epoch = timeClient.getEpochTime();
  const bool epochLooksValid = (epoch > 1600000000UL);
  if (epochLooksValid) {
    const unsigned long slot = epoch / slotSeconds;
    const unsigned long inSlot = epoch % slotSeconds;

    if (inSlot < sendWindowSeconds && (!lastSentSlot5mInit || slot != lastSentSlot5m)) {
      lastSentSlot5mInit = true;
      lastSentSlot5m = slot;

      getBME280Readings();

      events.send("ping", NULL, millis());
      events.send(String(temperature).c_str(), "temperature", millis());
      events.send(String(humidity).c_str(), "humidity", millis());
      events.send(String(pressure, 1).c_str(), "pressure", millis());
      events.send(String(density).c_str(), "dust", millis());
      events.send(String(procent_density).c_str(), "quality", millis());
      events.send(airQualityText.c_str(), "qualityText", millis());

      (void)postReadingsToServer();
    }
  }
}

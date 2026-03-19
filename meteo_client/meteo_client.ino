#include <TFT_eSPI.h>

#ifndef TFT_LIME
#define TFT_LIME 0x07E0
#endif

#include <WiFi.h>
#include <NTPClient.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const int backlightPin = 21; // Backlight Control 

const char* ssid = "SSID";
const char* password = "pass";
const char* serverUrl = "http://127.0.0.1/wclient/";
const char* ntpServer = "pl.pool.ntp.org";
long gmtOffset_sec = 7200;
const int daylightOffset_sec = 7200;
const char* monthNames[] = {"stycznia", "lutego", "marca", "kwietnia", "maja", "czerwca", "lipca", "sierpnia", "wrzesnia", "pazdziernika", "listopada", "grudnia"};
const char* dayNames[] = {"niedziela", "poniedzialek", "wtorek", "sroda", "czwartek", "piatek", "sobota"};

unsigned long lastNtpUpdate = 0;
const unsigned long ntpUpdateInterval = 86400000; // 24 godziny w milisekundach

TFT_eSPI tft = TFT_eSPI();
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

// --- DODANE ZMIENNE GLOBALNE DO STEROWANIA JASNOŚCI ----------
int brightnessMax = 180;      // jasność przy zachodzie (0-255)
int brightnessMin = 20;       // minimalna jasność w nocy (0-255)
int currentBrightness = 60;   // aktualnie ustawiona jasność
int lastBacklightUpdateHour = -1;
bool haveSunTimes = false;
String sunriseStr = "";       // zapisujemy czasy jako stringi
String sunsetStr = "";
String sunriseDiffStr = "";
String sunsetDiffStr = "";
String poraDniaStr = "";

// --- DODATKOWE ZMIENNE DO OGRANICZENIA ODŚWIEŻANIA GUI ----------
unsigned long lastGuiUpdate = 0;
const unsigned long guiInterval = 1000;   
unsigned long lastRssiUpdate = 0;
const unsigned long rssiInterval = 5000;   

String lastTimeDisplayed = "";
int lastGreetingHour = -1;
int lastRSSI = -9999;
String lastSunriseStr = "";
String lastSunsetStr = "";
String lastSunriseDiffStr = "";
String lastSunsetDiffStr = "";
String lastPoraDniaStr = "";

void setupWiFiAndOTA() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  tft.setCursor(20, 100);
  tft.println("meteo_client");
  delay(2000); 

  tft.setTextSize(2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // limit czasu oczekiwania na WiFi (ms). Po przekroczeniu nastąpi restart ESP32.
  const unsigned long wifiTimeout = 15000;
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 225);
    tft.print("Laczenie z siecia Wi-Fi...");
    delay(500);
    if (millis() - wifiStart > wifiTimeout) {
      // Wyświetl komunikat i zrestartuj urządzenie
      tft.fillRect(0, 225, 320, 20, TFT_BLACK);
      tft.setCursor(0, 225);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.print("Blad laczenia z WiFi - restartuje...");
      delay(3000);
      ESP.restart();
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(WiFi.localIP());
      tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  tft.setCursor(20, 100);
  tft.println("meteo_client");
    tft.setCursor(0, 225);
    tft.setTextSize(2);  
    tft.print("Polaczono pomyslnie");
    delay(1000);
    tft.fillRect(0, 225, 200, 20, TFT_BLACK);

    timeClient.begin();
    timeClient.setUpdateInterval(60000); // Update time every 60 seconds
      tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  tft.setCursor(20, 100);
  tft.println("meteo_client");
    tft.setCursor(0, 225);  
    tft.setTextSize(2);
    tft.print("Synchronizacja czasu...");
    delay(1000);
    tft.fillRect(0, 225, 200, 20, TFT_BLACK);

    // OTA setup
    ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
      });

    ArduinoOTA.setHostname("meteo_client");

    ArduinoOTA.begin();

    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
      tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  tft.setCursor(20, 100);
  tft.println("meteo_client");
    tft.setCursor(0, 225);  
    tft.setTextSize(2);
    tft.print("Pobieram dane...");
    delay(1000);
    tft.fillScreen(TFT_BLACK);
  } else {
    tft.setCursor(0, 225);  
    tft.print("Blad laczenia z WiFi!");
  }
}


uint16_t getTemperatureColor(float temperature) {
  if (temperature < -10.0) {
    return TFT_NAVY;      // bardzo zimno
  } else if (temperature < 0.0) {
    return TFT_CYAN;      // zimno
  } else if (temperature < 5.0) {
    return TFT_BLUE;      // chłodno
  } else if (temperature < 10.0) {
    return TFT_SKYBLUE;   // umiarkowanie chłodno
  } else if (temperature < 15.0) {
    return TFT_GREEN;     // łagodnie
  } else if (temperature < 20.0) {
    return TFT_LIME;      // przyjemnie ciepło
  } else if (temperature < 25.0) {
    return TFT_YELLOW;    // ciepło
  } else if (temperature < 30.0) {
    return TFT_ORANGE;    // gorąco
  } else if (temperature < 35.0) {
    return TFT_RED;       // bardzo gorąco
  } else {
    return TFT_MAGENTA;   // ekstremalnie gorąco
  }
}
int getWiFiSignalStrength() {
  long rssi = WiFi.RSSI(); // Wartość RSSI w dBm
  return rssi;
}
unsigned long lastWiFiUpdate = 0;
const unsigned long wifiUpdateInterval = 5000; // Interwał aktualizacji sygnału WiFi (5 sekund)

void displayWiFiSignalStrength(int signalStrength) {
  unsigned long now = millis();
  if (now - lastRssiUpdate < rssiInterval) return; // throttle RSSI

  String s = String(signalStrength) + " dBm";
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int x = 230;            // wyrównanie do wilgotności
  int y = 2;              // nad wilgotnością (tak, by nie zachodzić na y=24)
  int approxCharW = 6;    // przybliżona szerokość znaku dla size=1
  int textW = s.length() * approxCharW * 2; // textSize=2
  int textH = 16;         // przybliżona wysokość linii dla textSize=2 (~16 px)
  int clearH = textH + 2; // minimalny zapas, łącznie ~18 px (nie dojdzie do y=24)

  // Czyść tylko wąski pasek nad wilgotnością
  tft.fillRect(x, y, textW + 2, clearH, TFT_BLACK);

  tft.setCursor(x, y);
  tft.print(s);

  lastRSSI = signalStrength;
  lastRssiUpdate = now;
}

void displayTimeGreeting(int currentHour) {
	if (currentHour == lastGreetingHour &&
	    sunriseStr == lastSunriseStr &&
	    sunsetStr == lastSunsetStr &&
	    sunriseDiffStr == lastSunriseDiffStr &&
	    sunsetDiffStr == lastSunsetDiffStr &&
	    poraDniaStr == lastPoraDniaStr) {
	  return;
	}

	lastGreetingHour = currentHour;
	lastSunriseStr = sunriseStr;
	lastSunsetStr = sunsetStr;
	lastSunriseDiffStr = sunriseDiffStr;
	lastSunsetDiffStr = sunsetDiffStr;
	lastPoraDniaStr = poraDniaStr;

	tft.fillRect(0, 205, tft.width(), 64, TFT_BLACK);

	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.setTextSize(2);

	if (sunriseStr.length() > 0 && sunsetStr.length() > 0) {
		int sh = parseHourFromHHMM(sunriseStr);
		int sth = parseHourFromHHMM(sunsetStr);

		if (sh >= 0 && sth >= 0) {
			int nightLen = (sh - sth + 24) % 24;
			if (nightLen == 0) nightLen = 24;
			int elapsed = (currentHour - sth + 24) % 24;

			if (elapsed < nightLen) {
				// jest noc -> pokaz wschód
				tft.setCursor(6, 225);
				tft.print("Wschod: ");
				tft.print(sunriseStr);
				if (sunriseDiffStr.length()) {
					tft.print(" (za ");
					tft.print(sunriseDiffStr);
					tft.print(")");
				}
				return;
			} else {
				// jest dzień -> pokaz zachód
				tft.setCursor(6, 225);
				tft.print("Zachod: ");
				tft.print(sunsetStr);
				if (sunsetDiffStr.length()) {
					tft.print(" (za ");
					tft.print(sunsetDiffStr);
					tft.print(")");
				}
				return;
			}
		}
	}

	// fallback: brak dokładnych danych — spróbuj użyć poraDniaStr lub wyświetl brak danych
	if (poraDniaStr == "night") {
		tft.setCursor(6, 225);
		if (sunriseStr.length()) {
			tft.print("Wschod: ");
			tft.print(sunriseStr);
			if (sunriseDiffStr.length()) {
				tft.print(" (za ");
				tft.print(sunriseDiffStr);
				tft.print(")");
			}
		} else {
			tft.print("Wschod: brak danych");
		}
	} else if (poraDniaStr == "day") {
		tft.setCursor(6, 225);
		if (sunsetStr.length()) {
			tft.print("Zachod: ");
			tft.print(sunsetStr);
			if (sunsetDiffStr.length()) {
				tft.print(" (za ");
				tft.print(sunsetDiffStr);
				tft.print(")");
			}
		} else {
			tft.print("Zachod: brak danych");
		}
	} else {
		tft.setCursor(6, 208);
		tft.print("Brak danych slonecznych");
	}
}
time_t convertUtcToLocalTime(time_t utcTime) {
  struct tm timeinfo;
  gmtime_r(&utcTime, &timeinfo); // Konwersja na strukturę czasu UTC

  if (timeinfo.tm_isdst) {
    utcTime += 3600; // Dodanie godziny przy zmianie czasu letniego
  }

  return utcTime;
}
void displayWeatherData(float temperature, float humidity, float pressure, const char* sunrise, const char* sunset, const char* sunrise_diff, const char* sunset_diff, const char* pora_dnia) {
  tft.fillScreen(TFT_BLACK);

  // Temperatura 
  uint16_t temperatureColor = getTemperatureColor(temperature);
  tft.setTextSize(6);
  tft.setTextColor(temperatureColor, TFT_BLACK);
  tft.setCursor(0, 10);
  tft.print(temperature, 1);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("'C");

  // Wilgotność 
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(220, 24);
  tft.print((int)humidity);
  tft.setTextSize(2);
  tft.print("%");

  // Ciśnienie
  tft.setTextSize(4);
  tft.setCursor(0, 90);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(pressure, 1);
  tft.setTextSize(2);
  tft.print("hPa");

  // Dzień tygodnia i data 
  unsigned long epochTime = timeClient.getEpochTime();
  time_t localTime = convertUtcToLocalTime(epochTime);
  struct tm* ptm = localtime(&localTime);

  // linia 1: dzień tygodnia
  char dayLine[24];
  snprintf(dayLine, sizeof(dayLine), "%s", dayNames[ptm->tm_wday]);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 130);
  tft.print(dayLine);

  // linia 2: data
  char dateLine[32];
  snprintf(dateLine, sizeof(dateLine), "%d %s %d", ptm->tm_mday, monthNames[ptm->tm_mon], ptm->tm_year + 1900);
  tft.setCursor(0, 154);
  tft.print(dateLine);

}

void processDataFromServer(String data) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, data);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  if (!doc.containsKey("temperature") ||
      !doc.containsKey("humidity") ||
      !doc.containsKey("pressure") ||
      !doc.containsKey("sunrise") ||
      !doc.containsKey("sunset") ||
      !doc.containsKey("sunrise_diff") ||
      !doc.containsKey("sunset_diff") ||
      !doc.containsKey("pora_dnia")) {
    Serial.println("JSON structure mismatch, missing keys.");
    return;
  }

  float temperature = doc["temperature"].as<float>();
  float humidity = doc["humidity"].as<float>();
  float pressure = doc["pressure"].as<float>();

  const char* sunrise = doc["sunrise"].as<const char*>();
  const char* sunset = doc["sunset"].as<const char*>();
  const char* sunrise_diff = doc["sunrise_diff"].as<const char*>();
  const char* sunset_diff = doc["sunset_diff"].as<const char*>();
  const char* pora_dnia = doc["pora_dnia"].as<const char*>();

  // Zapisz godziny i dodatkowe napisy (jako stringi) — parse zrobimy przy obliczeniu jasności
  if (sunrise) sunriseStr = String(sunrise); else sunriseStr = "";
  if (sunset)  sunsetStr  = String(sunset);  else sunsetStr  = "";
  if (sunrise_diff) sunriseDiffStr = String(sunrise_diff); else sunriseDiffStr = "";
  if (sunset_diff)  sunsetDiffStr  = String(sunset_diff);  else sunsetDiffStr  = "";
  if (pora_dnia) poraDniaStr = String(pora_dnia); else poraDniaStr = "";
  haveSunTimes = (sunriseStr.length() > 0 && sunsetStr.length() > 0);

  displayWeatherData(temperature, humidity, pressure, sunrise, sunset, sunrise_diff, sunset_diff, pora_dnia);
}

// --- DODANE FUNKCJE DLA PODŚWIETLENIA ----------------------
int clamp(int v, int a, int b) { return v < a ? a : (v > b ? b : v); }

int parseHourFromHHMM(const String &s) {
  if (s.length() < 2) return -1;
  int h = -1, m = 0;
  if (sscanf(s.c_str(), "%d:%d", &h, &m) == 2) {
    if (h >= 0 && h < 24) return h;
  }
  return -1;
}

int calculateBacklightBrightness(int currentHour) {
  // Jeśli mamy czasy słońca, parsujemy je z stringów i obliczamy okres nocy.
  if (haveSunTimes) {
    int sunriseHour = parseHourFromHHMM(sunriseStr);
    int sunsetHour  = parseHourFromHHMM(sunsetStr);
    if (sunriseHour < 0 || sunsetHour < 0) {
      if (currentHour >= 18 || currentHour < 6) return brightnessMin;
      return brightnessMax;
    }

    int nightLen = (sunriseHour - sunsetHour + 24) % 24;
    if (nightLen == 0) nightLen = 24;
    int elapsed = (currentHour - sunsetHour + 24) % 24;

    // jeśli aktualna godzina poza nocą -> pełna jasność
    if (elapsed >= nightLen) return brightnessMax;

    int half = nightLen / 2;
    int range = brightnessMax - brightnessMin;
    int b;
    if (elapsed <= half) {
      // zmniejszanie jasności od zachodu do środka nocy
      b = brightnessMax - (range * elapsed) / max(1, half);
    } else {
      // zwiększanie jasności od środka nocy do wschodu
      b = brightnessMin + (range * (elapsed - half)) / max(1, nightLen - half);
    }
    return clamp(b, 0, 255);
  }

  // fallback: prosta reguła dnia/nocy jeśli brak czasów słońca
  if (currentHour >= 18 || currentHour < 6) return brightnessMin;
  return brightnessMax;
}

void updateBacklightIfNeeded() {
  int h = timeClient.getHours();
  if (h == lastBacklightUpdateHour) return;
  lastBacklightUpdateHour = h;

  int newB = calculateBacklightBrightness(h);
  if (newB != currentBrightness) {
    currentBrightness = newB;
    // Ustawienie jasności (0-255)
    analogWrite(backlightPin, currentBrightness);
    Serial.printf("Backlight updated: %d (hour %d)\n", currentBrightness, h);
  }
}

void updateWeatherData() {
  int currentMinute = timeClient.getMinutes();
  static int lastNtpMinute = -1;
  if (currentMinute != lastNtpMinute) {
    lastNtpMinute = currentMinute;

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpCode = http.POST("");

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        String payload = http.getString();
        Serial.println("Received weather data:");
        Serial.println(payload);

        processDataFromServer(payload);
      } else {
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}
void displayClock() {
  unsigned long epochTime = timeClient.getEpochTime();
  time_t localTime = convertUtcToLocalTime(epochTime);
  struct tm* ptm = localtime(&localTime);
  char timeText[10];
  snprintf(timeText, sizeof(timeText), "%02d:%02d", ptm->tm_hour, ptm->tm_min);
  String tstr = String(timeText);

  // rysuj tylko gdy czas zmienił się
  if (tstr != lastTimeDisplayed) {
    // wyczyść obszar czasu (dostosuj wymiary jeśli potrzeba)
    tft.fillRect(180, 60, 140, 64, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(4);
    tft.setCursor(200, 82);
    tft.print(timeText);
    lastTimeDisplayed = tstr;
  }

  // RSSI rysowane oddzielnie w throttlowanej funkcji
  displayWiFiSignalStrength(getWiFiSignalStrength());
}

void updateGmtOffset() {
  // Ustal obecny czas w formie epoch
  unsigned long epochTime = timeClient.getEpochTime();
  time_t localTime = convertUtcToLocalTime(epochTime);
  struct tm* ptm = localtime(&localTime);

  int currentMonth = ptm->tm_mon + 1; 
  int currentDay = ptm->tm_mday;
  int currentWeekday = ptm->tm_wday; 

  if ((currentMonth > 3 && currentMonth < 10) || 
      (currentMonth == 3 && currentDay - currentWeekday > 24) ||
      (currentMonth == 10 && currentDay - currentWeekday <= 24)) {
    gmtOffset_sec = 7200; 
  } else {
    gmtOffset_sec = 3600; 
  }
}

void setup() {
  Serial.begin(115200);
  // Konfiguracja podświetlenia: użycie analogWrite
  pinMode(backlightPin, OUTPUT);
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  analogWrite(backlightPin, 60);
  setupWiFiAndOTA(); 
  updateGmtOffset(); 
  timeClient.update(); 
  updateGmtOffset(); 
  timeClient.setTimeOffset(gmtOffset_sec);
  lastNtpUpdate = millis(); 
}

void refreshWeatherDataIfNeeded() {
  int currentMinute = timeClient.getMinutes();
  static int lastNtpMinute = -1;

  if (currentMinute != lastNtpMinute) {
    lastNtpMinute = currentMinute;
    updateWeatherData();
  }
}

void displayClockAndGreeting() {
  unsigned long now = millis();
  if (now - lastGuiUpdate < guiInterval) return; // throttle GUI
  lastGuiUpdate = now;

  int currentHour = timeClient.getHours();
  displayClock(); 
  displayTimeGreeting(currentHour); 
}
void updateTimeIfNeeded() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastNtpUpdate >= ntpUpdateInterval) {
    timeClient.update(); // Aktualizacja czasu przez NTPClient
    updateGmtOffset(); // Sprawdzenie czasu letniego, zimowego
     timeClient.setTimeOffset(gmtOffset_sec);
    lastNtpUpdate = currentMillis; // Zaktualizuj czas ostatniej aktualizacji
  }
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
     updateTimeIfNeeded(); // Aktualizuj czas jeśli minęły 24 godziny
     refreshWeatherDataIfNeeded(); // Odświeżanie danych pogodowych
     displayClockAndGreeting(); // Wyświetlanie zegara i pozdrowień
    updateBacklightIfNeeded(); // Aktualizuj podświetlenie raz na godzinę
      }
}

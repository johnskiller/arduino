#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <TimeClient.h>

#define PIN D7
long lastUpdate = millis();
long lastSecond = millis();
int ledsInString = 12;
String hours, minutes, seconds;
int currentSecond, currentMinute, currentHour;

char ssid[] = "Skyroot";  //  your network SSID (name)
char pass[] = "ironhex9";       // your network password

const float UTC_OFFSET = 8;
TimeClient timeClient(UTC_OFFSET);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ledsInString, PIN);

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  strip.begin();
  strip.setBrightness(128);
  strip.show();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.updateTime();
  updateTime() ;
  lastUpdate = millis();
  lastSecond = millis();
}

void loop()
{
  if ((millis() - lastUpdate) > 1800000) updateTime();

  if ((millis() - lastSecond) > 1000)
  {
    int s = currentSecond * ledsInString /  60 ;
    int m = currentMinute * ledsInString /  60 ;
    int h = currentHour *  ledsInString / 12;
    if (h == 12) h = 0;
    strip.setPixelColor(s, strip.Color(0, 0, 0) );
    strip.setPixelColor(m, strip.Color(0, 0, 0) );
    strip.setPixelColor(h, strip.Color(0, 0, 0) );
    strip.show();
    lastSecond = millis();
    currentSecond++;
    if (currentSecond > 59)
    { currentSecond = 0;
      currentMinute++;
      if (currentMinute > 59) {
        currentMinute = 0;
        currentHour++;
        if (currentHour >= 12) currentHour = 0;
      }
    }
    String currentTime = String(currentHour) + ':' + String(currentMinute) + ':' + String(currentSecond);
    Serial.println(currentTime);

    s = currentSecond * ledsInString /  60 ;
    m = currentMinute * ledsInString /  60 ;
    h = currentHour *  ledsInString / 12;
    Serial.println(String(h) + "-" + String(m) + "-" + String(s) + "  : " + String(currentSecond % 5 * 50 + 50));


    strip.setPixelColor(h, strip.Color(64, 0, 0));
    if (h == m) {
      strip.setPixelColor(m, strip.Color(64, 64, 0));

    } else {
      strip.setPixelColor(m, strip.Color(0, 64, 0));
    }
    int g = (h == s) ? 64 : 0;
    int r = (m == s) ? 64 : 0;
    int b = currentSecond % 5 * 50 + 30;
    strip.setPixelColor(s, strip.Color(g, r, b));
  }

  strip.show();
}


void updateTime()
{
  hours = timeClient.getHours();
  minutes = timeClient.getMinutes();
  seconds = timeClient.getSeconds();
  currentHour = hours.toInt();
  if (currentHour > 12) currentHour = currentHour - 12;
  currentMinute = minutes.toInt();
  currentSecond = seconds.toInt();
  lastUpdate = millis();
}

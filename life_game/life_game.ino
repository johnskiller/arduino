/*****************************************************************
 *****************************************************************

  CONWAY'S GAME OF LIFE V1

  DATE : 19 JAN 2018

  CONCEPTOR : AERODYNAMICS

  This program intends to run a CONWAYS'S Game of Life on an Arduino
  and to display it on a 16x32 LED MATRIX


* *****************************************************************
*******************************************************************/

//////////////////////////
//      LIBRARIES       //
//////////////////////////

// mqtt
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_GFX.h>   // Core graphics library
#include <Wire.h>
#include <Adafruit_SSD1306.h>
//#include <RGBmatrixPanel.h> // Hardware-specific library
//#include <EEPROM.h> // To store on EEPROM Memory

#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//////////////////////////
//       VARIABLES      //
//////////////////////////

//Definition of the pattern if you use the pattern initialization
int pattern_size[] = {7, 22}; // row x Column
char pattern_init[] =
  ".........*,\
.......*.*,\
......*.*,\
**...*..*...........**,\
**....*.*...........**,\
.......*.*,\
.........*!";

#define L_W  64
#define L_H  32
Adafruit_AM2320 am2320 = Adafruit_AM2320();
bool WORLD[L_W][L_H]; // Creation of the wordl
bool temp[L_W][L_H];
int step_GOL; //used to know the generation
int game = 0;


const char* ssid = "Skyroot";
const char* password =  "ironhex9";
const char* mqttServer = "m16.cloudmqtt.com";
const int mqttPort = 18278;
const char* mqttUser = "hnakzoke";
const char* mqttPassword = "-zUur2fr2ADL";

WiFiClient espClient;
PubSubClient client(espClient);

//////////////////////////
//       OBJECTS        //
//////////////////////////



/*************************************************************************************************************/
//////////////////////////
//          SETUP       //
//////////////////////////
void setup() {
  Serial.begin(115200); //use to print the game on the serial monitor (to debug)

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
  

client.setServer(mqttServer, mqttPort);
client.setCallback(callback);

while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
      client.publish("esp/test", "Hello from ESP8266");
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
}

// set pull up r for sda and scl for am2320
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

  am2320.begin();
  //Randomly initialazing the world for the first step
  /*
    randomSeed(millis());
    for (byte i = 0; i < L_W; i++) {
    for (byte j = 0; j < L_H; j++) {
      WORLD[i][j] = random(0, 2);
    }
    }
  */

  //init_WORLD(); // Uncomment if you want to init with a specific pattern

  step_GOL = 0;
  //matrix.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  print_WORLD(); //Display the first generation
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text

  display.display();
}


bool newGen = 1;
int fps = 0;
const int interval = 2000;
unsigned long last = 0;
float tm = 0;
float hum = 0;
/*************************************************************************************************************/
//////////////////////////
//    LOOP FUNCTION     //
//////////////////////////
void loop() {
  unsigned long currentMillis = millis();
  if (newGen == 1 || step_GOL == 600) { // This if reboot the world after 60 generation to avoid static world
    game ++;
    newGen = 0;
    step_GOL = 0;
    //matrix.fillScreen(0);
    display.clearDisplay();
    delay(500);
    randomSeed(currentMillis);
    for (byte i = 0; i < L_W; i++) {
      for (byte j = 0; j < L_H; j++) {
        WORLD[i][j] = random(0, 2);
      }
    }
  }

  if (currentMillis - last > interval ) {
    tm = am2320.readTemperature();
    hum = am2320.readHumidity();
    if( isnan(tm) ) {
      am2320.begin();
    }
    last = currentMillis;
    char charBuf[20];
    client.publish("esp/temp", dtostrf(tm, 3, 1, charBuf));
 

    client.publish("esp/hum", dtostrf(hum,3,1,charBuf));
  }
  //This double "for" is used to update the world to the next generation
  //The buffer state is written on the EEPROM Memory

  int changes = 0;
  for (byte i = 0; i < L_W; i++) {
    for (byte j = 0; j < L_H; j++) {

      byte num_alive = worldAt(i - 1, j - 1) + worldAt(i - 1, j) + worldAt(i - 1, j + 1) + worldAt(i, j - 1) + worldAt(i, j + 1)
                       + worldAt(i + 1, j - 1) + worldAt(i + 1, j) + worldAt(i + 1, j + 1);
      bool state = worldAt(i, j);

      //RULE#1 if you are surrounded by 3 cells --> you live
      if (num_alive == 3) {
        //EEPROM.write(i * 31 + j , 1);
        temp[i][j] = 1;
        changes ++;
      }
      //RULE#2 if you are surrounded by 2 cells --> you stay in your state
      else if (num_alive == 2) {
        //EEPROM.write(i * 31 + j , state);
        temp[i][j] = state;
      }
      //RULE#3 otherwise you die from overpopulation or subpopulation
      else {
        //EEPROM.write(i * 31 + j , 0);
        temp[i][j] = 0;
        changes ++;
      }

    }
  }
  if ( changes == 0 ) { // nothing changed
    newGen = 1;
  }

  //Updating the World
  for (byte i = 0; i < L_W; i++) {
    for (byte j = 0; j < L_H; j++) {
      WORLD[i][j] = temp[i][j]; // EEPROM.read(i * 31 + j);
    }
  }

  //Displaying the world
  print_WORLD();

  //Increasing the generation
  step_GOL++;


  display.setCursor(0, 0);            // Start at top-left corner
  display.print(F("FPS: "));
  display.print(fps);
  display.print(" gen: ");
  display.println(step_GOL);
  display.print(" game: ");
  display.println(game);


  display.setCursor(0, 55);
  display.print(F("T: "));
  display.print(tm);

  display.print(F("C  H: "));
  display.print(hum);
  display.print("%");
  display.display();

  delay(1);
  unsigned long t = millis() - currentMillis;
  fps = 1000 / (t);
  
}

inline bool worldAt(int x, int y) {
  if (x < 0) x = L_W;
  if (x >= L_W) x = 0;
  if (y < 0) y = L_H;
  if (y >= L_H) y = 0;
  return WORLD[x][y];
}
/*************************************************************************************************************/
//////////////////////////
//       FUNCTIONS      //
//////////////////////////


// PRINT THE WORLD
void print_WORLD()
{
  display.clearDisplay();
  for (byte j = 0; j < L_H; j++) {
    for (byte i = 0; i < L_W; i++) {
      if (WORLD[i][j] == 0) {
        //matrix.drawPixel(j, i, matrix.Color333(0, 0, 0));
        //display.drawLine(0, 0, display.width()-1, i, WHITE);
      }
      else
      {
        //matrix.drawPixel(j, i, matrix.Color333(0, 1, 2));
        display.drawLine(i * 2, j * 2, i * 2 + 1, j * 2, WHITE);
        display.drawLine(i * 2, j * 2 + 1, i * 2 + 1, j * 2 + 1, WHITE);
      }
    }
  }
  //display.display();
}

//Those two function are used to display the world on the serial monitor
//Not beautiful but useful to debug

void print_WORLD_SERIAL()
{
  clearscreen();
  Serial.print("Step = "); Serial.println(step_GOL);
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 32; j++) {
      if (WORLD[i][j] == 0) {
        Serial.print(".");
        Serial.print(" ");
      }
      else
      {
        Serial.print("*");
        Serial.print(" ");
      }
    }
    Serial.println("");
  }
  Serial.println("");

}

void clearscreen() {
  for (int i = 0; i < 10; i++) {
    Serial.println("\n\n\n\n");
  }
}

//This function is used to init the world with a know pattern
//It read . and * to convert them to 0 and 1.
//Inspired from life 1.05 format
// NB : this function needs improvment to center the pattern

void init_WORLD() {
  int k = 0;
  int row = 0;
  int column = 0;
  while (pattern_init[k] != '!') {
    if (pattern_init[k] == ',') {
      row++;
      k++;
      column = 0;
    }
    else if (pattern_init[k] == '.') {
      WORLD[row + 2][column + 4] = 0;
      k++;
      column ++;
    }
    else  {
      WORLD[row + 2][column + 4] = 1;
      k++;
      column ++;
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}

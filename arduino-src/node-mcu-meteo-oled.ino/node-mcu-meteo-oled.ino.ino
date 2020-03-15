#define _DEBUG_
#define SERIAL_BAUD 115200
//Librarie for NodeMCu Esp8266
#include <ThingerESP8266.h>
#include <Adafruit_BME280.h>
//Library for Oled Screen
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//LED Pins
#define SCREEN 2  // D4 : LED PIN
#define LED 14    // D5 : LED PIN
#define ROUGE 12  // D6 : RED PIN
#define VERT 13   // D7 : GREEN PIN
#define BLEU 15   // D8 : BLUE PIN

// Constantes for Screen OLED
static const unsigned char PROGMEM logo_bmp[] =
{ 
  B11111111, B11111111,
  B11111111, B11111111,
  B10000000, B00000001,
  B11000000, B00000011,
  B11100000, B00000111,
  B11110000, B00001111,
  B11111000, B00011111,
  B11111100, B00111111,
  B11111110, B01111111,
  B11111100, B00111111,
  B11111000, B00011111,
  B11110000, B00001111,
  B11100000, B00000111,
  B11000000, B00000011,
  B10000000, B00000001,
  B11111111, B11111111,
  
};
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
int INIT_TEMP  =   0;
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixel
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     LED_BUILTIN // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Reference Pressure
#define SEALEVELPRESSURE_HPA (1013.25)
//Wifi Login / Password
#define WIFI_SSID "Bbox-B9A24374"
#define WIFI_PWD "5FD555E559ED152F46CE7729674C4C"
// Somme variable for mesures Temp, Pressure, Humidity,..
float temperature, humidity, pressure;
// reseting boolean to command reboot
boolean reseting = false;
// Instance for BME sensors 
Adafruit_BME280 bme;
//Instnce of ESP
ThingerESP8266  meteoStation("turboxav", "meteostation", "turboxav");

/**
 * SetUp
 */

void setup() {  
  Serial.begin(SERIAL_BAUD);
  pinMode(SCREEN, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(ROUGE, OUTPUT);
  pinMode(VERT, OUTPUT);
  pinMode(BLEU, OUTPUT);
  initMeteoStation();
  digitalWrite(LED,HIGH);
  digitalWrite(SCREEN, HIGH);
  bme.begin(0x76);
  initScreen();
  delay(500);  
}

/**
 * Initialize meteo Station
 *  Wifi connection
 *    all ressources to thinger.io
 *      - Led : Boolean -> To command the LED 
 *      - Led State : ON / OFF -> To check led state
 *      - Meteo : {
 *        - temperature
 *        - pressure
 *        - humidity
 *       }
 *      
 */

void initMeteoStation() {
  // Connecte to Wifi
  meteoStation.add_wifi(WIFI_SSID, WIFI_PWD);
  // Record an INPUT value for Led Pin Value to command IT
  meteoStation["led"] << digitalPin(LED);
  meteoStation["screen"] << digitalPin(SCREEN);
   meteoStation["screen-state"] >> [](pson& out) { 
    out["state"] =  digitalRead(SCREEN) ? "ON":"OFF";
  };
  // Reccord an OUTPUT for Led State to read it
  meteoStation["led-state"] >> [](pson& out) { 
    out["state"] =  digitalRead(LED) ? "ON":"OFF";
  };
  // record OUTPUT for meteo reading
  meteoStation["meteo"] >> [](pson& out){
    out["temperature"] = temperature;
    out["pressure"] = pressure;
    out["humidity"] = humidity;
  };

 //
 meteoStation["reseting"] = [](){
    reseting = true;
};
}

/**
 * Initialisze the screen
 *  Animation
 *  Display
 */

void initScreen() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!screen.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  screen.display();
  delay(500); 
  screen.clearDisplay();
}

/**
 * Loop
 */

void loop() {  
  mesure();  
  displayMesures();
  if(reseting == true) {
    meteoStation.handle();
    ESP.reset();
  }
  delay(10);
}

/**
  * Mesure  
  *   Temprature °C
  *   Pressure hPa
  *   Humidity %
  */

void mesure(){
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  meteoStation.handle();  
}

/**
 * Display mesures
 *  Display on Oled :
 *   - Temprature °C
 *   - Pressure hPa
 *   - Humidity %
 *  RGB Led color
 *  Cold    : Temperature < 19 °C => Blue
 *  Normal  : Temperature between  19 & 22.5 => Green 
 *  Hot     : Temperature > 22.5 => Red
 *  
 */

void displayMesures() {

  if(digitalRead(SCREEN) == LOW){
    screen.ssd1306_command(SSD1306_DISPLAYOFF); 
    ledRgb(0, 0, 0);
    return;
  }else{
    screen.ssd1306_command(SSD1306_DISPLAYON);
  }
  
  if(temperature < 19){
     ledRgb(0, 0, 255 );
  } else if (temperature >= 19 && temperature <= 22.5)  {
     ledRgb(0, 255, 0 );
  } else if (temperature > 22.5){
     ledRgb(255, 0, 0 );
  }
  
  screen.clearDisplay();
  
  screen.setTextSize(2);      // Normal 1:1 pixel scale
  screen.setTextColor(WHITE); // Draw white text
  screen.invertDisplay(true);
  screen.setCursor(0, 0);     // Start at top-left corner
  
  screen.println("Meteo");
  screen.setTextSize(1); 
  //screen.invertDisplay(false);
  screen.print("Temperature: ");
  screen.print(temperature);
  screen.println(" C");
 
  screen.print("Humidite: ");
  screen.print(humidity);
  screen.println(" %");
 
  screen.print("Pression: ");
  screen.print(pressure);
  screen.println(" hPa");
  screen.display();
  delay(10);
  
}

/**
 * Display a color on RGB led
 */

void ledRgb(int rouge, int vert,int bleu ){
  if(digitalRead(LED) == HIGH){
    analogWrite(ROUGE,rouge);
    analogWrite(VERT,vert);
    analogWrite(BLEU,bleu);
  }else{
    analogWrite(ROUGE,0);
    analogWrite(VERT,0);
    analogWrite(BLEU,0);
  }
}

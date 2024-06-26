#include "SPI.h"
#include <DS3231.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

/*
TFT Wires
DC (7 due 2 wemos)
RST RST
CS MISO (8 Due 15 wemos)
SDA MOSI
SCL SCK
*/

// TODO Mérjünk csak egyszer és ne frissítsünk? Kíméljük?

//RTC INIT
DS3231 myRTC;
bool century = false;
bool h12Flag;
bool pmFlag;

// BME680 INIT
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME680 bme; 

// TFT init (TFT_CS,TFT_DC)
Adafruit_GC9A01A tft(15, 2);

//Clock data
char daysOfTheWeek[8][12] = {"E", "H", "K", "SZE", "CS", "P", "SZO", "V"};
unsigned long clockPreviousMillis = 0;
const long clockInterval = 1000;
int seconds = 0;
int minutes = 0;
int hours = 0;
int year = 0;
int month = 0;
int day = 0;
int week = 0;

//round robin
unsigned long currentMillis;
unsigned long startMillis;
unsigned long startMillis2;
const unsigned long period = 60000;
const unsigned long period2 = 5000;
int currentScrean = 0;
int robin = 0;

void setup() {
  delay(100);
  Serial.begin(9600);

  delay(100);
  Wire.begin();

  delay(100);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(GC9A01A_BLACK);

  delay(100);
  bme.begin();
  delay(100);
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  //LED INIT
  Serial.begin(9600);
  while (!Serial);
  Serial.println("---START---");

  startMillis = millis();
  startMillis2 = millis();
  currentScrean = 1;
}

void loop() {
  currentMillis = millis();

  if(millis() % 1000 == 10 ) {
    
    switch (currentScrean) {
      case 1:
        screen01();
        break;
      case 2:
        screen02();
        break;
      case 3:
        screen03();
        break;
      case 4:
        screen04();
        break;
      case 5:
        screen05();
        break;
      case 6:
        screen06();
        break;
      default:
        error();
        break;
    }
    roundRobin();
  }
  //timer();
}

void roundRobin(){
  robin++;
  if(robin==1) screen01_Init();
  if(robin<20) currentScrean = 1;
  if(robin==20) screen02_Init();
  if(robin>=20 and robin<30) currentScrean = 2;
  if(robin==30) screen03_Init();
  if(robin>=30 and robin<40) currentScrean = 3;
  if(robin==40) screen04_Init();
  if(robin>=40 and robin<50) currentScrean = 4;
  if(robin==50) screen05_Init();
  if(robin>=50 and robin<60) currentScrean = 5;
  if(robin==60) screen06_Init();
  if(robin>=60 and robin<70) currentScrean = 6;
  if(robin>=70) {
    robin = 0;
    serialLog();
  }
  //Serial.println(robin);
}

void timer() {
  /*if (currentMillis - startMillis >= period) {
    process01();
    startMillis = currentMillis;
  }
  if (currentMillis - startMillis2 >= period2) {
    dataLogger();
    startMillis2 = currentMillis;
  }*/
}

void error(){
  tft.fillScreen(GC9A01A_BLACK);
  drawCenteredText("--HIBA--");
}

void screen01_Init(){
  tft.fillScreen(GC9A01A_BLACK);
  drawClockFace(GC9A01A_WHITE);
}

void screen01() {
  unsigned long currentMillis = millis();
  if (currentMillis - clockPreviousMillis >= clockInterval) {
    drawClock(hours, minutes, seconds, GC9A01A_BLACK, GC9A01A_BLACK);
    clockPreviousMillis = currentMillis;
    hours = myRTC.getHour(h12Flag, pmFlag);
    minutes = myRTC.getMinute();
    seconds = myRTC.getSecond();
    drawClock(hours, minutes, seconds, GC9A01A_WHITE, GC9A01A_RED);
  }
}

void screen02_Init(){
  tft.fillScreen(GC9A01A_BLACK);
  drawClockFace(GC9A01A_WHITE);
  String dateString = "";
  dateString = dateString + "20"+myRTC.getYear()+"."+myRTC.getMonth(century)+"."+myRTC.getDate()+" "+daysOfTheWeek[myRTC.getDoW()];
  drawCenteredText(dateString.c_str());
}

void screen02(){
  delay(900);
}

void screen03_Init(){
  tft.fillScreen(GC9A01A_BLACK);
  bme.performReading();
  float temperature = bme.temperature;
  float values[] = {temperature, 100.0-temperature};
  int numValues = sizeof(values) / sizeof(values[0]);
  drawPieChart(values,numValues, true, true, GC9A01A_RED);
  drawCenteredText("C");
}

void screen03(){
  delay(900);
}

void screen04_Init(){
  tft.fillScreen(GC9A01A_BLACK);
  bme.performReading();
  float humidity = bme.humidity;
  float values[] = {humidity, 100.0-humidity};
  int numValues = sizeof(values) / sizeof(values[0]);
  drawPieChart(values,numValues, true, true, GC9A01A_BLUE);
  drawCenteredText("%");
}

void screen04(){
  delay(900);
}

void screen05_Init(){
  tft.fillScreen(GC9A01A_BLACK);
  bme.performReading();
  float pressure = bme.pressure / 100.0;
  float values[] = {pressure, 2000.0-pressure};
  int numValues = sizeof(values) / sizeof(values[0]);
  drawPieChart(values,numValues, true, true, GC9A01A_GREEN);
  drawCenteredText("hPa");
}

void screen05(){
  delay(900);
}

void screen06_Init(){
  tft.fillScreen(GC9A01A_BLACK);
  bme.performReading();
  float gasResistance = bme.gas_resistance / 1000.0;
  float values[] = {gasResistance, 100.0-gasResistance};
  int numValues = sizeof(values) / sizeof(values[0]);
  drawPieChart(values,numValues, true, true, GC9A01A_YELLOW);
  drawCenteredText("Qual");
}

void screen06(){
  delay(900);
}

void drawClockFace(uint16_t colorMain){
  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;
  int radius = min(centerX, centerY) - 2;

  // Draw clock face
  tft.drawCircle(centerX, centerY, radius, colorMain);

  // Draw hour marks and numbers
  for (int i = 0; i < 12; i++) {
    float angle = (i * 30 - 90); // Elforgatva 90 fokkal balra
    float radian = angle * PI / 180;
    int x1 = centerX + (radius - 10) * cos(radian);
    int y1 = centerY + (radius - 10) * sin(radian);
    int x2 = centerX + radius * cos(radian);
    int y2 = centerY + radius * sin(radian);
    tft.drawLine(x1, y1, x2, y2, colorMain);

    // Számok rajzolása
    int numX = centerX + (radius - 20) * cos(radian);
    int numY = centerY + (radius - 20) * sin(radian);
    tft.setCursor(numX - 5, numY - 5);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(1);
    int hourNum = i == 0 ? 12 : i;
    tft.print(hourNum);
  }
}

void drawClock(int hour, int minute, int second, uint16_t colorMain, uint16_t colorSecondary) {

  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;
  int radius = min(centerX, centerY) - 35;

  // Draw hour hand
  float hourAngle = ((hour % 12) + minute / 60.0) * 30 - 90;
  float hourRadian = hourAngle * PI / 180;
  int hourX = centerX + (radius - 20) * cos(hourRadian);
  int hourY = centerY + (radius - 20) * sin(hourRadian);
  tft.drawLine(centerX, centerY, hourX, hourY, colorMain);

  // Draw minute hand
  float minuteAngle = (minute + second / 60.0) * 6 - 90;
  float minuteRadian = minuteAngle * PI / 180;
  int minuteX = centerX + (radius - 5) * cos(minuteRadian);
  int minuteY = centerY + (radius - 5) * sin(minuteRadian);
  tft.drawLine(centerX, centerY, minuteX, minuteY, colorMain);

  // Draw second hand
  float secondAngle = second * 6 - 90;
  float secondRadian = secondAngle * PI / 180;
  int secondX = centerX + radius * cos(secondRadian);
  int secondY = centerY + radius * sin(secondRadian);
  tft.drawLine(centerX, centerY, secondX, secondY, colorSecondary);
}

void drawPieChart(float values[], int numValues, bool fill, bool firstBig, uint16_t color) {
  drawScale();
  // Kördiagram pozíciója és mérete
  int cx = tft.width() / 2;    // Középpont x koordináta
  int cy = tft.height() / 2;   // Középpont y koordináta
  float radius = (min(tft.width(), tft.height()) / 2)-10;
  float radiusSmaller = radius/3;

  // Színek a kördiagram szeleteihez
  uint16_t colors[] = {GC9A01A_RED, GC9A01A_DARKGREY, GC9A01A_BLUE, GC9A01A_YELLOW};
  colors[0] = color;

  // Összérték kiszámítása
  float total = 0;
  for (int i = 0; i < numValues; i++) {
    total += values[i];
  }

  // Kezdő szög
  float startAngle = 0;

  // Kördiagram szeleteinek rajzolása és számok kiírása
  for (int i = 0; i < numValues; i++) {
    float endAngle = startAngle + 360.0 * values[i] / total;
    
    if(i>0 && firstBig) radius = radiusSmaller;

    if(fill){
      // Szelet kirajzolása
      fillArc(cx, cy, radius, startAngle, endAngle, colors[i]);
      //drawOutlineArc(cx, cy, radius, startAngle, endAngle, colors[i]);
      drawOutlineArc(cx, cy, radius, startAngle, endAngle, GC9A01A_WHITE);
    }

    // Szelet közepére szám kiírása
    if(i==0 || !firstBig){
      float midAngle = (startAngle + endAngle) / 2.0;
      int labelX = cx + cos(radians(midAngle)) * ((radius*0.8) / 2); // Középpont x koordináta
      int labelY = cy + sin(radians(midAngle)) * ((radius*0.8) / 2); // Középpont y koordináta
      drawCenteredText(values[i], labelX, labelY);
    }

    startAngle = endAngle;
  }


  /*startAngle = 0;
  for (int i = 0; i < numValues; i++) {
    float endAngle = startAngle + 360.0 * values[i] / total;
    
    if(i>0 && firstBig) radius = radiusSmaller;



    startAngle = endAngle;
  }*/
  drawScale();
}

// Arc kitöltése Adafruit GFX könyvtárral
void fillArc(int16_t x, int16_t y, int16_t radius, float startAngle, float endAngle, uint16_t color) {
  startAngle = radians(startAngle);
  endAngle = radians(endAngle);

  for (int16_t r = 0; r <= radius; r++) {
    for (float angle = startAngle; angle <= endAngle; angle += 0.05) {
      int16_t x0 = x - cos(angle) * r; // Vízszintes tükrözés
      int16_t y0 = y - sin(angle) * r; // Függőleges tükrözés
      tft.drawPixel(x0, y0, color);
    }
  }
}

void drawOutlineArc(int16_t x, int16_t y, int16_t radius, float startAngle, float endAngle, uint16_t color) {
  startAngle = radians(startAngle);
  endAngle = radians(endAngle);

  // Körvonalak rajzolása
  for (float angle = startAngle; angle <= endAngle; angle += 0.005) {
    int16_t x0 = x - cos(angle) * radius;
    int16_t y0 = y - sin(angle) * radius;
    tft.drawPixel(x0, y0, color);
  }

  // Körcikk két szélső vonalainak rajzolása
  int16_t x1 = x + cos(startAngle) * radius; // Kezdő pont
  int16_t y1 = y + sin(startAngle) * radius;
  int16_t x2 = x + cos(endAngle) * radius;   // Végső pont
  int16_t y2 = y + sin(endAngle) * radius;
  tft.drawLine(tft.width() - x, tft.height() - y, tft.width() - x1, tft.height() - y1, color);
  tft.drawLine(tft.width() - x, tft.height() - y, tft.width() - x2, tft.height() - y2, color);
}

void drawScale() {
  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;
  int radius = min(centerX, centerY) - 10; // Kisebb legyen, mint a kijelző szélessége

  tft.drawCircle(centerX, centerY, radius, GC9A01A_WHITE);

  for (int i = 0; i <= 10; i++) {
    float angle = map(i, 0, 10, 180, -180);
    float radian = angle * PI / 180;
    
    int x1 = centerX + radius * cos(radian);
    int y1 = centerY + radius * sin(radian);
    int x2 = centerX + (radius - 10) * cos(radian);
    int y2 = centerY + (radius - 10) * sin(radian);

    tft.drawLine(x1, y1, x2, y2, GC9A01A_WHITE);

    // Számok rajzolása
    int number = (10 - i) * 10;
    if (number == 100) continue; // A 100-at nem írjuk ki

    int numX = centerX + (radius - 20) * cos(radian);
    int numY = centerY + (radius - 20) * sin(radian);

    tft.setCursor(numX - 5, numY - 5);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(1);
    tft.print(number);
  }
}

// Szöveg kiírása középre
void drawCenteredText(float value, int x, int y) {
  tft.setTextColor(GC9A01A_WHITE); // Szöveg színe fekete
  tft.setTextSize(2); // Szöveg mérete 2x
  
  // Vízszintesen tükrözés
  x = tft.width() - x; // A TFT kijelző szélességéből kivonjuk az x koordinátát

  // Függőleges tükrözés
  y = tft.height() - y;

  String text(value);

  int16_t x1, y1;
  uint16_t w, h;
  // Szöveg méretének kiszámítása
  tft.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &w, &h);

  drawTextBg(text.c_str(), x-w/2, y-h/2,GC9A01A_DARKGREY);

  tft.setCursor(x - w/2, y - h/2); // Szöveg kiírásának helye
  tft.print(text); // Szöveg kiírása
}

void drawTextBg(const char* text, int x, int y, uint16_t color){
  // Szöveg méretének kiszámítása
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  // Téglalap koordináták és méretek
  int rectX = x - 4; // 1px margó a bal oldalon
  int rectY = y - 4; // 1px margó felül
  int rectW = w + 6; // 2px margó a két oldalon
  int rectH = h + 6; // 2px margó felül és alul

  // Szürke téglalap rajzolása
  tft.fillRect(rectX, rectY, rectW, rectH, color);
}

void drawCenteredText(const char* text) {
 int16_t x1, y1;
  uint16_t w, h;
  tft.setTextSize(2);
  tft.setTextColor(GC9A01A_WHITE);

  // Szöveg méretének kiszámítása
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  // Szöveg középre helyezése
  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;
  int x = centerX - w / 2;
  int y = centerY - h / 2;

  drawTextBg(text, x, y,GC9A01A_DARKGREY);

  // Szöveg kiírása
  tft.setCursor(x, y);
  tft.print(text);
}

void serialLog(){
    float temperature = bme.temperature;
    float humidity = bme.humidity;
    float pressure = bme.pressure / 100.0; // Convert to hPa
    float gasResistance = bme.gas_resistance / 1000.0; // Convert to kΩ
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

    // Eredmények kiírása a soros monitorra
    Serial.print(F("Hőmérséklet: "));
    Serial.print(temperature);
    Serial.println(F(" °C"));

    Serial.print(F("Páratartalom: "));
    Serial.print(humidity);
    Serial.println(F(" %"));

    Serial.print(F("Nyomás: "));
    Serial.print(pressure);
    Serial.println(F(" hPa"));

    Serial.print("Magasság: ");
    Serial.print(altitude);
    Serial.println(" m");

    Serial.print(F("Gázellenállás: "));
    Serial.print(gasResistance);
    Serial.println(F(" kOhm"));

    Serial.print("RTC T: ");
    Serial.print(myRTC.getTemperature(), 2);

    Serial.println(F("---"));
}
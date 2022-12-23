#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define ENCODER_BTN     4
#define ENCODER_CLK    15
#define ENCODER_DT      2

#define OLED_RESET     -1
Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool selectMenu = true;
int menuCounter = 0;
int delayValue    = 0;
int longValue     = 0;
int intervalValue = 0;
int countValue    = 0;
void setup() {
  Serial.begin(9600);

  // Rotary Encoder
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_BTN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoder, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_BTN), releaseEncoder, FALLING);
  Serial.println("[ROTA]: Attached interrupt");

  // OLED
  if (!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[OLED]: Allocation failed");
  } else {
    Serial.println("[OLED]: Initialized");
  }
}

void readEncoder() {
  int dtValue = digitalRead(ENCODER_DT);

  // Clockwise
  if (dtValue == HIGH) {
    if (selectMenu) {
      menuCounter++;
      menuCounter = (menuCounter + 5) % 5;
    }
    else {
      if (menuCounter == 0) delayValue++;
      if (menuCounter == 1) longValue++;
      if (menuCounter == 2) intervalValue++;
      if (menuCounter == 3) countValue++;
    }
  }

  // Counter Clockwise
  if (dtValue == LOW) {
    if (selectMenu) {
      menuCounter--;
      menuCounter = (menuCounter + 5) % 5;
    }
    else {
      if (menuCounter == 0) delayValue--;
      if (menuCounter == 1) longValue--;
      if (menuCounter == 2) intervalValue--;
      if (menuCounter == 3) countValue--;
    }
  }

  return;
}

void releaseEncoder() {
  selectMenu = !selectMenu;
  return;
}

void drawMenu(int nums) {
  if (nums == 4) return;
  int offset = SCREEN_WIDTH / 8;

  if (nums == 0)  OLED.setTextColor(BLACK, WHITE);
  drawText("DLY", offset, 0, "N");
  if (nums == 0)  OLED.setTextColor(WHITE, BLACK);

  if (nums == 1)  OLED.setTextColor(BLACK, WHITE);
  drawText("LNG", offset + (SCREEN_WIDTH / 4), 0, "N");
  if (nums == 1)  OLED.setTextColor(WHITE, BLACK);

  if (nums == 2)  OLED.setTextColor(BLACK, WHITE);
  drawText("INTVL", offset + (SCREEN_WIDTH / 4 * 2), 0, "N");
  if (nums == 2)  OLED.setTextColor(WHITE, BLACK);

  if (nums == 3)  OLED.setTextColor(BLACK, WHITE);
  drawText("N", offset + (SCREEN_WIDTH / 4 * 3), 0, "N");
  if (nums == 3)  OLED.setTextColor(WHITE, BLACK);
  return;
}

void loop() {
  OLED.clearDisplay();

  // menuCounter = (menuCounter + 4) % 4;

  drawMenu(menuCounter);
  Serial.print(delayValue);
  Serial.print(" ");
  Serial.print(longValue);
  Serial.print(" ");
  Serial.print(intervalValue);
  Serial.print(" ");
  Serial.println(countValue);

  Serial.println(digitalRead(ENCODER_BTN));

  OLED.display();
  delay(500);
}

void drawText(String text, int x, int y, String anchor) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;
  OLED.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  if (anchor == "CENTER") {
    x -= (width / 2);
    y -= (height / 2);
  } else if (anchor == "N") {
    x -= (width / 2);
  }

  OLED.setCursor(x, y);
  OLED.print(text);
}

#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ENCODER_BTN 4
#define ENCODER_CLK 15
#define ENCODER_DT 5

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long lastFrameTime;
bool selectMenu = true;
int menuCounter = 0;

int delayValue = 0;
int longValue = 0;
int intervalValue = 0;
int countValue = 0;

int encoderState = 0;
const unsigned char lookupTable[7][4] = {
    {0, 2, 4, 0},
    {3, 0, 1, 0 | 16},
    {3, 2, 0, 0},
    {3, 2, 1, 0},
    {6, 0, 4, 0},
    {6, 5, 0, 0 | 32},
    {6, 5, 4, 0},
};
void setup()
{
  Serial.begin(9600);

  // Rotary Encoder
  pinMode(ENCODER_BTN, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_BTN), releaseEncoder, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), readEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoder, CHANGE);
  Serial.println("[ROTA]: Attached interrupt");

  if (!OLED.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
}

void loop()
{
  // Serial.println(1 / ((micros() - lastFrameTime) * 1000 * 1000));
  OLED.clearDisplay();

  drawMenu(menuCounter);
  drawSubMenu(menuCounter);

  OLED.display();
  // lastFrameTime = micros();
}

void readEncoder()
{
  // Grab state of input pins.
  unsigned char pinstate = (digitalRead(ENCODER_CLK) << 1) | digitalRead(ENCODER_DT);

  // Determine new state from the pins and state table.
  encoderState = lookupTable[encoderState & 0xf][pinstate];

  // Return emit bits, ie the generated event.
  int direction = encoderState & 48;

  // Clockwise
  if (direction == 16)
  {
    if (selectMenu)
    {
      menuCounter++;
      menuCounter = (menuCounter + 5) % 5;
    }
    else
    {
      if (menuCounter == 0)
        delayValue++;
      if (menuCounter == 1)
        longValue++;
      if (menuCounter == 2)
        intervalValue++;
      if (menuCounter == 3)
        countValue++;
    }
  }

  // Counter Clockwise
  if (direction == 32)
  {
    if (selectMenu)
    {
      menuCounter--;
      menuCounter = (menuCounter + 5) % 5;
    }
    else
    {
      if (menuCounter == 0)
      {
        delayValue--;

        if (delayValue < 0)
          delayValue = 0;
      }
      if (menuCounter == 1)
      {
        longValue--;

        if (longValue < 0)
          longValue = 0;
      }
      if (menuCounter == 2)
      {
        intervalValue--;

        if (intervalValue < 0)
          intervalValue = 0;
      }
      if (menuCounter == 3)
      {
        countValue--;

        if (countValue < 0)
          countValue = 0;
      }
    }
  }

  return;
}

void releaseEncoder()
{
  selectMenu = !selectMenu;
  return;
}

void drawMenu(int nums)
{
  // Start Menu
  if (nums == 4)
    return;
  int offset = SCREEN_WIDTH / 8;
  OLED.setTextSize(1);
  OLED.setTextColor(WHITE, BLACK);

  // Delay Menu
  if (nums == 0)
  {
    OLED.fillRect(0, 0, (SCREEN_WIDTH / 4), 8, WHITE);
    OLED.setTextColor(BLACK, WHITE);
  }
  drawText("DLY", offset, 0, "N");
  if (nums == 0)
    OLED.setTextColor(WHITE, BLACK);

  // Long Menu
  if (nums == 1)
  {
    OLED.fillRect(SCREEN_WIDTH / 4, 0, (SCREEN_WIDTH / 4), 8, WHITE);
    OLED.setTextColor(BLACK, WHITE);
  }
  drawText("LNG", offset + (SCREEN_WIDTH / 4), 0, "N");
  if (nums == 1)
    OLED.setTextColor(WHITE, BLACK);

  // Interval Menu
  if (nums == 2)
  {
    OLED.fillRect((SCREEN_WIDTH / 4 * 2), 0, (SCREEN_WIDTH / 4), 8, WHITE);
    OLED.setTextColor(BLACK, WHITE);
  }
  drawText("INTVL", offset + (SCREEN_WIDTH / 4 * 2), 0, "N");
  if (nums == 2)
    OLED.setTextColor(WHITE, BLACK);

  // Number Menu
  if (nums == 3)
  {
    OLED.fillRect((SCREEN_WIDTH / 4 * 3), 0, (SCREEN_WIDTH / 4), 8, WHITE);
    OLED.setTextColor(BLACK, WHITE);
  }
  drawText("N", offset + (SCREEN_WIDTH / 4 * 3), 0, "N");
  if (nums == 3)
    OLED.setTextColor(WHITE, BLACK);
  return;
}

void drawSubMenu(int nums)
{
  OLED.setTextSize(3);
  if (selectMenu) // Main Menu
    OLED.setTextColor(WHITE, BLACK);
  else // Sub Menu
  {
    OLED.setTextColor(BLACK, WHITE);

    if (nums != 4)
    {
      OLED.fillRect(0, 8, SCREEN_WIDTH, SCREEN_HEIGHT - 8, WHITE);
    }
  }

  // Start Menu
  if (nums == 4)
  {
    OLED.setTextColor(WHITE, BLACK);
    drawText("Start?", SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "S");
    OLED.setTextSize(1);
    drawText(String(delayValue) + "s/" + String(longValue) + "s/" + String(intervalValue) + "s/" + String(countValue), SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "N");
    return;
  }

  // Delay Menu
  if (nums == 0)
  {
    drawText(String(delayValue) + "s", SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "CENTER");
    return;
  }

  // Long Menu
  if (nums == 1)
  {
    drawText(String(longValue) + "s", SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "CENTER");
    return;
  }

  // Interval Menu
  if (nums == 2)
  {
    drawText(String(intervalValue) + "s", SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "CENTER");
    return;
  }

  if (nums == 3)
  {
    drawText(String(countValue), SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "CENTER");
    return;
  }

  drawText("0s", SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "CENTER");

  return;
}

void drawText(String text, int x, int y, String anchor)
{
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;
  OLED.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  if (anchor == "CENTER")
  {
    x -= (width / 2);
    y -= (height / 2);
  }
  else if (anchor == "N")
  {
    x -= (width / 2);
  }
  else if (anchor == "S")
  {
    x -= (width / 2);
    y -= (height);
  }

  OLED.setCursor(x, y);
  OLED.print(text);
}

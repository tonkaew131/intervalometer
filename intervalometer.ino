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
long lastDebounceTime = 0;
bool selectMenu = true;
int menuCounter = 0;

int delayValue = 0;
int longValue = 1;
int intervalValue = 1;
int countValue = 1;

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

void IRAM_ATTR readEncoder()
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

        if (longValue < 1)
          longValue = 1;
      }
      if (menuCounter == 2)
      {
        intervalValue--;

        if (intervalValue < 1)
          intervalValue = 1;
      }
      if (menuCounter == 3)
      {
        countValue--;

        if (countValue < 1)
          countValue = 1;
      }
    }
  }

  return;
}

unsigned long startTime = 0;
bool timerState = false;
void releaseEncoder()
{
  if (menuCounter == 4)
  {
    // Starting
    if (timerState == false)
    {
      startTime = millis();
      timerState = true;
    }
    else // Stopping
    {
      timerState = false;
    }
    return;
  }

  selectMenu = !selectMenu;
  return;
}

void setup()
{
  Serial.begin(9600);

  // Rotary Encoder
  pinMode(ENCODER_BTN, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_CLK, INPUT_PULLUP);
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
  if ((millis() - lastDebounceTime) > 150)
  {
    if (digitalRead(ENCODER_BTN) == 0)
    {
      releaseEncoder();
      lastDebounceTime = millis();
    }
  }

  // Serial.println(1 / ((micros() - lastFrameTime) * 1000 * 1000));
  OLED.clearDisplay();

  drawMenu(menuCounter);
  drawSubMenu(menuCounter);
  handleTimer();

  OLED.display();
  // lastFrameTime = micros();
}

void handleTimer()
{
  if (timerState == false)
    return;

  int second = (int)((millis() - startTime) / 1000);

  OLED.setTextColor(WHITE, BLACK);

  // Delay phase
  if (second < delayValue)
  {
    OLED.setTextSize(3);
    drawText(String((int)(delayValue - second)), SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "S");
    OLED.setTextSize(1);
    drawText("Start in", SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "N");
    return;
  }

  second -= delayValue;

  int timerCount = ceil(second / (longValue + intervalValue)) + 1;
  second = (second % (longValue + intervalValue));

  // Shooting phase
  if ((second < longValue) && (timerCount <= countValue))
  {
    OLED.setTextSize(2);
    OLED.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    OLED.setTextColor(BLACK, WHITE);
    drawText(String("SHOOTING"), SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "S");
    OLED.setTextSize(1);
    drawText(String((int)(longValue - second)) + "s", SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 8, "N");

    drawText(String(timerCount) + "/" + String(countValue), SCREEN_WIDTH / 2, 2, "N");
    return;
  }

  if (timerCount > countValue)
  {
    OLED.setTextSize(3);
    OLED.setTextColor(BLACK, WHITE);
    OLED.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    drawText("DONE!", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, "CENTER");

    OLED.setTextSize(1);
    drawText(String(countValue) + "/" + String(countValue), SCREEN_WIDTH / 2, 2, "N");
    return;
  }

  second -= longValue;
  drawText(String(timerCount) + "/" + String(countValue), SCREEN_WIDTH / 2, 2, "N");
  drawText(String((int)(intervalValue - second)), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, "CENTER");
  return;
}

void drawMenu(int nums)
{
  if (timerState == true)
  {
    return;
  }

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
  if (timerState == true)
  {
    return;
  }

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

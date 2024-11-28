#include <SPI.h>
#include <TFT_eSPI.h>
#include "TAMC_GT911.h"

#define TOUCH_SDA 33
#define TOUCH_SCL 32
#define TOUCH_INT 25
#define TOUCH_RST 26
#define TOUCH_WIDTH 320
#define TOUCH_HEIGHT 240

// #define TFT_GREY 0x7BEF
// #define TFT_BLUE 0x001F
// #define TFT_RED  0xF800
// #define TFT_GREEN 0x07E0
// #define TFT_CYAN 0x07FF
// #define TFT_MAGENTA 0xF81F
// #define TFT_YELLOW 0xFFE0
// #define TFT_WHITE 0xFFFF
// #define TFT_ORANGE 0xFDA0

#define TFT_GREY     0x8410
#define TFT_BLUE     0xF800  // Was 0x001F
#define TFT_RED      0x001F  // Was 0xF800
#define TFT_GREEN    0x07E0  // Stays same
#define TFT_CYAN     0xFFE0  // Was 0x07FF
#define TFT_MAGENTA  0xF81F  // Stays same
#define TFT_YELLOW   0x07FF  // Was 0xFFE0
#define TFT_WHITE    0xFFFF  // Stays same
#define TFT_ORANGE   0x051F  // Adjusted for BGR


TFT_eSPI tft = TFT_eSPI();
TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);

enum MenuState
{
    MAIN_MENU,
    MOTOR_MENU,
    MOTOR_CONTROL
};

MenuState currentMenu = MAIN_MENU;
int currentMotor = 1;
int motorTargets[3] = {0, 0, 0};
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_TIME = 300;
const unsigned long NAV_DEBOUNCE_TIME = 200;

struct Button
{
    int x;
    int y;
    int w;
    int h;
    const char *label;
};

void drawButton(Button btn, uint16_t color)
{
    tft.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 8, color);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.drawString(btn.label, btn.x + 10, btn.y + (btn.h / 2) - 8);
}

bool isPressed(Button btn, uint16_t touch_x, uint16_t touch_y)
{
    return (touch_x >= btn.x && touch_x <= btn.x + btn.w &&
            touch_y >= btn.y && touch_y <= btn.y + btn.h);
}

void drawMainMenu()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("WR", 140, 40);

    Button startBtn = {100, 120, 120, 50, "START"};
    drawButton(startBtn, TFT_CYAN);
}

void drawMotorMenu()
{
    tft.fillScreen(TFT_BLACK);

    Button motors[3] = {
        {60, 60, 200, 40, "MOTOR 1"},
        {60, 120, 200, 40, "MOTOR 2"},
        {60, 180, 200, 40, "MOTOR 3"}};

    for (int i = 0; i < 3; i++)
    {
        drawButton(motors[i], TFT_GREEN);
    }

    Button backBtn = {10, 240, 100, 40, "BACK"};
    drawButton(backBtn, TFT_GREY);
}

void drawMotorControl()
{
    tft.fillScreen(TFT_BLACK);

    char title[20];
    sprintf(title, "MOTOR %d", currentMotor);
    tft.setTextSize(2);
    tft.drawString(title, 120, 20);

    char target[20];
    sprintf(target, "Target: %d", motorTargets[currentMotor - 1]);
    tft.drawString(target, 100, 50);

    Button minusBtn = {60, 100, 80, 50, "-1"};
    Button plusBtn = {180, 100, 80, 50, "+1"};
    drawButton(minusBtn, TFT_RED);
    drawButton(plusBtn, TFT_GREEN);

    Button leftBtn = {20, 200, 80, 30, "<"};
    Button backBtn = {120, 200, 80, 30, "BACK"};
    Button rightBtn = {220, 200, 80, 30, ">"};

    if (currentMotor > 1)
        drawButton(leftBtn, TFT_BLUE);
    drawButton(backBtn, TFT_GREY);
    if (currentMotor < 3)
        drawButton(rightBtn, TFT_BLUE);
}

void handleTouch()
{
    tp.read();
    if (tp.isTouched)
    {
        uint16_t touch_x = tp.points[0].x;
        uint16_t touch_y = tp.points[0].y;
        unsigned long currentTime = millis();

        switch (currentMenu)
        {
        case MAIN_MENU:
        {
            if (currentTime - lastButtonPress >= NAV_DEBOUNCE_TIME)
            {
                Button startBtn = {100, 120, 120, 50, "START"};
                if (isPressed(startBtn, touch_x, touch_y))
                {
                    currentMenu = MOTOR_MENU;
                    drawMotorMenu();
                    lastButtonPress = currentTime;
                }
            }
            break;
        }

        case MOTOR_MENU:
        {
            if (currentTime - lastButtonPress >= NAV_DEBOUNCE_TIME)
            {
                Button motors[3] = {
                    {60, 60, 200, 40, "MOTOR 1"},
                    {60, 120, 200, 40, "MOTOR 2"},
                    {60, 180, 200, 40, "MOTOR 3"}};

                Button backBtn = {10, 240, 100, 40, "BACK"};

                if (isPressed(backBtn, touch_x, touch_y))
                {
                    currentMenu = MAIN_MENU;
                    drawMainMenu();
                    lastButtonPress = currentTime;
                }

                for (int i = 0; i < 3; i++)
                {
                    if (isPressed(motors[i], touch_x, touch_y))
                    {
                        currentMotor = i + 1;
                        currentMenu = MOTOR_CONTROL;
                        drawMotorControl();
                        lastButtonPress = currentTime;
                        break;
                    }
                }
            }
            break;
        }

        case MOTOR_CONTROL:
        {
            Button minusBtn = {60, 100, 80, 50, "-1"};
            Button plusBtn = {180, 100, 80, 50, "+1"};
            Button leftBtn = {20, 200, 80, 30, "<"};
            Button backBtn = {120, 200, 80, 30, "BACK"};
            Button rightBtn = {220, 200, 80, 30, ">"};

            if (currentTime - lastButtonPress >= DEBOUNCE_TIME)
            {
                if (isPressed(minusBtn, touch_x, touch_y))
                {
                    motorTargets[currentMotor - 1]--;
                    drawMotorControl();
                    lastButtonPress = currentTime;
                }
                if (isPressed(plusBtn, touch_x, touch_y))
                {
                    motorTargets[currentMotor - 1]++;
                    drawMotorControl();
                    lastButtonPress = currentTime;
                }
            }

            if (currentTime - lastButtonPress >= NAV_DEBOUNCE_TIME)
            {
                if (isPressed(backBtn, touch_x, touch_y))
                {
                    currentMenu = MOTOR_MENU;
                    drawMotorMenu();
                    lastButtonPress = currentTime;
                }
                if (currentMotor > 1 && isPressed(leftBtn, touch_x, touch_y))
                {
                    currentMotor--;
                    drawMotorControl();
                    lastButtonPress = currentTime;
                }
                if (currentMotor < 3 && isPressed(rightBtn, touch_x, touch_y))
                {
                    currentMotor++;
                    drawMotorControl();
                    lastButtonPress = currentTime;
                }
            }
            break;
        }
        }
    }
}

void setup()
{
    Serial.begin(115200);

    tp.begin();
    tp.setRotation(ROTATION_RIGHT);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    drawMainMenu();
}

void loop()
{
    handleTouch();
    delay(50);
}
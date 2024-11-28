#define USER_SETUP_INFO "ESP32_2432S032C"

#define ST7789_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define TFT_BL   27

// Touch settings
#define TFT_ETOUCH_CS_PIN 33
#define TFT_ETOUCH_IRQ_PIN 32

#define SPI_FREQUENCY  24000000

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_GFXFF
#define SMOOTH_FONT
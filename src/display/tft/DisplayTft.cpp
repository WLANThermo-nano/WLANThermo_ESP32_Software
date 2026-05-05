/*************************************************** 
    Copyright (C) 2020  Martin Koerner

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
    HISTORY: Please refer Github History
    
****************************************************/
#include "DisplayTft.h"
#include "Settings.h"
#include "TaskConfig.h"
#include "Preferences.h"
#include "lvScreen.h"
#include "lvTheme.h"
#include "PCA9533.h"

#define TFT_TOUCH_CALIBRATION_ARRAY_SIZE 5u
#define I2C_BRIGHTNESS_CONTROL_ADDRESS 0x0D

extern const uint16_t DisplayTftCharged[];
extern const uint16_t DisplayTftCharging[];
extern const uint16_t DisplayTftStartScreenImg[25400];

TFT_eSPI DisplayTft::tft = TFT_eSPI();
uint16_t DisplayTft::lvBuffer[320 * 4];

DisplayTft::DisplayTft()
{
  this->disabled = false;
  this->orientation = DisplayOrientation::_0;
}

void DisplayTft::hwInit()
{
  DisplayTft::setBrightness(0u);

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(0x31a6);
  tft.pushImage(33, 70, 254, 100, DisplayTftStartScreenImg);

  // configure dimming IC
  this->setBrightness(100u);

  // configure dimming IC (old TFT, aktuell noch in gebrauch)
  // arduino-esp32 2.x: ESP-IDF I2C driver has ~10ms overhead per failed transaction;
  // probe first to avoid 17x failed calls (~170ms) when chip is absent.
  PCA9533 pca9533;
  byte pingResult = pca9533.ping();
  Serial.print("Setup LED Controller: ");
  Serial.println(pingResult);
  if (pingResult == 0)
  {
    pca9533.init();
    pca9533.setPSC(REG_PSC0, 0);
    pca9533.setPSC(REG_PSC1, 29);
    pca9533.setMODE(IO0, LED_MODE_PWM0);
    pca9533.setMODE(IO1, LED_MODE_PWM0);
    pca9533.setMODE(IO2, LED_MODE_PWM0);
    pca9533.setMODE(IO3, LED_MODE_PWM0);
    pca9533.setPWM(REG_PWM0, 255);
  }
}

void DisplayTft::init()
{
  xTaskCreatePinnedToCore(
      DisplayTft::task,           /* Task function. */
      "DisplayTft::task",         /* String with name of task. */
      5000,                       /* Stack size in bytes. */
      this,                       /* Parameter passed as input of the task */
      TASK_PRIORITY_DISPLAY_TASK, /* Priority of the task. */
      NULL,                       /* Task handle. */
      1);                         /* CPU Core */
}

boolean DisplayTft::initDisplay()
{

  if (this->disabled)
  {
    Serial.printf("DisplayTft::init: display disabled\n");
    return true;
  }

  lv_init();

  if (!isCalibrated())
  {
    calibrate();
  }

  lv_display_t *disp = lv_display_create(320, 240);
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(disp, DisplayTft::displayFlushing);
  lv_display_set_buffers(disp, lvBuffer, NULL, sizeof(lvBuffer), LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, DisplayTft::touchRead);

  lv_theme_t *theme = lvTheme_Init(disp, lv_color_hex(0x0aa5c4), lv_color_hex(0x444444),
                                   LVTHEME_FLAG_DARK | LVTHEME_FLAG_NO_FOCUS,
                                   lv_font_get_default(), lv_font_get_default(),
                                   lv_font_get_default(), lv_font_get_default());

  lvScreen_Open(lvScreenType::Home);
  setBrightness(this->brightness);

  return true;
}

boolean DisplayTft::isCalibrated()
{
  Preferences prefs;
  uint16_t touchCalibration[TFT_TOUCH_CALIBRATION_ARRAY_SIZE];
  size_t touchCalibrationSize;

  prefs.begin("TFT", true);
  touchCalibrationSize = prefs.getBytesLength("Touch");
  prefs.end();

  return (((sizeof(uint16_t) * TFT_TOUCH_CALIBRATION_ARRAY_SIZE) == touchCalibrationSize));
}

void DisplayTft::calibrate()
{
  Preferences prefs;
  uint16_t touchCalibration[TFT_TOUCH_CALIBRATION_ARRAY_SIZE];
  size_t touchCalibrationSize;

  this->blocked = true;

  tft.fillScreen((0xFFFF));

  tft.setCursor(20, 0, 2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(1);
  tft.println("calibration run");

  tft.calibrateTouch(touchCalibration, TFT_RED, TFT_BLACK, 15);

  this->blocked = false;

  prefs.begin("TFT", false);
  touchCalibrationSize = prefs.putBytes("Touch", touchCalibration, sizeof(uint16_t) * TFT_TOUCH_CALIBRATION_ARRAY_SIZE);
  prefs.end();
}

void DisplayTft::setBrightness(uint8_t brightness)
{
  this->brightness = brightness;
  int value = (int)(this->brightness * 2.55);

  Wire.beginTransmission(I2C_BRIGHTNESS_CONTROL_ADDRESS);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t DisplayTft::getBrightness()
{
  return this->brightness;
}

void DisplayTft::drawCharging()
{
  // set brightness
  Wire.beginTransmission(I2C_BRIGHTNESS_CONTROL_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  // set brightness
  Wire.beginTransmission(I2C_BRIGHTNESS_CONTROL_ADDRESS);
  Wire.write(100);
  Wire.endTransmission();

  if (gSystem->battery->isCharging())
  {
    tft.pushImage(89, 56, 160, 127, DisplayTftCharging);
  }
  else
  {
    tft.pushImage(89, 56, 160, 127, DisplayTftCharged);
  }
}

void DisplayTft::task(void *parameter)
{
  DisplayTft *display = (DisplayTft *)parameter;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint32_t bootScreenTimeout = 200u; // 1s
  while (bootScreenTimeout || display->system->isInitDone() != true)
  {
    vTaskDelay(10);
    if (bootScreenTimeout)
      bootScreenTimeout--;
  }

  while (display->initDisplay() == false)
    vTaskDelay(1000);

  for (;;)
  {
    //Serial.printf("DisplayTft::task, highWaterMark: %d\n", uxTaskGetStackHighWaterMark(NULL));

    display->update();
    // Wait for the next cycle.
    vTaskDelay(TASK_CYCLE_TIME_DISPLAY_FAST_TASK);
  }
}

void DisplayTft::update()
{
  static uint32_t lastMillis = millis();
  uint32_t currentMillis;

  if (this->disabled || this->blocked)
    return;

  if (gSystem->otaUpdate.isUpdateInProgress())
  {
    lvScreen_Open(lvScreenType::Update);
  }

  lvScreen_Update();

  currentMillis = millis();
  lv_tick_inc(currentMillis - lastMillis);
  lastMillis = currentMillis;
  lv_task_handler();
}

void DisplayTft::displayFlushing(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)px_map, w * h, true);
  tft.endWrite();

  lv_display_flush_ready(disp);
}

void DisplayTft::touchRead(lv_indev_t *indev, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;

  bool touched = tft.getTouch(&touchX, &touchY);

  if (!touched)
  {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  if (touchX > 320 || touchY > 240)
  {
    Serial.printf("Touch coordinates issue: x: %d, y: %d\n", touchX, touchY);
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  data->state = LV_INDEV_STATE_PRESSED;
  data->point.x = touchX;
  data->point.y = touchY;
}

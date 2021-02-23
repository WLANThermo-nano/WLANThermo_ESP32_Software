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
#include "lv_qrcode.h"
#include "lvScreen.h"
#include "PCA9533.h"

#define TFT_TOUCH_CALIBRATION_ARRAY_SIZE 5u
#define I2C_BRIGHTNESS_CONTROL_ADDRESS 0x0D

extern const uint16_t DisplayTftStartScreenImg[25400];

TFT_eSPI DisplayTft::tft = TFT_eSPI();

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
  PCA9533 pca9533;
  pca9533.init();
  Serial.println("Setup LED Controller:");
  Serial.println(pca9533.ping());
  pca9533.setPSC(REG_PSC0, 0);
  pca9533.setPSC(REG_PSC1, 29);
  pca9533.setMODE(IO0, LED_MODE_PWM0);
  pca9533.setMODE(IO1, LED_MODE_PWM0);
  pca9533.setMODE(IO2, LED_MODE_PWM0);
  pca9533.setMODE(IO3, LED_MODE_PWM0);
  pca9533.setPWM(REG_PWM0, 255);
}

void DisplayTft::init()
{
  xTaskCreatePinnedToCore(
      DisplayTft::task,           /* Task function. */
      "DisplayTft::task",         /* String with name of task. */
      10000,                      /* Stack size in bytes. */
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

  lv_disp_buf_init(&lvDispBuffer, lvBuffer, NULL, LV_HOR_RES_MAX * 10);

  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = DisplayTft::displayFlushing;
  disp_drv.buffer = &lvDispBuffer;
  lv_disp_drv_register(&disp_drv);

  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = DisplayTft::touchRead;
  lv_indev_drv_register(&indev_drv);

  LV_THEME_DEFAULT_INIT(lv_theme_get_color_primary(), lv_theme_get_color_secondary(),
                        LV_THEME_MATERIAL_FLAG_DARK,
                        lv_theme_get_font_small(), lv_theme_get_font_normal(), lv_theme_get_font_subtitle(), lv_theme_get_font_title());

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
  tft.fillScreen(0x31a6);
  tft.setTextColor(TFT_WHITE, 0x31a6);
  tft.setTextSize(3);

  // set brightness
  Wire.beginTransmission(I2C_BRIGHTNESS_CONTROL_ADDRESS);
  Wire.write(100);
  Wire.endTransmission();

  if (gSystem->battery->isCharging())
  {
    tft.drawCentreString("CHARGING...", 160, 120, 1);
  }
  else
  {
    tft.drawCentreString("READY!", 160, 120, 1);
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
    display->update();
    // Wait for the next cycle.
    vTaskDelay(TASK_CYCLE_TIME_DISPLAY_FAST_TASK);
  }
}

void DisplayTft::update()
{
  static uint8_t updateInProgress = false;
  static boolean wakeup = false;
  static uint32_t lastMillis = millis();
  uint32_t currentMillis;

  if (this->disabled || this->blocked)
    return;

  if (gSystem->otaUpdate.isUpdateInProgress())
  {
    if (false == updateInProgress)
    {
      updateInProgress = true;
    }
    return;
  }

  lvScreen_Update();

  currentMillis = millis();
  lv_tick_inc(currentMillis - lastMillis);
  lastMillis = currentMillis;
  lv_task_handler();
}

void DisplayTft::displayFlushing(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors(&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

bool DisplayTft::touchRead(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;

  bool touched = tft.getTouch(&touchX, &touchY);

  if (!touched)
  {
    return false;
  }

  if (touchX > 320 || touchY > 240)
  {
    Serial.printf("Touch coordinates issue: x: %d, y: %d\n", touchX, touchY);
  }
  else
  {

    data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

    data->point.x = touchX;
    data->point.y = touchY;
  }

  return false;
}

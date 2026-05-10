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
#include "lvTheme.h"
#include "PCA9533.h"
#include "ArduinoLog.h"

#define TFT_TOUCH_CALIBRATION_ARRAY_SIZE 5u
#define I2C_BRIGHTNESS_CONTROL_ADDRESS 0x0D

extern const uint16_t DisplayTftCharged[];
extern const uint16_t DisplayTftCharging[];
extern const uint16_t DisplayTftStartScreenImg[25400];

static const uint32_t TIMEOUT_VALUES[] = {
    0,          // OFF
    30000,      // 30 s
    60000,      // 60 s
    120000,     // 120 s
    300000,     // 300 s
};

static constexpr uint8_t TIMEOUT_VALUE_COUNT =
    sizeof(TIMEOUT_VALUES) / sizeof(TIMEOUT_VALUES[0]);

void DisplayTft::setTimeoutIndex(uint8_t index)
{
  if (index >= TIMEOUT_VALUE_COUNT) return; // ungültiger Index
  setTimeout(TIMEOUT_VALUES[index]);
}

uint8_t DisplayTft::getTimeoutIndex() const
{
  for (uint8_t i = 0; i < TIMEOUT_VALUE_COUNT; i++)
  {
    if (TIMEOUT_VALUES[i] == timeout)
    {
      return i;
    }
  }

  return 0; // Fallback → AUS
}

TFT_eSPI DisplayTft::tft = TFT_eSPI();

DisplayTft::DisplayTft()
{
  this->disabled = false;
  this->orientation = DisplayOrientation::_0;
}

void DisplayTft::hwInit()
{
  DisplayTft::applyBrightness(0u);

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(0x31a6);
  tft.pushImage(33, 70, 254, 100, DisplayTftStartScreenImg);

  // configure dimming IC
  this->applyBrightness(100u);

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
    Log.notice("DisplayTft::init: display disabled" CR);
    return true;
  }

  lv_init();

  if (!isCalibrated())
  {
    calibrate();
  }
  else
  {
    setCalibration();
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
  indev_drv.user_data = this;
  lv_indev_drv_register(&indev_drv);

  lv_theme_t *theme = lvTheme_Init(lv_color_hex(0x0aa5c4), lv_theme_get_color_secondary(),
                                   LVTHEME_FLAG_DARK | LVTHEME_FLAG_NO_FOCUS,
                                   lv_theme_get_font_small(), lv_theme_get_font_normal(),
                                   lv_theme_get_font_subtitle(), lv_theme_get_font_title());

  lv_theme_set_act(theme);

  lvScreen_Open(lvScreenType::Home);

  isTimeout = false;
  fadeStep = 1;
  fadeIntervalMs = 20;
  lastFadeMillis = 0;
  targetBrightness = this->brightness;
  applyBrightness(this->brightness);
  onUserActivity();

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

void DisplayTft::setCalibration()
{
  Preferences prefs;
  uint16_t touchCalibration[TFT_TOUCH_CALIBRATION_ARRAY_SIZE];

  prefs.begin("TFT", true);
  size_t size = prefs.getBytes("Touch", touchCalibration, sizeof(uint16_t) * TFT_TOUCH_CALIBRATION_ARRAY_SIZE);
  prefs.end();

  if (size == sizeof(uint16_t) * TFT_TOUCH_CALIBRATION_ARRAY_SIZE)
  {
    tft.setTouch(touchCalibration);
    Log.notice("DisplayTft: touch calibration applied from NVS" CR);
  }
}

void DisplayTft::setTimeout(uint32_t newTimeout)
{
  this->timeout = newTimeout;

  // Timeout komplett deaktiviert
  if (timeout == 0)
  {
    isTimeout = false;

    // Display sicher aktivieren
    applyBrightness(this->brightness);
    targetBrightness = this->brightness;

    return;
  }

  // Timeout aktiv (neu gesetzt oder geändert)
  lastActivityMillis = millis();

  // Falls wir gerade im Timeout waren → aufwecken
  if (isTimeout)
  {
    isTimeout = false;
    applyBrightness(this->brightness);
    targetBrightness = this->brightness;
  }
}

uint32_t DisplayTft::getTimeout()
{
  return this->timeout;
}

void DisplayTft::setUserBrightness(uint8_t setBrightness)
{
  this->brightness = setBrightness;

  // nur direkt anwenden, wenn wir NICHT im Timeout sind
  if (!isTimeout)
  {
    applyBrightness(this->brightness);
  }
}

uint8_t DisplayTft::getUserBrightness()
{
  return this->brightness;
}

void DisplayTft::applyBrightness(uint8_t brightness)
{
  currentBrightness = brightness;
  int value = (int)(brightness * 2.55);

  Wire.beginTransmission(I2C_BRIGHTNESS_CONTROL_ADDRESS);
  Wire.write(value);
  Wire.endTransmission();
}

void DisplayTft::setTargetBrightness(uint8_t brightness)
{
  targetBrightness = brightness;
}

void DisplayTft::updateFade()
{
  if (currentBrightness == targetBrightness)
  {
    return; // nichts zu tun
  }

  uint32_t now = millis();
  if (now - lastFadeMillis < fadeIntervalMs)
  {
    return;
  }

  lastFadeMillis = now;

  if (currentBrightness < targetBrightness)
  {
    applyBrightness(currentBrightness + fadeStep);
  }
  else if (currentBrightness > targetBrightness)
  { // wird aktuell nicht benoetigt, da direktes Aufwachen
    applyBrightness(currentBrightness - fadeStep);
  }
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
    // Serial.printf("DisplayTft::task, highWaterMark: %d\n", uxTaskGetStackHighWaterMark(NULL));

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
  handleDisplayTimeout();
  updateFade();

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
  auto *self = static_cast<DisplayTft *>(indev_driver->user_data);
  uint16_t touchX, touchY;
  bool touched = self->tft.getTouch(&touchX, &touchY);

  // kein Touch gefunden
  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
    self->ignoreTouchUntilRelease = false;
    return false;
  }

  // Timeout aktiv? → nur aufwecken
  if (self->isTimeout)
  {
    self->onUserActivity(); // Helligkeit zurücksetzen
    self->ignoreTouchUntilRelease = true;
    data->state = LV_INDEV_STATE_REL;
    return false; // Touch NICHT an LVGL geben
  }

  // nach Wake-up: Touch noch gesperrt?
  if (self->ignoreTouchUntilRelease)
  {
    data->state = LV_INDEV_STATE_REL;
    return false;
  }

  // normaler Touch
  self->onUserActivity();

  if (touchX > 320 || touchY > 240)
  {
    Log.notice("Touch coordinates issue: x: %d, y: %d" CR, touchX, touchY);
    data->state = LV_INDEV_STATE_REL;
    return false;
  }

  data->state = LV_INDEV_STATE_PR;
  data->point.x = touchX;
  data->point.y = touchY;

  return false;
}

void DisplayTft::handleDisplayTimeout()
{
  // Timeout deaktiviert
  if (timeout == 0)
  {
    return;
  }

  uint32_t now = millis();

  if (!isTimeout && (now - lastActivityMillis >= timeout))
  {
    setTargetBrightness(this->timeoutbrightness);
    isTimeout = true;
  }
}

void DisplayTft::onUserActivity()
{
  lastActivityMillis = millis();

  // Timeout deaktiviert → sicherstellen, dass Display aktiv ist
  if (timeout == 0)
  {
    isTimeout = false;
    applyBrightness(this->brightness);
    targetBrightness = this->brightness;
    return;
  }

  if (isTimeout)
  {
    applyBrightness(this->brightness); // ZURÜCK ZUM USER-WERT
    isTimeout = false;
    // Zielwert anpassen, damit updateFade() nichts mehr tut
    targetBrightness = this->brightness;
  }
}

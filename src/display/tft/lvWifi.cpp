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
#include "lvWifi.h"
#include "lvScreen.h"
#include "display/DisplayBase.h"
#include "display/tft/DisplayTft.h"

LV_FONT_DECLARE(Font_Gothic_A1_Medium_h16);

static lvWifiType lvWifi = {NULL};

static void lvWifi_BtnClose(lv_event_t *e);

void lvWifi_Create(void *userData)
{
  lvWifi.screen = lv_obj_create(NULL);
  lv_obj_remove_flag(lvWifi.screen, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *cont = lv_obj_create(lvWifi.screen);
  lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_radius(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 4, 0);
  lv_obj_remove_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *btn = lv_btn_create(lvWifi.screen);
  lv_obj_add_event_cb(btn, lvWifi_BtnClose, LV_EVENT_CLICKED, NULL);
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_set_size(btn, 50, 35);
  lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -5, 7);

  lvWifi.qrCode = lv_qrcode_create(cont);
  lv_qrcode_set_size(lvWifi.qrCode, 132);
  lv_qrcode_set_dark_color(lvWifi.qrCode, lv_color_black());
  lv_qrcode_set_light_color(lvWifi.qrCode, lv_color_white());

  lvWifi.labelFirst = lv_label_create(cont);
  lv_label_set_text(lvWifi.labelFirst, "");
  lv_obj_set_style_text_align(lvWifi.labelFirst, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(lvWifi.labelFirst, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_font(lvWifi.labelFirst, &Font_Gothic_A1_Medium_h16, 0);
  lv_obj_set_style_text_color(lvWifi.labelFirst, lv_color_white(), 0);
  lv_obj_set_size(lvWifi.labelFirst, 320, 30);

  lvWifi.labelSecond = lv_label_create(cont);
  lv_label_set_text(lvWifi.labelSecond, "");
  lv_obj_set_style_text_align(lvWifi.labelSecond, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(lvWifi.labelSecond, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_font(lvWifi.labelSecond, &Font_Gothic_A1_Medium_h16, 0);
  lv_obj_set_style_text_color(lvWifi.labelSecond, lv_color_white(), 0);
  lv_obj_set_size(lvWifi.labelSecond, 320, 30);

  lvWifi_Update(true);

  lv_screen_load(lvWifi.screen);
}

void lvWifi_Update(boolean forceUpdate)
{
  WifiState newWifiState = gSystem->wlan.getWifiState();
  static WifiState wifiState = newWifiState;

  if ((wifiState != newWifiState) || forceUpdate)
  {
    String qrCode;

    switch (newWifiState)
    {
    case WifiState::SoftAPNoClient:
      qrCode = String("WIFI:T:WPA;S:") + gSystem->wlan.getAccessPointName() + ";P:12345678;;";
      lv_qrcode_update(lvWifi.qrCode, qrCode.c_str(), qrCode.length());
      lv_label_set_text_fmt(lvWifi.labelFirst, "WLAN: %s", gSystem->wlan.getAccessPointName());
      lv_label_set_text_fmt(lvWifi.labelSecond, "PW: %s", "12345678");
      break;
    case WifiState::SoftAPClientConnected:
      qrCode = "http://192.168.66.1";
      lv_qrcode_update(lvWifi.qrCode, qrCode.c_str(), qrCode.length());
      lv_label_set_text(lvWifi.labelFirst, "");
      lv_label_set_text(lvWifi.labelSecond, qrCode.c_str());
      break;
    case WifiState::ConnectedToSTA:
      qrCode = "http://" + WiFi.localIP().toString();
      lv_qrcode_update(lvWifi.qrCode, qrCode.c_str(), qrCode.length());
      lv_label_set_text(lvWifi.labelFirst, WiFi.SSID().c_str());
      lv_label_set_text(lvWifi.labelSecond, qrCode.c_str());
      break;
    case WifiState::ConnectingToSTA:
    case WifiState::AddCredentials:
      break;
    default:
      qrCode = "";
      lv_qrcode_update(lvWifi.qrCode, qrCode.c_str(), qrCode.length());
      lv_label_set_text(lvWifi.labelFirst, "");
      lv_label_set_text(lvWifi.labelSecond, qrCode.c_str());
      break;
    }
    wifiState = newWifiState;
  }
}

void lvWifi_Delete(void)
{
  lv_obj_delete(lvWifi.screen);
}

void lvWifi_BtnClose(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvScreen_Open(lvScreenType::Home);
  }
}

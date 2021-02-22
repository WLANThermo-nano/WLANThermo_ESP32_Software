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
#include "lv_qrcode.h"
#include "display/DisplayBase.h"
#include "display/tft/DisplayTft.h"

LV_FONT_DECLARE(Font_Gothic_A1_Medium_h16);

static lvWifiType lvWifi = {NULL};

static void lvWifi_CloseEvent(lv_obj_t *obj, lv_event_t event);

void lvWifi_Create(void *userData)
{
  /* create screen for wifi */
  lvWifi.screen = lv_obj_create(NULL, NULL);

  lv_obj_set_style_local_bg_color(lvWifi.screen, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));

  lvWifi.qrCode = lv_qrcode_create(lvWifi.screen, 132, LV_COLOR_BLACK, LV_COLOR_WHITE);
  lv_obj_set_pos(lvWifi.qrCode, 94, 41);

  lvWifi.labelFirst = lv_label_create(lvWifi.screen, NULL);
  lv_label_set_text(lvWifi.labelFirst, "");
  lv_label_set_align(lvWifi.labelFirst, LV_LABEL_ALIGN_CENTER);
  lv_label_set_long_mode(lvWifi.labelFirst, LV_LABEL_LONG_BREAK);
  lv_obj_set_style_local_text_font(lvWifi.labelFirst, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Gothic_A1_Medium_h16);
  lv_obj_set_style_local_text_color(lvWifi.labelFirst, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_size(lvWifi.labelFirst, 320, 30);
  lv_obj_set_pos(lvWifi.labelFirst, 0, 180);

  lvWifi.labelSecond = lv_label_create(lvWifi.screen, NULL);
  lv_label_set_text(lvWifi.labelSecond, "");
  lv_label_set_align(lvWifi.labelSecond, LV_LABEL_ALIGN_CENTER);
  lv_label_set_long_mode(lvWifi.labelSecond, LV_LABEL_LONG_BREAK);
  lv_obj_set_style_local_text_font(lvWifi.labelSecond, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Gothic_A1_Medium_h16);
  lv_obj_set_style_local_text_color(lvWifi.labelSecond, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_size(lvWifi.labelSecond, 320, 30);
  lv_obj_set_pos(lvWifi.labelSecond, 0, 210);

  /* create close button */
  lvWifi.btnClose = lv_btn_create(lvWifi.screen, NULL);
  lv_obj_add_protect(lvWifi.btnClose, LV_PROTECT_CLICK_FOCUS);
  lv_obj_set_style_local_bg_color(lvWifi.btnClose, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_obj_set_style_local_bg_grad_color(lvWifi.btnClose, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_obj_set_style_local_border_width(lvWifi.btnClose, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_clip_corner(lvWifi.btnClose, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
  lv_obj_set_style_local_radius(lvWifi.btnClose, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(lvWifi.btnClose, 40, 40);
  lv_obj_set_pos(lvWifi.btnClose, 280, 0);
  lv_obj_set_event_cb(lvWifi.btnClose, lvWifi_CloseEvent);

  lv_obj_t * img = lv_img_create(lvWifi.btnClose, NULL);
  lv_obj_set_click(img, false);
  lv_img_set_src(img, LV_SYMBOL_CLOSE);
  lv_obj_set_style_local_image_recolor(img, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  lvWifi_Update(true);

  lv_scr_load(lvWifi.screen);
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
      qrCode = "WIFI:T:WPA;S:" + gSystem->wlan.getAccessPointName() + ";P:12345678;;";
      lv_qrcode_update(lvWifi.qrCode, qrCode.c_str(), qrCode.length());
      lv_label_set_text_fmt(lvWifi.labelFirst, "WLAN: %s", gSystem->wlan.getAccessPointName().c_str());
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
  lv_qrcode_delete(lvWifi.qrCode);
  lv_obj_del(lvWifi.screen);
}

void lvWifi_CloseEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvScreen_Open(lvScreenType::Home);
  }
}
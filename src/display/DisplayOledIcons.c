 /*************************************************** 
    Copyright (C) 2016  Steffen Ochs

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

#include <Arduino.h>

const uint8_t xbmtemp[108] = {
 0x00,0x00,0x00,0x80,0x1f,0x00,0xc0,0x3f,0x00,0xe0,0x70,0x00,0x60,0x60,0x00,
 0x60,0x60,0x00,0x60,0x60,0x00,0xe0,0x7f,0x00,0x60,0x60,0x00,0x60,0x60,0x00,
 0x60,0x60,0x00,0x60,0x60,0x00,0x60,0x60,0x00,0x60,0x60,0x00,0x60,0x60,0x00,
 0x60,0x60,0x00,0x60,0x60,0x00,0x60,0x60,0x00,0x60,0x60,0x00,0x60,0x60,0x00,
 0x60,0x60,0x00,0xe0,0x7f,0x00,0x60,0x60,0x00,0x60,0x60,0x00,0x70,0xe0,0x00,
 0x30,0xcf,0x00,0x98,0x9f,0x01,0xd8,0xbf,0x01,0xd8,0xbf,0x01,0x98,0x9f,0x01,
 0x38,0xcf,0x01,0x70,0xe0,0x00,0xe0,0x79,0x00,0xc0,0x3f,0x00,0x00,0x0f,0x00,
 0x00,0x00,0x00 };

const uint8_t xbmpit[96] = {
 0x00,0x00,0x00,0x00,0x0c,0x00,0x00,0x0c,0x00,0x00,0x1c,0x00,0x00,0x1c,0x00,
 0x00,0x1e,0x00,0xc0,0x1f,0x00,0xe0,0x1f,0x00,0xe0,0x0f,0x00,0xe4,0x0f,0x00,
 0xe6,0x1f,0x00,0xff,0x1f,0x00,0xff,0x3e,0x00,0x7f,0x3e,0x00,0xfe,0x3e,0x00,
 0xfc,0x3f,0x00,0xf8,0x1f,0x00,0xf0,0x9f,0x00,0xe0,0x8f,0x01,0xc2,0xcf,0x03,
 0xe7,0xff,0x03,0xff,0xfd,0x03,0xff,0xfc,0x03,0xfe,0xfc,0x01,0x3c,0xfe,0x00,
 0x38,0x3f,0x00,0x80,0x3f,0x00,0xc0,0x0f,0x00,0xe0,0x03,0x00,0xe0,0x00,0x00,
 0x60,0x00,0x00,0x00,0x00,0x00 };

const uint8_t xbmsys[112] = {
 0x00,0xc0,0x03,0x00,0x00,0xe0,0x07,0x00,0x00,0x60,0x06,0x00,0x80,0x31,0x8c,
 0x01,0xc0,0x3f,0xfc,0x07,0x60,0x1c,0x38,0x06,0x20,0x00,0x00,0x04,0x60,0x00,
 0x00,0x06,0x60,0x00,0x00,0x06,0xc0,0xc0,0x03,0x02,0x70,0xf0,0x0f,0x0e,0x3c,
 0x30,0x0c,0x3c,0x0e,0x18,0x18,0x70,0x06,0x18,0x18,0x60,0x06,0x18,0x18,0x60,
 0x0e,0x18,0x18,0x70,0x3c,0x30,0x0c,0x3c,0x70,0xf0,0x0f,0x0e,0xc0,0xc0,0x03,
 0x02,0x60,0x00,0x00,0x06,0x60,0x00,0x00,0x06,0x20,0x00,0x00,0x04,0x60,0x1c,
 0x78,0x06,0xc0,0x3f,0xfc,0x03,0x00,0x30,0x0c,0x00,0x00,0x60,0x06,0x00,0x00,
 0xe0,0x07,0x00,0x00,0xc0,0x03,0x00 };

const uint8_t xbmarrow[6] = {
  0x0c,0x06,0x7f,0x7f,0x06,0x0c };

const uint8_t xbmarrow1[7] = {
  0x0c,0x1e,0x3f,0x2d,0x0c,0x0c,0x0c  };

const uint8_t xbmarrow2[7] = {
  0x0c,0x0c,0x0c,0x2d,0x3f,0x1e,0x0c };
  
const uint8_t xbmcharge[10] = {
  0x0c,0xdd,0x3c,0x7f,0xff,0xff,0x7f,0x3c,0xdd,0x0c };

const uint8_t xbmback[112] = {
 0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0xc0,0x01,0x00,0x00,0xe0,0x03,
 0x00,0x00,0xf0,0x07,0x00,0x00,0xf8,0x0f,0x00,0x00,0xfc,0x1f,0x00,0x00,0xfe,
 0x3f,0x00,0x00,0xff,0x7f,0x00,0x80,0xff,0xff,0x00,0xc0,0xff,0xff,0x01,0xe0,
 0xff,0xff,0x03,0xf0,0xfb,0xef,0x07,0x00,0xf8,0x0f,0x00,0x00,0xf8,0x0f,0x00,
 0x00,0xf8,0x0f,0x00,0x00,0xf8,0x0f,0x00,0x00,0xf8,0x0f,0x00,0x00,0xf8,0x0f,
 0x00,0x00,0xf8,0x0f,0x00,0x00,0xf8,0x0f,0x00,0x00,0xf8,0x0f,0x00,0x00,0xf8,
 0x0f,0x00,0x00,0xf8,0x0f,0x00,0x00,0xf8,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

const uint8_t xbmnano[795] = {
 0x00,0x00,0x00,0x00,0xfe,0xc1,0x1f,0xff,0xe0,0x07,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0xfe,0xe3,0x3f,0xff,0xf1,0x0f,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x06,0x73,0x30,0x83,0x19,0x18,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x06,0x33,0x30,0x83,0x19,0x18,0x00,0x00,0x00,0x00,0x00,
 0xff,0xff,0xff,0x3f,0x06,0xb3,0x3f,0x83,0x19,0x98,0xff,0xff,0xff,0xff,0x01,
 0xff,0xff,0xff,0x1f,0x87,0x3b,0xbf,0xc3,0x0d,0xcc,0xff,0xff,0xff,0xff,0x01,
 0x00,0x00,0x00,0x00,0x83,0x19,0x98,0xc1,0x0c,0x0c,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x83,0x19,0x98,0xc1,0xfc,0x07,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x83,0x19,0x98,0xc1,0xf8,0x03,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x07,0x00,0x00,0x00,
 0x07,0x00,0x38,0x03,0x00,0x04,0x20,0x00,0x0c,0xf0,0xff,0x7f,0x00,0x00,0x00,
 0x07,0x0c,0x38,0x03,0x00,0x0e,0x60,0x00,0x0c,0xfc,0xff,0xff,0x03,0x00,0x00,
 0x06,0x0c,0x18,0x03,0x00,0x0e,0xe0,0x00,0x0c,0xff,0xff,0xff,0x0f,0x00,0x00,
 0x06,0x1e,0x18,0x03,0x00,0x1f,0xe0,0x01,0xcc,0x07,0x00,0xff,0x3f,0x00,0x00,
 0x0e,0x1e,0x1c,0x03,0x00,0x1b,0xe0,0x03,0xec,0x01,0x00,0xf8,0x7f,0x00,0x00,
 0x0c,0x3f,0x0c,0x03,0x80,0x3b,0x60,0x07,0x6c,0x00,0x06,0xc0,0xff,0x00,0x00,
 0x0c,0x33,0x0c,0x03,0x80,0x31,0x60,0x0e,0x0c,0x00,0x3e,0x80,0xff,0x01,0x00,
 0x0c,0x33,0x0c,0x03,0xc0,0x71,0x60,0x1c,0x0c,0x00,0xfe,0x01,0xff,0x03,0x00,
 0x9c,0x73,0x0e,0x03,0xc0,0x60,0x60,0x38,0x0d,0x00,0xfe,0x07,0xfe,0x07,0x00,
 0x98,0x61,0x06,0x03,0xe0,0xe0,0x60,0x70,0x0d,0x00,0xf0,0x1f,0xfc,0x0f,0x00,
 0xd8,0xe1,0x06,0x03,0xe0,0xff,0x60,0xe0,0x0c,0x00,0x80,0x3f,0xf8,0x0f,0x00,
 0xd8,0xc0,0x06,0x03,0xf0,0xff,0x61,0xc0,0x0d,0x00,0x00,0x7e,0xf0,0x1f,0x00,
 0xf8,0xc0,0x07,0x03,0x30,0x80,0x61,0xa0,0x0f,0x00,0x06,0xf8,0xe0,0x1f,0x00,
 0xf0,0xc0,0x03,0x03,0x38,0x80,0x63,0x20,0x0f,0x00,0x1e,0xf0,0xe1,0x3f,0x00,
 0x70,0x80,0x03,0x03,0x18,0x00,0x63,0x30,0x0e,0x00,0x7e,0xe0,0xc3,0x3f,0x00,
 0x70,0x80,0x03,0xff,0x1d,0x00,0x67,0x10,0x0c,0x00,0xfe,0xc1,0xc3,0x7f,0x00,
 0x20,0x00,0x01,0xff,0x0d,0x00,0x66,0x08,0x08,0x00,0xf8,0x83,0x87,0x7f,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0x87,0x87,0x7f,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x81,0x0f,0x0f,0xff,0x00,
 0xff,0x39,0x70,0xf8,0x8f,0x3f,0xe0,0x00,0x07,0xf8,0x0f,0x0f,0x0f,0xff,0x00,
 0xff,0x39,0x70,0xf8,0x8f,0x7f,0xe0,0x00,0x07,0xfc,0x1f,0x1e,0x1e,0xff,0x00,
 0xff,0x39,0x70,0xf8,0x8f,0xff,0xe0,0x81,0x07,0x3e,0x3e,0x1e,0x1e,0xff,0x00,
 0x38,0x38,0x70,0x38,0x80,0xe3,0xe0,0x81,0x07,0x0f,0x78,0x3c,0x1e,0xfe,0x00,
 0x38,0x38,0x70,0x38,0x80,0xe3,0xf0,0xc3,0x0f,0x07,0x70,0x3c,0x1c,0xfe,0x00,
 0x38,0x38,0x70,0x38,0x80,0xe3,0xf0,0xc3,0x0f,0xc7,0x71,0x3c,0x1c,0xfe,0x01,
 0x38,0xf8,0x7f,0xf8,0x8f,0xff,0xf0,0xc3,0x8f,0xe3,0xe3,0x78,0x3c,0xfe,0x01,
 0x38,0xf8,0x7f,0xf8,0x8f,0xff,0x70,0xe7,0x8e,0xf3,0xe7,0x78,0x3c,0xfe,0x01,
 0x38,0xf8,0x7f,0xf8,0x8f,0x3f,0x70,0xe7,0x8e,0xf3,0xe7,0x70,0x3c,0xfe,0x01,
 0x38,0x38,0x70,0x38,0x80,0x3f,0x78,0xe7,0x9e,0xe3,0xe3,0x06,0x3c,0xfe,0x01,
 0x38,0x38,0x70,0x38,0x80,0x3b,0x38,0x66,0x1c,0xc7,0x71,0xfe,0x38,0xfe,0x01,
 0x38,0x38,0x70,0x38,0x80,0x7b,0x38,0x7e,0x1c,0x07,0x70,0xff,0x03,0xfe,0x01,
 0x38,0x38,0x70,0x38,0x80,0x73,0x38,0x7e,0x1c,0x0f,0x78,0xe0,0xff,0xe0,0x01,
 0x38,0x38,0x70,0xf8,0x8f,0xf3,0x38,0x3c,0x1c,0x3e,0x3e,0x00,0xfe,0x0f,0x00,
 0x38,0x38,0x70,0xf8,0x8f,0xf3,0x1c,0x3c,0x38,0xfc,0x1f,0x00,0xc0,0xff,0x00,
 0x38,0x38,0x70,0xf8,0x8f,0xe3,0x1d,0x18,0x38,0xf8,0x0f,0x00,0x00,0xf8,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x01,0x00,0x00,0x00,0x00 };

const uint8_t xbmlink[795] = {
 0x00,0x00,0x00,0x00,0x80,0x01,0x8c,0x3f,0x18,0x07,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x80,0x01,0x8c,0x7f,0x98,0x03,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0xc0,0x01,0xce,0x61,0xdc,0x01,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0xc0,0x00,0xc6,0x60,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,
 0xff,0xff,0xff,0xff,0xc3,0x00,0xc6,0x60,0x7c,0x80,0xff,0xff,0xff,0xff,0x01,
 0xff,0xff,0xff,0xff,0xe1,0x00,0xe7,0x70,0x7e,0xc0,0xff,0xff,0xff,0xff,0x01,
 0x00,0x00,0x00,0x00,0x60,0x00,0x63,0x30,0xe6,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0xe0,0x3f,0x63,0x30,0xc6,0x01,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0xe0,0x1f,0x63,0x30,0x86,0x03,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x07,0x00,0x00,0x00,
 0x07,0x00,0x38,0x03,0x00,0x04,0x20,0x00,0x0c,0xf0,0xff,0x7f,0x00,0x00,0x00,
 0x07,0x0c,0x38,0x03,0x00,0x0e,0x60,0x00,0x0c,0xfc,0xff,0xff,0x03,0x00,0x00,
 0x06,0x0c,0x18,0x03,0x00,0x0e,0xe0,0x00,0x0c,0xff,0xff,0xff,0x0f,0x00,0x00,
 0x06,0x1e,0x18,0x03,0x00,0x1f,0xe0,0x01,0xcc,0x07,0x00,0xff,0x3f,0x00,0x00,
 0x0e,0x1e,0x1c,0x03,0x00,0x1b,0xe0,0x03,0xec,0x01,0x00,0xf8,0x7f,0x00,0x00,
 0x0c,0x3f,0x0c,0x03,0x80,0x3b,0x60,0x07,0x6c,0x00,0x06,0xc0,0xff,0x00,0x00,
 0x0c,0x33,0x0c,0x03,0x80,0x31,0x60,0x0e,0x0c,0x00,0x3e,0x80,0xff,0x01,0x00,
 0x0c,0x33,0x0c,0x03,0xc0,0x71,0x60,0x1c,0x0c,0x00,0xfe,0x01,0xff,0x03,0x00,
 0x9c,0x73,0x0e,0x03,0xc0,0x60,0x60,0x38,0x0d,0x00,0xfe,0x07,0xfe,0x07,0x00,
 0x98,0x61,0x06,0x03,0xe0,0xe0,0x60,0x70,0x0d,0x00,0xf0,0x1f,0xfc,0x0f,0x00,
 0xd8,0xe1,0x06,0x03,0xe0,0xff,0x60,0xe0,0x0c,0x00,0x80,0x3f,0xf8,0x0f,0x00,
 0xd8,0xc0,0x06,0x03,0xf0,0xff,0x61,0xc0,0x0d,0x00,0x00,0x7e,0xf0,0x1f,0x00,
 0xf8,0xc0,0x07,0x03,0x30,0x80,0x61,0xa0,0x0f,0x00,0x06,0xf8,0xe0,0x1f,0x00,
 0xf0,0xc0,0x03,0x03,0x38,0x80,0x63,0x20,0x0f,0x00,0x1e,0xf0,0xe1,0x3f,0x00,
 0x70,0x80,0x03,0x03,0x18,0x00,0x63,0x30,0x0e,0x00,0x7e,0xe0,0xc3,0x3f,0x00,
 0x70,0x80,0x03,0xff,0x1d,0x00,0x67,0x10,0x0c,0x00,0xfe,0xc1,0xc3,0x7f,0x00,
 0x20,0x00,0x01,0xff,0x0d,0x00,0x66,0x08,0x08,0x00,0xf8,0x83,0x87,0x7f,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0x87,0x87,0x7f,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x81,0x0f,0x0f,0xff,0x00,
 0xff,0x39,0x70,0xf8,0x8f,0x3f,0xe0,0x00,0x07,0xf8,0x0f,0x0f,0x0f,0xff,0x00,
 0xff,0x39,0x70,0xf8,0x8f,0x7f,0xe0,0x00,0x07,0xfc,0x1f,0x1e,0x1e,0xff,0x00,
 0xff,0x39,0x70,0xf8,0x8f,0xff,0xe0,0x81,0x07,0x3e,0x3e,0x1e,0x1e,0xff,0x00,
 0x38,0x38,0x70,0x38,0x80,0xe3,0xe0,0x81,0x07,0x0f,0x78,0x3c,0x1e,0xfe,0x00,
 0x38,0x38,0x70,0x38,0x80,0xe3,0xf0,0xc3,0x0f,0x07,0x70,0x3c,0x1c,0xfe,0x00,
 0x38,0x38,0x70,0x38,0x80,0xe3,0xf0,0xc3,0x0f,0xc7,0x71,0x3c,0x1c,0xfe,0x01,
 0x38,0xf8,0x7f,0xf8,0x8f,0xff,0xf0,0xc3,0x8f,0xe3,0xe3,0x78,0x3c,0xfe,0x01,
 0x38,0xf8,0x7f,0xf8,0x8f,0xff,0x70,0xe7,0x8e,0xf3,0xe7,0x78,0x3c,0xfe,0x01,
 0x38,0xf8,0x7f,0xf8,0x8f,0x3f,0x70,0xe7,0x8e,0xf3,0xe7,0x70,0x3c,0xfe,0x01,
 0x38,0x38,0x70,0x38,0x80,0x3f,0x78,0xe7,0x9e,0xe3,0xe3,0x06,0x3c,0xfe,0x01,
 0x38,0x38,0x70,0x38,0x80,0x3b,0x38,0x66,0x1c,0xc7,0x71,0xfe,0x38,0xfe,0x01,
 0x38,0x38,0x70,0x38,0x80,0x7b,0x38,0x7e,0x1c,0x07,0x70,0xff,0x03,0xfe,0x01,
 0x38,0x38,0x70,0x38,0x80,0x73,0x38,0x7e,0x1c,0x0f,0x78,0xe0,0xff,0xe0,0x01,
 0x38,0x38,0x70,0xf8,0x8f,0xf3,0x38,0x3c,0x1c,0x3e,0x3e,0x00,0xfe,0x0f,0x00,
 0x38,0x38,0x70,0xf8,0x8f,0xf3,0x1c,0x3c,0x38,0xfc,0x1f,0x00,0xc0,0xff,0x00,
 0x38,0x38,0x70,0xf8,0x8f,0xe3,0x1d,0x18,0x38,0xf8,0x0f,0x00,0x00,0xf8,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x01,0x00,0x00,0x00,0x00 };

 const char *menutextde[16] = {"","","","TYP","ALARM","","PITMASTER","CHANNEL","SET","ACTIVE","","SSID","IP","HOST-NAME","UNIT","FIRMWARE"};
 

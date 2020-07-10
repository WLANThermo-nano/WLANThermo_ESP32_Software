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
#pragma once

#include <Arduino.h>

#define LOG_BUFFER_SIZE 5000u

class LogRingBuffer : public Print
{
public:
  LogRingBuffer();
  ~LogRingBuffer();
  size_t write(uint8_t character);
  String get();

private:
  char buffer[LOG_BUFFER_SIZE];
  size_t bufferSize;
  size_t bufferIndex;
  boolean isFull;
};

extern LogRingBuffer gLogRingBuffer;
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
#include "LogRingBuffer.h"

LogRingBuffer::LogRingBuffer()
{
  bufferIndex = 0u;
  isFull = false;
}

LogRingBuffer::~LogRingBuffer()
{
  if (buffer != NULL)
  {
    delete (buffer);
  }
}

size_t LogRingBuffer::write(uint8_t character)
{
  buffer[bufferIndex++] = character;

  // Check for overflow
  if (bufferIndex >= LOG_BUFFER_SIZE)
  {
    bufferIndex = 0u;
    isFull = true;
  }
}

String LogRingBuffer::get()
{
  String logString;

  if ((false == isFull))
  {
    for (size_t i = 0u; i < bufferIndex; i++)
    {
      logString += buffer[i];
    }
  }
  else
  {
    for (size_t i = 0; i < LOG_BUFFER_SIZE; i++)
    {
      size_t index = bufferIndex + i;

      if (index >= LOG_BUFFER_SIZE)
      {
        index -= LOG_BUFFER_SIZE;
      }

      logString += buffer[index];
    }
  }

  return logString;
}

LogRingBuffer gLogRingBuffer;
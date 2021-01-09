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

#define TASK_PRIORITY_OTA_UPDATE 100
#define TASK_PRIORITY_SYSTEM_TASK 30
#define TASK_PRIORITY_MAIN_TASK 3
#define TASK_PRIORITY_CONNECT_TASK 2
#define TASK_PRIORITY_DISPLAY_TASK 2
#define TASK_PRIORITY_BLUETOOTH_TASK 2
#define TASK_PRIORITY_PBGUARD_TASK 1

#define TASK_CYCLE_TIME_SYSTEM_TASK 200
#define TASK_CYCLE_TIME_MAIN_TASK 200

#define TASK_CYCLE_TIME_CONNECT_TASK 1000

#define TASK_CYCLE_TIME_DISPLAY_FAST_TASK 10
#define TASK_CYCLE_TIME_DISPLAY_SLOW_TASK 100

#define TASK_CYCLE_TIME_BLUETOOTH_TASK 1000

/*************************************************** 
    Copyright (C) 2019  Martin Koerner

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

#ifdef HW_MINI_V1
#include "system/SystemMiniV1.h"
#include "display/DisplayNextion.h"
SystemBase *gSystem = new SystemMiniV1();
DisplayBase *gDisplay = new DisplayNextion();
#elif HW_MINI_V2
#include "system/SystemMiniV2.h"
#include "display/DisplayNextion.h"
SystemBase *gSystem = new SystemMiniV2();
DisplayBase *gDisplay = new DisplayNextion();
#elif HW_MINI_V3
#include "system/SystemMiniV3.h"
#include "display/DisplayTft.h"
SystemBase *gSystem = new SystemMiniV3();
DisplayBase *gDisplay = new DisplayTft();
#elif HW_NANO_V3
#include "system/SystemNanoVx.h"
#include "display/DisplayOled.h"
SystemBase *gSystem = new SystemNanoVx();
DisplayBase *gDisplay = new DisplayOled();
#elif HW_LINK_V1
#include "system/SystemLinkV1.h"
#include "display/DisplayOledLink.h"
SystemBase *gSystem = new SystemLinkV1();
DisplayBase *gDisplay = new DisplayOledLink();
#endif

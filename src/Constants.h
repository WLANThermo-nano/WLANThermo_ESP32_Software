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

#ifndef C_CONSTANTS_H_
#define C_CONSTANTS_H_

#define DEFAULT_APNAME "WLANTHERMO-AP"

#ifdef HW_NANO_V3
#define DEFAULT_HOSTNAME "NANO-"
#elif HW_LINK_V1
#define DEFAULT_HOSTNAME "LINK-"
#else
#define DEFAULT_HOSTNAME "MINI-"
#endif

#endif /* C_CONSTANTS_H_ */

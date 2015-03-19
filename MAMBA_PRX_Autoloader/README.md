MAMBA/PRX Autoloader (c) 2015 NzV

Add load of MAMBA and/or VSH plugins (with MAMBA or PRX Loader) at system boot using New Core as loader.

-The generated "sys_init_osd.self" is fw dependent (CEX an DEX version include in the same self)!
-The generated "sys_init_osd.self" should replace sys_init_osd.self in /dev_flash/sys/internal/
 who need to be previously renamed sys_init_osd_orig.self
 
===============================
			[FLAGS]
===============================

Flags can be placed in /dev_usb000/core_flags/ or /dev_usb001/core_flags/ or /dev_hdd0/tmp/core_flags/

"failsafe"  	Start in normal mode (MAMBA and VSH plugins not loaded)
"mamba_off"   	Don't load  MAMBA (PRX Loader will be used instead of MAMBA to load VSH plugins)
"noplugins"   	Don't load  VSH plugins at boot
"verbose"   	Enable log and write it in /dev_usb000 or /dev_usb001 or /dev_hdd0

===============================
		[VSH PLUGINS]
===============================

-If flag "mamba_off" is not set VSH Plugins will be loaded from file /dev_hdd0/mamba_plugins.txt with MAMBA
-Else they will be loaded from file /dev_hdd0/prx_plugins.txt with prx loader

==============================================================

This program is based in:

- New Core by MiralaTijera and changes by Estwald 

- MAMBA by Estwald and unofficial updates by _NZV_ (GetVSHProcess, PS3M_API)
  Some part of code for load MAMBA payload come from Iris Manager and his fork (IRISMAN, GAMESONIC MANAGER, MANAGUNZ)

- PRX Loader by User and unofficial updates by _NZV_ (GetVSHProcess)
  Some part of code for load PRX Loader payload come from "payload autoloader" by KW

==============================================================

(c) 2013 MiralaTijera <www.elotrolado.net> (Original Core)
(c) 2013 Estwald <www.elotrolado.net> (New Core)

"New Core" is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

"New Core" is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with "New Core". If not, see <http://www.gnu.org/licenses/>.

==============================================================
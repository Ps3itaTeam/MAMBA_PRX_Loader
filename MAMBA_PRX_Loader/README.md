-----------------------------------------------------------------------------------------------------
MAMBA/PRX Loader (+Autoloader) by NzV
-----------------------------------------------------------------------------------------------------

	MAMBA/PRX Loader is an homebrew who allow you to load MAMBA and/or VSH plugins (with MAMBA or
	PRX Loader) by two different way : at system boot or after.

-----------------------------------------------------------------------------------------------------
How to use it after system boot:
-----------------------------------------------------------------------------------------------------

	- Launch "MAMBA/PRX Loader"
	- It will load MAMBA and VSH plugins, when it does you will get back to xmb and earn one beep if
	  no error has occurred, if you earn more beep an error has occurred
	  (see log: /dev_hdd/tmp/MAMBA_PRX_Loader.log)
	
	Note: If you want to load only MAMBA and no VSH plugins hold L1 when "MAMBA/PRX Loader" start.
		  if you want to load only VSH plugins and no MAMBA (PRX Loader will be used) hold R1 when
		  "MAMBA/PRX Loader" start.
		  
-----------------------------------------------------------------------------------------------------
How to use it at system boot (Autoloader):
-----------------------------------------------------------------------------------------------------

	- Launch "MAMBA/PRX Loader"
	- Hold CROSS to install "MAMBA/PRX Autoloader" to your PS3, when it does you will earn one beep
	  if no error has occurred and your ps3 will restart it self, if you earn more beep an error has
	  occurred (see log: /dev_hdd/tmp/MAMBA_PRX_Loader.log) and you will get back to xmb.
	- Hold SQUARE to uninstall "MAMBA/PRX Autoloader" from your PS3, when it does you will earn one
	  beep if no error has occurred, if you earn more beep an error has occurred
	  (see log: /dev_hdd/tmp/MAMBA_PRX_Loader.log)
	
	Boot flags (can be placed in /dev_usb000/core_flags/ or /dev_usb001/core_flags/
	or /dev_hdd0/tmp/core_flags/):

		- "failsafe"  		Start in normal mode (MAMBA and VSH plugins are not loaded)
		- "mamba_off"   	Don't load  MAMBA (PRX Loader will be used instead of MAMBA to load VSH plugins)
		- "noplugins"   	Don't load  VSH plugins at boot
		- "verbose"   		Enable log and write it in /dev_usb000 or /dev_usb001 or /dev_hdd0
	

-----------------------------------------------------------------------------------------------------
Supported CFW:
-----------------------------------------------------------------------------------------------------

	-CEX: 3.55, 4.21, 4.30, 4.31, 4.40, 4.41, 4.46, 4.50, 4.53, 4.55, 4.60, 4.65, 4.66, 4.70
	-DEX: 3.55, 4.21, 4.30, 4.41, 4.46, 4.50, 4.53, 4.55, 4.65, 4.66, 4.70
	
	
-----------------------------------------------------------------------------------------------------
VSH plugins:
-----------------------------------------------------------------------------------------------------

	Load with MAMBA:

		-To add or remove VSH plugins it is necessary to edit /dev_hdd0/mamba_plugins.txt

	Load with PRX Loader (only if MAMBA is disabled):

		-To add or remove VSH plugins it is necessary to edit /dev_hdd0/prx_plugins.txt 

-----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------NzV---

	

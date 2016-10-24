
This is a modified arduino bootloader which supports dual boot. Here the trick is to save a backup ROM in external eeprom which is then invoked from the bootloader on hardware reset. This dualboot feature will be invoked only if the pin 10 and 9 of arduino (here uno/pro mini) is shorted before and while a hardware reset is performed. After hardwareset the on board LED will glow for 1 seconds if the pin 10 and 11 are shorted while reset is performed. If we remove the short before 1 seconds (ie before the LED is turned off), then it enter into dualboot operation. It do a swap operation ie it will backup the current ROM to external eeprom and writes the existing rom in external eeprom to internal flash and starts booting into the new ROM. If we do the same again, the reverse will happen  as we are swapping between the backup ROM and current ROM. 

At that same time, this is arduino supported bootloader so we can upload sketches via arduino software or commandline as usual.

Currently You can try with UNO as the board even if the baord is pro-mini-16. Bootloader works at 115200 BAUD.

For detailed explanation please visit http://blog.vinu.co.in

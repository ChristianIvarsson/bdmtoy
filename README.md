# nonameadapter

! IMPORTANT ! IMPORTANT ! IMPORTANT ! IMPORTANT ! IMPORTANT ! IMPORTANT !
* Firmwares built / uploaded before 21/9/2022 must be updated to function with the current host code/apps.



Info will follow... Basically it's a oldbdm, newbdm, sbdm, jtag, nexus adapter ..thing.
I got really tired of having a bunch of different adapters for different targets so I started working on one to rule them all.

Currently supports:
* Old BDM: Trionic 5.2/5.5(Most replacement flash is supported too), 7, 8, 8 mcp.
* NEXUS: Confirmed working on mpc5566 but there is nothing supported due to their locked down nature.
* SBDM: SIU from 9-5 MY06+, SID from 9-5MY04-05 (Including their eeprom) and some toy code for a random BMW cluster I found in a box.
* New BDM: EDC16C39 main flash and eeprom.


The adapter is based around a simple stm32f103 board (with VERY ugly code atm).
Pinouts can be found in common.h inside the firmware folder.
Jumper cables works fine for SBDM but you really need a decent, preferably shielded, cable for old/new BDM since it can run at up to 12 MHz.

Currently only gui apps (Linux app pictured, there's also one for Windows)
![alt text](/bdmstuff.png)

Ain't many wires to keep track of:
![alt text](/pinout.png)

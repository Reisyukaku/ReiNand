# ReiNand
*The original open source N3DS CFW!*
<sub> (now with O3DS support!)</sub>


**Compiling:**

You'll need armips added to your Path. [HERE](https://www.dropbox.com/s/ceuv2qeqp38lpah/armips.exe?dl=0) is a pre-compiled version.

    make - Compiles All. (launcher and a9lh)(Default:N3DS)
    make launcher - Compiles CakeHax/CakeBrah payload
    make a9lh - Compiles arm9loaderhax payload
    make cons=[variable] - Compiles for a specific console. Variable can be either 'o3ds' or 'n3ds' (no quotes)

Copy everything in 'out' folder to SD root and run!


**Features:**

* Ninjhax/MSET/Spider/A9LH support!

* Emunand/Patched Sysnand (with '.:Rei' version string)

* Sig checks disabled

* Firm partition update protection

* Ability to modify splash screen

* Ability to modify process patches
    * Region free CIAs
    * Region free Carts
    * Auto updates disabled
    * EShop update check disabled
    * RO verification disabled
    * and [MORE](https://reisyukaku.org/3DS/ReiNand/patches)

**Credits:**
 
 Cakes team for teaching me a few things and just being helpful in general! And for ROP/mset related code, and crypto libs.
    
 3DBREW for saving me plenty of reverse engineering time.
    
 Patois/Cakes for CakesBrah.
 
 Yifanlu for custom loader module!
 
 Steveice10/Gudenaurock for helping a lot with arm11 stuff!
 
 Normmatt for sdmmc.c and generally being helpful!
 
 AuroraWright for being helpful!
    
 Me (Rei) for coding everything else.
 
 The community for your support and help!
 
 
 **License**
 
This software is licensed under GPLv3. A copy of the License is attached.
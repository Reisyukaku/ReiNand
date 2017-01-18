# ReiNand
[![License (GPL version 3)](https://img.shields.io/badge/license-GNU%20GPL%20version%203-red.svg?style=flat-square)](http://opensource.org/licenses/GPL-3.0)

*The original open source N3DS CFW!*
<sub> (now with O3DS support!)</sub>


**Compiling:**

You'll need armips and bin2c added to your Path. [HERE](https://reisyukaku.org/downloads/buildtools.zip) are the pre-compiled binaries for Windows.

    make - Compiles All. (launcher and a9lh)
    make a9lh - Compiles arm9loaderhax payload

Copy everything in 'out' folder to SD root and run!


**Features:**

* Ninjhax/MSET/Spider/A9LH support!

* Emunand/Patched Sysnand (with '.:Rei' version string)

* Sig checks disabled

* Reboot patch

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
 
 AuroraWright for being helpful with better sysmod injection and stuff!
 
 Roxas75 for the reboot code.
    
 Me (Rei) for coding everything else.
 
 The community for your support and help!
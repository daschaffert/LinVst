# LinVst

-----

LinVst enables Windows vst's to be used as Linux vst's in Linux vst capable DAW's.

-------

How to use LinVst

To use LinVst, the linvst.so file simply needs to be renamed to match the windows vst dll's filename.

(the .so and .dll extensions are left as they are and are not part of the renaming)

Both the windows vst dll file and the renamed linvst.so file are then a matched pair and need to be kept together in the same directory/folder.

For Example

copy lin-vst-server.exe and lin-vst-server.so to /usr/bin

copy linvst.so to wherever the windows vst dll file is that someone wants to run in a Linux DAW, 
and then rename linvst.so to the windows vst dll filename 

linvst.so gets renamed to test.so for test.dll

Use the renamed file (test.so) in a Linux VST capable DAW 

Load test.so within the Linux DAW and then test.so will load and run the (name associated) test.dll windows vst 

linvstconvert (GUI or CLI) can batch name convert linvst.so to mutiple windows vst dll names that are located in a folder/directory (the linvstconvert CLI version needs to be run from within the dll's folder/directory).

sudo permission might be needed for some folders/directories that don't have user permissions.

After the naming conversion, the newly created files (.so files) are ready to be used in Linux vst DAW's from the folder that was used for the naming conversion.

Copying/moving plugins (and in some cases their associated presets etc) to a folder/directory with user permissions (if possible) is generally a good idea unless the vst plugin requires a fixed path.

-------

Using a folder of windows dll files as an example.

Say the folder contains 

Delay.dll

Compressor.dll

Chorus.dll

Synth.dll

then after the renaming (using the renaming utility) the folder will look like

Delay.dll
Delay.so

Compressor.dll
Compressor.so

Chorus.dll
Chorus.so

Synth.dll
Synth.so

The files ending with .so can be used inside Linux Vst DAWS to load and manage the associated dll files (ie Delay.so loads and manages Delay.dll).

Say for instance that a windows vst installation has installed a windows vst named Delay.dll into the VstPlugins directory inside of a WINEPREFIX ie (~/.wine/drive_c/Program Files/VstPlugins), then the renaming utility needs to be run on the VstPlugins directory.

After the renaming there is Delay.dll and Delay.so in the ~/.wine/drive_c/Program Files/VstPlugins directory.

The Linux DAW plugin search path is then appended to include ~/.wine/drive_c/Program Files/VstPlugins and then the plugin(s) can be loaded.

Another way is to make a symbolic link to ~/.wine/drive_c/Program Files/VstPlugins/Delay.so or to the whole ~/.wine/drive_c/Program Files/VstPlugins directory from a more convenient folder such as /home/user/vst and then append /home/user/vst to the Linux DAW's plugin search path.

There can be multiple WINEPREFIXES (by default there is one ~/.wine) and each WINEPREFIX can have a different wine setup, including dll overrides etc.

Different windows vst plugins that might require different wine setups can be installed into different WINEPREFIXES by creating a WINEPREFIX (export WINEPREFIX=~/.wine-new wine winecfg) before running the windows vst installation program.

When a plugin is loaded from within a WINEPREFIX, it picks up on that WINEPREFIXES individual setup (also works for symbolic links as discussed above).

------

linvst.so needs to be renamed to the windows vst name (the .so and .dll extensions are left as they are and are not part of the renaming).

Both the windows vst dll file and the renamed linvst.so file need to be in the same folder/directory.

linvst.so is a Linux vst template that loads and runs any windows vst that linvst.so is renamed to.

linvst.so can be copied and renamed multiple times to load multiple windows vst's.

Basically, linvst.so becomes the windows vst once it's renamed to the windows vst's name and can then be used in Linux vst hosts.

Individual plugins can have their own WINEPREFIX environment.

If a windows vst dll file and it's associated renamed linvst.so file are located within a WINEPREFIX then the plugin will use that WINEPREFIX.

Symlinks can point to renamed linvst.so files located within a WINEPREFIX.

------

Common Problems/Fixes

A large number of vst plugin crashes/problems can be basically narrowed down to the following dll's and then worked around by overriding/disabling them.

Quite a few vst plugins rely on the Visual C++ Redistributable dlls msvcr120.dll msvcr140.dll msvcp120.dll msvcp140.dll etc

Some vst plugins might crash if the Visual C++ Redistributable dlls are not present in /home/username/.wine/drive_c/windows/system32 for 64 bit vst's and /home/username/.wine/drive_c/windows/syswow64 for 32 bit vst's, and then overridden in the winecfg Libraries tab

Quite a few plugins rely on the d2d1 dll 

Some vst's might crash if Wines inbuilt d2d1 is active (which it is by default).

d2d1 can be disabled in the winecfg Libraries tab.

wininet is used by some vst's for net access including registration and online help etc and sometimes wines inbuilt wininet might cause a crash or have unimplemented functions.

wininet can be overridden with wininet.dll and iertutil.dll and nsi.dll

Occasionally other dlls might need to be overridden such as gdiplus.dll etc

For details about overriding a dll, see the next section (Wine Config).

------

Wine Config

Sometimes usernames and passwords might need to be copied and pasted into the window because manual entry might not work in all cases.

Sometimes a windows vst needs a Wine dll override.

If the Wine debugger displays "unimplemented function in XXXX.dll" somewhere in it's output, then that dll usually needs to be overriden with a windows dll.

Overriding a dll involves copying the windows dll to a wine windows directory and then running winecfg to configure wine to override the dll.

64 bit .dlls are copied to /home/username/.wine/drive_c/windows/system32 

32 bit .dlls are copied to /home/username/.wine/drive_c/windows/syswow64

Run winecfg and select the Libraries tab and then select the dll to override from the list or type the name.

After adding the dll, check with the edit option that the dll's settings are native first and then builtin.

https://www.winehq.org/docs/wineusr-guide/config-wine-main

Sometimes required dll's might be missing and additional windows redistributable packages might need to be installed.

Some windows vst's use D3D, and Wine uses Linux OpenGL to implement D3D, so a capable Linux OpenGL driver/setup might be required for some windows vst's.

Disabling d2d1 in the Libraries section of winecfg might help with some windows vst's.

Some D3D dll overrides might be needed for some windows vst's.

D3D/OpenGL Wine config advice can be found at gaming forums and other forums.

Additional dll's (dll overrides) might have to be added to Wine for some Windows vst's to work.

Winetricks might help with some plugins https://github.com/Winetricks/winetricks

Some plugins might use wininet for internet connections (online registration, online help, etc) which might cause problems depending on Wines current implementation.

Running winetricks wininet and/or installing winbind for a distro (sudo apt-get install winbind) might help (wininet and it's associated dll's can also be manually installed as dll overrides).

Turning off the vst's multiprocessor support and/or GPU acceleration might help in some cases, due to what features Wine currently supports (Wine version dependent).

On some slower systems Wine can initially take a long time to load properly when Wine is first used, which might cause a LinVst crash.
The solution is to initialise Wine first by running winecfg or any other Wine based program, so that Wine has been initialised before LinVst is used.

Drag and Drop features in vst's are not supported.

Upgrading to the latest wine-stable version is recommended.

------

Latency

Some distros/wine can result in varying latency results.

LinVst has produced reasonable latency results on Ubuntu Studio and Debian with a realtime kernel, but not on some other distros, so some distros (and their setups, wine etc) might be better than others for latency.

------

LinVst tested with Wine 2 and Linux Tracktion 7, Linux Ardour 5.6, Linux Bitwig Studio 2, Linux Reaper 5.4

Drag and Drop features are not supported.

Tested vst's

Kontakt Player 5.6.8 (turn multiprocessing off). Requires Wine 2.0 and above

Some additional dll overrides (below) might be needed for Kontakt and Wine 2.0.
Kontakt and Wine 2.8 staging only need an additional msvcp140.dll override. 
To override dll's, copy windows dlls to drive_c/windows/system32 and then override the dlls to be native using the winecfg Libraries option.

(Kontakt Wine 2.0 additional dll's, msvcp140.dll concrt140.dll api-ms-win-crt-time-l1-1-0.dll api-ms-win-crt-runtime-l1-1-0.dll ucrtbase.dll)

Guitar Rig 5 (same dll overrides as Kontakt)

Reaktor 6 (msvcp140.dll concrt140.dll dll overrides for Wine 2.0)

FM8

Line 6 Helix Native (msvcr120.dll and gdiplus.dll overrides) (copy and paste username and password into the registration window)

S-Gear Amp Sim

TH3 Amp Sim

IK Amplitube 4

IK SampleTank

Addictive Drums 2 (Addictive Drums 2 requires that the dll (and therefore the renamed linvst.so) needs to be loaded from the installation directory, ie a fixed path).

EZDrummer2

Mercuriall Spark Amp Sim

Melda MXXX Multi Effects (turn GPU acceleration off)

T-RackS

Nebula4

VUMT

Sforzando

Zampler RX

Stillwell plugins

Cobalt (Sanford) Synth

OP-X PRO-II (Disable d2d1 in the Libraries section of winecfg)

MT-PowerDrumKit (Disable d2d1 in the Libraries section of winecfg)

Ignite Amps TPA-1 Amp Sim

LePou Amp Sims

Nick Crow Lab Amp Sims

Voxengo Boogex Guitar Effects

Klanghelm MJUC Compressor

TDR SlickEQ mixing/mastering equalizer 

Toneboosters TrackEssentials (disable d2d1 for Ferox)

--------

To make

The plugininterfaces folder contained within the VST2_SDK folder, needs to be placed in the LinVst files folder. https://www.steinberg.net/en/company/developers.html

Wine libwine development files.

For Ubuntu/Debian, sudo apt-get install libwine-development-dev

wine-devel packages for other distros.

libX11 development needed for embedded version (sudo apt-get install libx11-dev)
 
Include and Library paths might need to be changed in the Makefile for various 64 bit and 32 bit Wine development path locations (otherwise 32 bit compiles might try to link with 64 bit libraries etc).

Additional 32 bit development libraries are probably needed.

On Ubuntu/Debian 64 bits 

sudo dpkg --add-architecture i386

sudo apt-get install libc6-dev-i386

sudo apt-get install gcc-multilib g++-multilib

sudo apt-get install libwine-development-dev:i386

For Debian add deb http://ftp.de.debian.org/debian distro main non-free to /etc/apt/sources.list (distro gets replaced with distro ie stretch etc)

sudo apt-get update

--------

sudo make clean

make

sudo make install

Installs lin-vst-server.exe and lin-vst-server.exe.so to /usr/bin and installs linvst.so to /vst in the source code folder

(also installs lin-vst-server32.exe and lin-vst-server32.exe.so to /usr/bin for 32 bit vst support)

Makefile-embed6432 and Makefile-noembed6432 build a LinVst version that autodetects and automatically runs both 64 bit vst's and 32 bit vst's.

Makefile-embed6432 and Makefile-embed are for the host embedded window option.

Makefile-noembed6432 and Makefile-noembed are for a standalone window version that can be useful for some hosts (Tracktion).





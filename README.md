# LinVst-Mac

------------

LinVst for the Mac.

------------

How To Use:

Copy/move the windows vst dll file to the linvst.vst Resources folder.

Set wine path in the Resources folder's config.plist

for example

	<key>wine-path</key>
	<string>/opt/local/bin/wine</string>
	
The linvst.vst can be duplicated and renamed multiple times and have various windows vst dll files placed in their Resources folder, which makes it possible to use multiple windows vst's within Mac DAWs.	

Tested with Wine 1.79 (Wineskin) on 32 bit 10.6.8 Snow Leopard with Reaper 5.

---------------

To Make

Xcode 3.2 project (using g++ 4.2) for 32 bit Osx 10.6.8, builds the linvst.vst vst.

Makefile builds the lin-vst-server.so file.

The lin-vst-server.so file needs o be built first before the Xcode project linvst.vst vst is built.

Needs the affectx.h file from the Vst2 SDK to be placed in the projects folder.

The Vst2 SDK is available from Steinberg and is included in the Vst3 SDK https://www.steinberg.net/en/company/developers.html

LinVst-Mac can be compiled for 64 bits.




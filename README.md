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

Makefilelinvstserver32 builds a 32 bit linvst server lin-vst-server.so
Makefilelinvstserver64 builds a 64 bit linvst server lin-vst-server.so

Makefilelinvst builds the linvst.vst vst bundle.

The lin-vst-server.so file needs to be placed in the linvst.vst Resources folder after the linvst.vst bundle is built.

make -f Makefilelinvstserver32 or make -f Makefilelinvstserver64

unzip the linvst.vst file for use as a bundle template and then sudo make -f Makefilelinvst

Needs wineg++ and g++ or equivalent.

Needs the aeffect.h and affectx.h file from the Vst2 SDK to be placed in the projects folder.

aeffect.h needs 

<PRE><code>#if (defined(__GNUC__) && defined(__MACH__) && defined(__WINE__))
#undef TARGET_API_MAC_CARBON
#endif</code></PRE>

placed just before the #if TARGET_API_MAC_CARBON line to build.

The Vst2 SDK is available from Steinberg and is included in the Vst3 SDK https://www.steinberg.net/en/company/developers.html

LinVst-Mac can be compiled for 64 bits systems (Yosemite etc) with more recent Xcode/g++/wineg++ versions and more recent Wine 64 bit systems.

wineg++ uses -m32 for 32 bits and -m64 for 64 bits and also needs to be pointed to the right wine linking directory for 32 bits and 64 bits using -L/pathtolibs

The Wineskin 64 bit Wine bundle uses lib for 32 bit wine libraries and lib64 for 64 bit wine libraries, copy the Wineskin bundle folders (bin, lib, lib64, share) to /opt/local.

For example, use -L/opt/local/lib64 -L/opt/local/lib64/wine to link to the wine 64 bit libs and -I/opt/local/include/wine/windows for the headers.

wineg++ and winegcc and winecpp and winebuild need to be from an earlier wine build (or other workaround) otherwise there can be Xcode/gnu libc++ libstdc++ linking/include confusion https://github.com/LMMS/lmms/issues/698

gcc is available from Macports, Fink, Homebrew https://wiki.winehq.org/MacOS

64 bit and 32 bit development needs sudo port install gcc6 +universal




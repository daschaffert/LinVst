# LinVst*

-----

LinVst* may help with some Linux vst incompatability problems that occur in some Linux DAWS, such as gtk errors that occur in some Linux vst plugins (lsp vst plugins/amsynth vst plugin etc).

LinVst* loads a Linux vst and manages it within a Linux DAW.

-------

How to use,

linvst*.so gets renamed to pluginname*.so

For example,

For a linux vst plugin named Delay.so, linvst*.so would get renamed to Delay*.so

Delay*.so would then be used inside a Linux DAW (not Delay.so)

Delay.so and Delay*.so need to be kept in the same location after the renaming as they are then a matched pair.

To install,

Copy lin-vst-server to /usr/bin

convert(rename) linvst*.so to the pluginname.

linvst*convert can do the conversion automatically for multiple plugins.

-----------

The window embedded version works in Linux Reaper and Linux Bitwig.

The standalone window version works in Tracktion.

-----------

The lsp vst plugins need the lsp-plugins-vst-core.so file copied to /usr/lib/vst for plugin scanning to be successful.

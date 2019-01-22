[![Build Status](https://travis-ci.com/7FM/pvr.iptvsimple.svg?branch=master)](https://travis-ci.com/7FM/pvr.iptvsimple)

# IPTV Simple PVR w/ Recording Support
IPTV Live TV and Radio PVR client addon for [Kodi] (http://kodi.tv)

## Build instructions

### Linux

1. `git clone https://github.com/xbmc/xbmc.git`
2. `git clone https://github.com/gonzalo-hvega/pvr.iptvsimple`
3. `cd pvr.iptvsimple && mkdir build && cd build`
4. `cmake -DADDONS_TO_BUILD=pvr.iptvsimple -DADDON_SRC_PREFIX=../.. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../../xbmc/addons -DPACKAGE_ZIP=1 ../../xbmc/cmake/addons`
5. `make`

### Android

1. Read README.android from XBMC, will have to build complete XBMC and create APK. (NOT FOR BEGINERS!)
2. This will require you to clone bootstrap git and point to my git project or modify txt file in xbmc/project/cmake/addons/addons/pvr.iptvsimple to point to my git project

##### Useful links

* [Kodi's PVR user support] (http://forum.kodi.tv/forumdisplay.php?fid=167)
* [Kodi's PVR development support] (http://forum.kodi.tv/forumdisplay.php?fid=136)
* [Original Repository] (https://github.com/kodi-pvr/pvr.iptvsimple)
* [Gonzalo Vega's Repository] (https://github.com/gonzalo-hvega/pvr.iptvsimple)

##### Gonzalo Vega (gonzalo-hvega) modification

##### Features
1. Time shifting support
2. EPG timers support
3. Recordings folder integration
4. SMB file protocol support (requires FFMPEG binary built w/ SMB)
5. Resume playback & watched list
6. Records management (delete recordings from kodi interface)

##### Requirements
1. FFMPEG binary

##### Tested with
1. Ubuntu 16.04 LTS
2. Android Fire HD8 Armv7

##### Credits
Credit to the following authors/projects who I borrowed some ideas and/or code from:
1. Anton Fedchin, Official PVR IPTV Simple for Kodi
2. Radek Kubera, Branched Version of IPTV Simple with Recording (Basis of this Project)
3. DBViewer


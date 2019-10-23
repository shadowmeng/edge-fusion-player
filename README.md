![http://mpv.io/](https://raw.githubusercontent.com/mpv-player/mpv.io/master/source/images/mpv-logo-128.png)

## mpv

--------------


Based on mpv, I added vo_driver named multidirect3d which is used for supporting multiple display or projector.
and use grid to adjust intersection part between projectors, and set number of overlapped grid.

build steps


# Installing MSYS2
  Download an installer from https://msys2.github.io/

  Both the i686 and the x86_64 version of MSYS2 can build 32-bit and 64-bit mpv binaries when running on a 64-bit version of Windows, but the x86_64 version is preferred since the larger address space makes it less prone to fork() errors.

  Start a MinGW-w64 shell (mingw64.exe). Note: This is different from the MSYS2 shell that is started from the final installation dialog. You must close that shell and open a new one.

  For a 32-bit build, use mingw32.exe.
  
# Updating MSYS2

  To prevent errors during post-install, the MSYS2 core runtime must be updated separately.
  
# Download yasm
  http://yasm.tortall.net/Download.html
  rename yasm.exe and copy to /bin

# Check for core updates. If instructed, close the shell window and reopen it
# before continuing.
  pacman -Syu

# Update everything else
  pacman -Su
  pacman -S make
  pacman -S diffutils
  pacman -S msys2-w32api-runtime

# Installing mpv dependencies

# Install MSYS2 build dependencies and a MinGW-w64 compiler
pacman -S git python $MINGW_PACKAGE_PREFIX-{pkg-config,gcc}

# Here do not pull ffmpeg, we must use ffmpeg-mpv
pacman -S $MINGW_PACKAGE_PREFIX-{libjpeg-turbo,lua51,angleproject-git}

# Building and Install ffmpeg-mpv
git clone https://github.com/mpv-player/ffmpeg-mpv
make && make install

# Building mpv

git clone https://github.com/shadowmeng/edge-fusion-player.git && cd mpv
/usr/bin/python3 bootstrap.py

# Finally, compile and install mpv. Binaries will be installed to /mingw64/bin or /mingw32/bin.

/usr/bin/python3 waf configure CC=gcc.exe --check-c-compiler=gcc --prefix=$MSYSTEM_PREFIX

/usr/bin/python3 waf install

# Launch
enter build dir

cd build

./mpv --vo=multidirect3d xxx.mp4
 
we can press Ctrl key to switch between grid screen and frame screen.
In grid screen, you can drag control point with mouse to adjust the shape of grid.

![http://mpv.io/](https://github.com/shadowmeng/edge-fusion-player/blob/master/demo.png)
![http://mpv.io/](https://github.com/shadowmeng/edge-fusion-player/blob/master/demo1.png)

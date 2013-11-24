msc
===

Project moved from google code: https://code.google.com/p/msc-coder/

Project contains helper utilities for my Master's degree thesis project. For more details please visit:

http://ogorkis.net/education/msc/

Applying FFmpeg patch
---------------------

1. Copy msc.patch file to main ffmpeg directory
2. Run path command
```
patch -p1 msc.patch
```
3. Run configure script
```
./configure
```
4. Run make (-j5 flag to speed up building on multicore machines)
```
make -j5
```

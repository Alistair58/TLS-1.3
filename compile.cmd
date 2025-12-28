cd build
cmake .. -G "MinGW Makefiles" -DWINDOWS
mingw32-make 
move "TLS-1.3-Server.exe" ".."
move "TLS-1.3-Client.exe" ".."
cd .. 

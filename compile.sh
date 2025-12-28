cd build
cmake .. -D LINUX=1
make
mv TLS-1.3-Server ..
mv TLS-1.3-Client ..
cd .. 
cd build && \
cmake .. -DTEST=0 && \
make && \
mv TLS-1.3-Server .. && \
mv TLS-1.3-Client .. && \
cd .. 
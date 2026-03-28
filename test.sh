cd build && \
cmake .. -DTEST=1 && \
make && \
mv TLS-1.3-Test .. && \
cd .. && \
./TLS-1.3-Test
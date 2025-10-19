cmake_minimum_required(VERSION 3.13)

project(TLS-1.3-Client C)
project(TLS-1.3-Server C)

add_executable(TLS-1.3-Client
    src/client.c    
    src/aes.c 
    src/bigmaths.c 
    src/gcm.c 
    src/random.c 
    src/rsa.c 
    src/sha.c 
    src/x509.c 
    src/x25519.c
    src/structs.c
)

add_executable(TLS-1.3-Server
    src/server.c    
    src/aes.c 
    src/bigmaths.c 
    src/gcm.c 
    src/random.c 
    src/rsa.c 
    src/sha.c 
    src/x509.c 
    src/x25519.c
    src/structs.c
)

set(CMAKE_BUILD_TYPE Debug)

target_link_libraries(TLS-1.3-Client PRIVATE ws2_32)
target_link_libraries(TLS-1.3-Server PRIVATE ws2_32)
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
    src/base64.c
    src/der.c
    src/keystore.c
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
    src/base64.c
    src/der.c
    src/keystore.c
)

set(CMAKE_BUILD_TYPE Debug)
target_compile_options(TLS-1.3-Client PRIVATE
    )
target_compile_options(TLS-1.3-Server PRIVATE
    )

if(${WINDOWS})
    target_link_libraries(TLS-1.3-Client PRIVATE ws2_32)
    target_link_libraries(TLS-1.3-Server PRIVATE ws2_32)
elseif(${LINUX})
    find_package(X11 REQUIRED)
    target_include_directories(TLS-1.3-Client PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(TLS-1.3-Client PRIVATE ${X11_LIBRARIES} m)
    target_include_directories(TLS-1.3-Server PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(TLS-1.3-Server PRIVATE ${X11_LIBRARIES} m)
endif()
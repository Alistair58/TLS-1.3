cmake_minimum_required(VERSION 3.13)

project(TLS-1.3 LANGUAGES C CXX)



if(${TEST})
    add_executable(TLS-1.3-Test 
        test/pki/x509test.cpp
        test/pki/base64test.cpp
        test/pki/keystoretest.cpp
        test/crypto/rsatest.cpp
        test/other/argstest.cpp
        src/bigmaths/bigmaths.c 
        src/crypto/aes.c 
        src/crypto/gcm.c 
        src/crypto/params.c
        src/crypto/rsa.c 
        src/crypto/sha.c 
        src/crypto/x25519.c
        src/other/args.c
        src/other/random.c 
        src/pki/x509.c 
        src/pki/base64.c
        src/pki/der.c
        src/pki/keystore.c
    )
    set(CMAKE_BUILD_TYPE Debug)

    if(WIN32)
        target_link_libraries(TLS-1.3-Test PRIVATE ws2_32)
    elseif(UNIX)
        find_package(X11 REQUIRED)
        target_link_libraries(TLS-1.3-Test PRIVATE ${X11_LIBRARIES} m)
        target_include_directories(TLS-1.3-Test PRIVATE ${X11_INCLUDE_DIR})
    endif()

    include(FetchContent)
    FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    
    target_link_libraries(TLS-1.3-Test PRIVATE
        GTest::gtest_main
        GTest::gmock
    )
    

    include(GoogleTest)
    gtest_discover_tests(TLS-1.3-Test)

    target_compile_options(TLS-1.3-Test PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -g
        -O0
    )

    target_link_options(TLS-1.3-Test PRIVATE
        -fsanitize=address,undefined
    )

    target_include_directories(TLS-1.3-Test PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/test
    )
else()
    add_executable(TLS-1.3-Client
        src/main/client.c
        src/bigmaths/bigmaths.c 
        src/crypto/aes.c 
        src/crypto/gcm.c 
        src/crypto/params.c
        src/crypto/rsa.c 
        src/crypto/sha.c 
        src/crypto/x25519.c
        src/other/args.c
        src/other/random.c 
        src/pki/x509.c 
        src/pki/base64.c
        src/pki/der.c
        src/pki/keystore.c
    )

    add_executable(TLS-1.3-Server  
        src/main/server.c
        src/bigmaths/bigmaths.c 
        src/crypto/aes.c 
        src/crypto/gcm.c 
        src/crypto/params.c
        src/crypto/rsa.c 
        src/crypto/sha.c 
        src/crypto/x25519.c
        src/other/args.c
        src/other/random.c 
        src/pki/x509.c 
        src/pki/base64.c
        src/pki/der.c
        src/pki/keystore.c
    )


    set(CMAKE_BUILD_TYPE Debug)

    if(WIN32)
        target_link_libraries(TLS-1.3-Client PRIVATE ws2_32)
        target_link_libraries(TLS-1.3-Server PRIVATE ws2_32)
    elseif(UNIX)
        find_package(X11 REQUIRED)
        target_include_directories(TLS-1.3-Client PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(TLS-1.3-Client PRIVATE ${X11_LIBRARIES} m)
        target_include_directories(TLS-1.3-Server PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(TLS-1.3-Server PRIVATE ${X11_LIBRARIES} m)
    endif()
endif()
cmake_minimum_required(VERSION 3.13)

project(TLS-1.3 LANGUAGES C CXX)



if(${TEST})
    add_executable(TLS-1.3-Test 
        test/x509test.cpp
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
        -O1
    )

    target_link_options(TLS-1.3-Test PRIVATE
        -fsanitize=address,undefined
    )

    target_include_directories(TLS-1.3-Test PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/test
    )
else()
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
cmake_minimum_required(VERSION 3.5)

project(rtpe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rtpe main.cpp)


target_sources(rtpe
    PUBLIC
        misc_tools.h
        mediagatewey.h
        mediagatewey.cpp
)

target_link_libraries(rtpe
    sioclient
    avformat
    avcodec
    avutil
    pjsua2-x86_64-unknown-linux-gnu
    pjsua-x86_64-unknown-linux-gnu
    pjsip-ua-x86_64-unknown-linux-gnu
    pjsip-simple-x86_64-unknown-linux-gnu
    pjsip-x86_64-unknown-linux-gnu
    pjmedia-codec-x86_64-unknown-linux-gnu
    pjmedia-x86_64-unknown-linux-gnu
    pjmedia-videodev-x86_64-unknown-linux-gnu
    pjmedia-audiodev-x86_64-unknown-linux-gnu
    pjmedia-x86_64-unknown-linux-gnu
    pjnath-x86_64-unknown-linux-gnu
    pjlib-util-x86_64-unknown-linux-gnu
    srtp-x86_64-unknown-linux-gnu
    resample-x86_64-unknown-linux-gnu
    gsmcodec-x86_64-unknown-linux-gnu
    speex-x86_64-unknown-linux-gnu
    ilbccodec-x86_64-unknown-linux-gnu
    g7221codec-x86_64-unknown-linux-gnu
    yuv-x86_64-unknown-linux-gnu
    webrtc-x86_64-unknown-linux-gnu
    pj-x86_64-unknown-linux-gnu
    openh264 opencore-amrnb opencore-amrwb
    ssl crypto
    uuid  pthread  asound  avformat avcodec avutil
)

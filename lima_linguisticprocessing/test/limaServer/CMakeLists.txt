

########### next target ###############

include_directories( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/test ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

SET(limaserver_SRCS
   LimaServer.cpp
   main.cpp
   analysisthread.cpp
)

set (limaserver_MOC_HEADERS 
    LimaServer.h
    analysisthread.h )


qt5_wrap_cpp (limaserver_MOC_OUTFILES ${limaserver_MOC_HEADERS})

# qt5_add_resources(limaserver amoseserver.qrc)

# add_executable(limaserver ${amoseserver_SRCS} ${amoseserver_QRCS} ${amoseserver_MOC_OUTFILES})
add_executable(limaserver ${limaserver_SRCS} ${limaserver_MOC_OUTFILES})

target_link_libraries(limaserver
    lima-common
    lima-linguisticprocessing
    ${QHTTPSERVER_NAMES}
    ${QT_LIBRARIES}
    ${Boost_LIBRARIES})

install(TARGETS limaserver DESTINATION bin)

########### next target ###############

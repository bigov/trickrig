#
# TrickRig udp server based in ENet lib.
#

# серверная часть
SET( SRV_SRC "${CMAKE_SOURCE_DIR}/server/enet/server.cpp" )
# управление/контроль
SET( CTRL_SRC "${CMAKE_SOURCE_DIR}/server/enet/ctrl.cpp" )

find_package( PkgConfig REQUIRED )
pkg_check_modules( ENET REQUIRED libenet )

add_executable( server ${SRV_SRC} )
target_link_libraries( server ${ENET_LIBRARIES} )

add_executable( ctrl ${CTRL_SRC} )
target_link_libraries( ctrl ${ENET_LIBRARIES} )

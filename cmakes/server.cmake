#
# TrickRig udp server based in ENet lib.
#

# серверная часть
SET( SRV_SRC
  "${CMAKE_SOURCE_DIR}/server/server.cpp"
  "${CMAKE_SOURCE_DIR}/server/enetw.cpp"
)

# управление/контроль
SET( CTRL_SRC
  "${CMAKE_SOURCE_DIR}/server/ctrl.cpp"
  "${CMAKE_SOURCE_DIR}/server/enetw.cpp"
)

find_package( PkgConfig REQUIRED )
pkg_check_modules( ENET REQUIRED libenet )

# еще есть параметры обнаруженного пакета:
#  ${ENET_LIBRARY_DIRS} и ${ENET_INCLUDE_DIRS}

add_executable( server ${SRV_SRC} )
target_link_libraries( server ${ENET_LIBRARIES} )

add_executable( ctrl ${CTRL_SRC} )
target_link_libraries( ctrl ${ENET_LIBRARIES} )

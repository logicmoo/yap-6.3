
set ( TRIES_SOURCES
  core_tries.c
  base_tries.c      
  tries.c
  )

add_library (tries SHARED ${TRIES_SOURCES})

target_link_libraries(tries libYap)

set_target_properties (tries PROPERTIES PREFIX "")

install(TARGETS  tries
  LIBRARY DESTINATION ${dlls} 
  ARCHIVE DESTINATION ${dlls})


set ( ITRIES_SOURCES
  core_tries.c
  base_itries.c      
  itries.c
  )

add_library (itries SHARED ${ITRIES_SOURCES})

target_link_libraries(itries libYap)

set_target_properties (itries PROPERTIES PREFIX "")

install(TARGETS  itries
  LIBRARY DESTINATION ${dlls}
  ARCHIVE DESTINATION ${dlls} )


#  CORE
# --------------------------------------------------

file(GLOB core_sources core/*.cc core/io/*.cc)
file(GLOB core_headers core/*.h core/io/*.h)

add_library(core STATIC ${core_sources} ${core_headers})

target_link_libraries(core INTERFACE ${CONAN_LIBS})

#  RUDP
# --------------------------------------------------

file(GLOB rudp_sources rudp/*.cc)
file(GLOB rudp_headers rudp/*.h)
file(GLOB rudp_command_sources rudp/command/*.cc)
file(GLOB rudp_command_headers rudp/command/*.h)
file(GLOB rudp_dispatch_sources rudp/dispatch/*.cc)
file(GLOB rudp_dispatch_headers rudp/dispatch/*.h)
file(GLOB rudp_peer_headers rudp/peer/*.h)
file(GLOB rudp_peer_sources rudp/peer/*.cc)
file(GLOB rudp_protocol_sources rudp/protocol/*.cc)
file(GLOB rudp_protocol_headers rudp/protocol/*.h)

add_library(rudp STATIC
        ${rudp_sources} ${rudp_headers}
        ${rudp_command_sources} ${rudp_command_headers}
        ${rudp_dispatch_sources} ${rudp_dispatch_headers}
        ${rudp_peer_sources} ${rudp_peer_headers}
        ${rudp_protocol_sources} ${rudp_protocol_headers})

target_link_libraries(rudp INTERFACE core)

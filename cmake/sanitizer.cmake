include(CheckCXXSourceCompiles)

set(SAN_FLAGS "-fno-omit-frame-pointer -fsanitize=undefined -fsanitize=leak -fsanitize=address -fsanitize-address-use-after-scope")

message(STATUS "SAN_FLAGS ${SAN_FLAGS}.")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}  ${SAN_FLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SAN_FLAGS}")
SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${SAN_FLAGS}")

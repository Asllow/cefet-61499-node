file(GLOB SRCS "src/*.c")

list(REMOVE_ITEM SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/lua.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/luac.c")

idf_component_register(
    SRCS ${SRCS} 
    INCLUDE_DIRS "src"
)
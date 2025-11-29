cmake_minimum_required(VERSION 3.10)
project(stencil_compiler C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include_directories(include)

# -- Compiling stencil assembly files

# EARLIER VERSION .. BB told to comment it out.
# file(GLOB STENCIL_ASM_FILES
#     stencils/*.s
# )
# 
# set(STENCIL_OBJ_FILES "")
# 
# foreach(ASM_FILE IN LISTS STENCIL_ASM_FILES)
#     get_filename_component(FILE_NAME ${ASM_FILE} NAME_WE)
# 
#     set(OUT_OBJ ${CMAKE_CURRENT_BINARY_DIR}/stencils/${FILE_NAME}.o)
#     add_custom_command(
#         OUTPUT ${OUT_OBJ}
#         COMMAND ${CMAKE_C_COMPILER} -c ${ASM_FILE} -o ${OUT_OBJ}
#         DEPENDS ${ASM_FILE}
#         COMMENT "Assembling stencil: ${FILE_NAME}.s → ${FILE_NAME}.o"
#     )
# 
#     list(APPEND STENCIL_OBJ_FILES ${OUT_OBJ})
# endforeach()
# 
# add_custom_target(build_stencils ALL
#     DEPENDS ${STENCIL_OBJ_FILES}
# )

# -- Main compiler sources

set(SRC_FILES
    src/codebuffer.cpp
    src/irgen.cpp
    src/irinterp.cpp
    src/lexer.cpp
    src/main.cpp
    src/parser.cpp
    src/stencil.cpp
)

if(WIN32)
    list(APPEND SRC_FILES src/codegen_simple_windows.cpp)
else()
    list(APPEND SRC_FILES src/codegen_simple.cpp)
endif()


# Stencil generation
file(GLOB STENCIL_SRC stencils/*.s)
set(STENCIL_BINS "")

foreach(F IN LISTS STENCIL_SRC)
    get_filename_component(BASE ${F} NAME_WE)

    set(OUT_OBJ ${CMAKE_CURRENT_SOURCE_DIR}/stencils/${BASE}.o)
    set(OUT_BIN ${CMAKE_CURRENT_SOURCE_DIR}/stencils/${BASE}.bin)

    # Compile .s -> .o
    add_custom_command(
        OUTPUT ${OUT_OBJ}
        COMMAND ${CMAKE_C_COMPILER} -c ${F} -o ${OUT_OBJ}
        DEPENDS ${F}
        COMMENT "Compiling ${BASE}.s -> ${BASE}.o"
        VERBATIM
    )

    # Convert .o -> .bin
    add_custom_command(
        OUTPUT ${OUT_BIN}
        COMMAND objcopy -O binary -j .text ${OUT_OBJ} ${OUT_BIN}
        DEPENDS ${OUT_OBJ}
        COMMENT "Converting ${BASE}.o -> ${BASE}.bin"
        VERBATIM
    )

    list(APPEND STENCIL_BINS ${OUT_BIN})
endforeach()

# Target to generate all stencils
add_custom_target(generate_stencils ALL DEPENDS ${STENCIL_BINS})

# -- Main Compiler Target:
add_executable(stencilc ${SRC_FILES})
add_dependencies(stencilc generate_stencils)


add_library(
        core
        Error.cpp
        Matrix.cpp
        Solver.cpp
        ../include/Error.h
        ../include/Matrix.h
        ../include/Solver.h
)

target_include_directories(core PUBLIC ../include)

add_executable(app
        main.cpp
)

set_target_properties(app PROPERTIES ENABLE_EXPORTS OFF)

target_link_libraries(app PRIVATE core)


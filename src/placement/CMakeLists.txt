set(header_file lab3.hpp input.hpp output.hpp graph_partition.hpp)

# include directories
target_include_directories(${PROJECT_NAME} 
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)


# install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/${PROJECT_NAME} DESTINATION "include"
    FILES_MATCHING 
        PATTERN "*.hpp"
        PATTERN "*.hh"
        PATTERN "*.h"
)

# add sources
target_sources(${PROJECT_NAME} 
    INTERFACE
    ${header_file}
)




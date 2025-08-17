# Create the communicator library
add_library(communicator
    tcp_communicator.cpp
    rdma_communicator.cpp
)

target_include_directories(communicator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IBVERBS_INCLUDE_DIRS}
)

target_link_libraries(communicator PUBLIC
    ${IBVERBS_LIBRARIES}
)

target_compile_options(communicator PRIVATE
    -Wall
    -Wextra
    -g
)

# Define the headers
set(HEADER_FILES
    communicator.h
    tcp_communicator.h
    rdma_communicator.h
)

# Install headers
install(FILES ${HEADER_FILES} DESTINATION include/communicator)

# Install the library
install(TARGETS communicator
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
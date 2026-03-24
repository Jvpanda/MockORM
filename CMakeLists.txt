add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/sqlite3)

set(ABS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/AbsDatabase.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/Queries.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/AbsDatabase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Queries.cpp
)

add_library(AbsDatabase STATIC ${ABS_SOURCES})

target_include_directories(AbsDatabase
    PUBLIC
        sqlite3
        ${CMAKE_CURRENT_SOURCE_DIR}/headers
)

target_link_libraries(AbsDatabase
    PUBLIC
    sqlite3
)

add_library(sqlite3 STATIC ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.c)

target_include_directories(sqlite3
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(sqlite3
    PRIVATE
        SQLITE_THREADSAFE=1
        SQLITE_ENABLE_FTS5
        SQLITE_ENABLE_JSON1
)

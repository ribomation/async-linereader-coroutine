
add_library(sync-thread STATIC)
target_sources(sync-thread PRIVATE
    lib/file-reader.hxx
    lib/file-reader.cxx
)
target_include_directories(sync-thread PUBLIC lib)


add_executable(read-several-files )
target_sources(read-several-files PRIVATE
        app/read-several-files.cxx
)
target_link_libraries(read-several-files PRIVATE sync-thread)




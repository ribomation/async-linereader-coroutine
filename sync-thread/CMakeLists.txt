add_library(sync-file-reader STATIC
        lib/file-reader.hxx
        lib/file-reader.cxx
)
target_include_directories(sync-file-reader PUBLIC lib)


add_executable(sync-word-count
        app/sync-word-count.cxx
)
target_link_libraries(sync-word-count PRIVATE sync-file-reader)

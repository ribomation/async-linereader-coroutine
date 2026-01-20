add_library(coro-runtime STATIC
        lib/coro-runtime.hxx
        lib/coro-runtime.cxx
        lib/coro-scheduler.hxx
        lib/coro-scheduler.cxx
        lib/task-pool.hxx
        lib/task-pool.cxx
        lib/coro-task.hxx
        lib/coro-task.cxx
        lib/file-reader.hxx
        lib/file-reader.cxx
)
target_include_directories(coro-runtime PUBLIC lib)

add_executable(async-word-count
        app/async-word-count.cxx
)
target_link_libraries(async-word-count coro-runtime)


find_program(CLANG_FORMAT NAMES clang-format)
if(CLANG_FORMAT)
    message(STATUS "which clang-format: ${CLANG_FORMAT}")
endif()

function(get_tree_nodes _prefix _nodes)
    set(nodes)
    file(GLOB files ${_prefix}/*)
    list(SORT files)
    foreach(file ${files})
        get_filename_component(node ${file} NAME_WE)
        list(APPEND nodes ${node})
    endforeach()
    list(REMOVE_DUPLICATES nodes)
    set(${_nodes} ${nodes} PARENT_SCOPE)
endfunction()

function(test _target)

    get_target_property(target_type ${_target} TYPE)

    if(target_type STREQUAL "EXECUTABLE"
    OR target_type STREQUAL "STATIC_LIBRARY"
    OR target_type STREQUAL "SHARED_LIBRARY")
        target_weak_compile_options(${_target} PRIVATE -W)
        target_weak_compile_options(${_target} PRIVATE -Wextra)
        target_weak_compile_options(${_target} PRIVATE -Wpedantic)

        if(NOT MSVC)
            target_weak_compile_options(${_target} PRIVATE -Wall)
        endif()

        if(CLANG_TIDY AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set_target_properties(${_target} PROPERTIES
                CXX_CLANG_TIDY "${CLANG_TIDY};-checks=-clang-diagnostic-unused-command-line-argument"
            )
        endif()

        if(CPPCHECK AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            set_target_properties(${_target} PROPERTIES
                CXX_CPPCHECK "${CPPCHECK};--enable=warning,performance,portability;--template=gcc"
            )
        endif()
    endif()

    set(driver ${_target})
    if(target_type STREQUAL "EXECUTABLE")
        add_custom_target(${_target}.run
            COMMAND ${_target}
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        )
        add_dependencies(${_target}.run ${_target})
        set(driver ${_target}.run)
    endif()

    add_test(NAME ${_target}
        COMMAND ${CMAKE_COMMAND} --build . --config $<CONFIG> --target ${driver}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    )
endfunction()


function(test_formatting _root _lib _prefix)
    set(options)
    set(one_value_args)
    set(multi_value_args EXCLUDE)
    cmake_parse_arguments(ARGS "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ARGS_EXCLUDE)
        set(ARGS_EXCLUDE "^$")
    else()
        string(REPLACE ";" "|" ARGS_EXCLUDE "${ARGS_EXCLUDE}")
    endif()

    if(NOT IS_ABSOLUTE ${_prefix})
        set(_prefix "${CMAKE_CURRENT_SOURCE_DIR}/${_prefix}")
    endif()

    if(NOT IS_DIRECTORY ${_prefix} OR NOT EXISTS ${_prefix})
        message(FATAL_ERROR "'${_prefix}' is not a valid directory.")
    endif()

    if(NOT TARGET ${_root})
        add_custom_target(${_root})
    endif()

    get_tree_nodes(${_prefix} nodes)
    foreach(node ${nodes})
        if(node MATCHES ${ARGS_EXCLUDE})
            continue()
        endif()

        set(target ${_root}.${node})
        set(formatted "${CMAKE_CURRENT_BINARY_DIR}/${target}.hpp")
        set(node "${_prefix}/${node}")
        set(original "${node}.hpp")
        if(EXISTS ${original})
            file(RELATIVE_PATH comment ${PROJECT_SOURCE_DIR} ${original})
            set(comment "checking whether '${comment}' is well formatted...")

            add_custom_target(${target}
                COMMAND ${CLANG_FORMAT} ${original} > ${formatted}
                COMMAND ${CMAKE_COMMAND} -E compare_files ${original} ${formatted}
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                COMMENT ${comment}
            )
        endif()

        if(IS_DIRECTORY ${node})
            test_formatting(${target} ${_lib} ${node} ${ARGN})
        endif()

        if(TARGET ${target})
            add_dependencies(${_root} ${target})
            test(${target})
        endif()
    endforeach()
endfunction()  

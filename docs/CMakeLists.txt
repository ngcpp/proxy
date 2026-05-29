project(msft_proxy_docs)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(_GEN_SCRIPT "${CMAKE_SOURCE_DIR}/tools/gen_doc_examples.py")
set(_EXTRACT_SCRIPT "${CMAKE_SOURCE_DIR}/tools/extract_example_code.py")
set(EXAMPLES_DIR "${CMAKE_BINARY_DIR}/examples_from_docs")
file(MAKE_DIRECTORY "${EXAMPLES_DIR}")

# Re-run configure when any doc Markdown file or the generator script changes.
file(GLOB_RECURSE _MD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.md")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_MD_FILES} "${_GEN_SCRIPT}")

# Generate the example list at configure time and capture JSON to a variable.
execute_process(
    COMMAND ${Python3_EXECUTABLE} "${_GEN_SCRIPT}" --format json
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE _doc_examples_json
    ERROR_VARIABLE _gen_error
    RESULT_VARIABLE _gen_result
)
if(_gen_result)
    message(FATAL_ERROR "gen_doc_examples.py failed:\n${_gen_error}")
endif()

# Parse JSON array: [{md: <path>, deps: [<cmake-target>, ...]}, ...]
string(JSON _len LENGTH "${_doc_examples_json}")
math(EXPR _last "${_len} - 1")
foreach(i RANGE 0 ${_last})
    string(JSON _md GET "${_doc_examples_json}" ${i} "md")
    string(JSON _n_deps LENGTH "${_doc_examples_json}" ${i} "deps")

    set(_dep_names "")
    if(_n_deps GREATER 0)
        math(EXPR _d_last "${_n_deps} - 1")
        foreach(j RANGE 0 ${_d_last})
            string(JSON _dep GET "${_doc_examples_json}" ${i} "deps" ${j})
            list(APPEND _dep_names "${_dep}")
        endforeach()
    endif()

    string(REPLACE "/" "_" _suffix "${_md}")
    string(REPLACE ".md" "" _suffix "${_suffix}")
    set(_cpp "${EXAMPLES_DIR}/${_suffix}.cpp")

    add_custom_command(
        OUTPUT "${_cpp}"
        COMMAND ${Python3_EXECUTABLE} "${_EXTRACT_SCRIPT}"
                "${CMAKE_CURRENT_SOURCE_DIR}/${_md}" "${_cpp}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_md}" "${_EXTRACT_SCRIPT}"
        VERBATIM
    )
    set_source_files_properties("${_cpp}" PROPERTIES GENERATED TRUE)

    add_executable(example_${_suffix} "${_cpp}")
    target_link_libraries(example_${_suffix} PRIVATE msft_proxy4::proxy ${_dep_names})
    target_compile_options(example_${_suffix} PRIVATE ${PROXY_BUILD_FLAGS})
    add_test(NAME example_${_suffix} COMMAND example_${_suffix})
endforeach()

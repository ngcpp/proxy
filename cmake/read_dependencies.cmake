# Read a JSON dependency registry and expose PROXY_<NAME>_URL / PROXY_<NAME>_SHA256 in the caller's scope for FetchContent_Declare.

function(proxy_read_dependencies _json_path)
  file(READ "${_json_path}" _json)
  set_property(
    DIRECTORY
    APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS "${_json_path}"
  )
  string(JSON _count LENGTH "${_json}")
  math(EXPR _last "${_count} - 1")
  foreach(_i RANGE 0 ${_last})
    string(JSON _name GET "${_json}" ${_i} "name")
    string(JSON _url GET "${_json}" ${_i} "url")
    string(JSON _sha GET "${_json}" ${_i} "sha256")
    string(TOUPPER "${_name}" _name)
    set("PROXY_${_name}_URL" "${_url}" PARENT_SCOPE)
    set("PROXY_${_name}_SHA256" "${_sha}" PARENT_SCOPE)
  endforeach()
endfunction()

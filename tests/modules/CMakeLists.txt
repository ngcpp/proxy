if(PROXY_BUILD_MODULES)
  target_sources(ngcpp_proxy_tests PRIVATE main.cpp)
  target_sources(
    ngcpp_proxy_tests
    PRIVATE FILE_SET CXX_MODULES FILES foo.ixx impl.ixx
  )
  target_link_libraries(ngcpp_proxy_tests PRIVATE ngcpp_proxy5::proxy_module)
endif()

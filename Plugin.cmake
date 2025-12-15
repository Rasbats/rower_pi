# ~~~
# Summary:      Local, non-generic plugin setup
# Copyright (c) 2020-2021 Mike Rossiter
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


# -------- Options ----------

set(OCPN_TEST_REPO
    "mike-rossiter/rower-alpha"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "mike-rossiter/rower-beta"
    CACHE STRING
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "mike-rossiter/rower-prod"
    CACHE STRING
    "Default repository for tagged builds not matching 'beta'"
)

#
#
# -------  Plugin setup --------
#
set(PKG_NAME rower_pi)
set(PKG_VERSION  1.0.0)
set(PKG_PRERELEASE "")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME rower)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME rower) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "Countdown/Duration Timer")
set(PKG_DESCRIPTION [=[
A simple timer.
]=])

set(PKG_AUTHOR "Mike Rossiter")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/Rasbats/rower_pi)
set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/rower.html)

set(SRC
    src/rower_pi.h
    src/rower_pi.cpp
    src/icons.h
    src/icons.cpp
    src/rowergui.h
    src/rowergui.cpp
    src/rowergui_impl.cpp
    src/rowergui_impl.h
)

set(PKG_API_LIB api-18)  #  A directory in libs/ e. g., api-17 or api-16

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.
endmacro ()

macro(add_plugin_libraries)
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/plugin_dc")
  target_link_libraries(${PACKAGE_NAME} ocpn::plugin-dc)

  # The wxsvg library enables SVG overall in the plugin
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/wxsvg")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxsvg)

endmacro ()

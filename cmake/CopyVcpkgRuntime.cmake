# Copies vcpkg runtime DLLs into the game output directory. Static triplets
# and dependency sets without DLLs naturally result in no copied files.
if(NOT DEFINED AVP_VCPKG_TRIPLET_DIR OR NOT DEFINED AVP_DEST_DIR)
    message(STATUS "NakedAVP: vcpkg DLL copy skipped (paths were not provided)")
    return()
endif()

if(AVP_CONFIG STREQUAL "Debug" AND EXISTS "${AVP_VCPKG_TRIPLET_DIR}/debug/bin")
    set(_avp_vcpkg_bin "${AVP_VCPKG_TRIPLET_DIR}/debug/bin")
else()
    set(_avp_vcpkg_bin "${AVP_VCPKG_TRIPLET_DIR}/bin")
endif()

if(EXISTS "${_avp_vcpkg_bin}")
    file(GLOB _avp_vcpkg_dlls LIST_DIRECTORIES FALSE "${_avp_vcpkg_bin}/*.dll")
    foreach(_avp_dll IN LISTS _avp_vcpkg_dlls)
        get_filename_component(_avp_dll_name "${_avp_dll}" NAME)
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${_avp_dll}" "${AVP_DEST_DIR}/${_avp_dll_name}"
            RESULT_VARIABLE _avp_copy_result
        )
        if(NOT _avp_copy_result EQUAL 0)
            message(FATAL_ERROR "Failed to copy vcpkg runtime DLL: ${_avp_dll_name}")
        endif()
    endforeach()
    message(STATUS "NakedAVP: copied vcpkg runtime DLLs from ${_avp_vcpkg_bin} to ${AVP_DEST_DIR}")
endif()

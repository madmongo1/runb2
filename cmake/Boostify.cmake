if (BoostifyIncluded)
    return()
    endif()
set(BoostifyIncluded 1)

macro(Boostify target)
    set(_boostify_components)
    set(_bostify_libs)

    foreach(_boostify_lib ${ARGN})
        if (NOT _boostify_components)
            set(_boostify_components COMPONENTS)
        endif()
        list(APPEND _boostify_components ${_boostify_lib})
        list(APPEND _boostify_libs "Boost::${_boostify_lib}")
    endforeach()

    hunter_add_package(Boost ${_boostify_components})
    find_package(Boost REQUIRED ${_boostify_components})

    foreach(_boostify_lib ${ARGN})
        list(APPEND _boostify_libs "Boost::${_boostify_lib}")
    endforeach()
    target_link_libraries(${target} PUBLIC ${_boostify_libs})

endmacro()

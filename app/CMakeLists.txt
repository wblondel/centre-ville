# Application Centre-Ville (Qt 6 Widgets) : exécutable cvw
#
# Phases 4 et 5 de PLAN_PORTAGE.md. Les fiches sont décrites par les .ui
# générés par tools/dfm2ui.py (app/ui) ; app/vcl reproduit le sous-ensemble de
# la VCL utilisé par le code des fiches.

if(CV_APP STREQUAL "ON")
  find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
else()
  find_package(Qt6 6.5 QUIET COMPONENTS Widgets)
  if(NOT Qt6_FOUND)
    message(STATUS "Qt 6 introuvable : l'application cvw n'est pas compilée (CV_APP=ON pour l'exiger)")
    return()
  endif()
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

# Couche VCL au-dessus de Qt
add_library(cvvcl STATIC
  vcl/vcl.h vcl/vcl.cpp
  vcl/proprietes.h
  vcl/graphiques.h vcl/graphiques.cpp
  vcl/listeimages.h vcl/listeimages.cpp
  vcl/actions.h vcl/actions.cpp
  vcl/controles.h vcl/controles.cpp
  vcl/fiche.h vcl/fiche.cpp
)
target_include_directories(cvvcl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/vcl
                                        ${CMAKE_CURRENT_SOURCE_DIR}/ressources)
target_link_libraries(cvvcl PUBLIC cvengine Qt6::Widgets)

if(MSVC)
  target_compile_options(cvvcl PUBLIC /W3)
else()
  target_compile_options(cvvcl PUBLIC -fsigned-char -Wall)
endif()

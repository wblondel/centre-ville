# Application Centre-Ville (Qt 6 Widgets) : exécutable cvw
#
# Phases 4 et 5 de PLAN_PORTAGE.md. Les fiches sont décrites par les .ui
# générés par tools/dfm2ui.py (app/ui) ; app/vcl reproduit le sous-ensemble de
# la VCL utilisé par le code des fiches.

# Qt 6.8.3 et antérieurs (FindWrapOpenGL) lient le framework AGL, retiré des
# SDK récents de macOS : AGL reste dans /System/Library/Frameworks, où
# find_library(WrapOpenGL_AGL) le trouve, mais l'édition des liens se fait
# avec le SDK, qui ne l'a plus (« framework 'AGL' not found »). Rien n'utilise
# AGL : on lui substitue toujours le framework OpenGL (les versions suivantes
# de Qt ne lient plus AGL et ignorent cette variable).
if(APPLE AND NOT DEFINED CACHE{WrapOpenGL_AGL})
  find_library(CV_FRAMEWORK_OPENGL OpenGL)
  set(WrapOpenGL_AGL "${CV_FRAMEWORK_OPENGL}" CACHE FILEPATH "AGL retiré des SDK de macOS : framework OpenGL à la place")
endif()

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

# Fiches : port de f_simul (phase 5) et fiches secondaires (stubs du sprint 2)
set(CV_FICHES
  f_afdistab f_afdistat f_afdistfp f_afdistft f_afdistp f_afdistpv f_carfours f_deff f_deffp
  f_defn f_defv f_defvp f_dimzone f_epidemie f_genres f_infbus f_infptn f_inftaxi f_inftram
  f_infveh f_infvehlib f_prmbus f_prmsim f_prmtaxi f_prmtram f_prmvehlib f_propos f_rech
  f_simul f_stats f_statsat f_statsc f_statsv
)
set(CV_SOURCES_FICHES)
foreach(fiche IN LISTS CV_FICHES)
  list(APPEND CV_SOURCES_FICHES ${fiche}.cpp ${fiche}.h ui/${fiche}.ui ui/${fiche}_composants.h)
endforeach()

# Fiches et démarrage : bibliothèque commune à cvw et aux tests de l'interface
add_library(cvwfiches STATIC
  application.cpp application.h
  base.cpp base.h
  grille/GrilleSimulation.cpp grille/GrilleSimulation.h
  ${CV_SOURCES_FICHES}
)
target_include_directories(cvwfiches PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ui ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cvwfiches PUBLIC cvvcl)
if(MSVC)
  target_compile_options(cvwfiches PRIVATE /bigobj)
endif()

# Les ressources (images) sont compilées dans chaque exécutable : une
# bibliothèque statique ne les enregistrerait pas sans Q_INIT_RESOURCE
add_executable(cvw WIN32 MACOSX_BUNDLE
  main.cpp
  ressources/images.qrc
)
target_link_libraries(cvw PRIVATE cvwfiches)
set_target_properties(cvw PROPERTIES
  OUTPUT_NAME cvw
  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
  MACOSX_BUNDLE_BUNDLE_NAME "Centre-Ville"
  MACOSX_BUNDLE_GUI_IDENTIFIER "fr.patquoi.centre-ville"
  MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
)

# Pile : le calcul récursif des distances (voie::CalculeDistance*) demande
# jusqu'à 512 Ko avec Clang, davantage en Debug MSVC ; l'original tournait avec
# 1 Mo. On réserve 16 Mo, comme les tests (sous Linux, la pile du fil
# principal suit « ulimit -s », 8 Mo par défaut).
if(MSVC)
  target_link_options(cvw PRIVATE /STACK:16777216)
elseif(APPLE)
  target_link_options(cvw PRIVATE -Wl,-stack_size,0x1000000)
elseif(MINGW)
  target_link_options(cvw PRIVATE -Wl,--stack,16777216)
endif()

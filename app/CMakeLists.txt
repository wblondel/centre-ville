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

# Numéro de version : project(VERSION) du CMakeLists.txt racine ; les numéros
# absents (6.0.0 n'a pas de 4e numéro) valent 0
foreach(partie PATCH TWEAK)
  set(CV_VERSION_${partie} "${PROJECT_VERSION_${partie}}")
  if("${CV_VERSION_${partie}}" STREQUAL "")
    set(CV_VERSION_${partie} 0)
  endif()
endforeach()

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
  vcl/controles.h vcl/controles.cpp vcl/grilletextes.cpp
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
# Version affichée par « À propos » (ImageVersion, LabelSousVersion) : project(VERSION)
target_compile_definitions(cvwfiches PUBLIC
  CV_VERSION_MAJEURE=${PROJECT_VERSION_MAJOR}
  CV_VERSION_MINEURE=${PROJECT_VERSION_MINOR}
  CV_VERSION_CORRECTIVE=${CV_VERSION_PATCH}
)
if(MSVC)
  target_compile_options(cvwfiches PRIVATE /bigobj)
endif()

# Les ressources (images) sont compilées dans chaque exécutable : une
# bibliothèque statique ne les enregistrerait pas sans Q_INIT_RESOURCE.
# Mémoire remise à zéro par operator new, comme pour les tests du moteur : le
# moteur d'origine laissait des champs non initialisés (centre_ville::FeuP
# sans feux piétons, NbVehlibs avant la v5.3... voir engine/README.md) ; sans
# cela, poser le premier feu de piétons libérait un pointeur indéterminé. Le
# moteur n'en dépend plus depuis la 6.0.1 (tests cvtests_motif), mais les
# fiches viennent de la VCL, qui remettait à zéro les champs de tout objet
# dérivé de TObject : la remise à zéro est gardée pour elles.
add_executable(cvw WIN32 MACOSX_BUNDLE
  main.cpp
  ressources/images.qrc
  ${PROJECT_SOURCE_DIR}/tests/support/allocation.cpp
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

# Icônes et version (phase 8). Le numéro de version n'est écrit qu'une fois :
# project(VERSION) du CMakeLists.txt racine (Info.plist, ressources de version
# de cvw.exe, d'où l'installeur Inno Setup le lit, noms des paquets).
if(WIN32)
  # Icon_0 (application) et Icon_1 (fichiers .cvw), comme src/cvw_resources.rc
  set(CV_ICONE_APPLICATION ${PROJECT_SOURCE_DIR}/res/ico/cvw.ico)
  set(CV_ICONE_DOCUMENT ${PROJECT_SOURCE_DIR}/res/ico/Newdoc_cvw.ico)
  configure_file(cvw.rc.in ${CMAKE_CURRENT_BINARY_DIR}/cvw.rc @ONLY)
  target_sources(cvw PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/cvw.rc)
elseif(APPLE)
  # Paquet Centre-Ville.app ; icônes .icns produites depuis les PNG de
  # app/ressources/icones (tools/extrait_icones.py). Le Qt officiel désigne ses
  # frameworks par @rpath : le paquet installé les cherche dans
  # Contents/Frameworks (où macdeployqt les copie), CMake retirant à
  # l'installation le chemin de la compilation et macdeployqt n'en ajoutant
  # pas (sans cela : « Library not loaded: @rpath/QtWidgets.framework »)
  set_target_properties(cvw PROPERTIES
    OUTPUT_NAME "Centre-Ville"
    MACOSX_BUNDLE_ICON_FILE centre-ville.icns
    INSTALL_RPATH "@executable_path/../Frameworks"
  )
  foreach(icone centre-ville application-x-centre-ville)
    set(icns ${CMAKE_CURRENT_BINARY_DIR}/icones/${icone}.icns)
    add_custom_command(OUTPUT ${icns}
      COMMAND ${CMAKE_COMMAND} -DPNG=${CMAKE_CURRENT_SOURCE_DIR}/ressources/icones/${icone}.png -DICNS=${icns}
              -P ${CMAKE_CURRENT_SOURCE_DIR}/icones_macos.cmake
      DEPENDS ressources/icones/${icone}.png icones_macos.cmake
      VERBATIM
    )
    target_sources(cvw PRIVATE ${icns})
    set_source_files_properties(${icns} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  endforeach()
endif()

# Aide HTML (phase 7) : docs/guide -> html/ et docs/tutorial -> tutocv/, à côté
# de l'exécutable comme dans l'installeur d'origine ({app}\bin\html,
# {app}\bin\tutocv) ; sous macOS, dans Contents/Resources du paquet (voir
# stRepAide dans base.cpp). Recopiée quand docs/ change, dossiers vidés
# d'abord : la compilation a la même aide que l'installation.
get_property(CV_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
set(CV_DOSSIER_CVW ${CMAKE_CURRENT_BINARY_DIR})
if(CV_MULTI_CONFIG)
  string(APPEND CV_DOSSIER_CVW "/$<CONFIG>")
endif()
if(APPLE)
  set(CV_DOSSIER_AIDE "${CV_DOSSIER_CVW}/Centre-Ville.app/Contents/Resources")
else()
  set(CV_DOSSIER_AIDE "${CV_DOSSIER_CVW}")
endif()
file(GLOB_RECURSE CV_FICHIERS_AIDE CONFIGURE_DEPENDS
  ${PROJECT_SOURCE_DIR}/docs/guide/*
  ${PROJECT_SOURCE_DIR}/docs/tutorial/*
)
add_custom_command(OUTPUT "${CV_DOSSIER_CVW}/aide.stamp"
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${CV_DOSSIER_AIDE}/html" "${CV_DOSSIER_AIDE}/tutocv"
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/docs/guide "${CV_DOSSIER_AIDE}/html"
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/docs/tutorial "${CV_DOSSIER_AIDE}/tutocv"
  COMMAND ${CMAKE_COMMAND} -E touch "${CV_DOSSIER_CVW}/aide.stamp"
  DEPENDS ${CV_FICHIERS_AIDE}
  COMMENT "Aide HTML de cvw (html/, tutocv/)"
  VERBATIM
)
add_custom_target(cvw_aide DEPENDS "${CV_DOSSIER_CVW}/aide.stamp")
add_dependencies(cvw cvw_aide)

# Installation (phase 8) : cmake --install <build> --prefix <dossier>
#   Windows : bin/cvw.exe, DLL de Qt et de MSVC, bin/qt.conf, plugins/ et
#             translations/ (windeployqt), bin/html, bin/tutocv : contenu de
#             l'installeur (inno-setup/inscvw95.iss) ;
#   macOS   : Centre-Ville.app (macdeployqt), aide dans Contents/Resources ;
#   Linux   : bin/cvw, bin/html, bin/tutocv, entrée de bureau, type MIME des
#             fichiers .cvw, icônes, cartes d'exemple (share/centre-ville/cartes) ;
#             Qt est déployé par linuxdeploy (AppImage).
include(GNUInstallDirs)
if(APPLE)
  install(TARGETS cvw BUNDLE DESTINATION .)
  set(CV_INSTALL_AIDE Centre-Ville.app/Contents/Resources)
else()
  install(TARGETS cvw RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  set(CV_INSTALL_AIDE ${CMAKE_INSTALL_BINDIR})
endif()
install(DIRECTORY ${PROJECT_SOURCE_DIR}/docs/guide/ DESTINATION ${CV_INSTALL_AIDE}/html)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/docs/tutorial/ DESTINATION ${CV_INSTALL_AIDE}/tutocv)

if(UNIX AND NOT APPLE)
  install(FILES linux/fr.patquoi.CentreVille.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
  install(FILES linux/fr.patquoi.CentreVille.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages)
  install(FILES ressources/icones/centre-ville.png
          DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps)
  install(FILES ressources/icones/application-x-centre-ville.png
          DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/mimetypes)
  # Cartes d'exemple, copiées au premier lancement de l'AppImage dans le dossier
  # de stRepLocalAppData (linux/apprun-hooks/centre-ville-cartes.sh)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/maps/ DESTINATION ${CMAKE_INSTALL_DATADIR}/centre-ville/cartes
          FILES_MATCHING PATTERN "*.cvw")
endif()

if(WIN32 OR APPLE)
  # Bibliothèques d'exécution de MSVC à côté de cvw.exe (InstallRequiredSystemLibraries)
  # plutôt que vc_redist.x64.exe, que windeployqt ne trouve qu'avec VCINSTALLDIR
  if(MSVC)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
    include(InstallRequiredSystemLibraries)
  endif()
  # windeployqt (DLL, greffons et traductions de Qt) ou macdeployqt (frameworks
  # et greffons dans le paquet)
  qt_generate_deploy_app_script(
    TARGET cvw
    OUTPUT_SCRIPT CV_SCRIPT_DEPLOIEMENT
    NO_COMPILER_RUNTIME
    NO_UNSUPPORTED_PLATFORM_ERROR
  )
  install(SCRIPT ${CV_SCRIPT_DEPLOIEMENT})
endif()

if(APPLE)
  # macdeployqt ne copie pas les traductions de Qt : qtbase_fr.qm (boutons des
  # boîtes de dialogue en français, application.cpp) va dans
  # Contents/Resources/translations, que l'on ajoute au qt.conf de macdeployqt
  find_file(CV_QTBASE_FR qtbase_fr.qm
    PATHS "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_TRANSLATIONS}"
    NO_DEFAULT_PATH
  )
  if(CV_QTBASE_FR)
    install(FILES ${CV_QTBASE_FR} DESTINATION Centre-Ville.app/Contents/Resources/translations)
  else()
    message(WARNING "qtbase_fr.qm introuvable : boîtes de dialogue en anglais dans Centre-Ville.app")
  endif()
  # Puis signature ad hoc de tout le paquet (pas de compte Apple Developer) :
  # macdeployqt a modifié les bibliothèques copiées, et ce qui est ajouté après
  # lui romprait le sceau des ressources (« paquet endommagé » pour Gatekeeper)
  install(CODE [[
    set(paquet "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/Centre-Ville.app")
    file(READ "${paquet}/Contents/Resources/qt.conf" contenu)
    if(NOT contenu MATCHES "Translations")
      file(APPEND "${paquet}/Contents/Resources/qt.conf" "Translations = Resources/translations\n")
    endif()
    execute_process(COMMAND codesign --force --deep --sign - "${paquet}" RESULT_VARIABLE resultat)
    if(resultat)
      message(FATAL_ERROR "codesign a échoué (${resultat}) sur ${paquet}")
    endif()
  ]])
endif()

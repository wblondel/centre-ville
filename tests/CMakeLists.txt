# Tests du moteur Centre-Ville (sans Qt)

# Catch2 : version installée si disponible, sinon téléchargée
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.8.1.tar.gz
    URL_HASH SHA256=18b3f70ac80fccc340d8c6ff0f339b2ae64944782f8d2fca2bd705cf47cadb79
    DOWNLOAD_EXTRACT_TIMESTAMP ON
  )
  FetchContent_MakeAvailable(Catch2)
endif()

# Code commun aux tests et à cvsim : harnais, export d'état, golden master
add_library(cvtest_support STATIC
  support/etat.cpp
  support/etat.h
  support/format_reference.cpp
  support/format_reference.h
  support/golden.cpp
  support/golden.h
  support/harnais.cpp
  support/harnais.h
  support/cartes.h
)
target_include_directories(cvtest_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/support)
# L'empreinte de l'état est calculée à chaque tour : optimisée même en Debug
if(NOT MSVC)
  set_source_files_properties(support/etat.cpp PROPERTIES COMPILE_OPTIONS "-O2")
endif()
target_link_libraries(cvtest_support PUBLIC cvengine)

# Mémoire remise à zéro par operator new (déterminisme, voir allocation.cpp)
set(CV_ALLOCATION ${CMAKE_CURRENT_SOURCE_DIR}/support/allocation.cpp)

# cvsim : charge une carte, simule N tours, sauvegarde ; régénère le golden master
add_executable(cvsim cvsim.cpp ${CV_ALLOCATION})
target_link_libraries(cvsim PRIVATE cvtest_support)

# Tests unitaires et de non-régression
add_executable(cvtests
  test_compat.cpp
  test_cvw.cpp
  test_golden.cpp
  test_generation.cpp
  test_corrections.cpp
  ${CV_ALLOCATION}
)
target_link_libraries(cvtests PRIVATE cvtest_support Catch2::Catch2WithMain)
target_compile_definitions(cvtests PRIVATE
  CV_DOSSIER_CARTES="${PROJECT_SOURCE_DIR}/maps"
  CV_DOSSIER_TUTORIEL="${PROJECT_SOURCE_DIR}/docs/tutorial"
  CV_DOSSIER_GOLDEN="${CMAKE_CURRENT_SOURCE_DIR}/golden"
  CV_DOSSIER_SORTIE="${CMAKE_CURRENT_BINARY_DIR}/sortie"
)
if(MSVC)
  target_compile_options(cvtests PRIVATE /bigobj)
endif()

# Les mêmes tests (sauf compat et corrections), mémoire remplie d'un motif au
# lieu d'être remise à zéro : le moteur ne doit pas dépendre de cette remise à
# zéro (champs initialisés depuis la 6.0.1, voir test_memoire.cpp)
add_executable(cvtests_motif
  test_cvw.cpp
  test_golden.cpp
  test_generation.cpp
  test_memoire.cpp
  support/allocation_motif.cpp
)
target_link_libraries(cvtests_motif PRIVATE cvtest_support Catch2::Catch2WithMain)
target_compile_definitions(cvtests_motif PRIVATE
  CV_DOSSIER_CARTES="${PROJECT_SOURCE_DIR}/maps"
  CV_DOSSIER_TUTORIEL="${PROJECT_SOURCE_DIR}/docs/tutorial"
  CV_DOSSIER_GOLDEN="${CMAKE_CURRENT_SOURCE_DIR}/golden"
  CV_DOSSIER_SORTIE="${CMAKE_CURRENT_BINARY_DIR}/sortie_motif"
)
if(MSVC)
  target_compile_options(cvtests_motif PRIVATE /bigobj)
endif()

# La récursion de voie::CalculeDistance*() est profonde : 256 à 512 Ko de pile
# avec Clang (Debug comme Release), davantage en Debug MSVC ou sous
# AddressSanitizer. L'original (C++Builder) tournait avec la pile par défaut de
# 1 Mo : l'application Qt devra en tenir compte (même réglage).
foreach(cible cvtests cvtests_motif cvsim)
  if(MSVC)
    target_link_options(${cible} PRIVATE /STACK:16777216)
  elseif(APPLE)
    target_link_options(${cible} PRIVATE -Wl,-stack_size,0x4000000)
  endif()
endforeach()

# Une entrée CTest par groupe de tests Catch2 (étiquettes) : noms ASCII, car
# les noms accentués des cas de test passent mal par la console Windows.
foreach(groupe IN ITEMS compat cvw simulation generation copie corrections)
  add_test(NAME moteur.${groupe} COMMAND cvtests "[${groupe}]")
  set_tests_properties(moteur.${groupe} PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()
# Bogues corrigés en 6.0.1 : le moteur 6.0.0 ne rendait pas la main (blocage
# des vehlibs), le délai fait alors échouer le test
set_tests_properties(moteur.corrections PROPERTIES TIMEOUT 120)
add_test(NAME moteur.memoire COMMAND cvtests_motif)
set_tests_properties(moteur.memoire PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Critère de fin de la phase 1 : charger une carte, simuler 100 tours, sauvegarder
# (carte au nom ASCII : sous Windows, argv n'est pas en UTF-8)
add_test(NAME cvsim.spaghettis_100_tours
  COMMAND cvsim "${PROJECT_SOURCE_DIR}/maps/Spaghettis.cvw" --tours 100 --graine 1
          --sauve "${CMAKE_CURRENT_BINARY_DIR}/sortie/cvsim_spaghettis_100.cvw"
)
set_tests_properties(cvsim.spaghettis_100_tours PROPERTIES FIXTURES_REQUIRED dossier_sortie)
add_test(NAME preparation.dossier_sortie
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/sortie")
set_tests_properties(preparation.dossier_sortie PROPERTIES FIXTURES_SETUP dossier_sortie)

# Tests de l'application Qt (si elle est compilée) : pilotage de l'interface
# sur maps/Démo.cvw, captures de la fenêtre principale dans sortie/captures
if(TARGET cvwfiches)
  find_package(Qt6 REQUIRED COMPONENTS Test)
  add_executable(test_interface app/test_interface.cpp ${PROJECT_SOURCE_DIR}/app/ressources/images.qrc ${CV_ALLOCATION})
  set_target_properties(test_interface PROPERTIES AUTOMOC ON AUTORCC ON)
  target_link_libraries(test_interface PRIVATE cvwfiches cvtest_support Qt6::Test)
  target_compile_definitions(test_interface PRIVATE CV_DOSSIER_GOLDEN="${CMAKE_CURRENT_SOURCE_DIR}/golden")
  if(MSVC)
    target_link_options(test_interface PRIVATE /STACK:16777216)
  elseif(APPLE)
    target_link_options(test_interface PRIVATE -Wl,-stack_size,0x1000000)
  endif()
  add_test(NAME interface.demo COMMAND test_interface)
  set_tests_properties(interface.demo PROPERTIES
    FIXTURES_REQUIRED dossier_sortie
    # QT_FORCE_STDERR_LOGGING : sous Windows, sans console, Qt écrirait ses
    # messages (qWarning, qFatal) dans OutputDebugString, invisibles pour CTest
    ENVIRONMENT "CV_CARTE=${PROJECT_SOURCE_DIR}/maps/Démo.cvw;CV_DOSSIER_CAPTURES=${CMAKE_CURRENT_BINARY_DIR}/sortie/captures;QT_QPA_PLATFORM=offscreen;QT_FORCE_STDERR_LOGGING=1"
    TIMEOUT 300
  )

  # Aide HTML (phase 7) : HelpContext -> page et ancre, aide copiée à côté de cvw
  # (dans Contents/Resources du paquet sous macOS)
  if(APPLE)
    set(CV_AIDE_CVW "$<TARGET_BUNDLE_CONTENT_DIR:cvw>/Resources")
  else()
    set(CV_AIDE_CVW "$<TARGET_FILE_DIR:cvw>")
  endif()
  add_executable(test_aide app/test_aide.cpp)
  set_target_properties(test_aide PROPERTIES AUTOMOC ON)
  target_link_libraries(test_aide PRIVATE cvwfiches Qt6::Test)
  target_compile_definitions(test_aide PRIVATE
    CV_SOURCE_BASE="${PROJECT_SOURCE_DIR}/src/base.cpp"
    CV_DOSSIER_DOCS="${PROJECT_SOURCE_DIR}/docs"
    CV_DOSSIER_UI="${PROJECT_SOURCE_DIR}/app/ui"
    CV_DOSSIER_AIDE_CVW="${CV_AIDE_CVW}"
  )
  add_dependencies(test_aide cvw)
  add_test(NAME interface.aide COMMAND test_aide)
  set_tests_properties(interface.aide PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_FORCE_STDERR_LOGGING=1"
  )

  # Cartes d'exemple de l'AppImage (phase 9) : copiées au premier lancement
  # dans le dossier de stRepLocalAppData (app/linux/apprun-hooks)
  add_executable(test_cartes app/test_cartes.cpp)
  set_target_properties(test_cartes PROPERTIES AUTOMOC ON)
  target_link_libraries(test_cartes PRIVATE cvwfiches Qt6::Test)
  target_compile_definitions(test_cartes PRIVATE
    CV_SCRIPT_CARTES="${PROJECT_SOURCE_DIR}/app/linux/apprun-hooks/centre-ville-cartes.sh"
    CV_DOSSIER_CARTES="${PROJECT_SOURCE_DIR}/maps"
    $<$<PLATFORM_ID:Darwin>:CV_PAQUET_CVW="$<TARGET_FILE:cvw>">
  )
  add_dependencies(test_cartes cvw)
  add_test(NAME interface.cartes COMMAND test_cartes)
  set_tests_properties(interface.cartes PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_FORCE_STDERR_LOGGING=1"
  )
endif()

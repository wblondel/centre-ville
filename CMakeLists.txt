cmake_minimum_required(VERSION 3.25)

project(CentreVille
  VERSION 5.4.3.2
  DESCRIPTION "Centre-Ville : simulateur de circulation urbaine"
  HOMEPAGE_URL "http://patquoi.fr"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CV_TESTS "Compiler les tests du moteur" ON)
# Application Qt : AUTO = compilée si Qt 6 est trouvé, ON = Qt 6 obligatoire, OFF = jamais
set(CV_APP "AUTO" CACHE STRING "Compiler l'application Qt cvw (AUTO, ON ou OFF)")
set_property(CACHE CV_APP PROPERTY STRINGS AUTO ON OFF)

# Moteur de simulation (sans Qt)
add_subdirectory(engine)

# Application (Qt 6 Widgets)
if(NOT CV_APP STREQUAL "OFF")
  add_subdirectory(app)
endif()

if(CV_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Image disque macOS (phase 8) : après cmake --build, « cpack --config
# <build>/CPackConfig.cmake » produit Centre-Ville-<version>-macOS.dmg :
# Centre-Ville.app (règles d'installation de app/), raccourci vers
# Applications, cartes d'exemple (maps/) et licence. Les autres paquets
# (installeur Windows, AppImage) : voir .github/workflows/build-installer.yml.
if(APPLE AND TARGET cvw)
  install(DIRECTORY maps/ DESTINATION Cartes COMPONENT dmg EXCLUDE_FROM_ALL)
  install(FILES docs/licence.txt DESTINATION . COMPONENT dmg EXCLUDE_FROM_ALL)
  set(CPACK_GENERATOR DragNDrop)
  set(CPACK_PACKAGE_NAME "Centre-Ville")
  set(CPACK_PACKAGE_VENDOR "Patquoi.fr")
  set(CPACK_PACKAGE_FILE_NAME "Centre-Ville-${PROJECT_VERSION}-macOS")
  set(CPACK_DMG_VOLUME_NAME "Centre-Ville ${PROJECT_VERSION}")
  set(CPACK_COMPONENTS_ALL Unspecified dmg)
  set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
  include(CPack)
endif()

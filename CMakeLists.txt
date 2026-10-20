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

# Moteur de simulation (sans Qt)
add_subdirectory(engine)

if(CV_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

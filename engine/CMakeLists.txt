# Moteur de simulation de Centre-Ville : bibliothèque statique sans Qt
add_library(cvengine STATIC
  simul.cpp
  simul.h
  format_cvw.h
  vue.cpp
  vue.h
  compat/alea.cpp
  compat/alea.h
  compat/chaine.cpp
  compat/chaine.h
  compat/fichier.cpp
  compat/fichier.h
  compat/temps.cpp
  compat/temps.h
)
add_library(cvengine::cvengine ALIAS cvengine)

target_include_directories(cvengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(cvengine PUBLIC cxx_std_20)

if(MSVC)
  # /utf-8 : sources et littéraux en UTF-8 ; /W3 : le code d'origine n'est pas
  # corrigé pendant le portage, on ne vise que la compilation.
  target_compile_options(cvengine PRIVATE /W3)
  target_compile_options(cvengine PUBLIC /utf-8)
  target_compile_definitions(cvengine PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
  # char signé comme sous C++Builder
  target_compile_options(cvengine PRIVATE -fsigned-char -Wall)
endif()

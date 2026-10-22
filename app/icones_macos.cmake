# Icône .icns de macOS depuis une image PNG de 256 x 256 (app/ressources/icones,
# tirées de res/ico par tools/extrait_icones.py) : sips réduit l'image aux
# tailles d'un .iconset, iconutil les assemble. Outils du système (macOS).
#
#   cmake -DPNG=<image.png> -DICNS=<sortie.icns> -P icones_macos.cmake

get_filename_component(nom "${ICNS}" NAME_WE)
get_filename_component(dossier "${ICNS}" DIRECTORY)
set(iconset "${dossier}/${nom}.iconset")
file(REMOVE_RECURSE "${iconset}")
file(MAKE_DIRECTORY "${iconset}")

# Nom dans l'iconset, taille en pixels (pas d'agrandissement au-delà de 256)
foreach(element IN ITEMS 16x16:16 16x16@2x:32 32x32:32 32x32@2x:64 128x128:128 128x128@2x:256 256x256:256)
  string(REPLACE ":" ";" element "${element}")
  list(GET element 0 suffixe)
  list(GET element 1 taille)
  execute_process(
    COMMAND sips -z ${taille} ${taille} "${PNG}" --out "${iconset}/icon_${suffixe}.png"
    OUTPUT_QUIET
    RESULT_VARIABLE resultat
  )
  if(resultat)
    message(FATAL_ERROR "sips a échoué (${resultat}) sur ${PNG}")
  endif()
endforeach()

execute_process(COMMAND iconutil -c icns -o "${ICNS}" "${iconset}" RESULT_VARIABLE resultat)
if(resultat)
  message(FATAL_ERROR "iconutil a échoué (${resultat}) sur ${iconset}")
endif()
file(REMOVE_RECURSE "${iconset}")

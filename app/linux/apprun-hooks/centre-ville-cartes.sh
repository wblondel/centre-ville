# Centre-Ville (AppImage) : au premier lancement, les cartes d'exemple sont
# copiées dans le dossier des fichiers de Centre-Ville, où « Ouvrir » et
# « Enregistrer » démarrent (stRepLocalAppData de app/base.cpp :
# QStandardPaths::GenericDataLocation + Patquoi.fr/Centre-Ville), comme
# l'installeur Windows les copie dans %LOCALAPPDATA%\Patquoi.fr\Centre-Ville.
# Aucun fichier existant n'est écrasé ; .cartes-copiees marque le premier
# lancement (une carte supprimée ensuite n'est pas recopiée).
# Lu par l'AppRun de linuxdeploy (apprun-hooks), qui s'arrête à la première
# erreur (set -e) : aucune commande ne doit échouer.
cv_cartes="${APPDIR:-$this_dir}/usr/share/centre-ville/cartes"
cv_dossier="${XDG_DATA_HOME:-$HOME/.local/share}/Patquoi.fr/Centre-Ville"
if [ -d "$cv_cartes" ] && [ ! -e "$cv_dossier/.cartes-copiees" ] && mkdir -p "$cv_dossier" 2>/dev/null; then
  for cv_carte in "$cv_cartes"/*.cvw; do
    [ -e "$cv_dossier/${cv_carte##*/}" ] || cp "$cv_carte" "$cv_dossier/" 2>/dev/null || true
  done
  : > "$cv_dossier/.cartes-copiees" 2>/dev/null || true
fi
unset cv_cartes cv_dossier cv_carte

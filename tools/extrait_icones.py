#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Extraction des icônes de Centre-Ville pour les paquets Linux et macOS.

Phase 8 de PLAN_PORTAGE.md : les icônes d'origine sont dans res/ico, au
format .ico de Windows (cvw.ico : application, ressource Icon_0 de
src/cvw_resources.rc ; Newdoc_cvw.ico : fichiers .cvw, Icon_1). Leur image de
256 x 256 est un PNG, écrit tel quel (octet pour octet) sous :
  - centre-ville.png : icône de l'application (.desktop, AppImage) ;
  - application-x-centre-ville.png : icône des fichiers .cvw (type MIME
    application/x-centre-ville).
Les PNG de res/ico (cvwico.png, Newdoc_cvwico.png) ne servent pas : une
vignette parasite y recouvre le coin en haut à gauche.
Sous macOS, les .icns sont produits à la compilation depuis ces PNG
(sips, iconutil : voir app/CMakeLists.txt).

Usage :
  python3 tools/extrait_icones.py [--res res/ico] [--sortie app/ressources/icones]
  python3 tools/extrait_icones.py --verifie   # échoue si la sortie n'est pas à jour

Aucune dépendance hors bibliothèque standard.
"""
import argparse
import os
import struct
import sys

ICONES = [
    ("cvw.ico", "centre-ville.png"),
    ("Newdoc_cvw.ico", "application-x-centre-ville.png"),
]


def png_256(chemin):
    """Image PNG de 256 x 256 d'un fichier .ico (ICONDIR + ICONDIRENTRY)."""
    with open(chemin, "rb") as f:
        donnees = f.read()
    reserve, genre, nombre = struct.unpack_from("<HHH", donnees, 0)
    if reserve != 0 or genre != 1:
        raise ValueError(f"{chemin} : pas un fichier .ico")
    for i in range(nombre):
        largeur, hauteur, _, _, _, _, taille, debut = struct.unpack_from("<BBBBHHII", donnees, 6 + 16 * i)
        image = donnees[debut:debut + taille]
        if (largeur, hauteur) == (0, 0) and image.startswith(b"\x89PNG\r\n\x1a\n"):
            return image
    raise ValueError(f"{chemin} : pas d'image PNG de 256 x 256")


def main():
    racine = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--res", default=os.path.join(racine, "res", "ico"))
    p.add_argument("--sortie", default=os.path.join(racine, "app", "ressources", "icones"))
    p.add_argument("--verifie", action="store_true", help="n'écrit rien ; échoue si la sortie n'est pas à jour")
    args = p.parse_args()

    a_jour = True
    for source, cible in ICONES:
        image = png_256(os.path.join(args.res, source))
        chemin = os.path.join(args.sortie, cible)
        actuel = open(chemin, "rb").read() if os.path.exists(chemin) else None
        if actuel == image:
            continue
        if args.verifie:
            print(f"pas à jour : {chemin}")
            a_jour = False
        else:
            os.makedirs(args.sortie, exist_ok=True)
            with open(chemin, "wb") as f:
                f.write(image)
            print(f"écrit : {chemin}")
    if not a_jour:
        sys.exit(1)


if __name__ == "__main__":
    main()

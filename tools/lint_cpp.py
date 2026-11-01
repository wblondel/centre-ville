#!/usr/bin/env python3
"""Lance clang-tidy sur le code C++ et compare les constats à une référence.

Le code porté garde le style de l'original : les constats déjà présents sont notés dans
tools/lint/clang-tidy-reference.txt, et seuls les nouveaux font échouer le script. Un constat est
identifié par son fichier, sa règle et son message, sans numéro de ligne : déplacer du code n'en
crée pas. La référence vaut pour la version de tools/lint/requirements.txt : les constats de
Linux, ceux de la CI, et quelques-uns vus seulement sous macOS, pour que le script passe aussi en local.

    python3 tools/lint_cpp.py --dossier build/dev            # compare à la référence
    python3 tools/lint_cpp.py --dossier build/dev --reference-a-jour  # la réécrit
"""

import argparse
import collections
import concurrent.futures
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys

RACINE = pathlib.Path(__file__).resolve().parent.parent
REFERENCE = RACINE / "tools" / "lint" / "clang-tidy-reference.txt"
CONSTAT = re.compile(r"^(.+?):(\d+):\d+: (warning|error): (.*) \[([\w.,-]+)\]$")
SOURCES = ("engine/", "app/", "tests/")


def sources(dossier):
    """Fichiers .cpp du projet dans compile_commands.json, sans le code généré."""
    with open(dossier / "compile_commands.json", encoding="utf-8") as f:
        entrees = json.load(f)
    fichiers = set()
    for entree in entrees:
        chemin = pathlib.Path(entree["directory"], entree["file"]).resolve()
        try:
            relatif = chemin.relative_to(RACINE).as_posix()
        except ValueError:
            continue
        if relatif.startswith(SOURCES) and relatif.endswith(".cpp") and "_autogen" not in relatif:
            fichiers.add(relatif)
    return sorted(fichiers)


def lance(outil, dossier, fichier, arguments):
    resultat = subprocess.run(  # noqa: S603 : outil et fichiers viennent du dépôt
        [outil, "-p", str(dossier), "--quiet", *arguments, fichier],
        cwd=RACINE,
        capture_output=True,
        text=True,
        errors="replace",
        check=False,
    )
    return resultat.stdout


def constats(sortie):
    """(cle, emplacement) de chaque constat du projet. Une clé peut revenir : même message ailleurs."""
    vus = {}
    for ligne in sortie.splitlines():
        m = CONSTAT.match(ligne)
        if not m:
            continue
        chemin = pathlib.Path(m[1])
        try:
            relatif = (chemin if chemin.is_absolute() else RACINE / chemin).resolve().relative_to(RACINE).as_posix()
        except ValueError:
            continue
        if not relatif.startswith(SOURCES):
            continue
        regle = m[5]
        vus[(relatif, m[2], regle, m[4])] = f"{relatif}: [{regle}] {m[4]}"
    return vus


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--dossier", default="build/dev", help="dossier de compilation (compile_commands.json)")
    p.add_argument("--clang-tidy", default=shutil.which("clang-tidy") or "clang-tidy")
    p.add_argument("--travaux", type=int, default=os.cpu_count())
    p.add_argument("--sortie", help="écrit les constats dans ce fichier, au format de la référence")
    p.add_argument("--reference-a-jour", action="store_true", help="réécrit la référence au lieu de comparer")
    args = p.parse_args()

    dossier = (RACINE / args.dossier).resolve()
    fichiers = sources(dossier)
    if not fichiers:
        sys.exit(f"Aucune source dans {dossier}/compile_commands.json")
    arguments = []
    if sys.platform == "darwin":
        sdk = subprocess.check_output(["xcrun", "--show-sdk-path"], text=True).strip()  # noqa: S607
        arguments.append(f"--extra-arg=-isysroot{sdk}")

    tous = {}
    with concurrent.futures.ThreadPoolExecutor(args.travaux) as travaux:
        for sortie in travaux.map(lambda f: lance(args.clang_tidy, dossier, f, arguments), fichiers):
            tous.update(constats(sortie))
    print(f"{len(fichiers)} fichiers analysés, {len(tous)} constats")

    # Sans compilation réussie, clang-tidy ne voit presque rien : ne pas conclure
    erreurs = sorted(k for k in tous if "clang-diagnostic-error" in k[2])
    if erreurs:
        for fichier, ligne, _, message in erreurs[:20]:
            print(f"ERREUR DE COMPILATION {fichier}:{ligne}: {message}")
        return 2

    actuels = collections.Counter(tous.values())
    if args.sortie:
        pathlib.Path(args.sortie).write_text("".join(f"{c}\n" for c in sorted(actuels.elements())), encoding="utf-8")
    if args.reference_a_jour:
        REFERENCE.write_text("".join(f"{c}\n" for c in sorted(actuels.elements())), encoding="utf-8")
        print(f"Référence réécrite : {REFERENCE.relative_to(RACINE)}")
        return 0

    reference = collections.Counter(REFERENCE.read_text(encoding="utf-8").splitlines())
    nouveaux = actuels - reference
    disparus = reference - actuels
    if disparus:
        print(f"{sum(disparus.values())} constat(s) de la référence ont disparu ; --reference-a-jour les retire.")
    if nouveaux:
        print(f"\n{sum(nouveaux.values())} NOUVEAU(X) CONSTAT(S) :")
        for (fichier, ligne, regle, message), cle in sorted(tous.items()):
            if nouveaux[cle] > 0:
                print(f"  {fichier}:{ligne}: [{regle}] {message}")
        print(
            "\nCorriger le code. Fausse alerte : // NOLINT(regle) sur la ligne, avec le motif."
            "\nLe nombre compte par fichier, règle et message : un constat identique déjà noté est listé avec lui."
        )
        return 1
    print("Aucun nouveau constat.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

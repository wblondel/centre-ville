#!/usr/bin/env python3
"""Lance clang-tidy sur le code C++ du projet ; le moindre constat fait échouer le script.

Règles : .clang-tidy (et tests/.clang-tidy pour les tests), version de tools/lint/requirements.txt.
Une fausse alerte s'annote dans le code, avec son motif : // NOLINT(regle) ou // NOLINTNEXTLINE(regle).

    python3 tools/lint_cpp.py --dossier build/dev
    python3 tools/lint_cpp.py --dossier build/dev --sortie constats.txt   # écrit aussi la liste
"""

import argparse
import concurrent.futures
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys

RACINE = pathlib.Path(__file__).resolve().parent.parent
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
    """Constats du projet : (fichier, ligne, règle, message) -> texte affiché."""
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
        vus[(relatif, m[2], regle, m[4])] = f"{relatif}:{m[2]}: [{regle}] {m[4]}"
    return vus


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--dossier", default="build/dev", help="dossier de compilation (compile_commands.json)")
    p.add_argument("--clang-tidy", default=shutil.which("clang-tidy") or "clang-tidy")
    p.add_argument("--travaux", type=int, default=os.cpu_count())
    p.add_argument("--sortie", help="écrit aussi les constats dans ce fichier")
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

    lignes = [tous[k] for k in sorted(tous)]
    if args.sortie:
        pathlib.Path(args.sortie).write_text("".join(f"{ligne}\n" for ligne in lignes), encoding="utf-8")
    for ligne in lignes:
        print(f"  {ligne}")
    if lignes:
        print(
            f"\n{len(lignes)} constat(s) : corriger le code. Fausse alerte : // NOLINT(regle) ou"
            " // NOLINTNEXTLINE(regle), avec le motif."
        )
        return 1
    print("Aucun constat.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

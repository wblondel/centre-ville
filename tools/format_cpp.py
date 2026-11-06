#!/usr/bin/env python3
"""Met en forme le C++ du projet avec clang-format (.clang-format), ou vérifie qu'il l'est.

Fichiers : les .cpp et .h suivis par git dans engine/, app/ et tests/, sauf ceux que
.clang-format-ignore écarte (fichiers générés par tools/dfm2ui.py et tools/extrait_images.py).
La version de clang-format est celle de tools/lint/requirements.txt.

    python3 tools/format_cpp.py              # met en forme
    python3 tools/format_cpp.py --verifie    # échoue si un fichier n'est pas en forme (CI)
"""

import argparse
import pathlib
import shutil
import subprocess
import sys

RACINE = pathlib.Path(__file__).resolve().parent.parent
DOSSIERS = ("engine", "app", "tests")


def fichiers():
    """Fichiers C++ suivis par git, chemins relatifs à la racine."""
    sortie = subprocess.check_output(  # noqa: S603 : arguments fixes
        ["git", "ls-files", "--", *(f"{d}/*.cpp" for d in DOSSIERS), *(f"{d}/*.h" for d in DOSSIERS)],  # noqa: S607
        cwd=RACINE,
        text=True,
    )
    return sorted(ligne for ligne in sortie.splitlines() if ligne)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--clang-format", default=shutil.which("clang-format") or "clang-format")
    p.add_argument("--verifie", action="store_true", help="ne modifie rien, échoue si un fichier n'est pas en forme")
    args = p.parse_args()

    liste = fichiers()
    if not liste:
        sys.exit("Aucun fichier C++ suivi par git")
    options = ["--dry-run", "--Werror"] if args.verifie else ["-i"]
    resultat = subprocess.run([args.clang_format, *options, *liste], cwd=RACINE, check=False)  # noqa: S603
    if args.verifie:
        if resultat.returncode:
            print("\nMise en forme à refaire : python3 tools/format_cpp.py", file=sys.stderr)
        else:
            print(f"{len(liste)} fichiers en forme.")
    return resultat.returncode


if __name__ == "__main__":
    sys.exit(main())

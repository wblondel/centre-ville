# Plan d'amélioration de la CI

État mesuré le 2026-11-01, sur les derniers runs réussis. Les points sont classés par ordre d'exécution. Une case cochée est faite.

## État actuel

| Workflow | Déclencheur | Coût d'un run | Attente |
|---|---|---|---|
| `ci.yml` | PR, `main` et chaque semaine | 90 min-machine | 27 min (ASan macOS) |
| `build-installer.yml` | tag `v*` | 34 min-machine | environ 25 min |
| `recette-windows.yml` | appelé avant la release, ou à la main | 15 min-machine | 7 min |
| `veille-inno-setup.yml` | chaque lundi | moins de 1 min | moins de 1 min |
| `deploy-docs.yml` | `docs/` sur `main` | moins de 1 min | moins de 1 min |

Déjà en place : permissions en lecture par défaut, écriture limitée au job de release, CodeQL (actions, C++, Python), détection de secrets avec blocage au push, alertes Dependabot, Catch2 vérifié par SHA256, 10 jobs requis sur `main`.

## 1. Sécurité de la chaîne de release (fait)

- [x] **`linuxdeploy`** : versions figées et SHA256 vérifiés, au lieu de `continuous` sans contrôle. Leur contenu part dans l'AppImage.
- [x] **Actions épinglées par SHA**, avec `.github/dependabot.yml` pour les mises à jour (`github-actions` et `pip`, groupées, mensuelles).
- [x] **`softprops/action-gh-release` retiré** : c'était la seule action tierce avec `contents: write`, et `gh release create` fait le même travail.
- [x] **Installeur 5.4.3.2** : SHA256 vérifié avant de l'exécuter.
- [x] **Injection de `inputs.tag`** : passé par `env:`.
- [x] **`persist-credentials: false`** sur tous les checkouts.
- [x] **Inno Setup** : version figée, depuis la 6.0.20 installeur de la release officielle (7.1.1, `github.com/jrsoftware/issrc`) vérifié par SHA256, au lieu de Chocolatey, dont le paquet s'arrête à la 6.7.1 et ne vérifie aucune empreinte. L'`ISCC.exe` utilisé est celui de cette installation, pas celui de l'image de GitHub. Dependabot ne suit pas les releases : `veille-inno-setup.yml` ouvre une issue à chaque nouvelle version.
- [x] **`deploy-docs.yml`** : `pages: write` et `id-token: write` au niveau du job.
- [x] **Règles du dépôt** : force-push et suppression bloqués sur `main`, contournement administrateur limité aux PR ; tags `v*` impossibles à déplacer ou à supprimer.
- [x] **`zizmor` bloquant** dans le job de lint.
- [x] **`SHA256SUMS` et attestation de provenance** à chaque release. Vérification : `gh attestation verify <paquet> --repo wblondel/centre-ville`.

Non retenu : l'option `sha_pinning_required` du dépôt. `jurplel/install-qt-action` appelle en interne des actions épinglées par étiquette, ce qui risquerait de bloquer toute la CI. `zizmor` vérifie les workflows du dépôt à chaque PR.

## 2. Fiabilité des releases (fait)

- [x] **Releases immuables** : les paquets publiés et leur tag ne peuvent plus changer. Une release ratée se corrige par une nouvelle version, et son nom de tag ne resservira pas.
- [x] **Recette Windows avant la publication**, sur l'artefact et non sur la release : un échec empêche la release.
- [x] **Contrôle du contenu** : exactement les 5 paquets attendus. Le job de release tourne aussi à la main, sans publier, ce qui l'éprouve hors d'un tag.
- [x] **`timeout-minutes` sur tous les jobs**, au lieu du défaut de 6 h.
- [x] **Images et Qt figés** : `ubuntu-24.04`, `macos-26`, `windows-2025-vs2026`, Qt 6.8.3. Checks requis nommés par système, pour qu'un changement d'image ne les touche pas.
- [x] **Run hebdomadaire** de la suite complète, le lundi.
- [x] **Remplacer `ubuntu-22.04`** pour l'AppImage, gardée pour sa glibc 2.35 : son support standard finit en avril 2027. Depuis la 6.0.14, l'AppImage est construite dans le conteneur `ubuntu:22.04`, épinglé par son empreinte, sur l'image `ubuntu-24.04`. La même glibc est demandée, et le contenu est celui de la 6.0.13, au dossier de compilation et à une mise à jour de glib d'Ubuntu près.

## 3. Optimisation (fait)

- [x] **« Tests moteur » et « Application Qt » fusionnés** en un job « Tests » par système : tout compiler une fois, tout tester.
- [x] **`ctest --parallel 4`** : la suite `dev` passe de 187 s à 100 s avec `-j8` sur un Mac, sans échec. `interface.demo` seul prend 99 s et fixe la limite.
- [x] **ASan requis sous Linux seulement**, macOS sur `main` et dans le run hebdomadaire. Un bogue propre à macOS sera vu après la fusion.
- [x] **Action composite `prepare-qt`** : les dépendances et la version de Qt ne sont écrites qu'à un endroit.
- [x] **`retention-days: 14`** sur les artefacts, au lieu de 90 jours.
- [x] **`concurrency`** : seuls les runs de PR s'annulent, ceux de `main` vont au bout.
- [x] **Nettoyage** : `port/**` retiré, une seule liste de chemins (job « changements »), aussi appliquée à `main`, `HOMEBREW_NO_AUTO_UPDATE`.

Pas de `ccache` pour l'instant : la compilation prend 2 min sous Linux, et ce sont les tests qui coûtent.

## 4. Manière de faire (fait)

- [x] **`tests-moteur.yml` renommé `ci.yml`** : il teste aussi l'application, ASan et le lint.
- [x] **Lint requis** : les 8 checks exigés comprennent les deux jobs de lint, pour que la fusion automatique ne puisse plus passer outre.
- [x] **Réglages du dépôt** : branches supprimées après fusion, merge commit seul autorisé.
- [x] **`SECURITY.md`** et signalement privé des failles activé.
- [x] **Couverture chaque semaine**, en artefact et sans seuil : un pourcentage imposé pousserait à tester les boutons d'aide. De la 6.0.15 à la 6.0.20, ce job mettait `src/` de côté pour vérifier que la compilation et les tests ne le lisaient pas ; `src/` est retiré du dépôt depuis la 6.0.21.

## Écarté volontairement

Sans intérêt pour un dépôt à un seul mainteneur et un jeu à un seul joueur : relectures obligatoires, `CODEOWNERS`, merge queue, commits signés, `harden-runner`, signature et notarisation des paquets.

## Linter (fait)

- **C++** : `clang-tidy` (`.clang-tidy` : bogues, CERT, analyseur, C++ Core Guidelines, modernisation, performance, portabilité, lisibilité), sans constat restant, et `clang-format` (`.clang-format`). Depuis la 6.0.17, le moindre constat ou écart de mise en forme fait échouer la CI.
- **Python** : `ruff` (`ruff.toml`), sans constat restant.
- **Workflows** : `actionlint`, `shellcheck` pour leurs scripts et `zizmor` pour leur sécurité, sans constat restant.
- **Versions** : figées dans `tools/lint/requirements.txt`.

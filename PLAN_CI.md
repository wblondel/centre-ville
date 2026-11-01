# Plan d'amélioration de la CI

État mesuré le 2026-11-01, sur les derniers runs réussis. Les points sont classés par ordre d'exécution. Une case cochée est faite.

## État actuel

| Workflow | Déclencheur | Coût d'un run | Attente |
|---|---|---|---|
| `tests-moteur.yml` | PR et `main` | 90 min-machine | 27 min (ASan macOS) |
| `build-installer.yml` | tag `v*` | 34 min-machine | environ 25 min |
| `recette-windows.yml` | appelé avant la release, ou à la main | 10 min-machine | 7 min |
| `deploy-docs.yml` | `docs/` sur `main` | moins de 1 min | moins de 1 min |

Déjà en place : permissions en lecture par défaut, écriture limitée au job de release, CodeQL (actions, C++, Python), détection de secrets avec blocage au push, alertes Dependabot, Catch2 vérifié par SHA256, 10 jobs requis sur `main`.

## 1. Sécurité de la chaîne de release (fait)

- [x] **`linuxdeploy`** : versions figées et SHA256 vérifiés, au lieu de `continuous` sans contrôle. Leur contenu part dans l'AppImage.
- [x] **Actions épinglées par SHA**, avec `.github/dependabot.yml` pour les mises à jour (`github-actions` et `pip`, groupées, mensuelles).
- [x] **`softprops/action-gh-release` retiré** : c'était la seule action tierce avec `contents: write`, et `gh release create` fait le même travail.
- [x] **Installeur 5.4.3.2** : SHA256 vérifié avant de l'exécuter.
- [x] **Injection de `inputs.tag`** : passé par `env:`.
- [x] **`persist-credentials: false`** sur tous les checkouts.
- [x] **Inno Setup** : version figée (6.7.1).
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
- [ ] **Remplacer `ubuntu-22.04`** pour l'AppImage, gardée pour sa glibc 2.35 : son support standard finit en avril 2027.

## 3. Optimisation (fait)

- [x] **« Tests moteur » et « Application Qt » fusionnés** en un job « Tests » par système : tout compiler une fois, tout tester.
- [x] **`ctest --parallel 4`** : la suite `dev` passe de 187 s à 100 s avec `-j8` sur un Mac, sans échec. `interface.demo` seul prend 99 s et fixe la limite.
- [x] **ASan requis sous Linux seulement**, macOS sur `main` et dans le run hebdomadaire. Un bogue propre à macOS sera vu après la fusion.
- [x] **Action composite `prepare-qt`** : les dépendances et la version de Qt ne sont écrites qu'à un endroit.
- [x] **`retention-days: 14`** sur les artefacts, au lieu de 90 jours.
- [x] **`concurrency`** : seuls les runs de PR s'annulent, ceux de `main` vont au bout.
- [x] **Nettoyage** : `port/**` retiré, une seule liste de chemins (job « changements »), aussi appliquée à `main`, `HOMEBREW_NO_AUTO_UPDATE`.

Pas de `ccache` pour l'instant : la compilation prend 2 min sous Linux, et ce sont les tests qui coûtent.

## 4. Manière de faire (priorité basse)

- [ ] **Renommer `tests-moteur.yml`** en `ci.yml` : il teste aussi l'application, ASan et le lint.
- [ ] **Réglages du dépôt** : `delete_branch_on_merge` activé, squash et rebase désactivés puisque seules les fusions par merge commit sont utilisées.
- [ ] **`SECURITY.md`** : comment signaler une faille.
- [ ] **Couverture en artefact**, sans seuil bloquant : `llvm-cov` existe déjà en local, et un pourcentage imposé pousserait à tester les boutons d'aide.

## Écarté volontairement

Sans intérêt pour un dépôt à un seul mainteneur et un jeu à un seul joueur : relectures obligatoires, `CODEOWNERS`, merge queue, commits signés, `harden-runner`, signature et notarisation des paquets.

## Linter (fait)

- **C++** : `clang-tidy`, règles de bogues et de performance (`.clang-tidy`). Le code porté garde le style de l'original pour rester comparable à `src/` : pas de `clang-format`, et les 251 constats existants sont dans `tools/lint/clang-tidy-reference.txt`. Seuls les nouveaux font échouer la CI.
- **Python** : `ruff` (`ruff.toml`), sans constat restant.
- **Workflows** : `actionlint`, `shellcheck` pour leurs scripts et `zizmor` pour leur sécurité, sans constat restant.
- **Versions** : figées dans `tools/lint/requirements.txt`.

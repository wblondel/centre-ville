# Plan d'amélioration de la CI

État mesuré le 2026-11-01, sur les derniers runs réussis. Les points sont classés par ordre d'exécution. Une case cochée est faite.

## État actuel

| Workflow | Déclencheur | Coût d'un run | Attente |
|---|---|---|---|
| `tests-moteur.yml` | PR et `main` | 90 min-machine | 27 min (ASan macOS) |
| `build-installer.yml` | tag `v*` | 34 min-machine | environ 25 min |
| `recette-windows.yml` | appelé après la release, ou à la main | 10 min-machine | 7 min |
| `deploy-docs.yml` | `docs/` sur `main` | moins de 1 min | moins de 1 min |

Déjà en place : permissions en lecture par défaut, `contents: write` limité au job de release, CodeQL (actions, C++, Python), détection de secrets avec blocage au push, alertes Dependabot, Catch2 vérifié par SHA256, 10 jobs requis sur `main`.

## 1. Sécurité de la chaîne de release (priorité haute)

Ces outils touchent des paquets publiés ou tournent avec un jeton.

- [ ] **`linuxdeploy` téléchargé depuis `continuous`, sans contrôle** (`build-installer.yml`), et son contenu part dans l'AppImage. Figer une version (`1-alpha-20251107-1`, plugin Qt `1-alpha-20250213-1`) et vérifier le SHA256. C'est le risque le plus concret.
- [ ] **Actions épinglées par étiquette** (37 constats `zizmor`), dont `softprops/action-gh-release` avec `contents: write`. Épingler par SHA, ajouter `.github/dependabot.yml` (`github-actions`, `pip`), puis activer `sha_pinning_required`.
- [ ] **Installeur 5.4.3.2 téléchargé en HTTP et exécuté** (`recette-windows.yml`). Vérifier le SHA256 (`5b2e3908…a3e6f0`) avant de le lancer.
- [ ] **Injection de `inputs.tag`** dans un script PowerShell (`recette-windows.yml`). Passer par `env:`.
- [ ] **`persist-credentials: false`** sur les checkouts (16 constats `artipacked`).
- [ ] **Inno Setup sans version** (`choco install innosetup`). Figer la version.
- [ ] **`deploy-docs.yml`** : `pages: write` et `id-token: write` au niveau du job.
- [ ] **Règles du dépôt** : bloquer force-push et suppression de `main` (la règle actuelle n'exige que les checks) ; protéger les tags `v*`, qui déclenchent une release.
- [ ] **`zizmor` bloquant** dans le job de lint, une fois les points ci-dessus faits.
- [ ] **Empreintes et provenance** : joindre `SHA256SUMS` à chaque release, et `actions/attest-build-provenance`. Les paquets n'étant pas signés, c'est la seule preuve de leur origine.

## 2. Fiabilité des releases (priorité haute)

- [ ] **Recette Windows avant la publication**, sur l'artefact et non sur la release. Aujourd'hui, une recette en échec laisse une release publiée.
- [ ] **`timeout-minutes` sur tous les jobs** : seul le job ASan en a un, les autres ont le défaut de 6 h.
- [ ] **Runners et Qt figés** : `ubuntu-24.04`, `macos-15`, `windows-2025` plutôt que `-latest`, et une version exacte de Qt plutôt que `6.8.*`. Un changement d'image ne doit pas casser une release sans modification du dépôt. Prévoir le remplacement de `ubuntu-22.04`, dont le support standard finit en avril 2027.
- [ ] **Run hebdomadaire** de la suite complète, pour voir les dérives d'images sans attendre une PR.

## 3. Optimisation (priorité moyenne)

- [ ] **Fusionner « Tests moteur » et « Application Qt »** en un job par système : tout compiler une fois, tout tester. « Application Qt » compile déjà les tests du moteur sans les lancer. Il faudra mettre à jour les checks requis.
- [ ] **`ctest --parallel`** : la suite `dev` passe de 187 s à 100 s avec `-j8` sur un Mac, sans échec. `interface.demo` seul prend 99 s et fixe la limite ; à mesurer sur les machines de CI, qui ont moins de cœurs. Le découper irait plus loin.
- [ ] **ASan requis sur Linux seulement**, macOS sur `main` et dans le run hebdomadaire : 27 min-machine de moins par PR. En contrepartie, un bogue propre à macOS serait vu après la fusion.
- [ ] **Action composite** pour les dépendances et Qt : le même bloc est répété 7 fois.
- [ ] **`retention-days`** sur les captures et les constats : 90 jours par défaut, 14 suffisent.
- [ ] **`concurrency`** : ne pas annuler les runs de `main`, seulement ceux des PR.
- [ ] **Nettoyage** : retirer `port/**`, et la liste de chemins en double entre `paths:` et le job « changements » ; `HOMEBREW_NO_AUTO_UPDATE` sur macOS.

Je ne ferais pas `ccache` pour l'instant : la compilation prend 2 min sous Linux, et ce sont les tests qui coûtent.

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
- **Workflows** : `actionlint`, sans constat restant.
- **Versions** : figées dans `tools/lint/requirements.txt`.

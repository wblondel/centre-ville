# Sécurité

## Signaler une faille

Utilisez le [signalement privé de GitHub](https://github.com/wblondel/centre-ville/security/advisories/new) plutôt qu'une issue publique. Décrivez la version touchée, comment reproduire le problème, et ce qu'il permet. Centre-Ville est maintenu par une seule personne, sur son temps libre : aucun délai de réponse n'est garanti.

Un fichier `.cvw` qui fait planter l'application ou lui fait lire ou écrire hors de la mémoire prévue est un signalement bienvenu.

## Versions suivies

Seule la dernière version publiée reçoit des corrections.

## Vérifier un paquet

Les paquets ne sont pas signés : SmartScreen et Gatekeeper avertissent au premier lancement. Les releases publiées depuis `v6.0.6-verif1` sont immuables et permettent de vérifier l'origine et l'intégrité d'un paquet :

```bash
gh attestation verify Centre-Ville-<version>-macOS.dmg --repo wblondel/centre-ville
sha256sum --check --ignore-missing SHA256SUMS
```

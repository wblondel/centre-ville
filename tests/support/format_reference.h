//---------------------------------------------------------------------------
// format_reference.h - Tests du moteur : spécification exécutable du .cvw
//
// Lecture/réécriture d'un fichier .cvw (versions 0x300 à 0x530) champ par
// champ, INDÉPENDAMMENT du moteur, d'après le format décrit par
// centre_ville::Charge/Sauve. Sert à vérifier le round-trip des cartes
// enregistrées dans un format antérieur : le moteur doit produire exactement
// la conversion « naïve » en 0x530 (champs absents = valeurs par défaut du
// constructeur), aux écarts près que le chargement corrige volontairement
// (feux fantômes, parkings et places indéfinis), listés champ par champ.
//---------------------------------------------------------------------------
#ifndef format_referenceH
#define format_referenceH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------
struct champ_cvw
 {
  std::size_t Debut, Taille; // position dans le fichier converti
  std::string Nom; // ex. "case(12,5).NumFeu[2]"
 };
//---------------------------------------------------------------------------
struct conversion_cvw
 {
  bool Ok=false;
  int Version=0;
  std::string Erreur;
  std::string Octets; // fichier au format 0x530
  std::vector<champ_cvw> Champs; // description de chaque champ écrit
 };
//---------------------------------------------------------------------------
// Convertit le contenu d'un .cvw (0x300 <= version <= 0x530) au format 0x530
conversion_cvw ConvertitEn530(const std::string &asOrigine);
//---------------------------------------------------------------------------
// Décrit les écarts entre la conversion de référence et un fichier produit
// par le moteur : une ligne par champ différent (« nom : attendu -> obtenu »).
std::vector<std::string> EcartsParChamp(const conversion_cvw &Reference, const std::string &asObtenu);
//---------------------------------------------------------------------------
#endif

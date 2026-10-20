//---------------------------------------------------------------------------
// etat.h - Tests du moteur : export de l'état complet de la simulation
//
// L'état est parcouru dans un ordre fixe et transmis à un « puits » :
//  - PuitsTexte : texte lisible, une ligne par objet (pour comparer à la main
//    avec l'oracle instrumenté ou entre deux versions) ;
//  - PuitsEmpreinte : empreinte 64 bits (FNV-1a par mot) des mêmes valeurs (pour le
//    golden master tour par tour).
// Les sections couvrent : centre_ville (compteurs, paramètres, statistiques),
// feux, feux piétons, parkings, véhicules, dépôt/bus/arrêts/lignes de bus,
// trams/arrêts/lignes de tram, taxis/places/files, places et files de
// parking, vehlibs/places, piétons (avec la chaîne d'infections : épidémie)
// et cases (contenu statique et dynamique, attente/trafic).
//---------------------------------------------------------------------------
#ifndef etatH
#define etatH
//---------------------------------------------------------------------------
#include <cstdint>
#include <string>
#include "simul.h"
//---------------------------------------------------------------------------
// Puits : reçoit les valeurs dans l'ordre du parcours. Le parcours est un
// modèle (template) instancié pour chaque puits : pas d'appel virtuel, et les
// noms de champs ne sont fabriqués que si le puits les utilise (Noms).
class PuitsTexte
 {
  bool Ignore=false;
  public:
  static const bool Noms=true;
  std::string Texte;
  void Section(const char *Nom);
  // Ignorable : l'objet est entièrement « vide » (valeurs par défaut) ; le
  // texte l'omet, l'empreinte le prend toujours en compte.
  void Objet(const char *Type, int Numero, bool Ignorable=false);
  void Champ(const char *Nom, long long Valeur);
  void FinObjet();
 };
//---------------------------------------------------------------------------
class PuitsEmpreinte
 {
  public:
  static const bool Noms=false;
  std::uint64_t Empreinte=14695981039346656037ULL; // FNV-1a 64 bits, par mot de 64 bits
  void Ajoute(std::uint64_t v)
   {
    Empreinte^=v;
    Empreinte*=1099511628211ULL;
    Empreinte^=Empreinte>>29; // brassage : chaque mot influe sur tous les bits
   }
  void Section(const char *Nom) { for(const char *c=Nom; *c; c++) Ajoute((unsigned char)*c); }
  void Objet(const char *, int Numero, bool=false) { Ajoute(0xFFFFFFFF00000000ULL^(std::uint64_t)(std::uint32_t)Numero); }
  void Champ(const char *, long long Valeur) { Ajoute((std::uint64_t)Valeur); }
  void FinObjet() {}
 };
//---------------------------------------------------------------------------
// Parties de l'état (une empreinte par partie dans le golden master)
enum partie_etat
 {
  peCentreVille, peFeux, peFeuxPietons, peParkings, peVehicules, peBus,
  peTrams, peTaxis, peParkingsPlaces, peVehlibs, pePietons, peCases,
  peDistances, // statique après CalculeDistances : hors état « complet »
  NB_PARTIES_ETAT
 };
extern const char *NomPartieEtat[NB_PARTIES_ETAT];
//---------------------------------------------------------------------------
class ExportEtat // ami de la classe infection (champs privés)
 {
  public:
  // Exporte une partie de l'état de cv ; Detail : tableaux attente/trafic
  // complets (sinon seulement les totaux [0][0], suffisants tour par tour)
  template <class P> static void Exporte(const centre_ville *cv, partie_etat Partie, P &p, bool Detail);
  // Toutes les parties (sauf les distances), en texte
  static std::string Texte(const centre_ville *cv, bool Detail);
  static std::uint64_t Empreinte(const centre_ville *cv, partie_etat Partie, bool Detail);
  private:
  template <class P> static void ExporteInfections(const pieton *p, P &s);
 };
//---------------------------------------------------------------------------
std::string Hexa(std::uint64_t v);
//---------------------------------------------------------------------------
#endif

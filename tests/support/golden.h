//---------------------------------------------------------------------------
// golden.h - Tests du moteur : golden master de la simulation
//
// Pour une carte et une configuration, à graine fixe :
//   AppelleAuChaos() -> srand(GRAINE_GOLDEN) (graine forcée, voir compat/alea.h)
//   Ouvre la carte, démarre la simulation (séquence d'ActionMarche), puis
//   effectue NB_TOURS_GOLDEN ticks du timer.
// Le texte produit contient, pour chaque tour, l'état du générateur
// aléatoire, quelques compteurs lisibles et une empreinte par partie de
// l'état complet (voir etat.h) ; puis l'état complet en texte au dernier
// tour. Il est comparé aux fichiers de tests/golden/.
//---------------------------------------------------------------------------
#ifndef goldenH
#define goldenH
//---------------------------------------------------------------------------
#include <string>
#include "simul.h"
//---------------------------------------------------------------------------
struct config_golden
 {
  const char *Nom;
  bool Sanglante; // ActionSanglante->Checked
  bool Epidemie; // ActionEpidemie->Checked
  int TourResurrection; // > 0 : résurrection demandée avant ce tour
 };
//---------------------------------------------------------------------------
const unsigned GRAINE_GOLDEN=12345;
const int NB_TOURS_GOLDEN=500;
const config_golden ConfigsGolden[]=
 {
  {"standard", false, false, 0},
  {"epidemie", true, true, 250}, // épidémie + mode sanglant + résurrection
 };
const int NB_CONFIGS_GOLDEN=sizeof(ConfigsGolden)/sizeof(ConfigsGolden[0]);
//---------------------------------------------------------------------------
std::string ProduitGolden(const std::string &asCheminCarte, const std::string &asNomCarte,
                          const config_golden &Config);
//---------------------------------------------------------------------------
// Nom du fichier de référence, relatif à tests/golden/
std::string FichierGolden(const std::string &asNomCarte, const config_golden &Config);
//---------------------------------------------------------------------------
// Génération aléatoire de réseau (Fichier | Nouveau, fiche Génération) :
// centre_ville par défaut NbX x NbY, paramètres par défaut de f_genres.dfm,
// point de départ tiré par rand() (x, puis y, puis direction : ordre du binaire d'origine).
struct config_generation
 {
  const char *Nom;
  unsigned Graine;
  int NbX, NbY;
 };
const config_generation ConfigsGeneration[]=
 {
  {"defaut_1", 1, NBXDEF, NBYDEF},
  {"defaut_12345", 12345, NBXDEF, NBYDEF},
  {"petit_777", 777, 40, 30},
 };
const int NB_CONFIGS_GENERATION=sizeof(ConfigsGeneration)/sizeof(ConfigsGeneration[0]);
std::string ProduitGoldenGeneration(const config_generation &Config, const std::string &asFichierCvw);
//---------------------------------------------------------------------------
// « Copier vers... » (PrepareCopie/CopieValide/Sauve) : sélection complète
// puis sélection réduite d'une case sur chaque bord.
std::string ProduitGoldenCopie(const std::string &asCheminCarte, const std::string &asNomCarte,
                               const std::string &asDossierSortie);
//---------------------------------------------------------------------------
#endif

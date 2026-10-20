//---------------------------------------------------------------------------
// harnais.h - Tests du moteur : pilotage de la simulation sans interface
//
// Reproduit, dans le même ordre, les appels au moteur faits par la fiche
// principale d'origine (src/f_simul.cpp) :
//  - Ouvre()         : TfrmSimulation::ActionOuvrirExecute
//  - Sauve()         : TfrmSimulation::ActionEnregistrerExecute
//  - VerifieReseau() : TfrmSimulation::VerifieReseau
//  - Demarre()       : TfrmSimulation::ActionMarcheExecute (premier démarrage)
//  - Tick()          : TfrmSimulation::TimerSimulTimer
// Seuls les effets d'affichage (barre d'état, curseur, menus...) sont omis.
//---------------------------------------------------------------------------
#ifndef harnaisH
#define harnaisH
//---------------------------------------------------------------------------
#include <string>
#include "simul.h"
#include "vue.h"
//---------------------------------------------------------------------------
namespace harnais
 {
  // Charge un fichier .cvw dans un nouveau centre_ville (variable globale cv).
  // Comme ActionOuvrir, commence par AppelleAuChaos().
  bool Ouvre(const std::string &asNomFichier);
  // Enregistre cv (comme ActionEnregistrer : AppelleAuChaos() puis Sauve)
  bool Sauve(const std::string &asNomFichier);
  // Enregistre cv sans appeler AppelleAuChaos()
  bool SauveSansChaos(const std::string &asNomFichier);
  // Port de TfrmSimulation::VerifieReseau. Renvoie true si le réseau est correct.
  bool VerifieReseau(AnsiString &asMsgErr);
  // Port de la partie « !Pause » de TfrmSimulation::ActionMarcheExecute, avec
  // VerifierAvantSimulation=false. L'enregistrement automatique qui précède
  // le démarrage est remplacé par son seul effet sur le moteur :
  // AppelleAuChaos() (Sauve ne modifie pas l'état).
  bool Demarre(AnsiString &asMsgErr);
  // Un tick du timer de simulation. Renvoie false si la simulation est
  // terminée (tous les véhicules arrivés), true si un tour a été effectué.
  bool Tick();
  // Libère cv
  void Ferme();
  // Lit un fichier entier
  std::string LitFichier(const std::string &asNomFichier);
 }
//---------------------------------------------------------------------------
#endif

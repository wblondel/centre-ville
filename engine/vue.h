//---------------------------------------------------------------------------
// vue.h - Portage de Centre-Ville : découplage moteur <-> interface
//
// Le moteur d'origine (src/simul.cpp) appelait directement les fiches VCL
// (frmSimulation, frmStatistiques, frmInfos*, frmRecherche...) et
// AfficheMessage (base.cpp). Chacun de ces accès est remplacé, un pour un et
// dans le même ordre, par un appel à l'interface IVueSimulation ci-dessous,
// via le pointeur global « vue » (homologue des pointeurs globaux de fiches).
// L'inventaire complet des accès remplacés est dans engine/README.md.
//
// Une implémentation VueNulle (sans affichage) sert aux tests et aux outils
// en ligne de commande.
//---------------------------------------------------------------------------
#ifndef vueH
#define vueH
//---------------------------------------------------------------------------
#include <vector>
#include "compat/chaine.h"
//---------------------------------------------------------------------------
// Types déplacés de src/base.h (paramètres et retour d'AfficheMessage)
//---------------------------------------------------------------------------
enum type_message : signed char {tmInformation, tmAvertissement, tmErreur, tmQuestionNecessaire, tmQuestionFacultative};
enum type_reponse : signed char {trOk, trAnnuler, trOui, trNon};
//---------------------------------------------------------------------------
// Fiches non modales rafraîchies à chaque tour (simul.cpp, fin de EffectueTourSimulation)
enum fiche_info : signed char
 {
  fiStatistiques,   // frmStatistiques   (rafraîchissement = PaintBoxStats->Repaint())
  fiStatsVehicules, // frmStatsVehicules (rafraîchissement = RafraichitInfos())
  fiInfosVehicule,  // frmInfosVehicule  (idem)
  fiInfosBus,       // frmInfosBus       (idem)
  fiInfosTram,      // frmInfosTram      (idem)
  fiInfosTaxi,      // frmInfosTaxi      (idem)
  fiInfosVehlib,    // frmInfosVehlib    (idem)
  fiInfosPieton     // frmInfosPieton    (idem)
 };
//---------------------------------------------------------------------------
// Objets recherchés par frmRecherche (RadioButtonXxx / CSpinEditXxx)
enum objet_recherche : signed char {orVehicule, orPieton, orBus, orTram, orTaxi, orVehlib};
//---------------------------------------------------------------------------
class IVueSimulation
 {
  public:
  virtual ~IVueSimulation() {}
  // --- frmSimulation ---
  // frmSimulation->RedessineCase(x, y, EffaceFond)
  virtual void RedessineCase(int x, int y, bool EffaceFond) = 0;
  // frmSimulation->SimulationSanglante (lecture)
  virtual bool SimulationSanglante() const = 0;
  // frmSimulation->EpidemieActivee (lecture)
  virtual bool EpidemieActivee() const = 0;
  // frmSimulation->ActionEpidemie->Checked (lecture)
  virtual bool ActionEpidemieCochee() const = 0;
  // frmSimulation->ResurrectionDemandee (lecture)
  virtual bool ResurrectionDemandee() const = 0;
  // frmSimulation->ResurrectionDemandee=Demandee (écriture)
  virtual void DefinitResurrectionDemandee(bool Demandee) = 0;
  // frmSimulation->SimulationModifiee=Modifiee (écriture)
  virtual void DefinitSimulationModifiee(bool Modifiee) = 0;
  // frmSimulation->StatusBar->Panels->Items[NumPanneau]->Text=asTexte
  virtual void AfficheStatut(int NumPanneau, const AnsiString &asTexte) = 0;
  // frmSimulation->StatusBar->Repaint()
  virtual void RepeintStatut() = 0;
  // --- Fiches d'informations et de statistiques ---
  // frmXxx->Visible
  virtual bool FicheVisible(fiche_info Fiche) const = 0;
  // frmStatistiques->PaintBoxStats->Repaint() ou frmXxx->RafraichitInfos()
  virtual void RafraichitFiche(fiche_info Fiche) = 0;
  // --- frmRecherche ---
  // frmRecherche&&frmRecherche->Visible
  virtual bool RechercheVisible() const = 0;
  // frmRecherche->RadioButtonXxx->Checked
  virtual bool RechercheCochee(objet_recherche Objet) const = 0;
  // frmRecherche->CSpinEditXxxChange(frmRecherche->CSpinEditXxx)
  virtual void RechercheActualise(objet_recherche Objet) = 0;
  // --- base.cpp ---
  // AfficheMessage(asMessage, asTitre, Type, OrdreBoutonDefaut, OrdreBoutonEchap)
  virtual type_reponse AfficheMessage(const AnsiString &asMessage, const AnsiString &asTitre,
                                      type_message Type, int OrdreBoutonDefaut, int OrdreBoutonEchap) = 0;
 };
//---------------------------------------------------------------------------
// Pointeur global utilisé par le moteur à la place de frmSimulation & co.
// Comme les pointeurs de fiches de la VCL, il vaut NULL tant que l'interface
// n'a pas été créée : c'est à l'application (ou au test) de l'affecter.
extern IVueSimulation *vue;
//---------------------------------------------------------------------------
// Implémentation sans affichage : les options de simulation sont des champs
// publics, les messages sont mémorisés pour pouvoir être vérifiés.
//---------------------------------------------------------------------------
class VueNulle : public IVueSimulation
 {
  public:
  struct message
   {
    AnsiString asMessage, asTitre;
    type_message Type;
   };
  bool Sanglante=false, // ActionSanglante->Checked
       Epidemie=false, // ActionEpidemie->Checked (EpidemieActivee en est la lecture)
       Resurrection=false, // ActionResurrection->Checked
       Modifiee=false; // SimulationModifiee
  type_reponse Reponse=trOk; // Réponse renvoyée par AfficheMessage
  long long NbRedessins=0;
  std::vector<message> Messages;
  AnsiString asStatut[4];
  void RedessineCase(int, int, bool) override { NbRedessins++; }
  bool SimulationSanglante() const override { return Sanglante; }
  bool EpidemieActivee() const override { return Epidemie; }
  bool ActionEpidemieCochee() const override { return Epidemie; }
  bool ResurrectionDemandee() const override { return Resurrection; }
  void DefinitResurrectionDemandee(bool Demandee) override { Resurrection=Demandee; }
  void DefinitSimulationModifiee(bool Modif) override { Modifiee=Modif; }
  void AfficheStatut(int NumPanneau, const AnsiString &asTexte) override
   {
    if ((NumPanneau>=0)&&(NumPanneau<4)) asStatut[NumPanneau]=asTexte;
   }
  void RepeintStatut() override {}
  bool FicheVisible(fiche_info) const override { return false; }
  void RafraichitFiche(fiche_info) override {}
  bool RechercheVisible() const override { return false; }
  bool RechercheCochee(objet_recherche) const override { return false; }
  void RechercheActualise(objet_recherche) override {}
  type_reponse AfficheMessage(const AnsiString &asMessage, const AnsiString &asTitre,
                              type_message Type, int, int) override
   {
    Messages.push_back(message{asMessage, asTitre, Type});
    return Reponse;
   }
 };
//---------------------------------------------------------------------------
#endif

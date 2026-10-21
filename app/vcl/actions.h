//---------------------------------------------------------------------------
// vcl/actions.h - Portage de Centre-Ville : TAction, TActionList, menus,
//                 raccourcis clavier
//
// Le modèle de la VCL est reproduit tel quel :
//  - une TAction est un objet non visuel (état Caption/Checked/Enabled...,
//    événement OnExecute) ; les menus et les boutons qui lui sont liés en sont
//    des « clients » mis à jour par un lien d'action (TActionLink) ;
//  - le lien ne propage un changement (Checked, Enabled, Visible) à un client
//    que si celui-ci avait la même valeur que l'action avant le changement
//    (IsCheckedLinked & co.), et l'affectation de Action à l'exécution recopie
//    toutes les propriétés de l'action ;
//  - chaque TMenuItem est un objet à part (un QAction pour une feuille, un
//    QMenu pour un sous-menu), même s'il est lié à une action ;
//  - TAction::Execute ne fait rien si l'action est désactivée.
//
// Raccourcis : Qt refuse les raccourcis ambigus, or Centre-Ville en a beaucoup
// (Ctrl+B = « Passage de bus » en édition, « Détail sur un bus » en
// simulation...). C'est la VCL qui choisit selon le contexte : menu contextuel
// du contrôle actif, puis menu principal, puis listes d'actions de la fiche
// (TWinControl::IsMenuKey). RepartiteurRaccourcis reproduit cet ordre : il
// réclame la touche (ShortcutOverride accepté) puis exécute l'élément trouvé.
//---------------------------------------------------------------------------
#ifndef vcl_actionsH
#define vcl_actionsH
//---------------------------------------------------------------------------
#include <QIcon>
#include <QKeySequence>
#include <map>
#include <vector>
#include "vcl.h"
//---------------------------------------------------------------------------
class QAction;
class QActionGroup;
class QKeyEvent;
class QMenu;
class QMenuBar;
class QWidget;
class TAction;
class TActionList;
class TMenuItem;
class TPopupMenu;
//---------------------------------------------------------------------------
// TShortCut : code de touche virtuelle Windows + scShift/scCtrl/scAlt
//---------------------------------------------------------------------------
typedef int TShortCut;
const TShortCut scShift=0x2000, scCtrl=0x4000, scAlt=0x8000;
TShortCut RaccourciVcl(const QKeyEvent *Evenement); // 0 si la touche n'a pas d'équivalent
QKeySequence SequenceQt(TShortCut Raccourci);
//---------------------------------------------------------------------------
// Client d'une action (TMenuItem, TToolButton) : homologue de TActionLink
//---------------------------------------------------------------------------
class ClientAction
 {
  public:
  virtual ~ClientAction() {}
  // Recopie complète (affectation de Action à l'exécution : CheckDefaults=false)
  virtual void ActionChange(TAction *Action) = 0;
  // Propagation d'un changement d'état (seulement si le client suivait l'action)
  virtual void LienChecked(bool Ancien, bool Nouveau) = 0;
  virtual void LienEnabled(bool Ancien, bool Nouveau) = 0;
  virtual void LienVisible(bool Ancien, bool Nouveau) = 0;
 };
//---------------------------------------------------------------------------
class TAction : public QObject
 {
  friend class TActionList;
  AnsiString asCaption, asHint;
  bool bChecked=false, bEnabled=true, bVisible=true, Verification=false;
  std::vector<ClientAction *> Clients;
  void DefinitChecked(bool Valeur);
  void DefinitEnabled(bool Valeur);
  void DefinitVisible(bool Valeur);
  public:
  TActionList *ActionList=nullptr;
  AnsiString Category;
  int ImageIndex=-1, GroupIndex=0, Tag=0, HelpContext=0;
  TShortCut ShortCut=0;
  bool AutoCheck=false;
  QIcon Icone;
  Propriete<AnsiString> Caption, Hint;
  Propriete<bool> Checked, Enabled, Visible;
  TNotifyEvent OnExecute;
  // Création à partir de l'<action> du .ui (propriétés de la VCL en « vcl... »)
  TAction(QAction *Modele, TActionList *Liste);
  bool Execute();
  void AjouteClient(ClientAction *c);
  void RetireClient(ClientAction *c);
 };
//---------------------------------------------------------------------------
class TActionList : public QObject
 {
  public:
  std::vector<TAction *> Actions;
  Propriete<int> ActionCount;
  TActionList(QObject *Parent) : QObject(Parent)
   {
    ActionCount.Definit([this]() { return (int)Actions.size(); }, [](const int &) {});
   }
  bool IsShortCut(TShortCut Raccourci, bool Executer);
 };
//---------------------------------------------------------------------------
// TMenuItem : feuille (QAction) ou sous-menu (QMenu)
//---------------------------------------------------------------------------
class TMenuItem : public QObject, public ClientAction
 {
  QAction *Element; // l'élément affiché (menuAction() pour un sous-menu)
  QMenu *SousMenu; // nullptr pour une feuille
  QActionGroup *Groupe=nullptr; // éléments radio (RadioItem) de même GroupIndex
  TAction *ActionLiee=nullptr;
  AnsiString asCaption, asHint;
  bool bChecked=false, bEnabled=true, bVisible=true;
  void AfficheCoche();
  void ElementDeclenche();
  void TurnSiblingsOff();
  public:
  TMenuItem *Parent=nullptr;
  std::vector<TMenuItem *> Items;
  int Tag=0, GroupIndex=0, ImageIndex=-1, HelpContext=0;
  bool RadioItem=false, AutoCheck=false;
  TShortCut ShortCut=0;
  Propriete<AnsiString> Caption, Hint;
  Propriete<bool> Checked, Enabled, Visible;
  Propriete<TAction *> Action;
  TNotifyEvent OnClick;
  TMenuItem(QAction *Feuille, QObject *Parent);
  TMenuItem(QMenu *Menu, QObject *Parent);
  QAction *ElementQt() const { return Element; }
  QMenu *MenuQt() const { return SousMenu; }
  int Count() const { return (int)Items.size(); }
  void Click();
  TMenuItem *Find(TShortCut Raccourci); // TMenu::FindItem(..., fkShortCut)
  void LieAction(TAction *a); // lien établi au chargement (sans recopie)
  void DefinitGroupe(QActionGroup *g);
  // ClientAction
  void ActionChange(TAction *a) override;
  void LienChecked(bool Ancien, bool Nouveau) override;
  void LienEnabled(bool Ancien, bool Nouveau) override;
  void LienVisible(bool Ancien, bool Nouveau) override;
 };
//---------------------------------------------------------------------------
class TMenu : public QObject
 {
  public:
  std::vector<TMenuItem *> Items;
  TMenu(QObject *Parent) : QObject(Parent) {}
  bool IsShortCut(TShortCut Raccourci, bool Executer);
 };
//---------------------------------------------------------------------------
class TMainMenu : public TMenu
 {
  public:
  QMenuBar *Barre;
  TMainMenu(QMenuBar *b);
 };
//---------------------------------------------------------------------------
class TPopupMenu : public TMenu
 {
  public:
  QMenu *Menu;
  TNotifyEvent OnPopup;
  TPopupMenu(QMenu *m);
  void Popup(int X, int Y); // coordonnées écran
 };
//---------------------------------------------------------------------------
// Contrôle possédant une propriété PopupMenu (TControl::PopupMenu)
//---------------------------------------------------------------------------
class AvecMenuContextuel
 {
  public:
  virtual ~AvecMenuContextuel() {}
  TPopupMenu *PopupMenu=nullptr;
 };
//---------------------------------------------------------------------------
// Répartition des raccourcis façon VCL
//---------------------------------------------------------------------------
struct FicheRaccourcis
 {
  QWidget *Fenetre=nullptr;
  TMainMenu *Menu=nullptr;
  std::vector<TActionList *> ListesActions;
  bool Principale=false;
  // KeyPreview : OnKeyDown/OnKeyUp de la fiche ; vrai si la touche est absorbée (Key=0)
  std::function<bool(QKeyEvent *, bool Relache)> Apercu;
  bool IsShortCut(TShortCut Raccourci, bool Executer); // TCustomForm::IsShortCut
 };
void InscritFicheRaccourcis(const FicheRaccourcis &Fiche);
void DesinscritFicheRaccourcis(QWidget *Fenetre);
//---------------------------------------------------------------------------
#endif

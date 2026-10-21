//---------------------------------------------------------------------------
// vcl/actions.cpp - Portage de Centre-Ville : actions, menus, raccourcis
//---------------------------------------------------------------------------
#include "actions.h"
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QPointer>
#include <QWidget>
#include <algorithm>
//---------------------------------------------------------------------------
// Touches
//---------------------------------------------------------------------------
namespace
 {
  // Touches à repérer par leur position physique (code natif) et non par le
  // caractère produit : rangée des chiffres (AZERTY : &é"'(...) et VK_OEM_5,
  // comme les codes de touches virtuelles de Windows
  int ToucheVirtuellePhysique(const QKeyEvent *e)
   {
#if defined(Q_OS_MACOS)
    switch(e->nativeVirtualKey()) // kVK_ANSI_xxx (Carbon)
     {
      case 0x12: return '1'; case 0x13: return '2'; case 0x14: return '3';
      case 0x15: return '4'; case 0x17: return '5'; case 0x16: return '6';
      case 0x1A: return '7'; case 0x1C: return '8'; case 0x19: return '9';
      case 0x1D: return '0';
      case 0x2A: return 0xDC; // kVK_ANSI_Backslash -> VK_OEM_5
      default: return 0;
     }
#elif defined(Q_OS_WIN)
    return (int)e->nativeVirtualKey(); // codes de touches virtuelles : exactement ceux de la VCL
#else
    switch(e->nativeScanCode()) // codes de touches X11/evdev+8
     {
      case 10: return '1'; case 11: return '2'; case 12: return '3'; case 13: return '4';
      case 14: return '5'; case 15: return '6'; case 16: return '7'; case 17: return '8';
      case 18: return '9'; case 19: return '0';
      case 51: return 0xDC; // touche « \ » -> VK_OEM_5
      default: return 0;
     }
#endif
   }
  //-------------------------------------------------------------------------
  int ToucheVirtuelle(int Touche, bool PaveNumerique)
   {
    if ((Touche>=Qt::Key_A)&&(Touche<=Qt::Key_Z)) return Touche; // 'A'..'Z'
    if ((Touche>=Qt::Key_0)&&(Touche<=Qt::Key_9))
     return PaveNumerique?0x60+(Touche-Qt::Key_0):Touche;
    if ((Touche>=Qt::Key_F1)&&(Touche<=Qt::Key_F24)) return 0x70+(Touche-Qt::Key_F1);
    switch(Touche)
     {
      case Qt::Key_Backspace: return 0x08;
      case Qt::Key_Tab: case Qt::Key_Backtab: return 0x09;
      case Qt::Key_Return: case Qt::Key_Enter: return 0x0D;
      case Qt::Key_Escape: return 0x1B;
      case Qt::Key_Space: return 0x20;
      case Qt::Key_PageUp: return 0x21;
      case Qt::Key_PageDown: return 0x22;
      case Qt::Key_End: return 0x23;
      case Qt::Key_Home: return 0x24;
      case Qt::Key_Left: return 0x25;
      case Qt::Key_Up: return 0x26;
      case Qt::Key_Right: return 0x27;
      case Qt::Key_Down: return 0x28;
      case Qt::Key_Insert: return 0x2D;
      case Qt::Key_Delete: return 0x2E;
      case Qt::Key_Asterisk: return PaveNumerique?0x6A:0;
      case Qt::Key_Plus: return PaveNumerique?0x6B:0xBB;
      case Qt::Key_Minus: return PaveNumerique?0x6D:0xBD;
      case Qt::Key_Slash: return PaveNumerique?0x6F:0xBF;
      case Qt::Key_Semicolon: return 0xBA;
      case Qt::Key_Equal: return 0xBB;
      case Qt::Key_Comma: return 0xBC;
      case Qt::Key_Period: return PaveNumerique?0x6E:0xBE;
      case Qt::Key_QuoteLeft: return 0xC0;
      case Qt::Key_BracketLeft: return 0xDB;
      case Qt::Key_Backslash: return 0xDC;
      case Qt::Key_BracketRight: return 0xDD;
      case Qt::Key_Apostrophe: return 0xDE;
      default: return 0;
     }
   }
  //-------------------------------------------------------------------------
  const char *NomTouche(int vk)
   {
    switch(vk)
     {
      case 0x08: return "Backspace"; case 0x09: return "Tab"; case 0x0D: return "Return";
      case 0x1B: return "Esc"; case 0x20: return "Space"; case 0x21: return "PgUp";
      case 0x22: return "PgDown"; case 0x23: return "End"; case 0x24: return "Home";
      case 0x25: return "Left"; case 0x26: return "Up"; case 0x27: return "Right";
      case 0x28: return "Down"; case 0x2D: return "Ins"; case 0x2E: return "Del";
      case 0xBA: return ";"; case 0xBB: return "="; case 0xBC: return ","; case 0xBD: return "-";
      case 0xBE: return "."; case 0xBF: return "/"; case 0xC0: return "`"; case 0xDB: return "[";
      case 0xDC: return "\\"; case 0xDD: return "]"; case 0xDE: return "'";
      default: return nullptr;
     }
   }
 }
//---------------------------------------------------------------------------
TShortCut RaccourciVcl(const QKeyEvent *e)
 {
  int Touche=e->key();
  if ((Touche==Qt::Key_Shift)||(Touche==Qt::Key_Control)||(Touche==Qt::Key_Alt)||
      (Touche==Qt::Key_Meta)||(Touche==Qt::Key_AltGr)||(Touche==Qt::Key_unknown)) return 0;
  bool PaveNumerique=e->modifiers()&Qt::KeypadModifier;
  int vk=PaveNumerique?0:ToucheVirtuellePhysique(e);
  if (!vk) vk=ToucheVirtuelle(Touche, PaveNumerique);
  if (!vk) return 0;
  Qt::KeyboardModifiers m=e->modifiers();
  TShortCut sc=vk;
  if (m&Qt::ShiftModifier) sc|=scShift;
  // Ctrl de la VCL = Qt::ControlModifier (⌘ sous macOS, comme les menus de Qt)
  // ou, sous macOS, la touche Control physique (Qt::MetaModifier) : les
  // raccourcis Ctrl+flèches et Ctrl+Espace, pris par le système avec Control,
  // restent accessibles avec ⌘, et ⌘Q/⌘H, pris par le système, avec Control.
  if (m&Qt::ControlModifier) sc|=scCtrl;
#if defined(Q_OS_MACOS)
  if (m&Qt::MetaModifier) sc|=scCtrl;
#endif
  if (m&Qt::AltModifier) sc|=scAlt;
  return sc;
 }
//---------------------------------------------------------------------------
QKeySequence SequenceQt(TShortCut Raccourci)
 {
  if (!Raccourci) return QKeySequence();
  int vk=Raccourci&0xFF;
  QString Nom;
  if (const char *n=NomTouche(vk)) Nom=QString::fromLatin1(n);
  else if (((vk>='0')&&(vk<='9'))||((vk>='A')&&(vk<='Z'))) Nom=QChar(vk);
  else if ((vk>=0x70)&&(vk<=0x87)) Nom=QString("F%1").arg(vk-0x6F);
  else if ((vk>=0x60)&&(vk<=0x69)) Nom=QString("Num+%1").arg(vk-0x60);
  else return QKeySequence();
  QString Texte;
  if (Raccourci&scCtrl) Texte+="Ctrl+";
  if (Raccourci&scAlt) Texte+="Alt+";
  if (Raccourci&scShift) Texte+="Shift+";
  return QKeySequence(Texte+Nom, QKeySequence::PortableText);
 }
//---------------------------------------------------------------------------
// TAction
//---------------------------------------------------------------------------
TAction::TAction(QAction *Modele, TActionList *Liste) : QObject(Liste), ActionList(Liste)
 {
  setObjectName(Modele->objectName());
  asCaption=A(Modele->text());
  asHint=A(Modele->statusTip());
  bChecked=Modele->isChecked();
  bEnabled=Modele->isEnabled();
  bVisible=Modele->isVisible();
  Category=A(Modele->property("categorie").toString());
  ImageIndex=Modele->property("vclImageIndex").isValid()?Modele->property("vclImageIndex").toInt():-1;
  GroupIndex=Modele->property("vclGroupIndex").toInt();
  ShortCut=Modele->property("vclShortCut").toInt();
  AutoCheck=Modele->property("vclAutoCheck").toBool();
  Tag=Modele->property("vclTag").toInt();
  HelpContext=Modele->property("vclHelpContext").toInt();
  Icone=Modele->icon();
  Caption.Definit([this]() { return asCaption; }, [this](const AnsiString &as) { asCaption=as; });
  Hint.Definit([this]() { return asHint; }, [this](const AnsiString &as) { asHint=as; });
  Checked.Definit([this]() { return bChecked; }, [this](const bool &b) { DefinitChecked(b); });
  Enabled.Definit([this]() { return bEnabled; }, [this](const bool &b) { DefinitEnabled(b); });
  Visible.Definit([this]() { return bVisible; }, [this](const bool &b) { DefinitVisible(b); });
  Liste->Actions.push_back(this);
 }
//---------------------------------------------------------------------------
void TAction::DefinitChecked(bool Valeur)
 { // TCustomAction::SetChecked : clients d'abord, puis groupe (GroupIndex)
  if (Verification) return;
  Verification=true;
  if (Valeur!=bChecked)
   {
    std::vector<ClientAction *> c=Clients;
    for(ClientAction *Client : c) Client->LienChecked(bChecked, Valeur);
    bChecked=Valeur;
    if ((GroupIndex>0)&&bChecked&&ActionList)
     for(TAction *a : ActionList->Actions)
      if ((a!=this)&&(a->GroupIndex==GroupIndex))
       a->Checked=false;
   }
  Verification=false;
 }
//---------------------------------------------------------------------------
void TAction::DefinitEnabled(bool Valeur)
 {
  if (Valeur==bEnabled) return;
  std::vector<ClientAction *> c=Clients;
  for(ClientAction *Client : c) Client->LienEnabled(bEnabled, Valeur);
  bEnabled=Valeur;
 }
//---------------------------------------------------------------------------
void TAction::DefinitVisible(bool Valeur)
 {
  if (Valeur==bVisible) return;
  std::vector<ClientAction *> c=Clients;
  for(ClientAction *Client : c) Client->LienVisible(bVisible, Valeur);
  bVisible=Valeur;
 }
//---------------------------------------------------------------------------
bool TAction::Execute()
 { // TCustomAction::Execute
  if (bEnabled&&AutoCheck)
   if ((!bChecked)||(bChecked&&(GroupIndex==0)))
    Checked=!bChecked;
  if (!bEnabled) return false;
  if (OnExecute)
   {
    OnExecute(this);
    return true;
   }
  return false;
 }
//---------------------------------------------------------------------------
void TAction::AjouteClient(ClientAction *c)
 {
  if (std::find(Clients.begin(), Clients.end(), c)==Clients.end()) Clients.push_back(c);
 }
//---------------------------------------------------------------------------
void TAction::RetireClient(ClientAction *c)
 {
  Clients.erase(std::remove(Clients.begin(), Clients.end(), c), Clients.end());
 }
//---------------------------------------------------------------------------
bool TActionList::IsShortCut(TShortCut Raccourci, bool Executer)
 { // TCustomActionList::IsShortCut : la première action portant le raccourci
  for(TAction *a : Actions)
   if (a->ShortCut==Raccourci)
    return Executer?a->Execute():(bool)a->Enabled;
  return false;
 }
//---------------------------------------------------------------------------
// TMenuItem
//---------------------------------------------------------------------------
TMenuItem::TMenuItem(QAction *Feuille, QObject *Parent) : QObject(Parent), Element(Feuille), SousMenu(nullptr)
 {
  setObjectName(Feuille->objectName());
  if (Feuille->property("vclSeparateur").toBool()) Feuille->setSeparator(true);
  asCaption=A(Feuille->text());
  asHint=A(Feuille->statusTip());
  bChecked=Feuille->isChecked();
  bEnabled=Feuille->isEnabled();
  bVisible=Feuille->isVisible();
  Tag=Feuille->property("vclTag").toInt();
  GroupIndex=Feuille->property("vclGroupIndex").toInt();
  RadioItem=Feuille->property("vclRadioItem").toBool();
  AutoCheck=Feuille->property("vclAutoCheck").toBool();
  HelpContext=Feuille->property("vclHelpContext").toInt();
  ImageIndex=Feuille->property("vclImageIndex").isValid()?Feuille->property("vclImageIndex").toInt():-1;
  ShortCut=Feuille->property("vclShortCut").toInt();
  QObject::connect(Feuille, &QAction::triggered, this, [this]() { ElementDeclenche(); });
  Caption.Definit([this]() { return asCaption; },
                  [this](const AnsiString &as) { asCaption=as; Element->setText(Q(as)); });
  Hint.Definit([this]() { return asHint; },
               [this](const AnsiString &as) { asHint=as; Element->setStatusTip(Q(as)); });
  Checked.Definit([this]() { return bChecked; },
                  [this](const bool &b)
                   {
                    if (b==bChecked) return;
                    bChecked=b;
                    if (b&&RadioItem) TurnSiblingsOff();
                    AfficheCoche();
                   });
  Enabled.Definit([this]() { return bEnabled; },
                  [this](const bool &b) { bEnabled=b; Element->setEnabled(b); });
  Visible.Definit([this]() { return bVisible; },
                  [this](const bool &b) { bVisible=b; Element->setVisible(b); });
  Action.Definit([this]() { return ActionLiee; },
                 [this](TAction *const &a)
                  {
                   if (ActionLiee) ActionLiee->RetireClient(this);
                   ActionLiee=a;
                   if (a) { a->AjouteClient(this); ActionChange(a); }
                  });
  AfficheCoche();
 }
//---------------------------------------------------------------------------
TMenuItem::TMenuItem(QMenu *Menu, QObject *Parent) : QObject(Parent), Element(Menu->menuAction()), SousMenu(Menu)
 {
  setObjectName(Menu->objectName());
  asCaption=A(Menu->title());
  asHint=A(Menu->statusTip());
  bEnabled=Menu->isEnabled();
  Tag=Menu->property("vclTag").toInt();
  HelpContext=Menu->property("vclHelpContext").toInt();
  ImageIndex=Menu->property("vclImageIndex").isValid()?Menu->property("vclImageIndex").toInt():-1;
  Caption.Definit([this]() { return asCaption; },
                  [this](const AnsiString &as) { asCaption=as; SousMenu->setTitle(Q(as)); });
  Hint.Definit([this]() { return asHint; }, [this](const AnsiString &as) { asHint=as; });
  Checked.Definit([this]() { return bChecked; }, [this](const bool &b) { bChecked=b; });
  Enabled.Definit([this]() { return bEnabled; },
                  [this](const bool &b) { bEnabled=b; SousMenu->setEnabled(b); });
  Visible.Definit([this]() { return bVisible; },
                  [this](const bool &b) { bVisible=b; SousMenu->menuAction()->setVisible(b); });
  Action.Definit([this]() { return ActionLiee; }, [this](TAction *const &a) { ActionLiee=a; });
 }
//---------------------------------------------------------------------------
void TMenuItem::DefinitGroupe(QActionGroup *g)
 {
  Groupe=g;
  if (!SousMenu) Element->setActionGroup(g);
  AfficheCoche();
 }
//---------------------------------------------------------------------------
void TMenuItem::AfficheCoche()
 { // La VCL n'affiche rien pour un élément non coché : checkable seulement si coché
  if (SousMenu) return;
  Element->setCheckable(bChecked);
  Element->setChecked(bChecked);
 }
//---------------------------------------------------------------------------
void TMenuItem::TurnSiblingsOff()
 {
  if (!Parent) return;
  for(TMenuItem *m : Parent->Items)
   if ((m!=this)&&m->RadioItem&&(m->GroupIndex==GroupIndex))
    m->Checked=false;
 }
//---------------------------------------------------------------------------
void TMenuItem::ElementDeclenche()
 { // Qt a pu inverser la coche : l'état est celui de la VCL, puis Click
  AfficheCoche();
  Click();
 }
//---------------------------------------------------------------------------
void TMenuItem::Click()
 { // TMenuItem::Click
  if (!bEnabled) return;
  bool AutoCheckLie=ActionLiee&&(ActionLiee->AutoCheck==AutoCheck);
  if (AutoCheck&&((!ActionLiee)||(!AutoCheckLie)))
   Checked=!bChecked;
  if (OnClick&&ActionLiee&&(ActionLiee->OnExecute))
   OnClick(this);
  else if (ActionLiee)
   ActionLiee->Execute();
  else if (OnClick)
   OnClick(this);
 }
//---------------------------------------------------------------------------
TMenuItem *TMenuItem::Find(TShortCut Raccourci)
 { // Parcours en profondeur, préfixe (TMenu::FindItem)
  if (Raccourci&&(ShortCut==Raccourci)) return this;
  for(TMenuItem *m : Items)
   if (TMenuItem *t=m->Find(Raccourci)) return t;
  return nullptr;
 }
//---------------------------------------------------------------------------
void TMenuItem::LieAction(TAction *a)
 { // Lien établi au chargement de la fiche : les propriétés sont déjà dans le .ui
  ActionLiee=a;
  if (!a) return;
  a->AjouteClient(this);
  if (!Element->property("vclShortCut").isValid()) ShortCut=a->ShortCut;
  if (!Element->property("vclImageIndex").isValid()) ImageIndex=a->ImageIndex;
  if (!Element->property("vclHelpContext").isValid()) HelpContext=a->HelpContext;
 }
//---------------------------------------------------------------------------
void TMenuItem::ActionChange(TAction *a)
 { // TMenuItem::ActionChange(Sender, CheckDefaults=false)
  Caption=a->Caption.Valeur();
  Checked=(bool)a->Checked;
  Enabled=(bool)a->Enabled;
  HelpContext=a->HelpContext;
  Hint=a->Hint.Valeur();
  ImageIndex=a->ImageIndex;
  Element->setIcon(a->Icone);
  ShortCut=a->ShortCut;
  Element->setShortcut(SequenceQt(ShortCut));
  Visible=(bool)a->Visible;
 }
//---------------------------------------------------------------------------
void TMenuItem::LienChecked(bool Ancien, bool Nouveau) { if (bChecked==Ancien) Checked=Nouveau; }
void TMenuItem::LienEnabled(bool Ancien, bool Nouveau) { if (bEnabled==Ancien) Enabled=Nouveau; }
void TMenuItem::LienVisible(bool Ancien, bool Nouveau) { if (bVisible==Ancien) Visible=Nouveau; }
//---------------------------------------------------------------------------
// Menus
//---------------------------------------------------------------------------
bool TMenu::IsShortCut(TShortCut Raccourci, bool Executer)
 { // TMenu::IsShortCut : la touche est consommée même si l'élément est désactivé
  TMenuItem *Element=nullptr;
  for(TMenuItem *m : Items)
   if ((Element=m->Find(Raccourci))) break;
  if (!Element) return false;
  if (Executer)
   {
    std::vector<TMenuItem *> Chaine;
    for(TMenuItem *m=Element; m; m=m->Parent) Chaine.insert(Chaine.begin(), m);
    QPointer<TMenu> Moi(this);
    for(TMenuItem *m : Chaine) // DoClick : les parents d'abord, tous activés
     {
      if (!m->Enabled) break;
      m->Click();
      if (!Moi) break;
     }
   }
  return true;
 }
//---------------------------------------------------------------------------
TMainMenu::TMainMenu(QMenuBar *b) : TMenu(b), Barre(b)
 {
  setObjectName(b->objectName());
 }
//---------------------------------------------------------------------------
TPopupMenu::TPopupMenu(QMenu *m) : TMenu(m), Menu(m)
 {
  setObjectName(m->objectName());
 }
//---------------------------------------------------------------------------
void TPopupMenu::Popup(int X, int Y)
 {
  if (OnPopup) OnPopup(this);
  Menu->exec(QPoint(X, Y));
 }
//---------------------------------------------------------------------------
// Répartiteur de raccourcis
//---------------------------------------------------------------------------
namespace
 {
  std::vector<FicheRaccourcis> Fiches;
  //-------------------------------------------------------------------------
  FicheRaccourcis *FicheDe(QWidget *w)
   {
    QWidget *f=w?w->window():nullptr;
    for(FicheRaccourcis &r : Fiches)
     if (r.Fenetre==f) return &r;
    return nullptr;
   }
  //-------------------------------------------------------------------------
  FicheRaccourcis *FichePrincipale()
   {
    for(FicheRaccourcis &r : Fiches)
     if (r.Principale) return &r;
    return nullptr;
   }
  //-------------------------------------------------------------------------
  bool TraiteRaccourci(QWidget *Cible, TShortCut Raccourci, bool Executer)
   { // TWinControl::IsMenuKey puis TApplication::IsShortCut
    FicheRaccourcis *f=FicheDe(Cible);
    if (!f) return false;
    for(QWidget *c=Cible; c; c=c->parentWidget())
     {
      if (AvecMenuContextuel *m=dynamic_cast<AvecMenuContextuel *>(c))
       if (m->PopupMenu&&m->PopupMenu->IsShortCut(Raccourci, Executer)) return true;
      if (c==f->Fenetre) break;
     }
    if (f->IsShortCut(Raccourci, Executer)) return true;
    FicheRaccourcis *p=FichePrincipale();
    if (p&&(p!=f)&&p->Fenetre->isEnabled()&&!QApplication::activeModalWidget())
     return p->IsShortCut(Raccourci, Executer);
    return false;
   }
  //-------------------------------------------------------------------------
  class RepartiteurRaccourcis : public QObject
   {
    public:
    bool eventFilter(QObject *o, QEvent *e) override
     {
      if ((e->type()!=QEvent::ShortcutOverride)&&(e->type()!=QEvent::KeyPress)&&
          (e->type()!=QEvent::KeyRelease)) return false;
      QWidget *w=qobject_cast<QWidget *>(o);
      // Seulement au premier destinataire (le contrôle actif, ou la fenêtre
      // si aucun contrôle n'a le focus), pas lors de la remontée aux parents
      QWidget *Focus=QApplication::focusWidget();
      if (!w||(Focus?(w!=Focus):!w->isWindow())) return false;
      QKeyEvent *k=static_cast<QKeyEvent *>(e);
      if (e->type()==QEvent::KeyRelease)
       { // KeyPreview : OnKeyUp de la fiche
        FicheRaccourcis *f=FicheDe(w);
        return f&&f->Apercu&&f->Apercu(k, true);
       }
      TShortCut sc=RaccourciVcl(k);
      if (!sc)
       {
        if (e->type()!=QEvent::KeyPress) return false;
        FicheRaccourcis *f=FicheDe(w);
        return f&&f->Apercu&&f->Apercu(k, false);
       }
      if (e->type()==QEvent::ShortcutOverride)
       {
        if (TraiteRaccourci(w, sc, false))
         {
          e->accept();
          return true;
         }
        return false;
       }
      // Raccourci (IsMenuKey), sinon KeyPreview : OnKeyDown de la fiche
      if (TraiteRaccourci(w, sc, true)) return true;
      FicheRaccourcis *f=FicheDe(w);
      return f&&f->Apercu&&f->Apercu(k, false);
     }
   };
  RepartiteurRaccourcis *Repartiteur=nullptr;
 }
//---------------------------------------------------------------------------
bool FicheRaccourcis::IsShortCut(TShortCut Raccourci, bool Executer)
 { // TCustomForm::IsShortCut : menu principal puis listes d'actions de la fiche
  if (Menu&&Menu->IsShortCut(Raccourci, Executer)) return true;
  for(TActionList *l : ListesActions)
   if (l->IsShortCut(Raccourci, Executer)) return true;
  return false;
 }
//---------------------------------------------------------------------------
void InscritFicheRaccourcis(const FicheRaccourcis &Fiche)
 {
  if (!Repartiteur)
   {
    Repartiteur=new RepartiteurRaccourcis;
    qApp->installEventFilter(Repartiteur);
   }
  DesinscritFicheRaccourcis(Fiche.Fenetre);
  Fiches.push_back(Fiche);
 }
//---------------------------------------------------------------------------
void DesinscritFicheRaccourcis(QWidget *Fenetre)
 {
  Fiches.erase(std::remove_if(Fiches.begin(), Fiches.end(),
                              [Fenetre](const FicheRaccourcis &f) { return f.Fenetre==Fenetre; }),
               Fiches.end());
 }
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// vcl/fiche.h - Portage de Centre-Ville : TForm
//
// TFormT<QDialog> (TForm) et TFormT<QMainWindow> (fiche principale)
// reproduisent le cycle de vie d'une fiche VCL :
//   - le constructeur de la fiche portée appelle ChargeFiche() à la place du
//     chargement du .dfm : setupUi (fiche générée par dfm2ui), liaison des
//     membres (COMPOSANTS_<fiche>), connexion des gestionnaires
//     (EVENEMENTS_<fiche>), puis OnCreate (OldCreateOrder=True : OnCreate
//     avant le corps du constructeur C++) ;
//   - Show/Hide/Visible, ShowModal/ModalResult, Close et OnCloseQuery,
//     OnShow/OnHide, KeyPreview (OnKeyDown/OnKeyUp avant le contrôle actif ;
//     sans KeyPreview, seulement quand aucun contrôle n'a le focus), Échap =
//     bouton Cancel ; ShowModal a sa propre boucle modale, que seul
//     ModalResult termine (Close et la case de fermeture donnent mrCancel) :
//     Hide pendant ShowModal cache la fiche sans la terminer, comme dans la
//     VCL (QDialog::exec, lui, s'arrêterait) ;
//   - FindComponent par nom, ComponentCount/Components[i] dans l'ordre du .dfm.
// Width/Height désignent la zone cliente (la VCL compte le cadre de la
// fenêtre ; les formules du code d'origine n'utilisent que des différences).
//---------------------------------------------------------------------------
#ifndef vcl_ficheH
#define vcl_ficheH
//---------------------------------------------------------------------------
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QEventLoop>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPointer>
#include <QScreen>
#include <QShowEvent>
#include <map>
#include <type_traits>
#include "actions.h"
#include "controles.h"
#include "listeimages.h"
//---------------------------------------------------------------------------
// Liaison des composants d'une fiche (X-macro COMPOSANTS_<fiche>)
//---------------------------------------------------------------------------
class LieurComposants
 {
  QWidget *Fiche;
  const char *Fichier; // f_simul...
  TActionList *DerniereListe=nullptr;
  std::map<QObject *, TMenuItem *> ElementsMenus;
  QObject *Lie(const char *Nom, const char *Classe);
  public:
  std::map<QString, QObject *> &Composants;
  std::vector<QObject *> Ordre; // composants dans l'ordre du .dfm (TComponent::Components)
  FicheRaccourcis Raccourcis;
  LieurComposants(QWidget *f, const char *Nom, std::map<QString, QObject *> &c) : Fiche(f), Fichier(Nom), Composants(c) {}
  template <class T> void Lie(const char *Nom, T *&Membre)
   {
    Membre=dynamic_cast<T *>(Lie(Nom, NomClasse<T>()));
   }
  template <class T> static const char *NomClasse()
   {
    if constexpr (std::is_same_v<T, TAction>) return "TAction";
    else if constexpr (std::is_same_v<T, TActionList>) return "TActionList";
    else if constexpr (std::is_same_v<T, TMenuItem>) return "TMenuItem";
    else if constexpr (std::is_same_v<T, TPopupMenu>) return "TPopupMenu";
    else if constexpr (std::is_same_v<T, TMainMenu>) return "TMainMenu";
    else if constexpr (std::is_same_v<T, TTimer>) return "TTimer";
    else if constexpr (std::is_same_v<T, TSaveDialog>) return "TSaveDialog";
    else if constexpr (std::is_same_v<T, TOpenDialog>) return "TOpenDialog";
    else if constexpr (std::is_same_v<T, TVirtualImageList>) return "TVirtualImageList";
    else if constexpr (std::is_same_v<T, TImageCollection>) return "TImageCollection";
    else return "";
   }
  void Termine(); // arbres des menus, liens des boutons, PrepareVcl
 };
#define LIE_COMPOSANT(Classe, Nom) Lieur.Lie(#Nom, Nom);
//---------------------------------------------------------------------------
int ToucheVirtuelleDe(const QKeyEvent *e); // code VK de la touche (OnKeyDown)
extern QWidget *FichePrincipaleQt; // fiche principale (parent des fiches secondaires)
TShiftState EtatTouches(const QKeyEvent *e);
//---------------------------------------------------------------------------
template <class B> class TFormT : public B, public FicheVclBase
 {
  bool PremierAffichage=true, Modale=false, EstPrincipale=false;
  int iModalResult=0;
  QEventLoop *BoucleModale=nullptr;
  bool BoucleTerminee=false;
  std::vector<QObject *> ListeComposants;
  protected:
  std::map<QString, QObject *> Composants;
  // Après setupUi : liaison, raccourcis, OnCreate (appelée par la fiche portée)
  void FinChargement(LieurComposants &Lieur)
   {
    Lieur.Termine();
    ListeComposants=Lieur.Ordre;
    Lieur.Raccourcis.Fenetre=this;
    Lieur.Raccourcis.Principale=EstPrincipale;
    Lieur.Raccourcis.Apercu=[this](QKeyEvent *e, bool Relache) { return Apercu(e, Relache); };
    Lieur.Raccourcis.ToucheDialogue=[this](QKeyEvent *e) { return ToucheDialogue(e); };
    InscritFicheRaccourcis(Lieur.Raccourcis);
    AppliqueStyle();
   }
  void DefinitPrincipale() { EstPrincipale=true; }
  public:
  Propriete<AnsiString> Caption;
  Propriete<int> Left, Top, Width, Height, ClientWidth, ClientHeight, ModalResult;
  Propriete<bool> Visible, Enabled;
  Propriete<TWindowState> WindowState;
  Propriete<TCursor> Cursor;
  bool KeyPreview=false;
  int HelpContext=0, Tag=0;
  TNotifyEvent OnCreate, OnShow, OnHide, OnDestroy, OnActivate, OnClick;
  std::function<void(TObject *, bool &)> OnCloseQuery;
  std::function<void(TObject *, WORD &, TShiftState)> OnKeyDown, OnKeyUp;
  std::function<void(TObject *, char &)> OnKeyPress;
  Propriete<int> ComponentCount;
  class TableauComposants
   { // TComponent::Components[i]
    const std::vector<QObject *> &Liste;
    public:
    explicit TableauComposants(const std::vector<QObject *> &l) : Liste(l) {}
    TComponent *operator[](int i) const { return Liste.at((std::size_t)i); }
   } Components{ListeComposants};
  explicit TFormT(TComponent *Owner) : B(ParentDe(Owner))
   {
    QWidget *w=this;
    ComponentCount.Definit([this]() { return (int)ListeComposants.size(); }, [](const int &) {});
    Caption.Definit([w]() { return A(w->windowTitle()); }, [w](const AnsiString &as) { w->setWindowTitle(Q(as)); });
    Left.Definit([w]() { return w->x(); }, [w](const int &v) { w->move(v, w->y()); });
    Top.Definit([w]() { return w->y(); }, [w](const int &v) { w->move(w->x(), v); });
    Width.Definit([w]() { return w->width(); }, [w](const int &v) { w->resize(v, w->height()); });
    Height.Definit([w]() { return w->height(); }, [w](const int &v) { w->resize(w->width(), v); });
    ClientWidth.Definit([w]() { return w->width(); }, [w](const int &v) { w->resize(v, w->height()); });
    ClientHeight.Definit([w]() { return w->height(); }, [w](const int &v) { w->resize(w->width(), v); });
    ModalResult.Definit([this]() { return iModalResult; }, [this](const int &mr) { DefinitModalResult(mr); });
    Visible.Definit([w]() { return w->isVisible(); }, [this](const bool &v) { if (v) Show(); else Hide(); });
    Enabled.Definit([w]() { return w->isEnabled(); }, [w](const bool &v) { w->setEnabled(v); });
    WindowState.Definit([w]() { return w->isMaximized()?wsMaximized:(w->isMinimized()?wsMinimized:wsNormal); },
                        [w](const TWindowState &s)
                         {
                          if (s==wsMaximized) w->showMaximized();
                          else if (s==wsMinimized) w->showMinimized();
                          else if (w->isMaximized()||w->isMinimized()) w->showNormal();
                         });
    Cursor.Definit([w]() { return w->cursor().shape()==Qt::WaitCursor?crHourGlass:crDefault; },
                   [w](const TCursor &c) { if (c==crHourGlass) w->setCursor(Qt::WaitCursor); else w->unsetCursor(); });
    if constexpr (std::is_base_of_v<QDialog, B>) // done() (ModalResult, Close) : fin de la boucle modale
     QObject::connect(this, &QDialog::finished, this, [this](int)
      {
       BoucleTerminee=true;
       if (BoucleModale) BoucleModale->exit();
      });
   }
  ~TFormT() override
   {
    DesinscritFicheRaccourcis(this);
   }
  static QWidget *ParentDe(TComponent *Owner)
   { // Fiches secondaires au-dessus de la fiche principale (MainFormOnTaskBar)
    QWidget *p=qobject_cast<QWidget *>(Owner);
    if (!p) p=FichePrincipaleQt;
    return p?p->window():nullptr;
   }
  void Show()
   {
    B::show();
    B::raise();
    B::activateWindow();
   }
  void Hide() { B::hide(); }
  void Close() { B::close(); }
  void Release() { B::hide(); B::deleteLater(); }
  void Repaint() { if (TCanvas::NbDessinsEnCours) B::update(); else B::repaint(); } // voir ControleVcl::Repaint
  void SetFocus() { B::activateWindow(); }
  int ShowModal()
   { // TCustomForm::ShowModal : « repeat HandleMessage until ModalResult<>0 ». La
     // boucle ne dépend pas de la visibilité : f_prmsim cache sa fiche le temps
     // d'une fiche fille modale, puis la réaffiche (QDialog::exec s'arrêterait au Hide)
    if constexpr (std::is_base_of_v<QDialog, B>)
     {
      iModalResult=mrNone;
      Modale=true;
      bool EtaitModale=this->testAttribute(Qt::WA_ShowModal);
      this->setAttribute(Qt::WA_ShowModal, true);
      this->setResult(0);
      QPointer<QDialog> Garde(this);
      QEventLoop Boucle;
      BoucleModale=&Boucle;
      BoucleTerminee=false;
      B::show();
      if (!BoucleTerminee) Boucle.exec(QEventLoop::DialogExec);
      if (!Garde) return mrCancel;
      BoucleModale=nullptr;
      if (!EtaitModale) this->setAttribute(Qt::WA_ShowModal, false);
      Modale=false;
      if (iModalResult==mrNone) iModalResult=mrCancel;
      return iModalResult;
     }
    else
     return mrNone;
   }
  void DefinitModalResult(int mr) override
   { // ModalResult<>0 pendant ShowModal : la boucle modale ferme la fiche après le
     // traitement du message en cours (le gestionnaire OnClick va jusqu'au bout)
    iModalResult=mr;
    if constexpr (std::is_base_of_v<QDialog, B>)
     if (Modale&&(mr!=mrNone))
      QMetaObject::invokeMethod(this, [this]() { if (Modale&&(iModalResult!=mrNone)) B::done(iModalResult); },
                                Qt::QueuedConnection);
   }
  QObject *FindComponent(const AnsiString &asNom)
   {
    auto i=Composants.find(Q(asNom));
    if (i!=Composants.end()) return i->second;
    return B::template findChild<QObject *>(Q(asNom));
   }
  protected:
  void AppliqueStyle()
   { // BorderStyle, FormStyle (propriétés « vcl... » de la fiche)
    QWidget *w=this;
    KeyPreview=w->property("vclKeyPreview").toBool();
    HelpContext=w->property("vclHelpContext").toInt();
    QString Bord=w->property("vclBorderStyle").toString();
    Qt::WindowFlags f=w->windowFlags();
    if ((Bord=="bsToolWindow")||(Bord=="bsSizeToolWin")) f|=Qt::Tool;
    if (w->property("vclFormStyle").toString()=="fsStayOnTop") f|=Qt::WindowStaysOnTopHint;
    if constexpr (std::is_base_of_v<QDialog, B>)
     if (w->property("vclBorderIcons").isValid())
      { // BorderIcons : boutons de la barre de titre (biMinimize : f_prmbus réduit sa fiche)
       QString Icones=w->property("vclBorderIcons").toString();
       f|=Qt::CustomizeWindowHint|Qt::WindowTitleHint;
       f&=~(Qt::WindowSystemMenuHint|Qt::WindowCloseButtonHint|Qt::WindowMinimizeButtonHint|
            Qt::WindowMaximizeButtonHint|Qt::WindowContextHelpButtonHint);
       if (Icones.contains("biSystemMenu"))
        {
         f|=Qt::WindowSystemMenuHint|Qt::WindowCloseButtonHint;
         if (Icones.contains("biMinimize")) f|=Qt::WindowMinimizeButtonHint;
         if (Icones.contains("biMaximize")) f|=Qt::WindowMaximizeButtonHint;
        }
      }
    if (f!=w->windowFlags()) w->setWindowFlags(f);
    if ((Bord=="bsDialog")||(Bord=="bsSingle")||(Bord=="bsToolWindow")) w->setFixedSize(w->size());
   }
  bool Apercu(QKeyEvent *e, bool Relache)
   { // KeyPreview : OnKeyDown/OnKeyUp puis OnKeyPress de la fiche avant le contrôle actif ;
     // sans KeyPreview, la fiche ne les reçoit que si elle a elle-même le focus
     // (aucun contrôle actif : TWinControl::DoKeyDown de la fiche)
    QWidget *Focus=QApplication::focusWidget();
    if (!KeyPreview&&Focus&&(Focus!=this)) return false;
    WORD Key=(WORD)ToucheVirtuelleDe(e);
    if (Key)
     {
      if (Relache) { if (OnKeyUp) OnKeyUp(this, Key, EtatTouches(e)); }
      else { if (OnKeyDown) OnKeyDown(this, Key, EtatTouches(e)); }
      if (!Key) return true;
     }
    if (!Relache&&OnKeyPress&&!e->text().isEmpty()&&(e->text().at(0).unicode()<256))
     {
      char c=(char)e->text().at(0).unicode();
      OnKeyPress(this, c);
      if (!c) return true;
     }
    return false;
   }
  bool ToucheDialogue(QKeyEvent *e)
   { // TButton::CMDialogKey : Entrée = bouton qui a le focus, sinon bouton Default ;
     // Échap = premier bouton Cancel ; sans bouton concerné, la touche continue
    if (e->modifiers()&~(Qt::KeypadModifier)) return false;
    bool Entree=(e->key()==Qt::Key_Return)||(e->key()==Qt::Key_Enter), Echap=(e->key()==Qt::Key_Escape);
    if (!Entree&&!Echap) return false;
    if (Entree)
     if (TButton *b=dynamic_cast<TButton *>(QApplication::focusWidget()))
      if (b->window()==this) { b->Click(); return true; }
    for(QPushButton *p : this->template findChildren<QPushButton *>())
     if (TButton *b=dynamic_cast<TButton *>(p))
      if ((Entree?b->Default:b->Cancel)&&b->isVisible()&&b->isEnabled())
       {
        b->Click();
        return true;
       }
    return false;
   }
  void showEvent(QShowEvent *e) override
   {
    B::showEvent(e);
    if (e->spontaneous()) return;
    if (PremierAffichage)
     {
      PremierAffichage=false;
      QString Position=this->property("vclPosition").toString();
      QWidget *Ref=(Position=="poMainFormCenter")?FichePrincipaleQt:nullptr;
      if ((Position=="poScreenCenter")||(Ref&&(Ref!=this)))
       {
        QRect Zone=Ref?Ref->frameGeometry():(this->screen()?this->screen()->availableGeometry():QRect());
        QRect g=this->frameGeometry();
        g.moveCenter(Zone.center());
        this->move(g.topLeft());
       }
     }
    if (OnShow) OnShow(this);
   }
  void hideEvent(QHideEvent *e) override
   {
    B::hideEvent(e);
    if (!e->spontaneous()&&OnHide) OnHide(this);
   }
  void closeEvent(QCloseEvent *e) override
   { // TCustomForm::Close : CloseQuery, puis caHide (fiche secondaire) ou fin de l'application
    bool CanClose=true;
    if (OnCloseQuery) OnCloseQuery(this, CanClose);
    if (!CanClose) { e->ignore(); return; }
    if (Modale&&(iModalResult==mrNone)) iModalResult=mrCancel;
    B::closeEvent(e);
    if (e->isAccepted()&&EstPrincipale) QCoreApplication::quit();
   }
  void mousePressEvent(QMouseEvent *e) override
   {
    B::mousePressEvent(e);
    if (e->button()==Qt::LeftButton) e->accept();
   }
  void mouseReleaseEvent(QMouseEvent *e) override
   { // TControl::WMLButtonUp : OnClick de la fiche (clic hors des contrôles)
    B::mouseReleaseEvent(e);
    if ((e->button()==Qt::LeftButton)&&this->rect().contains(e->position().toPoint())&&OnClick) OnClick(this);
   }
  void keyPressEvent(QKeyEvent *e) override
   {
    if constexpr (std::is_base_of_v<QDialog, B>)
     if ((e->key()==Qt::Key_Escape)||(e->key()==Qt::Key_Return)||(e->key()==Qt::Key_Enter))
      return; // touches de dialogue déjà traitées (ToucheDialogue) : pas de reject() de QDialog
    B::keyPressEvent(e);
   }
 };
//---------------------------------------------------------------------------
typedef TFormT<QDialog> TForm;
typedef TFormT<QMainWindow> TFormPrincipale;
//---------------------------------------------------------------------------
#endif

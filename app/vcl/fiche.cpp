//---------------------------------------------------------------------------
// vcl/fiche.cpp - Portage de Centre-Ville : liaison des composants des fiches
//---------------------------------------------------------------------------
#include "fiche.h"
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <cstring>
#include "index_images.h"
//---------------------------------------------------------------------------
namespace
 {
  // TVirtualImageList des fiches -> tables générées par tools/extrait_images.py
  struct ListeIndexee { const char *Fiche, *Nom; const char *const *Ressources; int Nombre; };
  const ListeIndexee ListesIndexees[]=
   {
    {"f_carfours", "VirtualImageList64", f_carfours_VirtualImageList64, NB_F_CARFOURS_VirtualImageList64},
    {"f_carfours", "VirtualImageList96", f_carfours_VirtualImageList96, NB_F_CARFOURS_VirtualImageList96},
    {"f_carfours", "VirtualImageList128", f_carfours_VirtualImageList128, NB_F_CARFOURS_VirtualImageList128},
    {"f_propos", "VirtualImageList", f_propos_VirtualImageList, NB_F_PROPOS_VirtualImageList},
    {"f_simul", "VirtualImageList16x16", f_simul_VirtualImageList16x16, NB_F_SIMUL_VirtualImageList16x16},
    {"f_simul", "VirtualImageList8x8", f_simul_VirtualImageList8x8, NB_F_SIMUL_VirtualImageList8x8},
    {"f_simul", "VirtualImageListGlyphes", f_simul_VirtualImageListGlyphes, NB_F_SIMUL_VirtualImageListGlyphes},
   };
 }
//---------------------------------------------------------------------------
// Align (alTop, alBottom, alLeft, alRight, alClient) : placement des contrôles
// à chaque redimensionnement du parent, dans l'ordre de la VCL
//---------------------------------------------------------------------------
namespace
 {
  void Aligne(QWidget *Parent)
   {
    QRect r=Parent->contentsRect();
    const char *Ordre[]={"alTop", "alBottom", "alLeft", "alRight", "alClient"};
    for(const char *a : Ordre)
     for(QObject *o : Parent->children())
      {
       QWidget *w=qobject_cast<QWidget *>(o);
       if (!w||w->isWindow()||w->isHidden()||(w->property("vclAlign").toString()!=QLatin1String(a))) continue;
       if (!std::strcmp(a, "alTop")) { w->setGeometry(r.left(), r.top(), r.width(), w->height()); r.setTop(r.top()+w->height()); }
       else if (!std::strcmp(a, "alBottom")) { w->setGeometry(r.left(), r.bottom()+1-w->height(), r.width(), w->height()); r.setBottom(r.bottom()-w->height()); }
       else if (!std::strcmp(a, "alLeft")) { w->setGeometry(r.left(), r.top(), w->width(), r.height()); r.setLeft(r.left()+w->width()); }
       else if (!std::strcmp(a, "alRight")) { w->setGeometry(r.right()+1-w->width(), r.top(), w->width(), r.height()); r.setRight(r.right()-w->width()); }
       else w->setGeometry(r);
      }
   }
  class Aligneur : public QObject
   {
    public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
     {
      if ((e->type()==QEvent::Resize)||(e->type()==QEvent::LayoutRequest)) Aligne(static_cast<QWidget *>(o));
      return false;
     }
   };
 }
//---------------------------------------------------------------------------
QObject *LieurComposants::Lie(const char *Nom, const char *Classe)
 {
  QString n=QString::fromUtf8(Nom);
  QObject *o=nullptr;
  if (!std::strcmp(Classe, "TActionList"))
   {
    DerniereListe=new TActionList(Fiche);
    Raccourcis.ListesActions.push_back(DerniereListe);
    o=DerniereListe;
   }
  else if (!std::strcmp(Classe, "TAction"))
   {
    QAction *q=Fiche->findChild<QAction *>(n);
    if (q&&DerniereListe) o=new TAction(q, DerniereListe);
   }
  else if (!std::strcmp(Classe, "TMenuItem"))
   {
    TMenuItem *m=nullptr;
    if (QMenu *Menu=Fiche->findChild<QMenu *>(n)) m=new TMenuItem(Menu, Fiche);
    else if (QAction *q=Fiche->findChild<QAction *>(n)) m=new TMenuItem(q, Fiche);
    if (m)
     {
      ElementsMenus[m->ElementQt()]=m;
      QString Action=m->ElementQt()->property("vclAction").toString();
      if (m->MenuQt()) Action=m->MenuQt()->property("vclAction").toString();
      if (!Action.isEmpty())
       if (TAction *a=dynamic_cast<TAction *>(Composants[Action])) m->LieAction(a);
     }
    o=m;
   }
  else if (!std::strcmp(Classe, "TPopupMenu"))
   {
    if (QMenu *Menu=Fiche->findChild<QMenu *>(n)) o=new TPopupMenu(Menu);
   }
  else if (!std::strcmp(Classe, "TMainMenu"))
   {
    if (QMenuBar *b=Fiche->findChild<QMenuBar *>(n)) Raccourcis.Menu=static_cast<TMainMenu *>(o=new TMainMenu(b));
   }
  else if (!std::strcmp(Classe, "TTimer")||!std::strcmp(Classe, "TOpenDialog")||!std::strcmp(Classe, "TSaveDialog"))
   {
    if (!std::strcmp(Classe, "TTimer")) o=new TTimer(Fiche);
    else if (!std::strcmp(Classe, "TSaveDialog")) o=new TSaveDialog(Fiche);
    else o=new TOpenDialog(Fiche);
    ConfigureNonVisuel(o, Fiche->property(("vcl_"+n).toUtf8().constData()).toStringList());
   }
  else if (!std::strcmp(Classe, "TVirtualImageList"))
   {
    for(const ListeIndexee &l : ListesIndexees)
     if (!std::strcmp(l.Fiche, Fichier)&&!std::strcmp(l.Nom, Nom))
      o=new TVirtualImageList(l.Ressources, l.Nombre, Fiche);
   }
  else if (!std::strcmp(Classe, "TImageCollection"))
   o=new TImageCollection;
  else
   o=Fiche->findChild<QWidget *>(n);
  if (o)
   {
    if (o->objectName().isEmpty()) o->setObjectName(n);
    Composants[n]=o;
   }
  else
   qWarning("Portage : composant %s (%s) introuvable dans la fiche %s", Nom, Classe, Fichier);
  return o;
 }
//---------------------------------------------------------------------------
void LieurComposants::Termine()
 {
  // Arbres des menus (Items/Parent), comme la VCL : séparateurs compris
  std::function<void(QMenu *, TMenuItem *, std::vector<TMenuItem *> &)> Arbre=
   [&](QMenu *Menu, TMenuItem *Parent, std::vector<TMenuItem *> &Items)
    {
     for(QAction *a : Menu->actions())
      {
       auto i=ElementsMenus.find(a);
       if (i==ElementsMenus.end()) continue;
       TMenuItem *m=i->second;
       m->Parent=Parent;
       Items.push_back(m);
       if (m->MenuQt()) Arbre(m->MenuQt(), m, m->Items);
      }
     // Éléments radio : un groupe exclusif par GroupIndex
     std::map<int, QActionGroup *> Groupes;
     for(TMenuItem *m : Items)
      if (m->RadioItem&&!m->MenuQt())
       {
        QActionGroup *&g=Groupes[m->GroupIndex];
        if (!g)
         {
          g=new QActionGroup(Menu);
          g->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
         }
        m->DefinitGroupe(g);
       }
    };
  for(auto &c : Composants)
   {
    if (TPopupMenu *p=dynamic_cast<TPopupMenu *>(c.second))
     Arbre(p->Menu, nullptr, p->Items);
    else if (TMainMenu *mm=dynamic_cast<TMainMenu *>(c.second))
     for(QAction *a : mm->Barre->actions())
      {
       auto i=ElementsMenus.find(a);
       if (i==ElementsMenus.end()) continue;
       TMenuItem *m=i->second;
       mm->Items.push_back(m);
       if (m->MenuQt()) Arbre(m->MenuQt(), m, m->Items);
      }
   }
  // Boutons de barres d'outils : action ou menu associé
  for(auto &c : Composants)
   if (TToolButton *b=dynamic_cast<TToolButton *>(c.second))
    {
     QString Action=b->property("vclAction").toString(), Menu=b->property("vclMenuItem").toString();
     b->LieAction(Action.isEmpty()?nullptr:dynamic_cast<TAction *>(Composants[Action]));
     if (!Menu.isEmpty()) b->MenuItem=dynamic_cast<TMenuItem *>(Composants[Menu]);
    }
  // Propriétés PopupMenu des contrôles
  for(QWidget *w : Fiche->findChildren<QWidget *>())
   if (AvecMenuContextuel *m=dynamic_cast<AvecMenuContextuel *>(w))
    {
     QString p=w->property("vclPopupMenu").toString();
     if (!p.isEmpty()) m->PopupMenu=dynamic_cast<TPopupMenu *>(Composants[p]);
    }
  // Propriétés de la VCL appliquées après setupUi
  // (liste d'abord : PrepareVcl peut détruire des widgets internes, comme la poignée du TStatusBar)
  std::vector<ControleVclBase *> Controles;
  for(QWidget *w : Fiche->findChildren<QWidget *>())
   if (ControleVclBase *c=dynamic_cast<ControleVclBase *>(w))
    Controles.push_back(c);
  for(ControleVclBase *c : Controles)
   c->PrepareVcl();
  // Align des contrôles
  for(QWidget *w : Fiche->findChildren<QWidget *>())
   {
    QString a=w->property("vclAlign").toString();
    QWidget *p=w->parentWidget();
    if (a.isEmpty()||(a=="alNone")||!p||p->property("vclAligneur").toBool()) continue;
    p->setProperty("vclAligneur", true);
    p->installEventFilter(new Aligneur(p));
    Aligne(p);
   }
  // Police de la fiche (Font.Height en pixels)
  QVariant h=Fiche->property("vclFontHeight");
  if (h.isValid())
   {
    QFont f=Fiche->font();
    f.setPixelSize(qAbs(h.toInt()));
    Fiche->setFont(f);
   }
 }
//---------------------------------------------------------------------------
QWidget *FichePrincipaleQt=nullptr;
//---------------------------------------------------------------------------
int ToucheVirtuelleDe(const QKeyEvent *e)
 {
  switch(e->key())
   {
    case Qt::Key_Shift: return 0x10;
    case Qt::Key_Control: return 0x11;
    case Qt::Key_Alt: return 0x12;
    default: return RaccourciVcl(e)&0xFF;
   }
 }
//---------------------------------------------------------------------------
TShiftState EtatTouches(const QKeyEvent *e)
 {
  TShiftState s;
  Qt::KeyboardModifiers m=e->modifiers();
  if (m&Qt::ShiftModifier) s<<ssShift;
  if (m&Qt::ControlModifier) s<<ssCtrl;
#if defined(Q_OS_MACOS)
  if (m&Qt::MetaModifier) s<<ssCtrl;
#endif
  if (m&Qt::AltModifier) s<<ssAlt;
  return s;
 }
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// vcl/controles.cpp - Portage de Centre-Ville : contrôles de la VCL sur Qt
//---------------------------------------------------------------------------
#include "controles.h"
#include <QAbstractButton>
#include <QAction>
#include <QButtonGroup>
#include <QContextMenuEvent>
#include <QDir>
#include <QFileDialog>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QStatusTipEvent>
#include <QStyleOption>
#include <QTabBar>
//---------------------------------------------------------------------------
// Commun
//---------------------------------------------------------------------------
template <class B> void ControleVcl<B>::PrepareVcl()
 { // Propriétés de la VCL sans équivalent dans le .ui
  QWidget *w=this;
  QVariant v=w->property("vclFontHeight");
  if (v.isValid())
   { // Font.Height < 0 : hauteur des caractères en pixels (indépendant des ppp de l'écran)
    QFont f=w->font();
    int h=v.toInt();
    f.setPixelSize(h<0?-h:qMax(1, (int)(0.85*h+0.5)));
    w->setFont(f);
   }
  v=w->property("vclFontColor");
  if (v.isValid())
   {
    QPalette p=w->palette();
    QColor c=CouleurQt(CouleurDepuisTexte(A(v.toString())));
    p.setColor(QPalette::WindowText, c);
    p.setColor(QPalette::ButtonText, c);
    p.setColor(QPalette::Text, c);
    w->setPalette(p);
   }
  v=w->property("vclColor");
  if (v.isValid()&&!w->property("vclTransparent").toBool())
   {
    QPalette p=w->palette();
    p.setColor(QPalette::Window, CouleurQt(CouleurDepuisTexte(A(v.toString()))));
    w->setPalette(p);
    w->setAutoFillBackground(true);
   }
  if (w->property("vclCursor").toString()==QLatin1String("crHandPoint"))
   w->setCursor(Qt::PointingHandCursor);
 }
//---------------------------------------------------------------------------
template class ControleVcl<QLabel>;
template class ControleVcl<QSpinBox>;
template class ControleVcl<QPushButton>;
template class ControleVcl<QCheckBox>;
template class ControleVcl<QRadioButton>;
template class ControleVcl<QGroupBox>;
template class ControleVcl<QToolButton>;
template class ControleVcl<QWidget>;
template class ControleVcl<QTableWidget>;
template class ControleVcl<QSlider>;
template class ControleVcl<QFrame>;
template class ControleVcl<QStatusBar>;
template class ControleVcl<QAbstractScrollArea>;
//---------------------------------------------------------------------------
namespace
 {
  // Libellé VCL -> texte affiché par un QLabel sans compagnon : « & » souligne
  // la lettre suivante (soulignement perdu), « && » donne « & »
  QString SansAccelerateur(const QString &s)
   {
    QString r;
    for(int i=0; i<s.size(); i++)
     if (s[i]=='&')
      {
       if ((i+1<s.size())&&(s[i+1]=='&')) { r+='&'; i++; }
      }
     else r+=s[i];
    return r;
   }
 }
//---------------------------------------------------------------------------
// TLabel
//---------------------------------------------------------------------------
TLabel::TLabel(QWidget *Parent) : ControleVcl<QLabel>(Parent)
 {
  setTextFormat(Qt::PlainText);
  Caption.Definit([this]() { return asCaption; },
                  [this](const AnsiString &as)
                   {
                    asCaption=as;
                    setText(buddy()?Q(as):SansAccelerateur(Q(as)));
                   });
 }
//---------------------------------------------------------------------------
void TLabel::PrepareVcl()
 {
  ControleVcl<QLabel>::PrepareVcl();
  Caption=A(text()); // texte du .ui = Caption de la VCL
 }
//---------------------------------------------------------------------------
void TLabel::mousePressEvent(QMouseEvent *e)
 {
  QLabel::mousePressEvent(e);
  if (OnClick) OnClick(this);
 }
//---------------------------------------------------------------------------
// TCSpinEdit
//---------------------------------------------------------------------------
TCSpinEdit::TCSpinEdit(QWidget *Parent) : ControleVcl<QSpinBox>(Parent)
 {
  Value.Definit([this]() { return value(); }, [this](const int &v) { setValue(v); });
  MinValue.Definit([this]() { return property("vclMinValue").toInt(); },
                   [this](const int &v)
                    {
                     setProperty("vclMinValue", v);
                     int Max=property("vclMaxValue").toInt();
                     if (v==Max) setRange(-2147483647, 2147483647); else setRange(v, Max);
                    });
  MaxValue.Definit([this]() { return property("vclMaxValue").toInt(); },
                   [this](const int &v)
                    {
                     setProperty("vclMaxValue", v);
                     int Min=property("vclMinValue").toInt();
                     if (v==Min) setRange(-2147483647, 2147483647); else setRange(Min, v);
                    });
  Increment.Definit([this]() { return singleStep(); }, [this](const int &v) { setSingleStep(v); });
  // §4.4 : Value=... déclenche OnChange, comme la frappe
  QObject::connect(this, &QSpinBox::valueChanged, this, [this](int) { if (OnChange) OnChange(this); });
 }
//---------------------------------------------------------------------------
void TCSpinEdit::PrepareVcl()
 {
  ControleVcl<QSpinBox>::PrepareVcl();
  if (property("vclEditorEnabled").isValid()&&!property("vclEditorEnabled").toBool())
   lineEdit()->setReadOnly(true); // flèches actives, saisie impossible
 }
//---------------------------------------------------------------------------
// TButton, TBitBtn
//---------------------------------------------------------------------------
TButton::TButton(QWidget *Parent) : ControleVcl<QPushButton>(Parent)
 {
  Caption.Definit([this]() { return A(text()); }, [this](const AnsiString &as) { setText(Q(as)); });
  QObject::connect(this, &QPushButton::clicked, this, [this]() { Click(); });
 }
//---------------------------------------------------------------------------
void TButton::PrepareVcl()
 {
  ControleVcl<QPushButton>::PrepareVcl();
  ModalResult=property("vclModalResult").toInt();
  Cancel=property("vclCancel").toBool();
  Default=isDefault();
 }
//---------------------------------------------------------------------------
void TButton::Click()
 { // TButton::Click : ModalResult de la fiche, puis OnClick
  if (ModalResult!=mrNone)
   if (FicheVclBase *f=dynamic_cast<FicheVclBase *>(window()))
    f->DefinitModalResult(ModalResult);
  if (OnClick) OnClick(this);
 }
//---------------------------------------------------------------------------
void TBitBtn::DefinitGlyphe(const TBitmap *b)
 {
  Glyph=b;
  setIcon(QIcon(AgranditSansLissage(b->Image, devicePixelRatioF())));
 }
//---------------------------------------------------------------------------
// TCheckBox, TRadioButton, TGroupBox, TRadioGroup
//---------------------------------------------------------------------------
TCheckBox::TCheckBox(QWidget *Parent) : ControleVcl<QCheckBox>(Parent)
 {
  Caption.Definit([this]() { return A(text()); }, [this](const AnsiString &as) { setText(Q(as)); });
  Checked.Definit([this]() { return isChecked(); }, [this](const bool &b) { setChecked(b); });
  // §4.4 : Checked=... déclenche OnClick (toggled et non clicked)
  QObject::connect(this, &QCheckBox::toggled, this, [this](bool) { if (OnClick) OnClick(this); });
 }
//---------------------------------------------------------------------------
TRadioButton::TRadioButton(QWidget *Parent) : ControleVcl<QRadioButton>(Parent)
 {
  Caption.Definit([this]() { return A(text()); }, [this](const AnsiString &as) { setText(Q(as)); });
  Checked.Definit([this]() { return isChecked(); }, [this](const bool &b) { setChecked(b); });
  // TRadioButton::SetChecked : OnClick quand le bouton devient coché
  QObject::connect(this, &QRadioButton::toggled, this, [this](bool b) { if (b&&OnClick) OnClick(this); });
 }
//---------------------------------------------------------------------------
TGroupBox::TGroupBox(QWidget *Parent) : ControleVcl<QGroupBox>(Parent)
 {
  Caption.Definit([this]() { return A(title()); }, [this](const AnsiString &as) { setTitle(Q(as)); });
 }
//---------------------------------------------------------------------------
TRadioGroup::TRadioGroup(QWidget *Parent) : ControleVcl<QGroupBox>(Parent)
 {
  Caption.Definit([this]() { return A(title()); }, [this](const AnsiString &as) { setTitle(Q(as)); });
  ItemIndex.Definit([this]() { return Groupe?Groupe->checkedId():-1; },
                    [this](const int &i)
                     {
                      if ((i>=0)&&(i<(int)Boutons.size())) Boutons[(std::size_t)i]->setChecked(true);
                      else if (Groupe&&Groupe->checkedButton())
                       {
                        Groupe->setExclusive(false);
                        Groupe->checkedButton()->setChecked(false);
                        Groupe->setExclusive(true);
                       }
                     });
 }
//---------------------------------------------------------------------------
void TRadioGroup::PrepareVcl()
 { // Éléments générés par dfm2ui : <Nom>_0, <Nom>_1...
  ControleVcl<QGroupBox>::PrepareVcl();
  Groupe=new QButtonGroup(this);
  for(int i=0; ; i++)
   {
    QRadioButton *b=findChild<QRadioButton *>(objectName()+"_"+QString::number(i), Qt::FindDirectChildrenOnly);
    if (!b) break;
    Boutons.push_back(b);
    Groupe->addButton(b, i);
    QObject::connect(b, &QRadioButton::toggled, this, [this](bool c) { if (c&&OnClick) OnClick(this); });
   }
 }
//---------------------------------------------------------------------------
// TSpeedButton
//---------------------------------------------------------------------------
TSpeedButton::TSpeedButton(QWidget *Parent) : ControleVcl<QToolButton>(Parent)
 {
  Down.Definit([this]() { return isCheckable()?isChecked():isDown(); },
               [this](const bool &b) { if (isCheckable()) setChecked(b); else setDown(b); });
  QObject::connect(this, &QToolButton::clicked, this, [this]() { if (OnClick) OnClick(this); });
 }
//---------------------------------------------------------------------------
void TSpeedButton::PrepareVcl()
 {
  ControleVcl<QToolButton>::PrepareVcl();
  GroupIndex=property("vclGroupIndex").toInt();
  AllowAllUp=property("vclAllowAllUp").toBool();
  NumGlyphs=qMax(1, property("vclNumGlyphs").toInt());
  QString Glyphe=property("vclGlyph").toString();
  if (!Glyphe.isEmpty())
   {
    TBitmap b;
    b.Image=QPixmap(Glyphe);
    if (NumGlyphs>1) b.Image=b.Image.copy(0, 0, b.Image.width()/NumGlyphs, b.Image.height());
    DefinitGlyphe(&b);
   }
 }
//---------------------------------------------------------------------------
void TSpeedButton::DefinitGlyphe(const TBitmap *b)
 {
  Glyph=b;
  setIconSize(b->Image.size());
  setIcon(QIcon(AgranditSansLissage(b->Image, devicePixelRatioF())));
 }
//---------------------------------------------------------------------------
void TSpeedButton::Click()
 {
  click();
 }
//---------------------------------------------------------------------------
// TShape, TImage, TStringGrid, TTrackBar
//---------------------------------------------------------------------------
void TShape::PrepareVcl()
 {
  ControleVcl<QWidget>::PrepareVcl();
  QString s=property("vclShape").toString();
  if (s=="stCircle") Shape=stCircle;
  else if (s=="stEllipse") Shape=stEllipse;
  else if (s=="stSquare") Shape=stSquare;
  else if (s=="stRoundRect") Shape=stRoundRect;
  else if (s=="stRoundSquare") Shape=stRoundSquare;
  if (property("vclBrushColor").isValid()) Brush->Color=CouleurDepuisTexte(A(property("vclBrushColor").toString()));
  if (property("vclPenColor").isValid()) Pen->Color=CouleurDepuisTexte(A(property("vclPenColor").toString()));
 }
//---------------------------------------------------------------------------
void TShape::paintEvent(QPaintEvent *)
 { // TShape::Paint (crayon de 1 pixel)
  QPainter p(this);
  TCanvas c;
  c.Commence(&p);
  c.Brush=Brush.Ref();
  c.Pen=Pen.Ref();
  int w=width(), h=height(), x=0, y=0;
  if ((Shape==stSquare)||(Shape==stRoundSquare)||(Shape==stCircle))
   {
    int m=qMin(w, h);
    x=(w-m)/2; y=(h-m)/2; w=h=m;
   }
  if ((Shape==stEllipse)||(Shape==stCircle))
   c.Ellipse(x, y, x+w, y+h);
  else
   c.Rectangle(x, y, x+w, y+h);
  c.Termine();
 }
//---------------------------------------------------------------------------
TImage::TImage(QWidget *Parent) : ControleVcl<QLabel>(Parent) {}
//---------------------------------------------------------------------------
void TImage::mousePressEvent(QMouseEvent *e)
 {
  QLabel::mousePressEvent(e);
  if (OnClick) OnClick(this);
 }
//---------------------------------------------------------------------------
TStringGrid::TStringGrid(QWidget *Parent) : ControleVcl<QTableWidget>(Parent), Cells(this)
 {
  ColCount.Definit([this]() { return columnCount(); }, [this](const int &n) { setColumnCount(n); });
  RowCount.Definit([this]() { return rowCount(); }, [this](const int &n) { setRowCount(n); });
 }
//---------------------------------------------------------------------------
AnsiString TStringGrid::LitCellule(int c, int r) const
 {
  QTableWidgetItem *i=item(r, c);
  return i?A(i->text()):AnsiString();
 }
//---------------------------------------------------------------------------
void TStringGrid::EcritCellule(int c, int r, const AnsiString &as)
 {
  QTableWidgetItem *i=item(r, c);
  if (!i) setItem(r, c, i=new QTableWidgetItem);
  i->setText(Q(as));
 }
//---------------------------------------------------------------------------
TTrackBar::TTrackBar(QWidget *Parent) : ControleVcl<QSlider>(Parent)
 {
  Position.Definit([this]() { return value(); }, [this](const int &v) { setValue(v); });
  Min.Definit([this]() { return minimum(); }, [this](const int &v) { setMinimum(v); });
  Max.Definit([this]() { return maximum(); }, [this](const int &v) { setMaximum(v); });
  QObject::connect(this, &QSlider::valueChanged, this, [this](int) { if (OnChange) OnChange(this); });
 }
//---------------------------------------------------------------------------
// TPanel, TPaintBox, TTabControl
//---------------------------------------------------------------------------
void TPanel::contextMenuEvent(QContextMenuEvent *e)
 {
  if (PopupMenu) PopupMenu->Popup(e->globalPos().x(), e->globalPos().y());
  else QFrame::contextMenuEvent(e);
 }
//---------------------------------------------------------------------------
void TPaintBox::paintEvent(QPaintEvent *)
 {
  QPainter p(this);
  Canvas->Commence(&p);
  if (OnPaint) OnPaint(this);
  Canvas->Termine();
 }
//---------------------------------------------------------------------------
TTabControl::TTabControl(QWidget *Parent) : ControleVcl<QWidget>(Parent)
 {
  TabIndex.Definit([this]() { return Onglets?Onglets->currentIndex():-1; },
                   [this](const int &i) { if (Onglets) Onglets->setCurrentIndex(i); });
 }
//---------------------------------------------------------------------------
void TTabControl::PrepareVcl()
 {
  ControleVcl<QWidget>::PrepareVcl();
  Onglets=new QTabBar(this);
  for(const QString &s : property("vclTabsStrings").toStringList()) Onglets->addTab(s);
  if (property("vclTabIndex").isValid()) Onglets->setCurrentIndex(property("vclTabIndex").toInt());
  Onglets->setGeometry(0, 0, width(), Onglets->sizeHint().height());
  QObject::connect(Onglets, &QTabBar::currentChanged, this, [this](int) { if (OnChange) OnChange(this); });
 }
//---------------------------------------------------------------------------
// TStatusBar
//---------------------------------------------------------------------------
TStatusPanel::TStatusPanel(TStatusBar *b, int Largeur, TAlignment Alignement, const AnsiString &asTexteInitial) :
  Barre(b), asTexte(asTexteInitial), iLargeur(Largeur), Alignment(Alignement)
 {
  Text.Definit([this]() { return asTexte; },
               [this](const AnsiString &as) { if (as!=asTexte) { asTexte=as; Barre->update(); } });
  Width.Definit([this]() { return iLargeur; },
                [this](const int &w) { if (w!=iLargeur) { iLargeur=w; Barre->update(); } });
 }
//---------------------------------------------------------------------------
TStatusPanels::~TStatusPanels()
 {
  for(TStatusPanel *p : Items) delete p;
 }
//---------------------------------------------------------------------------
TStatusBar::TStatusBar(QWidget *Parent) : ControleVcl<QStatusBar>(Parent)
 {
  SimpleText.Definit([this]() { return asSimpleText; },
                     [this](const AnsiString &as) { asSimpleText=as; update(); });
  Font->Name="MS Sans Serif";
  Font->Height=-11;
  Font->Color=clBtnText;
 }
//---------------------------------------------------------------------------
void TStatusBar::PrepareVcl()
 {
  ControleVcl<QStatusBar>::PrepareVcl();
  setSizeGripEnabled(false);
  for(const QString &s : property("vclPanels").toStringList())
   { // « Width=400;Alignment=taCenter;Text=(0, 0) »
    int Largeur=50;
    TStatusPanel::TAlignment Alignement=TStatusPanel::taLeftJustify;
    AnsiString asTexte;
    for(const QString &kv : s.split(';'))
     {
      int i=(int)kv.indexOf('=');
      QString k=kv.left(i), v=kv.mid(i+1);
      if (k=="Width") Largeur=v.toInt();
      else if (k=="Alignment") Alignement=(v=="taCenter")?TStatusPanel::taCenter:
                                         (v=="taRightJustify")?TStatusPanel::taRightJustify:
                                                               TStatusPanel::taLeftJustify;
      else if (k=="Text") asTexte=A(v);
     }
    Panels->Items.push_back(new TStatusPanel(this, Largeur, Alignement, asTexte));
   }
  SimplePanel=property("vclSimplePanel").toBool();
  AutoHint=property("vclAutoHint").toBool();
  if (property("vclFontName").isValid()) Font->Name=A(property("vclFontName").toString());
  if (property("vclFontHeight").isValid()) Font->Height=property("vclFontHeight").toInt();
  setMinimumHeight(height());
  setMaximumHeight(height());
 }
//---------------------------------------------------------------------------
void TStatusBar::paintEvent(QPaintEvent *)
 { // Panneaux à largeur fixe séparés par un trait, texte en retrait de 2 pixels
  QPainter p(this);
  QColor Fond=palette().color(QPalette::Window);
  p.fillRect(rect(), Fond);
  QFont f=Font->Qt();
  p.setFont(f);
  QColor Ombre=Fond.darker(140), Texte=palette().color(QPalette::WindowText);
  if (SimplePanel||Panels->Items.empty())
   {
    p.setPen(Texte);
    p.drawText(rect().adjusted(3, 0, -3, 0), Qt::AlignVCenter|Qt::AlignLeft, Q(asSimpleText));
    return;
   }
  int x=0;
  for(TStatusPanel *Panneau : Panels->Items)
   {
    int w=Panneau->Width;
    QRect r(x, 0, w, height());
    p.setPen(Ombre);
    p.drawLine(r.right(), 2, r.right(), height()-3);
    p.setPen(Texte);
    int a=(Panneau->Alignment==TStatusPanel::taCenter)?Qt::AlignHCenter:
          (Panneau->Alignment==TStatusPanel::taRightJustify)?Qt::AlignRight:Qt::AlignLeft;
    p.save();
    p.setClipRect(r.adjusted(0, 0, -2, 0));
    p.drawText(r.adjusted(3, 0, -4, 0), a|Qt::AlignVCenter|Qt::TextSingleLine, Q(Panneau->Text.Valeur()));
    p.restore();
    x+=w;
    if (x>=width()) break;
   }
 }
//---------------------------------------------------------------------------
void TStatusBar::resizeEvent(QResizeEvent *e)
 {
  QStatusBar::resizeEvent(e);
  if (OnResize) OnResize(this);
 }
//---------------------------------------------------------------------------
bool TStatusBar::event(QEvent *e)
 { // AutoHint : le texte d'aide du contrôle survolé va dans le premier panneau
  if (e->type()==QEvent::StatusTip)
   {
    if (AutoHint)
     {
      AnsiString as=A(static_cast<QStatusTipEvent *>(e)->tip());
      if (SimplePanel||Panels->Items.empty()) SimpleText=as;
      else Panels->Items[0]->Text=as;
     }
    return true;
   }
  return QStatusBar::event(e);
 }
//---------------------------------------------------------------------------
void TStatusBar::contextMenuEvent(QContextMenuEvent *e)
 {
  if (PopupMenu) PopupMenu->Popup(e->globalPos().x(), e->globalPos().y());
 }
//---------------------------------------------------------------------------
// TToolButton
//---------------------------------------------------------------------------
TToolButton::TToolButton(QWidget *Parent) : ControleVcl<QToolButton>(Parent)
 {
  setFocusPolicy(Qt::NoFocus); // les boutons de barre d'outils ne prennent pas le focus
  Caption.Definit([this]() { return A(text()); }, [this](const AnsiString &as) { setText(Q(as)); });
  Down.Definit([this]() { return bDown; },
               [this](const bool &b)
                {
                 bDown=b;
                 if (isCheckable()) setChecked(b); else setDown(b);
                });
  Action.Definit([this]() { return ActionLiee; },
                 [this](TAction *const &a)
                  { // TControl::SetAction : recopie complète des propriétés de l'action
                   if (a==ActionLiee) { if (a) ActionChange(a); return; }
                   if (ActionLiee) ActionLiee->RetireClient(this);
                   ActionLiee=a;
                   if (a) { a->AjouteClient(this); ActionChange(a); }
                  });
  MenuItem.Definit([this]() { return ElementMenu; }, [this](TMenuItem *const &m) { ElementMenu=m; });
  QObject::connect(this, &QToolButton::clicked, this, [this]() { Clic(); });
  QObject::connect(this, &QToolButton::toggled, this, [this](bool b) { bDown=b; });
 }
//---------------------------------------------------------------------------
void TToolButton::LieAction(TAction *a)
 {
  ActionLiee=a;
  if (a) a->AjouteClient(this);
  bDown=isChecked();
  Style=(property("vclStyle").toString()=="tbsCheck")?tbsCheck:tbsButton;
  ImageIndex=a?a->ImageIndex:property("vclImageIndex").toInt();
 }
//---------------------------------------------------------------------------
void TToolButton::Clic()
 { // TToolButton::Click : menu déroulant (MenuItem) ou action
  if (ElementMenu&&ElementMenu->MenuQt())
   {
    setDown(true);
    ElementMenu->MenuQt()->exec(mapToGlobal(QPoint(0, height())));
    setDown(false);
    return;
   }
  if (OnClick&&!ActionLiee) OnClick(this);
  else if (ActionLiee) ActionLiee->Execute();
 }
//---------------------------------------------------------------------------
void TToolButton::ActionChange(TAction *a)
 { // TToolButton::ActionChange(Sender, CheckDefaults=false)
  Caption=a->Caption.Valeur();
  Enabled=(bool)a->Enabled;
  Hint=a->Hint.Valeur();
  Visible=(bool)a->Visible;
  ImageIndex=a->ImageIndex;
  setIcon(a->Icone);
  Down=(bool)a->Checked;
 }
//---------------------------------------------------------------------------
void TToolButton::LienChecked(bool Ancien, bool Nouveau) { if (bDown==Ancien) Down=Nouveau; }
void TToolButton::LienEnabled(bool Ancien, bool Nouveau) { if ((bool)Enabled==Ancien) Enabled=Nouveau; }
void TToolButton::LienVisible(bool Ancien, bool Nouveau) { if ((bool)Visible==Ancien) Visible=Nouveau; }
//---------------------------------------------------------------------------
// Composants non visuels
//---------------------------------------------------------------------------
TTimer::TTimer(QObject *Parent) : QObject(Parent)
 {
  Minuteur.setInterval(1000);
  Enabled.Definit([this]() { return Minuteur.isActive(); },
                  [this](const bool &b) { if (b) { if (!Minuteur.isActive()) Minuteur.start(); } else Minuteur.stop(); });
  Interval.Definit([this]() { return Minuteur.interval(); },
                   [this](const int &i) { Minuteur.setInterval(i); });
  QObject::connect(&Minuteur, &QTimer::timeout, this, [this]() { if (OnTimer) OnTimer(this); });
 }
//---------------------------------------------------------------------------
bool TOpenDialog::Execute()
 {
  QString Titre=Q(Title);
  if (Titre.isEmpty()) Titre=Enregistrement?QString::fromUtf8("Enregistrer sous"):QString::fromUtf8("Ouvrir");
  QFileDialog d(nullptr, Titre);
  d.setAcceptMode(Enregistrement?QFileDialog::AcceptSave:QFileDialog::AcceptOpen);
  d.setFileMode(Enregistrement?QFileDialog::AnyFile:QFileDialog::ExistingFile);
  // Filtre VCL « Libellé|*.cvw|Tous fichiers (*.*)|*.* » -> « Libellé (*.cvw);;Tous fichiers (*) »
  QStringList Morceaux=Q(Filter).split('|'), Filtres;
  for(int i=0; i+1<Morceaux.size(); i+=2)
   {
    QString Libelle=Morceaux[i], Motifs=Morceaux[i+1];
    Motifs.replace(';', ' ');
    if (Motifs=="*.*") Motifs="*";
    if (Libelle.endsWith(')')) Libelle=Libelle.left(Libelle.lastIndexOf('(')).trimmed();
    Filtres<<(Libelle+" ("+Motifs+")");
   }
  if (!Filtres.isEmpty()) d.setNameFilters(Filtres);
  if (!DefaultExt.IsEmpty()) d.setDefaultSuffix(Q(DefaultExt));
  QString Nom=Q(FileName);
  QFileInfo Info(Nom);
  if (!Nom.isEmpty()&&Info.isAbsolute())
   {
    d.setDirectory(Info.absolutePath());
    d.selectFile(Info.fileName());
   }
  else
   {
    if (!InitialDir.IsEmpty()) d.setDirectory(Q(InitialDir));
    if (!Nom.isEmpty()) d.selectFile(Nom);
   }
  if (d.exec()!=QDialog::Accepted) return false;
  QStringList l=d.selectedFiles();
  if (l.isEmpty()) return false;
  FileName=A(QDir::toNativeSeparators(l.first()));
  return true;
 }
//---------------------------------------------------------------------------
void ConfigureNonVisuel(QObject *Composant, const QStringList &Proprietes)
 {
  for(const QString &kv : Proprietes)
   {
    int i=(int)kv.indexOf('=');
    QString k=kv.left(i), v=kv.mid(i+1);
    if (auto *t=dynamic_cast<TTimer *>(Composant))
     {
      if (k=="Interval") t->Interval=v.toInt();
      else if (k=="Enabled") t->Enabled=(v=="True");
      else if (k=="Tag") t->Tag=v.toInt();
     }
    else if (auto *d=dynamic_cast<TOpenDialog *>(Composant))
     {
      if (k=="DefaultExt") d->DefaultExt=A(v);
      else if (k=="Filter") d->Filter=A(v);
      else if (k=="Title") d->Title=A(v);
      else if (k=="Options") d->OverwritePrompt=v.contains("ofOverwritePrompt");
     }
   }
  // TTimer : Enabled vaut True par défaut dans la VCL
  if (auto *t=dynamic_cast<TTimer *>(Composant))
   if (!Proprietes.join(';').contains("Enabled=")) t->Enabled=true;
 }
//---------------------------------------------------------------------------

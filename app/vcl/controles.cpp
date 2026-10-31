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
#include <QFocusEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStatusTipEvent>
#include <QStyle>
#include <QStyleOption>
#include <QTabBar>
#include <cstring>
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
void DefinitCouleurControle(QWidget *w, const char *Propriete, TColor Couleur)
 {
  w->setProperty(Propriete, QString("0x%1").arg((uint)Couleur, 8, 16, QChar('0')));
  QPalette p=w->palette();
  QColor c=CouleurQt(Couleur);
  if (!std::strcmp(Propriete, "vclFontColor"))
   {
    p.setColor(QPalette::WindowText, c);
    p.setColor(QPalette::ButtonText, c);
    p.setColor(QPalette::Text, c);
   }
  else
   {
    p.setColor(QPalette::Window, c);
    if (!w->property("vclTransparent").toBool()) w->setAutoFillBackground(true);
   }
  w->setPalette(p);
 }
//---------------------------------------------------------------------------
void FocusAuClic(QWidget *w)
 { // TSpeedButton et TToolButton, jamais focalisés, passent ensuite à NoFocus
  if (w->focusPolicy()==Qt::TabFocus) w->setFocusPolicy(Qt::StrongFocus);
 }
//---------------------------------------------------------------------------
int TagDe(const TObject *Composant)
 {
  if (!Composant) return 0;
  if (auto *m=dynamic_cast<const TMenuItem *>(Composant)) return m->Tag;
  if (auto *a=dynamic_cast<const TAction *>(Composant)) return a->Tag;
  if (auto *t=dynamic_cast<const TTimer *>(Composant)) return t->Tag;
  return Composant->property("vclTag").toInt();
 }
//---------------------------------------------------------------------------
String NomDe(const TObject *Composant)
 {
  return Composant?A(Composant->objectName()):String();
 }
//---------------------------------------------------------------------------
String NomClasseDe(const TObject *Composant)
 { // Classes dérivées avant leurs bases (TBitBtn avant TButton)
  if (!Composant) return String();
  if (dynamic_cast<const TBitBtn *>(Composant)) return "TBitBtn";
  if (dynamic_cast<const TButton *>(Composant)) return "TButton";
  if (dynamic_cast<const TSpeedButton *>(Composant)) return "TSpeedButton";
  if (dynamic_cast<const TRadioGroup *>(Composant)) return "TRadioGroup";
  if (dynamic_cast<const TGroupBox *>(Composant)) return "TGroupBox";
  if (dynamic_cast<const TLabel *>(Composant)) return "TLabel";
  if (dynamic_cast<const TCSpinEdit *>(Composant)) return "TCSpinEdit";
  if (dynamic_cast<const TCheckBox *>(Composant)) return "TCheckBox";
  if (dynamic_cast<const TRadioButton *>(Composant)) return "TRadioButton";
  if (dynamic_cast<const TShape *>(Composant)) return "TShape";
  if (dynamic_cast<const TImage *>(Composant)) return "TImage";
  if (dynamic_cast<const TStringGrid *>(Composant)) return "TStringGrid";
  if (dynamic_cast<const TTrackBar *>(Composant)) return "TTrackBar";
  if (dynamic_cast<const TPanel *>(Composant)) return "TPanel";
  if (dynamic_cast<const TBevel *>(Composant)) return "TBevel";
  if (dynamic_cast<const TPaintBox *>(Composant)) return "TPaintBox";
  if (dynamic_cast<const TTabControl *>(Composant)) return "TTabControl";
  if (dynamic_cast<const TStatusBar *>(Composant)) return "TStatusBar";
  if (dynamic_cast<const TToolBar *>(Composant)) return "TToolBar";
  if (dynamic_cast<const TToolButton *>(Composant)) return "TToolButton";
  if (dynamic_cast<const TTimer *>(Composant)) return "TTimer";
  if (dynamic_cast<const TSaveDialog *>(Composant)) return "TSaveDialog";
  if (dynamic_cast<const TOpenDialog *>(Composant)) return "TOpenDialog";
  if (dynamic_cast<const TMenuItem *>(Composant)) return "TMenuItem";
  if (dynamic_cast<const TPopupMenu *>(Composant)) return "TPopupMenu";
  if (dynamic_cast<const TMainMenu *>(Composant)) return "TMainMenu";
  if (dynamic_cast<const TAction *>(Composant)) return "TAction";
  if (dynamic_cast<const TActionList *>(Composant)) return "TActionList";
  if (dynamic_cast<const TVirtualImageList *>(Composant)) return "TVirtualImageList";
  if (dynamic_cast<const TImageCollection *>(Composant)) return "TImageCollection";
  return Composant->metaObject()->className();
 }
//---------------------------------------------------------------------------
QIcon IconeGlyphe(const QPixmap &Glyphe, int NumGlyphs, qreal Echelle)
 { // TButtonGlyph : la 1re image pour l'état normal, la 2e (si NumGlyphs>=2) désactivé
  QIcon i;
  if (Glyphe.isNull()) return i;
  int n=qMax(1, NumGlyphs), w=Glyphe.width()/n, h=Glyphe.height();
  i.addPixmap(AgranditSansLissage(Glyphe.copy(0, 0, w, h), Echelle), QIcon::Normal);
  if (n>=2) i.addPixmap(AgranditSansLissage(Glyphe.copy(w, 0, w, h), Echelle), QIcon::Disabled);
  return i;
 }
//---------------------------------------------------------------------------
template class ControleVcl<QLabel>;
template class ControleVcl<QAbstractSpinBox>;
template class ControleVcl<QPushButton>;
template class ControleVcl<QCheckBox>;
template class ControleVcl<QRadioButton>;
template class ControleVcl<QGroupBox>;
template class ControleVcl<QToolButton>;
template class ControleVcl<QWidget>;
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
                    AjusteTaille();
                   });
 }
//---------------------------------------------------------------------------
void TLabel::PrepareVcl()
 {
  ControleVcl<QLabel>::PrepareVcl();
  AutoSize=!property("vclAutoSize").isValid()||property("vclAutoSize").toBool();
  Caption=A(text()); // texte du .ui = Caption de la VCL
  Prepare=true;
 }
//---------------------------------------------------------------------------
void TLabel::AjusteTaille()
 { // TCustomLabel::AdjustBounds : DT_CALCRECT (un espace si le texte est vide)
  if (!Prepare||!AutoSize||wordWrap()) return;
  QString t=text();
  if (t.isEmpty()) t=" ";
  QSize s=fontMetrics().size(0, t);
  int x=this->x();
  if (alignment()&Qt::AlignRight) x+=width()-s.width();
  setGeometry(x, y(), s.width(), s.height());
 }
//---------------------------------------------------------------------------
namespace
 { // TControl::WMLButtonUp : Click si le bouton gauche est relâché sur le
   // contrôle ; le contrôle garde le clic (la fiche n'en est pas avertie)
  bool ClicRelache(QWidget *w, QMouseEvent *e)
   {
    e->accept();
    return (e->button()==Qt::LeftButton)&&w->rect().contains(e->position().toPoint());
   }
 }
//---------------------------------------------------------------------------
void TLabel::mousePressEvent(QMouseEvent *e)
 {
  QLabel::mousePressEvent(e);
  e->accept();
 }
//---------------------------------------------------------------------------
void TLabel::mouseReleaseEvent(QMouseEvent *e)
 {
  QLabel::mouseReleaseEvent(e);
  if (ClicRelache(this, e)&&OnClick) OnClick(this);
 }
//---------------------------------------------------------------------------
// TCSpinEdit
//---------------------------------------------------------------------------
TCSpinEdit::TCSpinEdit(QWidget *Parent) : ControleVcl<QAbstractSpinBox>(Parent)
 {
  lineEdit()->setText("0"); // TCSpinEdit.Create : Text:='0', curseur au début
  lineEdit()->setCursorPosition(0);
  Value.Definit([this]()
                 { // GetValue : StrToInt(Text), MinValue si le texte n'est pas un nombre
                  bool Ok=false;
                  int v=lineEdit()->text().toInt(&Ok);
                  return Ok?v:(int)MinValue;
                 },
                [this](const int &v)
                 { // SetValue : Text:=IntToStr(CheckValue(v)) ; comme WM_SETTEXT, curseur au début
                  QString t=QString::number(CheckValue(v));
                  if (t==lineEdit()->text()) return;
                  lineEdit()->setText(t);
                  lineEdit()->setCursorPosition(0);
                 });
  // Simples champs, mémorisés dans les propriétés « vcl... » du .ui
  MinValue.Definit([this]() { return property("vclMinValue").toInt(); },
                   [this](const int &v) { setProperty("vclMinValue", v); });
  MaxValue.Definit([this]() { return property("vclMaxValue").toInt(); },
                   [this](const int &v) { setProperty("vclMaxValue", v); });
  Increment.Definit([this]() { return property("vclIncrement").isValid()?property("vclIncrement").toInt():1; },
                    [this](const int &v) { setProperty("vclIncrement", v); });
  // §4.4 : OnChange à chaque changement du texte (TControl::SetText ne change
  // rien si le texte est le même), par la frappe ou par Value=...
  QObject::connect(lineEdit(), &QLineEdit::textChanged, this, [this]() { if (OnChange) OnChange(this); });
 }
//---------------------------------------------------------------------------
void TCSpinEdit::PrepareVcl()
 {
  ControleVcl<QAbstractSpinBox>::PrepareVcl();
  if (property("vclValue").isValid()) Value=property("vclValue").toInt(); // lu après MinValue et MaxValue
  if (property("vclEditorEnabled").isValid()&&!property("vclEditorEnabled").toBool())
   lineEdit()->setReadOnly(true); // flèches actives, saisie impossible
 }
//---------------------------------------------------------------------------
int TCSpinEdit::CheckValue(int v)
 { // Bornes appliquées seulement si MaxValue<>MinValue
  int Min=MinValue, Max=MaxValue;
  if (Max!=Min)
   {
    if (v<Min) return Min;
    if (v>Max) return Max;
   }
  return v;
 }
//---------------------------------------------------------------------------
void TCSpinEdit::stepBy(int steps)
 { // Value:=Value+FIncrement (ou -), puis, comme QAbstractSpinBox, texte sélectionné
  Value=Value+steps*Increment;
  if (style()->styleHint(QStyle::SH_SpinBox_SelectOnStep, nullptr, this)) selectAll();
 }
//---------------------------------------------------------------------------
QAbstractSpinBox::StepEnabled TCSpinEdit::stepEnabled() const
 { // Flèches toujours actives (QAbstractSpinBox : aucune, faute de type de valeur)
  return StepUpEnabled|StepDownEnabled;
 }
//---------------------------------------------------------------------------
QValidator::State TCSpinEdit::validate(QString &input, int &pos) const
 { // Caractères admis par IsValidChar, quel que soit le texte obtenu
  Q_UNUSED(pos);
  for(QChar c : input)
   if (((c<'0')||(c>'9'))&&(c!='+')&&(c!='-')&&(c!=QLatin1Char(FormatSettings.DecimalSeparator)))
    return QValidator::Invalid;
  return QValidator::Acceptable;
 }
//---------------------------------------------------------------------------
void TCSpinEdit::focusInEvent(QFocusEvent *e)
 { // CMEnter (AutoSelect) : texte sélectionné en entrant dans le champ, sauf au
   // clic ; pas quand la fenêtre retrouve l'activation (après une boîte de
   // message, par exemple), le champ étant resté le contrôle actif de la fiche
  ControleVcl<QAbstractSpinBox>::focusInEvent(e);
  bool Entree=!ControleActif;
  ControleActif=true;
  if (Entree&&(e->reason()!=Qt::MouseFocusReason)) selectAll();
 }
//---------------------------------------------------------------------------
void TCSpinEdit::focusOutEvent(QFocusEvent *e)
 { // CMExit : seulement vers un autre contrôle de la fiche (la VCL n'envoie pas
   // CM_EXIT quand la fenêtre perd l'activation, se ferme ou qu'un menu s'ouvre)
  ControleVcl<QAbstractSpinBox>::focusOutEvent(e);
  if ((e->reason()==Qt::ActiveWindowFocusReason)||(e->reason()==Qt::PopupFocusReason)||
      (e->reason()==Qt::MenuBarFocusReason)) return;
  ControleActif=false;
  Valide();
 }
//---------------------------------------------------------------------------
void TCSpinEdit::Valide()
 {
  int v=Value;
  if (CheckValue(v)!=v) Value=v; // SetValue(Value)
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
TBitBtn::TBitBtn(QWidget *Parent) : TButton(Parent)
 {
  Glyph->OnChange=[this]() { AfficheGlyphe(); };
 }
//---------------------------------------------------------------------------
void TBitBtn::PrepareVcl()
 {
  TButton::PrepareVcl();
  NumGlyphs=qMax(1, property("vclNumGlyphs").toInt());
  QString Glyphe=property("vclGlyph").toString();
  if (!Glyphe.isEmpty())
   {
    TBitmap b;
    b.Image=QPixmap(Glyphe);
    DefinitGlyphe(&b);
   }
 }
//---------------------------------------------------------------------------
void TBitBtn::DefinitGlyphe(const TBitmap *b)
 {
  Glyph=b;
 }
//---------------------------------------------------------------------------
void TBitBtn::AfficheGlyphe()
 {
  const QPixmap &g=Glyph.Ref().Image;
  if (!g.isNull()) setIconSize(QSize(g.width()/qMax(1, NumGlyphs), g.height()));
  setIcon(IconeGlyphe(g, NumGlyphs, devicePixelRatioF()));
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
namespace
 { // Éléments d'un TRadioGroup (QRadioButton créés par uic) : un clic n'importe
   // où dans l'élément le coche, comme dans la VCL (Qt : la case et le texte)
  class ClicElement : public QObject
   {
    public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
     {
      if ((e->type()!=QEvent::MouseButtonPress)&&(e->type()!=QEvent::MouseButtonRelease)&&
          (e->type()!=QEvent::MouseButtonDblClick)&&(e->type()!=QEvent::MouseMove)) return false;
      QRadioButton *b=static_cast<QRadioButton *>(o);
      QMouseEvent *m=static_cast<QMouseEvent *>(e);
      QStyleOptionButton Option;
      Option.initFrom(b);
      Option.text=b->text();
      QRect Zone=b->style()->subElementRect(QStyle::SE_RadioButtonClickRect, &Option, b);
      QPoint p=m->position().toPoint();
      if (Zone.contains(p)||!b->rect().contains(p)) return false;
      // Même événement, au centre de la zone sensible de Qt
      QMouseEvent Copie(m->type(), QPointF(Zone.center()), m->scenePosition(), m->globalPosition(),
                        m->button(), m->buttons(), m->modifiers(), m->pointingDevice());
      QCoreApplication::sendEvent(b, &Copie);
      return true;
     }
   };
 }
//---------------------------------------------------------------------------
void TRadioGroup::PrepareVcl()
 { // Éléments générés par dfm2ui : <Nom>_0, <Nom>_1...
  ControleVcl<QGroupBox>::PrepareVcl();
  Groupe=new QButtonGroup(this);
  ClicElement *Filtre=new ClicElement(this);
  for(int i=0; ; i++)
   {
    QRadioButton *b=findChild<QRadioButton *>(objectName()+"_"+QString::number(i), Qt::FindDirectChildrenOnly);
    if (!b) break;
    Boutons.push_back(b);
    Groupe->addButton(b, i);
    FocusAuClic(b);
    b->installEventFilter(Filtre);
    QObject::connect(b, &QRadioButton::toggled, this, [this](bool c) { if (c&&OnClick) OnClick(this); });
   }
 }
//---------------------------------------------------------------------------
// TSpeedButton
//---------------------------------------------------------------------------
TSpeedButton::TSpeedButton(QWidget *Parent) : ControleVcl<QToolButton>(Parent)
 {
  setFocusPolicy(Qt::NoFocus); // TGraphicControl : jamais le focus
  Down.Definit([this]() { return isChecked(); }, [this](const bool &b) { DefinitDown(b); });
  Glyph->OnChange=[this]() { AfficheGlyphe(); };
  QObject::connect(this, &QToolButton::clicked, this, [this]() { if (OnClick) OnClick(this); });
 }
//---------------------------------------------------------------------------
void TSpeedButton::PrepareVcl()
 {
  ControleVcl<QToolButton>::PrepareVcl();
  GroupIndex=property("vclGroupIndex").toInt();
  AllowAllUp=property("vclAllowAllUp").toBool();
  NumGlyphs=qMax(1, property("vclNumGlyphs").toInt());
  setCheckable(GroupIndex!=0); // GroupIndex=0 : jamais enfoncé (Down reste à false)
  QString Glyphe=property("vclGlyph").toString();
  if (!Glyphe.isEmpty())
   {
    TBitmap b;
    b.Image=QPixmap(Glyphe);
    DefinitGlyphe(&b);
   }
 }
//---------------------------------------------------------------------------
void TSpeedButton::DefinitGlyphe(const TBitmap *b)
 {
  Glyph=b;
 }
//---------------------------------------------------------------------------
void TSpeedButton::AfficheGlyphe()
 {
  const QPixmap &g=Glyph.Ref().Image;
  if (!g.isNull()) setIconSize(QSize(g.width()/qMax(1, NumGlyphs), g.height()));
  setIcon(IconeGlyphe(g, NumGlyphs, devicePixelRatioF()));
 }
//---------------------------------------------------------------------------
void TSpeedButton::DefinitDown(bool Valeur)
 { // TSpeedButton::SetDown : un bouton enfoncé ne remonte que si AllowAllUp ;
   // enfoncé, il fait remonter les autres boutons du même GroupIndex (UpdateExclusive)
  if (GroupIndex==0) Valeur=false;
  if (Valeur==isChecked()) return;
  if (isChecked()&&!AllowAllUp) return;
  setChecked(Valeur);
  if (Valeur&&parentWidget())
   for(QObject *o : parentWidget()->children())
    if (TSpeedButton *b=dynamic_cast<TSpeedButton *>(o))
     if ((b!=this)&&(b->GroupIndex==GroupIndex)&&b->isChecked())
      b->setChecked(false);
 }
//---------------------------------------------------------------------------
void TSpeedButton::nextCheckState()
 { // TSpeedButton::MouseUp : SetDown(not FDown) puis Click
  DefinitDown(!isChecked());
 }
//---------------------------------------------------------------------------
void TSpeedButton::Click()
 {
  if (OnClick) OnClick(this);
 }
//---------------------------------------------------------------------------
// TShape, TImage, TStringGrid, TTrackBar
//---------------------------------------------------------------------------
TShape::TShape(QWidget *Parent) : ControleVcl<QWidget>(Parent)
 { // TBrush/TPen::OnChange : Shape->Brush->Color=... repeint la forme
  Brush.SurAcces=[this]() { update(); };
  Pen.SurAcces=[this]() { update(); };
 }
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
  e->accept();
 }
//---------------------------------------------------------------------------
void TImage::mouseReleaseEvent(QMouseEvent *e)
 {
  QLabel::mouseReleaseEvent(e);
  if (ClicRelache(this, e)&&OnClick) OnClick(this);
 }
//---------------------------------------------------------------------------
// TStringGrid : vcl/grilletextes.cpp
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
  Onglets->setDrawBase(false); // le cadre de la page est dessiné par paintEvent
  Onglets->setExpanding(false);
  Onglets->setUsesScrollButtons(false);
  Onglets->setElideMode(Qt::ElideRight);
  for(const QString &s : property("vclTabsStrings").toStringList()) Onglets->addTab(s);
  if (property("vclTabIndex").isValid()) Onglets->setCurrentIndex(property("vclTabIndex").toInt());
  Onglets->setGeometry(0, 0, width(), Onglets->sizeHint().height());
  Onglets->lower(); // sous les contrôles de la page
  QObject::connect(Onglets, &QTabBar::currentChanged, this, [this](int) { if (OnChange) OnChange(this); });
 }
//---------------------------------------------------------------------------
void TTabControl::paintEvent(QPaintEvent *)
 { // Cadre de la page, sous les onglets (TCustomTabControl : bord de 2 pixels)
  QPainter p(this);
  QStyleOptionTabWidgetFrame o;
  o.initFrom(this);
  int h=Onglets?Onglets->height():0;
  o.rect=rect().adjusted(0, qMax(0, h-2), 0, 0);
  o.shape=QTabBar::RoundedNorth;
  o.tabBarSize=Onglets?Onglets->size():QSize();
  o.lineWidth=style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, this);
  style()->drawPrimitive(QStyle::PE_FrameTabWidget, &o, &p, this);
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

//---------------------------------------------------------------------------
// vcl/controles.h - Portage de Centre-Ville : contrôles de la VCL sur Qt
//
// Chaque classe porte le nom du contrôle VCL qu'elle remplace et dérive du
// widget Qt le plus proche (TLabel : QLabel, TCSpinEdit : QAbstractSpinBox...) :
// les .ui générés par tools/dfm2ui.py les « promeuvent », et uic les instancie
// directement. Elles offrent les propriétés de la VCL (Caption, Checked,
// Value, Visible, Enabled, Tag...) et ses événements implicites
// (PLAN_PORTAGE.md §4.4) :
//   - TCSpinEdit::Value=... déclenche OnChange quand le texte change ;
//   - TCheckBox::Checked=... déclenche OnClick (toggled, pas clicked) ;
//   - TRadioButton::Checked=true déclenche OnClick ;
//   - TRadioGroup::ItemIndex=... déclenche OnClick.
// TSpeedButton suit la VCL : jamais le focus, Down exclusif par GroupIndex
// (AllowAllUp), Click() n'appelle qu'OnClick (sans changer Down).
// Les propriétés que Qt Designer ne connaît pas arrivent en propriétés
// dynamiques « vcl... » et sont appliquées par PrepareVcl() après setupUi.
//
// Composants non visuels : TTimer, TOpenDialog, TSaveDialog (créés à partir
// de la propriété dynamique « vcl_<Nom> » de la fiche).
//---------------------------------------------------------------------------
#ifndef vcl_controlesH
#define vcl_controlesH
//---------------------------------------------------------------------------
#include <QAbstractScrollArea>
#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QWidget>
#include <map>
#include <vector>
#include "actions.h"
#include "graphiques.h"
#include "listeimages.h"
//---------------------------------------------------------------------------
class QButtonGroup;
class QTabBar;
//---------------------------------------------------------------------------
// Color / Font->Color d'un contrôle : mémorisée en propriété « vcl... » et
// appliquée à la palette (fond rempli sauf contrôle Transparent)
void DefinitCouleurControle(QWidget *w, const char *Propriete, TColor Couleur);
//---------------------------------------------------------------------------
// Contrôle fenêtré de Windows (bouton, case à cocher, curseur...) : le clic lui
// donne le focus, ce qui fait sortir le contrôle précédent (CMExit d'un
// TCSpinEdit) ; sous macOS, Qt ne le leur donne qu'à la tabulation (TabFocus)
void FocusAuClic(QWidget *w);
//---------------------------------------------------------------------------
// TComponent::Tag, Name et ClassName() d'un composant désigné par TComponent*
// ou TObject* (QObject, qui n'a pas ces membres) :
// ((TComponent*)Sender)->Tag devient TagDe((TComponent*)Sender)
//---------------------------------------------------------------------------
int TagDe(const TObject *Composant);
String NomDe(const TObject *Composant);
String NomClasseDe(const TObject *Composant);
//---------------------------------------------------------------------------
// Glyph d'un TBitBtn ou d'un TSpeedButton : NumGlyphs images côte à côte
// (normale, puis désactivée) -> icône Qt, agrandie sans lissage
QIcon IconeGlyphe(const QPixmap &Glyphe, int NumGlyphs, qreal Echelle);
//---------------------------------------------------------------------------
// Fiche (TForm) vue des contrôles : TButton::ModalResult ferme la fiche modale
//---------------------------------------------------------------------------
class FicheVclBase
 {
  public:
  virtual ~FicheVclBase() {}
  virtual void DefinitModalResult(int mr) = 0;
 };
//---------------------------------------------------------------------------
// Interface commune, pour PrepareVcl() après setupUi
//---------------------------------------------------------------------------
class ControleVclBase
 {
  public:
  virtual ~ControleVclBase() {}
  virtual void PrepareVcl() {}
 };
//---------------------------------------------------------------------------
// Police d'un contrôle (TControl::Font, sous-ensemble) : Font->Color colore le
// texte du contrôle
//---------------------------------------------------------------------------
class PoliceControle
 {
  public:
  Propriete<TColor> Color;
 };
//---------------------------------------------------------------------------
// Propriétés communes à tous les contrôles (TControl / TWinControl)
//---------------------------------------------------------------------------
template <class B> class ControleVcl : public B, public ControleVclBase
 {
  PoliceControle Police;
  public:
  Propriete<int> Left, Top, Width, Height, Tag;
  Propriete<bool> Visible, Enabled;
  Propriete<AnsiString> Hint, Name;
  Propriete<TColor> Color;
  PoliceControle *const Font=&Police;
  TNotifyEvent OnClick;
  explicit ControleVcl(QWidget *Parent=nullptr) : B(Parent)
   {
    QWidget *w=this;
    FocusAuClic(w);
    Name.Definit([w]() { return A(w->objectName()); }, [w](const AnsiString &as) { w->setObjectName(Q(as)); });
    // Color et Font->Color : mêmes propriétés « vclColor »/« vclFontColor » que le .ui (PrepareVcl)
    Color.Definit([w]() { return w->property("vclColor").isValid()?CouleurDepuisTexte(A(w->property("vclColor").toString())):clBtnFace; },
                  [w](const TColor &c) { DefinitCouleurControle(w, "vclColor", c); });
    Font->Color.Definit([w]() { return w->property("vclFontColor").isValid()?CouleurDepuisTexte(A(w->property("vclFontColor").toString())):clWindowText; },
                        [w](const TColor &c) { DefinitCouleurControle(w, "vclFontColor", c); });
    Left.Definit([w]() { return w->x(); }, [w](const int &v) { w->move(v, w->y()); });
    Top.Definit([w]() { return w->y(); }, [w](const int &v) { w->move(w->x(), v); });
    Width.Definit([w]() { return w->width(); }, [w](const int &v) { w->resize(v, w->height()); });
    Height.Definit([w]() { return w->height(); }, [w](const int &v) { w->resize(w->width(), v); });
    Tag.Definit([w]() { return w->property("vclTag").toInt(); },
                [w](const int &v) { w->setProperty("vclTag", v); });
    // Visible et Enabled de la VCL : l'état propre du contrôle, pas celui hérité des parents
    Visible.Definit([w]() { return !w->isHidden(); }, [w](const bool &v) { w->setVisible(v); });
    Enabled.Definit([w]() { return !w->testAttribute(Qt::WA_ForceDisabled); },
                    [w](const bool &v) { w->setEnabled(v); });
    Hint.Definit([w]() { return A(w->statusTip()); },
                 [w](const AnsiString &as) { w->setStatusTip(Q(as)); w->setToolTip(Q(as)); });
   }
  // TControl::Repaint : dessin immédiat, sauf pendant le dessin d'un autre
  // contrôle (RafraichitActionsEdition depuis OnDrawCell) : dessin différé
  void Repaint() { if (TCanvas::NbDessinsEnCours) B::update(); else B::repaint(); }
  void Invalidate() { B::update(); }
  void Refresh() { Repaint(); }
  void Show() { B::show(); }
  void Hide() { B::hide(); }
  void SetFocus() { B::setFocus(); }
  bool Focused() const { return B::hasFocus(); }
  void BringToFront() { B::raise(); }
  void PrepareVcl() override; // police, couleurs, curseur
 };
//---------------------------------------------------------------------------
// AutoSize (True par défaut, sauf WordWrap) : à chaque Caption=... du code, le
// libellé prend la taille de son texte, son bord droit restant en place s'il
// est aligné à droite (TCustomLabel::AdjustBounds) : les valeurs des fiches
// d'informations, plus longues que celles du .dfm, restent entières. Au
// chargement, la taille du .dfm est gardée : avec les polices de remplacement,
// plus larges, les libellés fixes déborderaient sur leurs voisins (la VCL les
// aurait masqués, ses TLabel étant opaques)
class TLabel : public ControleVcl<QLabel>
 {
  AnsiString asCaption;
  bool Prepare=false;
  void AjusteTaille();
  public:
  bool AutoSize=true;
  Propriete<AnsiString> Caption;
  explicit TLabel(QWidget *Parent=nullptr);
  void PrepareVcl() override;
  protected:
  void mousePressEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;
 };
//---------------------------------------------------------------------------
// TCSpinEdit (CSPIN.pas) : un champ de texte et deux flèches, dont le texte
// fait foi. Value lit StrToInt(Text), ou MinValue si le texte n'est pas un
// nombre ; Value=... écrit IntToStr(CheckValue(v)). MinValue et MaxValue sont
// de simples champs (les changer ne touche ni au texte ni à OnChange), que
// CheckValue applique sauf s'ils valent tous deux 0 (v6.0.3 : CSPIN.pas, s'ils
// diffèrent) : à l'affectation de Value, aux flèches (Value=Value±Increment) et
// en quittant le champ (CMExit). La frappe n'est pas bornée (IsValidChar :
// chiffres, signes, séparateur décimal).
// OnChange suit le texte (EN_CHANGE) : frappe, ou Value=... qui le change.
// Base QAbstractSpinBox et non QSpinBox : sans type de valeur, Qt ne touche
// jamais au texte (QSpinBox le borne pendant la frappe et le remplace par sa
// valeur interne, signaux bloqués, en quittant le champ ou sur Entrée)
class TCSpinEdit : public ControleVcl<QAbstractSpinBox>
 {
  bool ControleActif=false; // contrôle actif de sa fiche (entre CM_ENTER et CM_EXIT)
  bool AffectationEnCours=false; // v6.0.2 : le texte change par Value=..., pas par la frappe
  int CheckValue(int v);
  public:
  Propriete<int> Value, MinValue, MaxValue, Increment;
  TNotifyEvent OnChange;
  TNotifyEvent OnExit; // v6.0.2 : en quittant le champ, et à Entrée, une fois la valeur dans les bornes
  bool Frappe() const { return !AffectationEnCours; } // v6.0.2 : dans OnChange : l'utilisateur est en train de taper
  bool GardeTouchesDeplacement=false; // v6.0.2 : flèches, Origine, Fin et pages restent au champ, avant les raccourcis de la fiche
  explicit TCSpinEdit(QWidget *Parent=nullptr);
  void PrepareVcl() override;
  void Valide(); // v6.0.2 : valeur ramenée dans les bornes (CMExit, et Entrée dans un champ, qui valide la fiche sans en sortir)
  void stepBy(int steps) override; // UpClick / DownClick (flèches, touches haut et bas)
  QValidator::State validate(QString &input, int &pos) const override; // IsValidChar
  protected:
  StepEnabled stepEnabled() const override;
  void focusInEvent(QFocusEvent *e) override; // CMEnter
  void focusOutEvent(QFocusEvent *e) override; // CMExit
 };
//---------------------------------------------------------------------------
class TButton : public ControleVcl<QPushButton>
 {
  public:
  Propriete<AnsiString> Caption;
  int ModalResult=mrNone;
  bool Default=false, Cancel=false;
  explicit TButton(QWidget *Parent=nullptr);
  void Click();
  void PrepareVcl() override;
 };
//---------------------------------------------------------------------------
class TBitBtn : public TButton
 {
  void AfficheGlyphe();
  public:
  SousObjet<TBitmap> Glyph; // Glyph=... ou GetBitmap(..., Glyph) : le bouton se redessine
  int NumGlyphs=1;
  explicit TBitBtn(QWidget *Parent=nullptr);
  void DefinitGlyphe(const TBitmap *b);
  void PrepareVcl() override;
 };
//---------------------------------------------------------------------------
class TCheckBox : public ControleVcl<QCheckBox>
 {
  public:
  Propriete<AnsiString> Caption;
  Propriete<bool> Checked;
  explicit TCheckBox(QWidget *Parent=nullptr);
  protected:
  // Zone sensible de la VCL : tout le contrôle (Qt : la case et le texte)
  bool hitButton(const QPoint &pos) const override { return rect().contains(pos); }
 };
//---------------------------------------------------------------------------
class TRadioButton : public ControleVcl<QRadioButton>
 {
  public:
  Propriete<AnsiString> Caption;
  Propriete<bool> Checked;
  explicit TRadioButton(QWidget *Parent=nullptr);
  protected:
  bool hitButton(const QPoint &pos) const override { return rect().contains(pos); }
 };
//---------------------------------------------------------------------------
class TGroupBox : public ControleVcl<QGroupBox>
 {
  public:
  Propriete<AnsiString> Caption;
  explicit TGroupBox(QWidget *Parent=nullptr);
 };
//---------------------------------------------------------------------------
class TRadioGroup : public ControleVcl<QGroupBox>
 {
  QButtonGroup *Groupe=nullptr;
  std::vector<QRadioButton *> Boutons;
  public:
  Propriete<AnsiString> Caption;
  Propriete<int> ItemIndex;
  explicit TRadioGroup(QWidget *Parent=nullptr);
  void PrepareVcl() override;
 };
//---------------------------------------------------------------------------
class TSpeedButton : public ControleVcl<QToolButton>
 {
  void AfficheGlyphe();
  void DefinitDown(bool Valeur); // TSpeedButton::SetDown
  public:
  Propriete<bool> Down;
  int GroupIndex=0, NumGlyphs=1;
  bool AllowAllUp=false;
  SousObjet<TBitmap> Glyph; // Glyph=... ou GetBitmap(..., Glyph) : le bouton se redessine
  explicit TSpeedButton(QWidget *Parent=nullptr);
  void DefinitGlyphe(const TBitmap *b);
  void Click(); // TControl::Click : OnClick seulement
  void PrepareVcl() override;
  protected:
  void nextCheckState() override; // clic : SetDown(!Down), puis OnClick (clicked)
 };
//---------------------------------------------------------------------------
enum TShapeType { stRectangle, stSquare, stRoundRect, stRoundSquare, stEllipse, stCircle };
class TShape : public ControleVcl<QWidget>
 {
  public:
  SousObjet<TBrush> Brush;
  SousObjet<TPen> Pen;
  TShapeType Shape=stRectangle;
  explicit TShape(QWidget *Parent=nullptr);
  void PrepareVcl() override;
  protected:
  void paintEvent(QPaintEvent *e) override;
 };
//---------------------------------------------------------------------------
class TImage : public ControleVcl<QLabel>
 {
  public:
  explicit TImage(QWidget *Parent=nullptr);
  protected:
  void mousePressEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;
 };
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// TStringGrid : grille de textes dessinée comme TCustomGrid/TStringGrid (comme
// GrilleSimulation pour le TDrawGrid), implémentée dans vcl/grilletextes.cpp :
//   - FixedCols/FixedRows (1 par défaut) restent en place au défilement ; les
//     cases fixes ont le style des en-têtes du système (gdsThemed, le dessin
//     par défaut de la VCL récente), les autres sont sur Color (clWindow),
//     séparées par des traits de GridLineWidth pixels (clSilver) ;
//   - Cells est creux, comme le TSparseList de la VCL : une case hors de
//     ColCount/RowCount est mémorisée sans être affichée ; réduire RowCount ou
//     ColCount efface les cases retirées ;
//   - texte en (Left+2, Top+2), coupé à la case (TStringGrid::DrawCell) ; une
//     police Symbol (SYMBOL_CHARSET) est traduite en Unicode, comme Windows
//     le fait pour les polices symboles (flèches des tableaux de distances) ;
//   - case courante (Col, Row), en (FixedCols, FixedRows) au départ : cadre
//     de focus quand la grille a le focus, surlignée (clHighlight) sinon ;
//     sélection rectangulaire avec goRangeSelect ;
//   - défilement par cases entières (TopRow, LeftCol), barres selon
//     ScrollBars et seulement si les cases débordent ;
//   - clic, glisser, molette, flèches, Page préc./suiv., Début/Fin déplacent
//     la case courante (OnClick, comme TCustomGrid::FocusCell).
// uic l'instancie depuis le .ui (dfm2ui.py) : rowCount/columnCount y sont les
// RowCount/ColCount du .dfm, les autres propriétés arrivent en « vcl... ».
//---------------------------------------------------------------------------
class TStringGrid : public ControleVcl<QAbstractScrollArea>
 {
  public:
  class Colonne
   {
    TStringGrid *g; int c;
    public:
    Colonne(TStringGrid *gg, int cc) : g(gg), c(cc) {}
    class Cellule
     {
      TStringGrid *g; int c, r;
      public:
      Cellule(TStringGrid *gg, int cc, int rr) : g(gg), c(cc), r(rr) {}
      operator AnsiString() const { return g->LitCellule(c, r); }
      Cellule &operator=(const AnsiString &as) { g->EcritCellule(c, r, as); return *this; }
      Cellule &operator=(const Cellule &x) { g->EcritCellule(c, r, (AnsiString)x); return *this; }
     };
    Cellule operator[](int r) { return Cellule(g, c, r); }
   };
  class Cellules
   {
    TStringGrid *g;
    public:
    explicit Cellules(TStringGrid *gg) : g(gg) {}
    Colonne operator[](int c) { return Colonne(g, c); }
   } Cells;
  class Tailles
   { // ColWidths[i], RowHeights[i]
    TStringGrid *g; bool Colonnes;
    public:
    class Taille
     {
      TStringGrid *g; bool Colonnes; int i;
      public:
      Taille(TStringGrid *gg, bool c, int ii) : g(gg), Colonnes(c), i(ii) {}
      operator int() const { return g->LitTaille(Colonnes, i); }
      Taille &operator=(int v) { g->EcritTaille(Colonnes, i, v); return *this; }
     };
    Tailles(TStringGrid *gg, bool c) : g(gg), Colonnes(c) {}
    Taille operator[](int i) { return Taille(g, Colonnes, i); }
   } ColWidths, RowHeights;
  Propriete<int> ColCount, RowCount, FixedCols, FixedRows, DefaultColWidth, DefaultRowHeight,
                 GridLineWidth, Col, Row, LeftCol, TopRow;
  explicit TStringGrid(QWidget *Parent=nullptr);
  void setRowCount(int n) { RowCount=n; } // propriétés du .ui (uic)
  void setColumnCount(int n) { ColCount=n; }
  AnsiString LitCellule(int c, int r) const;
  void EcritCellule(int c, int r, const AnsiString &as);
  int LitTaille(bool Colonne, int i) const;
  void EcritTaille(bool Colonne, int i, int Taille);
  TRect CellRect(int ACol, int ARow) const; // rectangle dans le viewport (vide si la case est cachée)
  void Repaint() { if (TCanvas::NbDessinsEnCours) viewport()->update(); else viewport()->repaint(); }
  void Invalidate() { viewport()->update(); }
  void PrepareVcl() override;
  protected:
  void paintEvent(QPaintEvent *e) override;
  void resizeEvent(QResizeEvent *e) override;
  void scrollContentsBy(int dx, int dy) override;
  void mousePressEvent(QMouseEvent *e) override;
  void mouseMoveEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;
  void wheelEvent(QWheelEvent *e) override;
  void keyPressEvent(QKeyEvent *e) override;
  void focusInEvent(QFocusEvent *e) override;
  void focusOutEvent(QFocusEvent *e) override;
  private:
  std::map<std::pair<int, int>, AnsiString> Donnees; // (colonne, ligne) -> texte
  std::vector<int> Largeurs, Hauteurs;
  int NbColonnes=5, NbLignes=5, NbFixesC=1, NbFixesL=1, LargeurDefaut=64, HauteurDefaut=24, Trait=1;
  int ColCour=1, LigCour=1, ColAncre=1, LigAncre=1, PremCol=1, PremLig=1; // case courante, ancre, LeftCol/TopRow
  bool PlageSelection=true, TraitsFixesH=true, TraitsFixesV=true, TraitsH=true, TraitsV=true;
  bool SelectionEnCours=false, PoliceSymbole=false;
  int CumulMolette=0;
  void DefinitColonnes(int n);
  void DefinitLignes(int n);
  int Largeur(int c) const { return ((c>=0)&&(c<(int)Largeurs.size()))?Largeurs[(std::size_t)c]:LargeurDefaut; }
  int Hauteur(int r) const { return ((r>=0)&&(r<(int)Hauteurs.size()))?Hauteurs[(std::size_t)r]:HauteurDefaut; }
  int DebutColonne(int c) const; // abscisse de la case (sans tenir compte du masquage)
  int DebutLigne(int r) const;
  int DerniereColonneGauche() const; // LeftCol maximal (dernière colonne entièrement visible)
  int DerniereLigneHaut() const;
  void MetAJourDefilement();
  void DefinitHautGauche(int Gauche, int Haut);
  void Montre(int c, int r); // TCustomGrid::ClampInView
  bool CaseEn(const QPoint &p, int &c, int &r) const;
  void Deplace(int c, int r, bool DeplaceAncre, bool Clic); // MoveCurrent / FocusCell
 };
//---------------------------------------------------------------------------
class TTrackBar : public ControleVcl<QSlider>
 {
  public:
  Propriete<int> Position, Min, Max;
  TNotifyEvent OnChange;
  explicit TTrackBar(QWidget *Parent=nullptr);
 };
//---------------------------------------------------------------------------
class TPanel : public ControleVcl<QFrame>, public AvecMenuContextuel
 {
  public:
  explicit TPanel(QWidget *Parent=nullptr) : ControleVcl<QFrame>(Parent) {}
  protected:
  void contextMenuEvent(QContextMenuEvent *e) override;
 };
//---------------------------------------------------------------------------
class TBevel : public ControleVcl<QFrame>
 {
  public:
  explicit TBevel(QWidget *Parent=nullptr) : ControleVcl<QFrame>(Parent) {}
 };
//---------------------------------------------------------------------------
class TPaintBox : public ControleVcl<QWidget>
 {
  TCanvas CanvasPeinture;
  public:
  TCanvas *Canvas=&CanvasPeinture;
  TNotifyEvent OnPaint;
  explicit TPaintBox(QWidget *Parent=nullptr) : ControleVcl<QWidget>(Parent) {}
  protected:
  void paintEvent(QPaintEvent *e) override;
 };
//---------------------------------------------------------------------------
// TTabControl : onglets en haut (QTabBar), cadre de la page dessous ; les
// contrôles de la page sont des enfants directs, comme dans le .dfm. Les
// onglets restent tous visibles (texte élidé si la police de remplacement est
// trop large), comme ceux de la VCL, sans boutons de défilement.
class TTabControl : public ControleVcl<QWidget>
 {
  QTabBar *Onglets=nullptr;
  public:
  Propriete<int> TabIndex;
  TNotifyEvent OnChange;
  explicit TTabControl(QWidget *Parent=nullptr);
  void PrepareVcl() override;
  protected:
  void paintEvent(QPaintEvent *e) override;
 };
//---------------------------------------------------------------------------
// TStatusBar : panneaux de largeur fixe dessinés comme la VCL
//---------------------------------------------------------------------------
class TStatusBar;
class TStatusPanel
 {
  TStatusBar *Barre;
  AnsiString asTexte;
  int iLargeur;
  public:
  enum TAlignment { taLeftJustify, taRightJustify, taCenter } Alignment=taLeftJustify;
  Propriete<AnsiString> Text;
  Propriete<int> Width;
  TStatusPanel(TStatusBar *b, int Largeur, TAlignment Alignement, const AnsiString &asTexteInitial);
 };
class TStatusPanels
 {
  public:
  std::vector<TStatusPanel *> Items;
  int Count() const { return (int)Items.size(); }
  ~TStatusPanels();
 };
class TStatusBar : public ControleVcl<QStatusBar>, public AvecMenuContextuel
 {
  TStatusPanels PanneauxVcl;
  AnsiString asSimpleText;
  public:
  TStatusPanels *Panels=&PanneauxVcl;
  bool SimplePanel=false, AutoHint=false;
  SousObjet<TFont> Font;
  Propriete<AnsiString> SimpleText;
  TNotifyEvent OnResize;
  explicit TStatusBar(QWidget *Parent=nullptr);
  void PrepareVcl() override;
  protected:
  void paintEvent(QPaintEvent *e) override;
  void resizeEvent(QResizeEvent *e) override;
  bool event(QEvent *e) override;
  void contextMenuEvent(QContextMenuEvent *e) override;
 };
//---------------------------------------------------------------------------
// TToolBar et TToolButton (boutons à position absolue, comme la VCL)
//---------------------------------------------------------------------------
class TToolBar : public ControleVcl<QWidget>
 {
  public:
  explicit TToolBar(QWidget *Parent=nullptr) : ControleVcl<QWidget>(Parent) {}
 };
//---------------------------------------------------------------------------
enum TToolButtonStyle { tbsButton, tbsCheck, tbsDropDown, tbsSeparator, tbsDivider };
class TToolButton : public ControleVcl<QToolButton>, public ClientAction
 {
  TAction *ActionLiee=nullptr;
  TMenuItem *ElementMenu=nullptr;
  bool bDown=false;
  void Clic();
  public:
  TToolButtonStyle Style=tbsButton;
  int ImageIndex=-1;
  Propriete<AnsiString> Caption;
  Propriete<bool> Down;
  Propriete<TAction *> Action;
  Propriete<TMenuItem *> MenuItem;
  explicit TToolButton(QWidget *Parent=nullptr);
  void LieAction(TAction *a); // lien établi au chargement (sans recopie)
  void Click() { Clic(); }
  // ClientAction
  void ActionChange(TAction *a) override;
  void LienChecked(bool Ancien, bool Nouveau) override;
  void LienEnabled(bool Ancien, bool Nouveau) override;
  void LienVisible(bool Ancien, bool Nouveau) override;
 };
//---------------------------------------------------------------------------
// Composants non visuels
//---------------------------------------------------------------------------
class TTimer : public QObject
 {
  QTimer Minuteur;
  public:
  int Tag=0;
  Propriete<bool> Enabled;
  Propriete<int> Interval;
  TNotifyEvent OnTimer;
  explicit TTimer(QObject *Parent=nullptr);
 };
//---------------------------------------------------------------------------
class TOpenDialog : public QObject
 {
  protected:
  bool Enregistrement=false;
  public:
  String FileName, InitialDir;
  AnsiString Filter, Title, DefaultExt;
  bool OverwritePrompt=false;
  explicit TOpenDialog(QObject *Parent=nullptr) : QObject(Parent) {}
  bool Execute();
 };
//---------------------------------------------------------------------------
class TSaveDialog : public TOpenDialog
 {
  public:
  explicit TSaveDialog(QObject *Parent=nullptr) : TOpenDialog(Parent) { Enregistrement=true; }
 };
//---------------------------------------------------------------------------
// Configuration d'un composant non visuel depuis la propriété « vcl_<Nom> »
void ConfigureNonVisuel(QObject *Composant, const QStringList &Proprietes);
//---------------------------------------------------------------------------
#endif

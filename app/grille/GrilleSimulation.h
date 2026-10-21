//---------------------------------------------------------------------------
// grille/GrilleSimulation.h - Portage de Centre-Ville : carte de simulation
//
// Remplace le TDrawGrid DrawGridSimul de la fiche principale (phase 4 de
// PLAN_PORTAGE.md) en reproduisant son comportement :
//   - cases de DefaultColWidth x DefaultRowHeight (16 x 16), sans lignes ;
//   - case courante (Col, Row) et ancre : la sélection (Selection) est le
//     rectangle qui les joint (goRangeSelect) ; le clic place les deux, le
//     glisser et Maj+clic déplacent l'ancre, Maj+flèches déplacent la case
//     courante ;
//   - OnClick au relâchement du bouton et à chaque déplacement au clavier ou à
//     la molette (FocusCell), OnDblClick sans déplacement, clic droit sans
//     changement de case puis menu contextuel PopupMenu ;
//   - défilement par cases entières (LeftCol, TopRow) ;
//   - dessin « classique » : fond Color (clHighlight pour les cases
//     sélectionnées autres que la case active), OnDrawCell appelé seulement
//     pour les cases de la zone à repeindre, puis cadre de focus en pointillés.
// HiDPI : les coordonnées restent logiques (16 px par case) ; les images sont
// agrandies sans lissage (vcl/listeimages.h).
//---------------------------------------------------------------------------
#ifndef GrilleSimulationH
#define GrilleSimulationH
//---------------------------------------------------------------------------
#include <QAbstractScrollArea>
#include <QTimer>
#include "vcl/controles.h"
//---------------------------------------------------------------------------
struct TGridCoord
 {
  int X=0, Y=0;
 };
struct TGridRect
 {
  int Left=0, Top=0, Right=0, Bottom=0;
 };
enum TGridOption { goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect,
                   goDrawFocusSelected, goRowSizing, goColSizing, goRowMoving, goColMoving,
                   goEditing, goTabs, goRowSelect, goAlwaysShowEditor, goThumbTracking };
typedef TEnsemble<TGridOption> TGridOptions;
enum TGridDrawStateElement { gdSelected, gdFocused, gdFixed };
typedef TEnsemble<TGridDrawStateElement> TGridDrawState;
typedef std::function<void(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State)> TDrawCellEvent;
//---------------------------------------------------------------------------
class GrilleSimulation : public ControleVcl<QAbstractScrollArea>, public AvecMenuContextuel
 {
  TCanvas CanvasGrille;
  TGridCoord Courante, Ancre, HautGauche;
  int NbColonnes=5, NbLignes=5;
  bool SelectionEnCours=false;
  int CumulMolette=0;
  QTimer DefilementAuto;
  QPoint DernierePosition;
  void MetAJourDefilement();
  void MetAJourSelection();
  QRect RectGrille(const TGridRect &r) const; // en pixels du viewport
  void InvalideSelection(const TGridRect &Ancienne);
  TGridCoord CaseEn(const QPoint &p, bool Bornee) const;
  void MoveCurrent(int ACol, int ARow, bool MoveAnchor);
  void MoveAnchor(TGridCoord NouvelleAncre);
  void FocusCell(int ACol, int ARow, bool MoveAnchor);
  void ClampInView(TGridCoord c);
  void DefinitHautGauche(int Gauche, int Haut);
  int ColonnesVisibles() const; // cases entièrement visibles
  int LignesVisibles() const;
  public:
  // Propriétés de TDrawGrid utilisées par la fiche
  Propriete<int> ColCount, RowCount, Col, Row, LeftCol, TopRow;
  int DefaultColWidth=16, DefaultRowHeight=16;
  TGridOptions Options;
  TColor Color=clBlack;
  SousObjet<TFont> Font;
  TCanvas *Canvas=&CanvasGrille;
  TGridRect Selection;
  TDrawCellEvent OnDrawCell;
  TNotifyEvent OnDblClick;
  explicit GrilleSimulation(QWidget *Parent=nullptr);
  void PrepareVcl() override;
  TRect CellRect(int ACol, int ARow) const;
  void InvalidateCell(int ACol, int ARow); // RedessineCase -> update(rect)
  void Repaint(); // TControl::Repaint : dessin immédiat
  protected:
  void paintEvent(QPaintEvent *e) override;
  void resizeEvent(QResizeEvent *e) override;
  void scrollContentsBy(int dx, int dy) override;
  void mousePressEvent(QMouseEvent *e) override;
  void mouseMoveEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;
  void mouseDoubleClickEvent(QMouseEvent *e) override;
  void wheelEvent(QWheelEvent *e) override;
  void keyPressEvent(QKeyEvent *e) override;
  void contextMenuEvent(QContextMenuEvent *e) override;
  void focusInEvent(QFocusEvent *e) override;
  void focusOutEvent(QFocusEvent *e) override;
 };
//---------------------------------------------------------------------------
typedef GrilleSimulation TDrawGrid;
//---------------------------------------------------------------------------
#endif

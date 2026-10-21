//---------------------------------------------------------------------------
// vcl/grilletextes.cpp - Portage de Centre-Ville : TStringGrid
// (comportement de TCustomGrid / TStringGrid de la VCL, voir controles.h)
//---------------------------------------------------------------------------
#include "controles.h"
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QStyleOption>
#include <QWheelEvent>
//---------------------------------------------------------------------------
namespace
 {
  // Codage de la police Symbol (Adobe Symbol, SYMBOL_CHARSET) -> Unicode, de
  // 0x20 à 0xFE : ce que Windows affiche avec cette police (0xAC à 0xAF :
  // flèches gauche, haut, droite, bas). 0 : caractère conservé.
  const char16_t CodageSymbol[0xFF-0x20]=
   {
    0x0020, 0x0021, 0x2200, 0x0023, 0x2203, 0x0025, 0x0026, 0x220B, // 20
    0x0028, 0x0029, 0x2217, 0x002B, 0x002C, 0x2212, 0x002E, 0x002F, // 28
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, // 30
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, // 38
    0x2245, 0x0391, 0x0392, 0x03A7, 0x0394, 0x0395, 0x03A6, 0x0393, // 40
    0x0397, 0x0399, 0x03D1, 0x039A, 0x039B, 0x039C, 0x039D, 0x039F, // 48
    0x03A0, 0x0398, 0x03A1, 0x03A3, 0x03A4, 0x03A5, 0x03C2, 0x03A9, // 50
    0x039E, 0x03A8, 0x0396, 0x005B, 0x2234, 0x005D, 0x22A5, 0x005F, // 58
    0x203E, 0x03B1, 0x03B2, 0x03C7, 0x03B4, 0x03B5, 0x03C6, 0x03B3, // 60
    0x03B7, 0x03B9, 0x03D5, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BF, // 68
    0x03C0, 0x03B8, 0x03C1, 0x03C3, 0x03C4, 0x03C5, 0x03D6, 0x03C9, // 70
    0x03BE, 0x03C8, 0x03B6, 0x007B, 0x007C, 0x007D, 0x223C, 0,      // 78
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                 // 80
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                 // 90
    0x20AC, 0x03D2, 0x2032, 0x2264, 0x2044, 0x221E, 0x0192, 0x2663, // A0
    0x2666, 0x2665, 0x2660, 0x2194, 0x2190, 0x2191, 0x2192, 0x2193, // A8
    0x00B0, 0x00B1, 0x2033, 0x2265, 0x00D7, 0x221D, 0x2202, 0x2022, // B0
    0x00F7, 0x2260, 0x2261, 0x2248, 0x2026, 0x23D0, 0x23AF, 0x21B5, // B8
    0x2135, 0x2111, 0x211C, 0x2118, 0x2297, 0x2295, 0x2205, 0x2229, // C0
    0x222A, 0x2283, 0x2287, 0x2284, 0x2282, 0x2286, 0x2208, 0x2209, // C8
    0x2220, 0x2207, 0x00AE, 0x00A9, 0x2122, 0x220F, 0x221A, 0x22C5, // D0
    0x00AC, 0x2227, 0x2228, 0x21D4, 0x21D0, 0x21D1, 0x21D2, 0x21D3, // D8
    0x25CA, 0x2329, 0x00AE, 0x00A9, 0x2122, 0x2211, 0x239B, 0x239C, // E0
    0x239D, 0x23A1, 0x23A2, 0x23A3, 0x23A7, 0x23A8, 0x23A9, 0x23AA, // E8
    0,      0x232A, 0x222B, 0x2320, 0x23AE, 0x2321, 0x239E, 0x239F, // F0
    0x23A0, 0x23A4, 0x23A5, 0x23A6, 0x23AB, 0x23AC, 0x23AD          // F8
   };
  QString TexteSymbol(const QString &s)
   {
    QString r=s;
    for(QChar &c : r)
     if ((c.unicode()>=0x20)&&(c.unicode()<0xFF)&&CodageSymbol[c.unicode()-0x20])
      c=QChar(CodageSymbol[c.unicode()-0x20]);
    return r;
   }
 }
//---------------------------------------------------------------------------
TStringGrid::TStringGrid(QWidget *Parent) : ControleVcl<QAbstractScrollArea>(Parent), Cells(this),
                                            ColWidths(this, true), RowHeights(this, false)
 {
  setFocusPolicy(Qt::StrongFocus);
  setFrameStyle(QFrame::Panel|QFrame::Sunken); // BorderStyle bsSingle, Ctl3D : bord creux de 2 pixels
  setLineWidth(2);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
  ColCount.Definit([this]() { return NbColonnes; }, [this](const int &n) { DefinitColonnes(n); });
  RowCount.Definit([this]() { return NbLignes; }, [this](const int &n) { DefinitLignes(n); });
  FixedCols.Definit([this]() { return NbFixesC; },
                    [this](const int &n)
                     {
                      NbFixesC=qBound(0, n, NbColonnes-1);
                      PremCol=qMax(PremCol, NbFixesC);
                      Deplace(qMax(ColCour, NbFixesC), LigCour, true, false);
                      MetAJourDefilement();
                     });
  FixedRows.Definit([this]() { return NbFixesL; },
                    [this](const int &n)
                     {
                      NbFixesL=qBound(0, n, NbLignes-1);
                      PremLig=qMax(PremLig, NbFixesL);
                      Deplace(ColCour, qMax(LigCour, NbFixesL), true, false);
                      MetAJourDefilement();
                     });
  DefaultColWidth.Definit([this]() { return LargeurDefaut; },
                          [this](const int &v)
                           { // TCustomGrid::SetDefaultColWidth : toutes les colonnes reprennent la largeur par défaut
                            LargeurDefaut=v;
                            Largeurs.assign((std::size_t)NbColonnes, v);
                            MetAJourDefilement();
                            viewport()->update();
                           });
  DefaultRowHeight.Definit([this]() { return HauteurDefaut; },
                           [this](const int &v)
                            {
                             HauteurDefaut=v;
                             Hauteurs.assign((std::size_t)NbLignes, v);
                             MetAJourDefilement();
                             viewport()->update();
                            });
  GridLineWidth.Definit([this]() { return Trait; },
                        [this](const int &v) { Trait=qMax(0, v); MetAJourDefilement(); viewport()->update(); });
  // TCustomGrid::SetCol/SetRow : FocusCell (déplacement puis Click)
  Col.Definit([this]() { return ColCour; }, [this](const int &c) { Deplace(c, LigCour, true, true); });
  Row.Definit([this]() { return LigCour; }, [this](const int &r) { Deplace(ColCour, r, true, true); });
  LeftCol.Definit([this]() { return PremCol; }, [this](const int &c) { DefinitHautGauche(c, PremLig); });
  TopRow.Definit([this]() { return PremLig; }, [this](const int &r) { DefinitHautGauche(PremCol, r); });
  Largeurs.assign((std::size_t)NbColonnes, LargeurDefaut);
  Hauteurs.assign((std::size_t)NbLignes, HauteurDefaut);
 }
//---------------------------------------------------------------------------
void TStringGrid::PrepareVcl()
 { // Propriétés du TStringGrid dans le .dfm (valeurs par défaut de la VCL sinon)
  ControleVcl<QAbstractScrollArea>::PrepareVcl();
  QString Options=property("vclOptions").isValid()?property("vclOptions").toString():
                  QString("goFixedVertLine,goFixedHorzLine,goVertLine,goHorzLine,goRangeSelect");
  TraitsFixesV=Options.contains("goFixedVertLine");
  TraitsFixesH=Options.contains("goFixedHorzLine");
  TraitsV=Options.contains("goVertLine");
  TraitsH=Options.contains("goHorzLine");
  PlageSelection=Options.contains("goRangeSelect");
  QString Barres=property("vclScrollBars").isValid()?property("vclScrollBars").toString():QString("ssBoth");
  setHorizontalScrollBarPolicy(((Barres=="ssBoth")||(Barres=="ssHorizontal"))?Qt::ScrollBarAsNeeded:Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(((Barres=="ssBoth")||(Barres=="ssVertical"))?Qt::ScrollBarAsNeeded:Qt::ScrollBarAlwaysOff);
  if (property("vclBorderStyle").toString()=="bsNone") setFrameStyle(QFrame::NoFrame);
  if (property("vclGridLineWidth").isValid()) GridLineWidth=property("vclGridLineWidth").toInt();
  if (property("vclDefaultColWidth").isValid()) DefaultColWidth=property("vclDefaultColWidth").toInt();
  if (property("vclDefaultRowHeight").isValid()) DefaultRowHeight=property("vclDefaultRowHeight").toInt();
  if (property("vclFixedCols").isValid()) FixedCols=property("vclFixedCols").toInt();
  if (property("vclFixedRows").isValid()) FixedRows=property("vclFixedRows").toInt();
  QStringList l=property("vclColWidths").toStringList();
  for(int i=0; i<(int)l.size(); i++) ColWidths[i]=l[i].toInt();
  l=property("vclRowHeights").toStringList();
  for(int i=0; i<(int)l.size(); i++) RowHeights[i]=l[i].toInt();
  // Police Symbol : absente hors de Windows, le texte est traduit (TexteSymbol)
  // et affiché dans la police du système, à la même taille
  if (font().family()==QLatin1String("Symbol"))
   {
    PoliceSymbole=true;
    QFont f=font();
    f.setFamilies(QApplication::font().families());
    setFont(f);
   }
  MetAJourDefilement();
 }
//---------------------------------------------------------------------------
// Cellules et dimensions
//---------------------------------------------------------------------------
AnsiString TStringGrid::LitCellule(int c, int r) const
 {
  auto i=Donnees.find({c, r});
  return (i!=Donnees.end())?i->second:AnsiString();
 }
//---------------------------------------------------------------------------
void TStringGrid::EcritCellule(int c, int r, const AnsiString &as)
 { // TStringGrid::SetCells : mémorisée même hors de la grille, puis case redessinée
  if (as.IsEmpty()) Donnees.erase({c, r}); else Donnees[{c, r}]=as;
  TRect Rect=CellRect(c, r);
  if (Rect.Width()>0) viewport()->update(Rect.Qt());
 }
//---------------------------------------------------------------------------
int TStringGrid::LitTaille(bool Colonne, int i) const
 {
  return Colonne?Largeur(i):Hauteur(i);
 }
//---------------------------------------------------------------------------
void TStringGrid::EcritTaille(bool Colonne, int i, int Taille)
 {
  std::vector<int> &t=Colonne?Largeurs:Hauteurs;
  if ((i<0)||(i>=(int)t.size())) return;
  t[(std::size_t)i]=Taille;
  MetAJourDefilement();
  viewport()->update();
 }
//---------------------------------------------------------------------------
void TStringGrid::DefinitColonnes(int n)
 { // TCustomGrid::SetColCount puis TStringGrid::SizeChanged
  n=qMax(1, n);
  if (n==NbColonnes) return;
  NbColonnes=n;
  Largeurs.resize((std::size_t)n, LargeurDefaut);
  for(auto i=Donnees.begin(); i!=Donnees.end(); )
   if (i->first.first>=n) i=Donnees.erase(i); else ++i;
  if (NbFixesC>=n) NbFixesC=n-1;
  if (ColCour>=n) ColCour=n-1;
  if (ColAncre>=n) ColAncre=n-1;
  PremCol=qBound(NbFixesC, PremCol, n-1);
  MetAJourDefilement();
  viewport()->update();
 }
//---------------------------------------------------------------------------
void TStringGrid::DefinitLignes(int n)
 {
  n=qMax(1, n);
  if (n==NbLignes) return;
  NbLignes=n;
  Hauteurs.resize((std::size_t)n, HauteurDefaut);
  for(auto i=Donnees.begin(); i!=Donnees.end(); )
   if (i->first.second>=n) i=Donnees.erase(i); else ++i;
  if (NbFixesL>=n) NbFixesL=n-1;
  if (LigCour>=n) LigCour=n-1;
  if (LigAncre>=n) LigAncre=n-1;
  PremLig=qBound(NbFixesL, PremLig, n-1);
  MetAJourDefilement();
  viewport()->update();
 }
//---------------------------------------------------------------------------
// Géométrie : chaque case est suivie d'un trait de GridLineWidth pixels ; les
// cases fixes d'abord, puis les autres à partir de LeftCol/TopRow
//---------------------------------------------------------------------------
int TStringGrid::DebutColonne(int c) const
 {
  int x=0;
  for(int i=0; (i<c)&&(i<NbFixesC); i++) x+=Largeur(i)+Trait;
  for(int i=PremCol; i<c; i++) x+=Largeur(i)+Trait;
  return x;
 }
//---------------------------------------------------------------------------
int TStringGrid::DebutLigne(int r) const
 {
  int y=0;
  for(int i=0; (i<r)&&(i<NbFixesL); i++) y+=Hauteur(i)+Trait;
  for(int i=PremLig; i<r; i++) y+=Hauteur(i)+Trait;
  return y;
 }
//---------------------------------------------------------------------------
TRect TStringGrid::CellRect(int ACol, int ARow) const
 { // TCustomGrid::CellRect : rectangle vide si la case est hors de la vue
  if ((ACol<0)||(ACol>=NbColonnes)||(ARow<0)||(ARow>=NbLignes)) return TRect();
  if (((ACol>=NbFixesC)&&(ACol<PremCol))||((ARow>=NbFixesL)&&(ARow<PremLig))) return TRect();
  int x=DebutColonne(ACol), y=DebutLigne(ARow);
  if ((x>=viewport()->width())||(y>=viewport()->height())) return TRect();
  return TRect(x, y, x+Largeur(ACol), y+Hauteur(ARow));
 }
//---------------------------------------------------------------------------
namespace
 { // TCustomGrid::CalcMaxTopLeft (CalcMaxCell) : première case défilante telle
   // que les suivantes tiennent dans Zone (le dernier trait peut être coupé)
  int CaseMax(int Nombre, int Fixes, int Zone, int Trait, int Fixe, const std::function<int(int)> &Taille)
   {
    int Resultat=Nombre-1, Ligne=Zone+Trait;
    for(int i=Nombre-1; i>=Fixes; i--)
     {
      int e=Taille(i);
      if (e<=0) continue;
      Ligne-=e+Trait;
      if (Ligne<Fixe) break;
      Resultat=i;
     }
    return qMax(Fixes, Resultat);
   }
 }
//---------------------------------------------------------------------------
int TStringGrid::DerniereColonneGauche() const
 {
  int Fixe=0;
  for(int i=0; i<NbFixesC; i++) Fixe+=Largeur(i)+Trait;
  return CaseMax(NbColonnes, NbFixesC, viewport()->width(), Trait, Fixe, [this](int i) { return Largeur(i); });
 }
//---------------------------------------------------------------------------
int TStringGrid::DerniereLigneHaut() const
 {
  int Fixe=0;
  for(int i=0; i<NbFixesL; i++) Fixe+=Hauteur(i)+Trait;
  return CaseMax(NbLignes, NbFixesL, viewport()->height(), Trait, Fixe, [this](int i) { return Hauteur(i); });
 }
//---------------------------------------------------------------------------
void TStringGrid::MetAJourDefilement()
 { // TCustomGrid::UpdateScrollRange : barres seulement si des cases débordent
   // (la largeur d'une barre réduit la zone de l'autre, sauf barres superposées)
  QSize Max=maximumViewportSize();
  int FixeC=0, FixeL=0;
  for(int i=0; i<NbFixesC; i++) FixeC+=Largeur(i)+Trait;
  for(int i=0; i<NbFixesL; i++) FixeL+=Hauteur(i)+Trait;
  auto MaxGauche=[&](int Zone) { return CaseMax(NbColonnes, NbFixesC, Zone, Trait, FixeC, [this](int i) { return Largeur(i); }); };
  auto MaxHaut=[&](int Zone) { return CaseMax(NbLignes, NbFixesL, Zone, Trait, FixeL, [this](int i) { return Hauteur(i); }); };
  int lh=horizontalScrollBar()->sizeHint().height(), lv=verticalScrollBar()->sizeHint().width();
  if (style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, this)) lh=lv=0;
  bool AutoriseH=horizontalScrollBarPolicy()!=Qt::ScrollBarAlwaysOff,
       AutoriseV=verticalScrollBarPolicy()!=Qt::ScrollBarAlwaysOff;
  bool BarreH=AutoriseH&&(MaxGauche(Max.width())>NbFixesC), BarreV=AutoriseV&&(MaxHaut(Max.height())>NbFixesL);
  if (BarreH&&!BarreV) BarreV=AutoriseV&&(MaxHaut(Max.height()-lh)>NbFixesL);
  if (BarreV&&!BarreH) BarreH=AutoriseH&&(MaxGauche(Max.width()-lv)>NbFixesC);
  int Largeur=Max.width()-(BarreV?lv:0), Hauteur=Max.height()-(BarreH?lh:0);
  int MaxG=MaxGauche(Largeur), MaxH=MaxHaut(Hauteur);
  horizontalScrollBar()->setRange(0, MaxG-NbFixesC);
  horizontalScrollBar()->setPageStep(qMax(1, NbColonnes-MaxG));
  horizontalScrollBar()->setSingleStep(1);
  verticalScrollBar()->setRange(0, MaxH-NbFixesL);
  verticalScrollBar()->setPageStep(qMax(1, NbLignes-MaxH));
  verticalScrollBar()->setSingleStep(1);
  if ((PremCol>MaxG)||(PremLig>MaxH)) DefinitHautGauche(qMin(PremCol, MaxG), qMin(PremLig, MaxH));
  horizontalScrollBar()->setValue(PremCol-NbFixesC);
  verticalScrollBar()->setValue(PremLig-NbFixesL);
 }
//---------------------------------------------------------------------------
void TStringGrid::DefinitHautGauche(int Gauche, int Haut)
 {
  Gauche=qBound(NbFixesC, Gauche, qMax(NbFixesC, NbFixesC+horizontalScrollBar()->maximum()));
  Haut=qBound(NbFixesL, Haut, qMax(NbFixesL, NbFixesL+verticalScrollBar()->maximum()));
  if ((Gauche==PremCol)&&(Haut==PremLig)) return;
  PremCol=Gauche;
  PremLig=Haut;
  horizontalScrollBar()->setValue(Gauche-NbFixesC);
  verticalScrollBar()->setValue(Haut-NbFixesL);
  viewport()->update();
 }
//---------------------------------------------------------------------------
void TStringGrid::scrollContentsBy(int, int)
 {
  int g=NbFixesC+horizontalScrollBar()->value(), h=NbFixesL+verticalScrollBar()->value();
  if ((g!=PremCol)||(h!=PremLig))
   {
    PremCol=g;
    PremLig=h;
    viewport()->update();
   }
 }
//---------------------------------------------------------------------------
void TStringGrid::resizeEvent(QResizeEvent *e)
 {
  QAbstractScrollArea::resizeEvent(e);
  MetAJourDefilement();
 }
//---------------------------------------------------------------------------
void TStringGrid::Montre(int c, int r)
 { // TCustomGrid::ClampInView : défile pour que la case soit entièrement visible
  int g=PremCol, h=PremLig;
  if (c>=NbFixesC)
   {
    if (c<g) g=c;
    else
     {
      int x=DebutColonne(c);
      while((g<c)&&(x+Largeur(c)>viewport()->width())) { x-=Largeur(g)+Trait; g++; }
     }
   }
  if (r>=NbFixesL)
   {
    if (r<h) h=r;
    else
     {
      int y=DebutLigne(r);
      while((h<r)&&(y+Hauteur(r)>viewport()->height())) { y-=Hauteur(h)+Trait; h++; }
     }
   }
  DefinitHautGauche(g, h);
 }
//---------------------------------------------------------------------------
// Case courante et sélection
//---------------------------------------------------------------------------
void TStringGrid::Deplace(int c, int r, bool DeplaceAncre, bool Clic)
 { // TCustomGrid::MoveCurrent (puis Click pour FocusCell) ; la case courante
   // reste hors des cases fixes
  c=qBound(qMin(NbFixesC, NbColonnes-1), c, NbColonnes-1);
  r=qBound(qMin(NbFixesL, NbLignes-1), r, NbLignes-1);
  ColCour=c;
  LigCour=r;
  if (DeplaceAncre||!PlageSelection) { ColAncre=c; LigAncre=r; }
  Montre(c, r);
  viewport()->update();
  if (Clic&&OnClick) OnClick(this);
 }
//---------------------------------------------------------------------------
bool TStringGrid::CaseEn(const QPoint &p, int &c, int &r) const
 { // TCustomGrid::MouseCoord
  c=r=-1;
  for(int i=0; i<NbColonnes; i++)
   {
    if ((i>=NbFixesC)&&(i<PremCol)) continue;
    int x=DebutColonne(i);
    if ((p.x()>=x)&&(p.x()<x+Largeur(i)+Trait)) { c=i; break; }
   }
  for(int i=0; i<NbLignes; i++)
   {
    if ((i>=NbFixesL)&&(i<PremLig)) continue;
    int y=DebutLigne(i);
    if ((p.y()>=y)&&(p.y()<y+Hauteur(i)+Trait)) { r=i; break; }
   }
  return (c>=0)&&(r>=0);
 }
//---------------------------------------------------------------------------
// Dessin
//---------------------------------------------------------------------------
void TStringGrid::paintEvent(QPaintEvent *e)
 { // TCustomGrid::Paint et TStringGrid::DrawCell (DefaultDrawing)
  QPainter p(viewport());
  const QRegion &Zone=e->region();
  int Largeur=viewport()->width(), Hauteur=viewport()->height();
  QColor Fond=CouleurQt(property("vclColor").isValid()?CouleurDepuisTexte(A(property("vclColor").toString())):clWindow),
         Trace=CouleurQt(clSilver);
  p.fillRect(viewport()->rect(), Fond); // au-delà de la dernière case : Color
  // Case active de la fiche (TCustomGrid::IsActiveControl) : pas de surlignage ;
  // cadre de focus seulement si la grille a le focus
  bool Active=(window()->focusWidget()==this), Focus=hasFocus();
  int SelG=qMin(ColCour, ColAncre), SelD=qMax(ColCour, ColAncre),
      SelH=qMin(LigCour, LigAncre), SelB=qMax(LigCour, LigAncre);
  QFont Police=font();
  QFontMetrics fm(Police);
  p.setFont(Police);
  for(int r=0, y=0; (r<NbLignes)&&(y<Hauteur); r++)
   {
    if ((r>=NbFixesL)&&(r<PremLig)) continue;
    int h=this->Hauteur(r);
    for(int c=0, x=0; (c<NbColonnes)&&(x<Largeur); c++)
     {
      if ((c>=NbFixesC)&&(c<PremCol)) continue;
      int w=this->Largeur(c);
      QRect Case(x, y, w, h);
      bool Fixe=(c<NbFixesC)||(r<NbFixesL);
      if (Zone.intersects(Case.adjusted(0, 0, Trait, Trait)))
       {
        QColor Texte=palette().color(QPalette::Text);
        if (Fixe)
         { // gdsThemed : case fixe au style des en-têtes
          QStyleOptionHeader o;
          o.initFrom(this);
          o.rect=Case;
          o.orientation=(r<NbFixesL)?Qt::Horizontal:Qt::Vertical;
          o.position=QStyleOptionHeader::Middle;
          o.section=(r<NbFixesL)?c:r;
          o.state&=~(QStyle::State_HasFocus|QStyle::State_MouseOver);
          o.state|=QStyle::State_Raised;
          style()->drawControl(QStyle::CE_HeaderSection, &o, &p, this);
         }
        else
         {
          bool Selectionnee=(c>=SelG)&&(c<=SelD)&&(r>=SelH)&&(r<=SelB),
               Courante=Active&&(c==ColCour)&&(r==LigCour);
          if (Selectionnee&&!Courante)
           {
            p.fillRect(Case, palette().color(QPalette::Active, QPalette::Highlight));
            Texte=palette().color(QPalette::Active, QPalette::HighlightedText);
           }
          else
           p.fillRect(Case, Fond);
         }
        // Canvas->TextRect(Rect, Rect.Left+2, Rect.Top+2, Cells[ACol][ARow])
        auto i=Donnees.find({c, r});
        if (i!=Donnees.end())
         {
          QString s=Q(i->second);
          if (PoliceSymbole) s=TexteSymbol(s);
          p.save();
          p.setClipRect(Case);
          p.setPen(Texte);
          p.drawText(QPoint(x+2, y+2+fm.ascent()), s);
          p.restore();
         }
        if (Focus&&!Fixe&&(c==ColCour)&&(r==LigCour))
         { // DrawFocusRect
          QStyleOptionFocusRect f;
          f.initFrom(this);
          f.rect=Case;
          f.backgroundColor=Fond;
          style()->drawPrimitive(QStyle::PE_FrameFocusRect, &f, &p, this);
         }
        // Traits (goFixedVertLine/goVertLine, goFixedHorzLine/goHorzLine)
        if (Trait>0)
         {
          if (Fixe?TraitsFixesV:TraitsV) p.fillRect(QRect(x+w, y, Trait, h+Trait), Trace);
          if (Fixe?TraitsFixesH:TraitsH) p.fillRect(QRect(x, y+h, w+Trait, Trait), Trace);
         }
       }
      x+=w+Trait;
     }
    y+=h+Trait;
   }
 }
//---------------------------------------------------------------------------
void TStringGrid::focusInEvent(QFocusEvent *e)
 {
  QAbstractScrollArea::focusInEvent(e);
  viewport()->update();
 }
//---------------------------------------------------------------------------
void TStringGrid::focusOutEvent(QFocusEvent *e)
 {
  QAbstractScrollArea::focusOutEvent(e);
  viewport()->update();
 }
//---------------------------------------------------------------------------
// Souris et clavier
//---------------------------------------------------------------------------
void TStringGrid::mousePressEvent(QMouseEvent *e)
 { // TCustomGrid::MouseDown : case courante (Maj : l'ancre seulement)
  setFocus(Qt::MouseFocusReason);
  if (e->button()!=Qt::LeftButton) return;
  int c, r;
  if (!CaseEn(e->position().toPoint(), c, r)||(c<NbFixesC)||(r<NbFixesL)) return;
  SelectionEnCours=true;
  if ((e->modifiers()&Qt::ShiftModifier)&&PlageSelection)
   {
    ColAncre=c;
    LigAncre=r;
    viewport()->update();
   }
  else
   Deplace(c, r, true, false);
 }
//---------------------------------------------------------------------------
void TStringGrid::mouseMoveEvent(QMouseEvent *e)
 { // Glisser (goRangeSelect) : l'ancre suit la souris
  if (!SelectionEnCours) return;
  int c, r;
  if (!CaseEn(e->position().toPoint(), c, r)||(c<NbFixesC)||(r<NbFixesL)) return;
  if (!PlageSelection) { Deplace(c, r, true, false); return; }
  if ((c==ColAncre)&&(r==LigAncre)) return;
  ColAncre=c;
  LigAncre=r;
  viewport()->update();
 }
//---------------------------------------------------------------------------
void TStringGrid::mouseReleaseEvent(QMouseEvent *e)
 { // TCustomGrid::MouseUp : fin de sélection puis Click
  if ((e->button()!=Qt::LeftButton)||!SelectionEnCours) return;
  mouseMoveEvent(e);
  SelectionEnCours=false;
  if (OnClick) OnClick(this);
 }
//---------------------------------------------------------------------------
void TStringGrid::wheelEvent(QWheelEvent *e)
 { // TCustomGrid::DoMouseWheelDown/Up : la molette déplace la case courante
  CumulMolette+=e->angleDelta().y();
  while(CumulMolette<=-120)
   {
    CumulMolette+=120;
    if (LigCour<NbLignes-1) Deplace(ColCour, LigCour+1, true, true);
   }
  while(CumulMolette>=120)
   {
    CumulMolette-=120;
    if (LigCour>NbFixesL) Deplace(ColCour, LigCour-1, true, true);
   }
  e->accept();
 }
//---------------------------------------------------------------------------
void TStringGrid::keyPressEvent(QKeyEvent *e)
 { // TCustomGrid::KeyDown (sans goTabs ni goEditing)
  bool Ctrl=e->modifiers()&Qt::ControlModifier, Maj=e->modifiers()&Qt::ShiftModifier;
#if defined(Q_OS_MACOS)
  Ctrl=Ctrl||(e->modifiers()&Qt::MetaModifier);
#endif
  int c=ColCour, r=LigCour, Page=qMax(1, verticalScrollBar()->pageStep());
  switch(e->key())
   {
    case Qt::Key_Up: if (Ctrl) { DefinitHautGauche(PremCol, PremLig-1); e->accept(); return; } r--; break;
    case Qt::Key_Down: if (Ctrl) { DefinitHautGauche(PremCol, PremLig+1); e->accept(); return; } r++; break;
    case Qt::Key_Left: c--; break;
    case Qt::Key_Right: c++; break;
    case Qt::Key_PageUp: r-=Page; break;
    case Qt::Key_PageDown: r+=Page; break;
    case Qt::Key_Home: if (Ctrl) r=NbFixesL; c=NbFixesC; break;
    case Qt::Key_End: if (Ctrl) r=NbLignes-1; c=NbColonnes-1; break;
    default: QAbstractScrollArea::keyPressEvent(e); return;
   }
  c=qBound(qMin(NbFixesC, NbColonnes-1), c, NbColonnes-1);
  r=qBound(qMin(NbFixesL, NbLignes-1), r, NbLignes-1);
  if ((c!=ColCour)||(r!=LigCour)) Deplace(c, r, !Maj, true);
  e->accept();
 }
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// vcl/graphiques.h - Portage de Centre-Ville : TColor, TRect, TCanvas, TBitmap
//
// TCanvas imite l'API de la VCL (Brush, Pen, Font, FillRect, TextOut,
// MoveTo/LineTo, Rectangle, Ellipse...) au-dessus d'un QPainter, pour que les
// routines de dessin (DrawGridSimulDrawCell, PaintBoxStatsPaint...) soient
// portées presque ligne à ligne. Les coordonnées sont celles de la VCL : un
// TRect exclut son bord droit et son bord bas.
//---------------------------------------------------------------------------
#ifndef vcl_graphiquesH
#define vcl_graphiquesH
//---------------------------------------------------------------------------
#include <QColor>
#include <QFont>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include "vcl.h"
//---------------------------------------------------------------------------
class QPainter;
class QWidget;
//---------------------------------------------------------------------------
// TColor : 0x00BBGGRR, ou couleur système (bit 31 levé, comme la VCL)
//---------------------------------------------------------------------------
typedef int TColor;
const TColor clBlack=0x000000, clMaroon=0x000080, clGreen=0x008000, clOlive=0x008080,
             clNavy=0x800000, clPurple=0x800080, clTeal=0x808000, clGray=0x808080,
             clSilver=0xC0C0C0, clRed=0x0000FF, clLime=0x00FF00, clYellow=0x00FFFF,
             clBlue=0xFF0000, clFuchsia=0xFF00FF, clAqua=0xFFFF00, clWhite=0xFFFFFF,
             clMoneyGreen=0xC0DCC0, clSkyBlue=0xF0CAA6, clCream=0xF0FBFF, clMedGray=0xA4A0A0,
             clNone=0x1FFFFFFF, clDefault=0x20000000;
// Couleurs système (COLOR_xxx | 0xFF000000)
const TColor clScrollBar=(TColor)0xFF000000, clBackground=(TColor)0xFF000001,
             clActiveCaption=(TColor)0xFF000002, clWindow=(TColor)0xFF000005,
             clWindowText=(TColor)0xFF000008, clHighlight=(TColor)0xFF00000D,
             clHighlightText=(TColor)0xFF00000E, clBtnFace=(TColor)0xFF00000F,
             clBtnShadow=(TColor)0xFF000010, clGrayText=(TColor)0xFF000011,
             clBtnText=(TColor)0xFF000012, clBtnHighlight=(TColor)0xFF000014,
             clInfoBk=(TColor)0xFF000018;
QColor CouleurQt(TColor Couleur);
TColor CouleurVcl(const QColor &Couleur);
TColor CouleurDepuisTexte(const AnsiString &asCouleur); // "clRed", "0x00FF0080"...
//---------------------------------------------------------------------------
struct TPoint
 {
  int X=0, Y=0;
  TPoint() {}
  TPoint(int x, int y) : X(x), Y(y) {}
 };
//---------------------------------------------------------------------------
struct TRect
 {
  int Left=0, Top=0, Right=0, Bottom=0;
  TRect() {}
  TRect(int l, int t, int r, int b) : Left(l), Top(t), Right(r), Bottom(b) {}
  int Width() const { return Right-Left; }
  int Height() const { return Bottom-Top; }
  QRect Qt() const { return QRect(Left, Top, Right-Left, Bottom-Top); }
 };
inline TRect Rect(int l, int t, int r, int b) { return TRect(l, t, r, b); }
//---------------------------------------------------------------------------
enum TBrushStyle { bsSolid, bsClear };
enum TPenStyle { psSolid, psDash, psDot, psDashDot, psDashDotDot, psClear };
enum TFontStyle { fsBold, fsItalic, fsUnderline, fsStrikeOut };
typedef TEnsemble<TFontStyle> TFontStyles;
//---------------------------------------------------------------------------
struct TBrush
 {
  TColor Color=clWhite;
  TBrushStyle Style=bsSolid;
 };
//---------------------------------------------------------------------------
struct TPen
 {
  TColor Color=clBlack;
  int Width=1;
  TPenStyle Style=psSolid;
 };
//---------------------------------------------------------------------------
struct TFont
 {
  AnsiString Name="MS Sans Serif";
  int Height=-11; // < 0 : hauteur des caractères en pixels (96 ppp)
  TColor Color=clWindowText;
  TFontStyles Style;
  QFont Qt() const;
 };
//---------------------------------------------------------------------------
// Sous-objet affecté par copie, comme les propriétés objet de la VCL
// (Canvas->Font=DrawGridSimul->Font copie la police au lieu du pointeur)
//---------------------------------------------------------------------------
template <class T> class SousObjet
 {
  T Objet;
  public:
  T *operator->() { return &Objet; }
  const T *operator->() const { return &Objet; }
  operator T *() { return &Objet; }
  T &Ref() { return Objet; }
  const T &Ref() const { return Objet; }
  SousObjet &operator=(const T *t) { Objet=*t; return *this; }
  SousObjet &operator=(const T &t) { Objet=t; return *this; }
  SousObjet &operator=(const SousObjet &s) { Objet=s.Objet; return *this; }
 };
//---------------------------------------------------------------------------
// TCanvas : API VCL au-dessus d'un QPainter (fourni par le paintEvent)
//---------------------------------------------------------------------------
class TCanvas
 {
  QPainter *Peintre=nullptr;
  QPoint PenPos;
  void PrepareStylo();
  public:
  SousObjet<TBrush> Brush;
  SousObjet<TPen> Pen;
  SousObjet<TFont> Font;
  void Commence(QPainter *p) { Peintre=p; }
  void Termine() { Peintre=nullptr; }
  QPainter *Painter() const { return Peintre; }
  bool Actif() const { return Peintre!=nullptr; }
  void FillRect(const TRect &r);
  void FrameRect(const TRect &r);
  void Rectangle(int x1, int y1, int x2, int y2);
  void Rectangle(const TRect &r) { Rectangle(r.Left, r.Top, r.Right, r.Bottom); }
  void Ellipse(int x1, int y1, int x2, int y2);
  void MoveTo(int x, int y);
  void LineTo(int x, int y);
  void TextOut(int x, int y, const AnsiString &asTexte);
  void TextRect(const TRect &r, int x, int y, const AnsiString &asTexte);
  int TextWidth(const AnsiString &asTexte);
  int TextHeight(const AnsiString &asTexte);
  void Draw(int x, int y, const QPixmap &Image);
  void DrawFocusRect(const TRect &r);
 };
//---------------------------------------------------------------------------
// TBitmap (Graphics::TBitmap) : image, utilisée pour les glyphes
//---------------------------------------------------------------------------
class TBitmap
 {
  public:
  QPixmap Image;
 };
//---------------------------------------------------------------------------
#endif

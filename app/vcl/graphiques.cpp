//---------------------------------------------------------------------------
// vcl/graphiques.cpp - Portage de Centre-Ville : TColor, TCanvas
//---------------------------------------------------------------------------
#include "graphiques.h"
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <map>
#include <string>
//---------------------------------------------------------------------------
QColor CouleurQt(TColor Couleur)
 {
  if ((unsigned)Couleur&0x80000000u)
   { // Couleur système : palette de Qt (équivalents des COLOR_xxx de Windows)
    QPalette p=QApplication::palette();
    switch((unsigned)Couleur&0xFFu)
     {
      case 0x05: return p.color(QPalette::Base); // clWindow
      case 0x08: return p.color(QPalette::Text); // clWindowText
      case 0x0D: return p.color(QPalette::Active, QPalette::Highlight); // clHighlight
      case 0x0E: return p.color(QPalette::Active, QPalette::HighlightedText); // clHighlightText
      case 0x0F: return p.color(QPalette::Button); // clBtnFace
      case 0x10: return p.color(QPalette::Dark); // clBtnShadow
      case 0x11: return p.color(QPalette::Disabled, QPalette::Text); // clGrayText
      case 0x12: return p.color(QPalette::ButtonText); // clBtnText
      case 0x14: return p.color(QPalette::Light); // clBtnHighlight
      case 0x18: return p.color(QPalette::ToolTipBase); // clInfoBk
      default: return p.color(QPalette::Window);
     }
   }
  return QColor(Couleur&0xFF, (Couleur>>8)&0xFF, (Couleur>>16)&0xFF);
 }
//---------------------------------------------------------------------------
TColor CouleurVcl(const QColor &Couleur)
 {
  return (TColor)(Couleur.red()|(Couleur.green()<<8)|(Couleur.blue()<<16));
 }
//---------------------------------------------------------------------------
TColor CouleurDepuisTexte(const AnsiString &asCouleur)
 {
  static const std::map<std::string, TColor> Noms=
   {{"clBlack", clBlack}, {"clMaroon", clMaroon}, {"clGreen", clGreen}, {"clOlive", clOlive},
    {"clNavy", clNavy}, {"clPurple", clPurple}, {"clTeal", clTeal}, {"clGray", clGray},
    {"clSilver", clSilver}, {"clRed", clRed}, {"clLime", clLime}, {"clYellow", clYellow},
    {"clBlue", clBlue}, {"clFuchsia", clFuchsia}, {"clAqua", clAqua}, {"clWhite", clWhite},
    {"clMoneyGreen", clMoneyGreen}, {"clSkyBlue", clSkyBlue}, {"clCream", clCream},
    {"clMedGray", clMedGray}, {"clNone", clNone}, {"clDefault", clDefault},
    {"clWindow", clWindow}, {"clWindowText", clWindowText}, {"clHighlight", clHighlight},
    {"clHighlightText", clHighlightText}, {"clBtnFace", clBtnFace}, {"clBtnShadow", clBtnShadow},
    {"clGrayText", clGrayText}, {"clBtnText", clBtnText}, {"clBtnHighlight", clBtnHighlight},
    {"clInfoBk", clInfoBk}};
  auto i=Noms.find(asCouleur.str());
  if (i!=Noms.end()) return i->second;
  return (TColor)std::stoul(asCouleur.str(), nullptr, 0);
 }
//---------------------------------------------------------------------------
// MS Sans Serif et Small Fonts, polices « raster » des fiches, ne sont pas
// dessinées par Qt, même sous Windows où il les trouve (il prend alors une
// police de secours plus large : libellés coupés). Leurs équivalents
// TrueType passent devant : Microsoft Sans Serif (aux mêmes mesures, fournie
// par Windows), Verdana ou Tahoma ; absents, les substitutions de
// InitialiseApplication (application.cpp) s'appliquent.
//---------------------------------------------------------------------------
QStringList FamillesQt(const QString &Famille)
 {
  if (Famille==QLatin1String("MS Sans Serif")) return {"Microsoft Sans Serif", Famille};
  if (Famille==QLatin1String("Small Fonts")) return {"Verdana", "Tahoma", Famille};
  return {Famille};
 }
//---------------------------------------------------------------------------
QFont TFont::Qt() const
 {
  QFont f;
  f.setFamilies(FamillesQt(Q(Name)));
  if (Height<0)
   f.setPixelSize(-Height); // Font.Height négatif = hauteur des caractères en pixels
  else
   if (Height>0)
    f.setPixelSize(qMax(1, (int)(0.85*Height+0.5))); // hauteur de cellule
  f.setBold(Style.Contains(fsBold));
  f.setItalic(Style.Contains(fsItalic));
  f.setUnderline(Style.Contains(fsUnderline));
  f.setStrikeOut(Style.Contains(fsStrikeOut));
  return f;
 }
//---------------------------------------------------------------------------
// TCanvas
//---------------------------------------------------------------------------
void TCanvas::PrepareStylo()
 {
  if (Pen->Style==psClear)
   Peintre->setPen(::Qt::NoPen);
  else
   {
    QPen p(CouleurQt(Pen->Color));
    p.setWidth(Pen->Width);
    switch(Pen->Style)
     {
      case psDash: p.setStyle(::Qt::DashLine); break;
      case psDot: p.setStyle(::Qt::DotLine); break;
      case psDashDot: p.setStyle(::Qt::DashDotLine); break;
      case psDashDotDot: p.setStyle(::Qt::DashDotDotLine); break;
      default: p.setStyle(::Qt::SolidLine); break;
     }
    Peintre->setPen(p);
   }
  if (Brush->Style==bsClear)
   Peintre->setBrush(::Qt::NoBrush);
  else
   Peintre->setBrush(CouleurQt(Brush->Color));
 }
//---------------------------------------------------------------------------
int TCanvas::NbDessinsEnCours=0;
//---------------------------------------------------------------------------
void TCanvas::FillRect(const TRect &r)
 {
  if (!Peintre||(Brush->Style==bsClear)) return;
  Peintre->fillRect(r.Qt(), CouleurQt(Brush->Color));
 }
//---------------------------------------------------------------------------
void TCanvas::FrameRect(const TRect &r)
 {
  if (!Peintre) return;
  QColor c=CouleurQt(Brush->Color);
  Peintre->fillRect(QRect(r.Left, r.Top, r.Width(), 1), c);
  Peintre->fillRect(QRect(r.Left, r.Bottom-1, r.Width(), 1), c);
  Peintre->fillRect(QRect(r.Left, r.Top, 1, r.Height()), c);
  Peintre->fillRect(QRect(r.Right-1, r.Top, 1, r.Height()), c);
 }
//---------------------------------------------------------------------------
void TCanvas::Rectangle(int x1, int y1, int x2, int y2)
 { // GDI : contour au crayon, intérieur à la brosse, bords droit et bas exclus
  if (!Peintre) return;
  PrepareStylo();
  Peintre->drawRect(QRect(x1, y1, x2-x1-1, y2-y1-1));
 }
//---------------------------------------------------------------------------
void TCanvas::Ellipse(int x1, int y1, int x2, int y2)
 {
  if (!Peintre) return;
  PrepareStylo();
  Peintre->drawEllipse(QRect(x1, y1, x2-x1-1, y2-y1-1));
 }
//---------------------------------------------------------------------------
void TCanvas::MoveTo(int x, int y)
 {
  PenPos=QPoint(x, y);
 }
//---------------------------------------------------------------------------
void TCanvas::LineTo(int x, int y)
 { // GDI : le point d'arrivée n'est pas tracé
  if (Peintre)
   {
    PrepareStylo();
    QPoint Fin(x, y), d=Fin-PenPos;
    if (d.x()||d.y())
     {
      QPoint Avant=Fin-QPoint((d.x()>0)-(d.x()<0), (d.y()>0)-(d.y()<0));
      if (Avant==PenPos)
       Peintre->drawPoint(PenPos);
      else
       Peintre->drawLine(PenPos, Avant);
     }
   }
  PenPos=QPoint(x, y);
 }
//---------------------------------------------------------------------------
void TCanvas::TextOut(int x, int y, const AnsiString &asTexte)
 { // Fond opaque à la couleur de la brosse, sauf brosse bsClear (SetBkMode)
  if (!Peintre) return;
  QFont f=Font->Qt();
  QFontMetrics m(f);
  QString t=Q(asTexte);
  if (Brush->Style!=bsClear)
   Peintre->fillRect(QRect(x, y, m.horizontalAdvance(t), m.height()), CouleurQt(Brush->Color));
  Peintre->setFont(f);
  Peintre->setPen(CouleurQt(Font->Color));
  Peintre->drawText(QPoint(x, y+m.ascent()), t);
 }
//---------------------------------------------------------------------------
void TCanvas::TextRect(const TRect &r, int x, int y, const AnsiString &asTexte)
 {
  if (!Peintre) return;
  Peintre->save();
  Peintre->setClipRect(r.Qt(), ::Qt::IntersectClip);
  FillRect(r);
  TBrushStyle Style=Brush->Style;
  Brush->Style=bsClear;
  TextOut(x, y, asTexte);
  Brush->Style=Style;
  Peintre->restore();
 }
//---------------------------------------------------------------------------
int TCanvas::TextWidth(const AnsiString &asTexte)
 {
  return QFontMetrics(Font->Qt()).horizontalAdvance(Q(asTexte));
 }
//---------------------------------------------------------------------------
int TCanvas::TextHeight(const AnsiString &asTexte)
 {
  (void)asTexte;
  return QFontMetrics(Font->Qt()).height();
 }
//---------------------------------------------------------------------------
void TCanvas::Draw(int x, int y, const QPixmap &Image)
 {
  if (Peintre) Peintre->drawPixmap(QPoint(x, y), Image);
 }
//---------------------------------------------------------------------------
void TCanvas::DrawFocusRect(const TRect &r)
 { // DrawFocusRect de Windows : pointillés en OU exclusif
  if (!Peintre) return;
  Peintre->save();
  Peintre->setCompositionMode(QPainter::RasterOp_NotDestination);
  QPen p(::Qt::white, 1, ::Qt::DotLine);
  p.setCosmetic(true);
  Peintre->setPen(p);
  Peintre->setBrush(::Qt::NoBrush);
  Peintre->drawRect(QRect(r.Left, r.Top, r.Width()-1, r.Height()-1));
  Peintre->restore();
 }
//---------------------------------------------------------------------------

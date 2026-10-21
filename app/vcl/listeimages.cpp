//---------------------------------------------------------------------------
// vcl/listeimages.cpp - Portage de Centre-Ville : TVirtualImageList
//---------------------------------------------------------------------------
#include "listeimages.h"
#include <QPainter>
#include <QtMath>
//---------------------------------------------------------------------------
QPixmap AgranditSansLissage(const QPixmap &Image, qreal Echelle)
 {
  if (Image.isNull()||(Echelle==1.0)) return Image;
  QPixmap p=Image.scaled(qRound(Image.width()*Echelle), qRound(Image.height()*Echelle),
                         Qt::IgnoreAspectRatio, Qt::FastTransformation);
  p.setDevicePixelRatio(Echelle);
  return p;
 }
//---------------------------------------------------------------------------
TVirtualImageList::TVirtualImageList(const char *const *Ressources, int NbImages, QObject *Parent) :
  QObject(Parent)
 {
  Images.reserve((std::size_t)NbImages);
  for(int i=0; i<NbImages; i++)
   Images.push_back(QPixmap(QString::fromUtf8(Ressources[i])));
  if (!Images.empty())
   {
    Width=Images[0].width();
    Height=Images[0].height();
   }
 }
//---------------------------------------------------------------------------
const QPixmap &TVirtualImageList::Image(int Index, qreal Echelle) const
 {
  int Cle=qRound(Echelle*100);
  if (Cle==100) return Images[(std::size_t)Index];
  std::vector<QPixmap> &v=Agrandies[Cle];
  if (v.empty())
   {
    v.reserve(Images.size());
    for(const QPixmap &p : Images) v.push_back(AgranditSansLissage(p, Echelle));
   }
  return v[(std::size_t)Index];
 }
//---------------------------------------------------------------------------
void TVirtualImageList::Draw(TCanvas *Canvas, int X, int Y, int Index, bool Enabled)
 { // Un indice hors liste ne dessine rien (TCustomImageList.Draw)
  if ((Index<0)||(Index>=Count())||!Canvas->Actif()) return;
  QPainter *p=Canvas->Painter();
  const QPixmap &Img=Image(Index, p->device()->devicePixelRatioF());
  if (Enabled)
   p->drawPixmap(QPoint(X, Y), Img);
  else
   {
    p->save();
    p->setOpacity(0.5);
    p->drawPixmap(QPoint(X, Y), Img);
    p->restore();
   }
 }
//---------------------------------------------------------------------------
void TVirtualImageList::GetBitmap(int Index, TBitmap *Bitmap) const
 {
  if ((Index>=0)&&(Index<Count())&&Bitmap)
   {
    Bitmap->Image=Images[(std::size_t)Index];
    Bitmap->Changed();
   }
 }
//---------------------------------------------------------------------------
QPixmap TVirtualImageList::Pixmap(int Index) const
 {
  if ((Index<0)||(Index>=Count())) return QPixmap();
  return Images[(std::size_t)Index];
 }
//---------------------------------------------------------------------------
QIcon TVirtualImageList::Icone(int Index) const
 { // Icônes des menus et barres d'outils : versions agrandies sans lissage
  QIcon i;
  if ((Index<0)||(Index>=Count())) return i;
  const QPixmap &p=Images[(std::size_t)Index];
  i.addPixmap(p);
  for(qreal e : {1.25, 1.5, 2.0, 3.0}) i.addPixmap(AgranditSansLissage(p, e));
  return i;
 }
//---------------------------------------------------------------------------

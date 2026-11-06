//---------------------------------------------------------------------------
// vcl/listeimages.cpp - Portage de Centre-Ville : TVirtualImageList
//---------------------------------------------------------------------------
#include "listeimages.h"
#include <QPainter>
#include <QtMath>
//---------------------------------------------------------------------------
QPixmap AgranditSansLissage(const QPixmap &Image, qreal Echelle)
 {
  if (Image.isNull()||(Echelle==1.0)) { return Image;
}
  QPixmap p=Image.scaled(qRound(Image.width()*Echelle), qRound(Image.height()*Echelle),
                         Qt::IgnoreAspectRatio, Qt::FastTransformation);
  p.setDevicePixelRatio(Echelle);
  return p;
 }
//---------------------------------------------------------------------------
TVirtualImageList::TVirtualImageList(const char *const *Ressources, int NbImages, QObject *Parent) :
  QObject(Parent)
 {
  Images.reserve(static_cast<std::size_t>(NbImages));
  for(int i=0; i<NbImages; i++) {
   Images.emplace_back(QString::fromUtf8(Ressources[i]));
}
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
  if (Cle==100) { return Images[static_cast<std::size_t>(Index)];
}
  std::vector<QPixmap> &v=Agrandies[Cle];
  if (v.empty())
   {
    v.reserve(Images.size());
    for(const QPixmap &p : Images) { v.push_back(AgranditSansLissage(p, Echelle));
}
   }
  return v[static_cast<std::size_t>(Index)];
 }
//---------------------------------------------------------------------------
void TVirtualImageList::Draw(TCanvas *Canvas, int X, int Y, int Index, bool Enabled)
 { // Un indice hors liste ne dessine rien (TCustomImageList.Draw)
  if ((Index<0)||(Index>=Count())||!Canvas->Actif()) { return;
}
  QPainter *p=Canvas->Painter();
  const QPixmap &Img=Image(Index, p->device()->devicePixelRatioF());
  if (Enabled) {
   p->drawPixmap(QPoint(X, Y), Img);
  } else
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
    Bitmap->Image=Images[static_cast<std::size_t>(Index)];
    Bitmap->Changed();
   }
 }
//---------------------------------------------------------------------------
QPixmap TVirtualImageList::Pixmap(int Index) const
 {
  if ((Index<0)||(Index>=Count())) { return {};
}
  return Images[static_cast<std::size_t>(Index)];
 }
//---------------------------------------------------------------------------

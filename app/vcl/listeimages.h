//---------------------------------------------------------------------------
// vcl/listeimages.h - Portage de Centre-Ville : TVirtualImageList
//
// Les TVirtualImageList de la VCL (v5.4) puisent dans une TImageCollection
// dont les PNG étaient embarqués dans les .dfm. Ils sont extraits par
// tools/extrait_images.py dans app/ressources/images.qrc, et
// app/ressources/index_images.h donne pour chaque liste la table indice ->
// ressource : les indices (IDB_* de f_simul.cpp, ImageIndex des actions)
// restent ceux de l'original.
//
// Haute densité (HiDPI) : chaque image est agrandie une fois pour toutes au
// facteur d'échelle de l'écran SANS lissage (plus proche voisin), pour garder
// le pixel-art net à 150 % ou 200 %.
//---------------------------------------------------------------------------
#ifndef vcl_listeimagesH
#define vcl_listeimagesH
//---------------------------------------------------------------------------
#include <QIcon>
#include <QPixmap>
#include <map>
#include <vector>
#include "graphiques.h"
//---------------------------------------------------------------------------
class TImageCollection : public QObject {}; // Images dans app/ressources (rien à l'exécution)
//---------------------------------------------------------------------------
class TVirtualImageList : public QObject
 {
  std::vector<QPixmap> Images;
  mutable std::map<int, std::vector<QPixmap>> Agrandies; // par facteur d'échelle (x100)
  const QPixmap &Image(int Index, qreal Echelle) const;
  public:
  int Width=16, Height=16;
  TVirtualImageList(const char *const *Ressources, int NbImages, QObject *Parent=nullptr);
  int Count() const { return (int)Images.size(); }
  void Draw(TCanvas *Canvas, int X, int Y, int Index, bool Enabled=true);
  void GetBitmap(int Index, TBitmap *Bitmap) const;
  QPixmap Pixmap(int Index) const;
  QIcon Icone(int Index) const;
 };
//---------------------------------------------------------------------------
// Agrandissement au plus proche voisin, pour un facteur d'échelle d'écran
QPixmap AgranditSansLissage(const QPixmap &Image, qreal Echelle);
//---------------------------------------------------------------------------
#endif

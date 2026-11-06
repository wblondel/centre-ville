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
using TColor = int;
const TColor clBlack = 0x000000, clMaroon = 0x000080, clGreen = 0x008000, clOlive = 0x008080, clNavy = 0x800000,
             clPurple = 0x800080, clTeal = 0x808000, clGray = 0x808080, clSilver = 0xC0C0C0, clRed = 0x0000FF,
             clLime = 0x00FF00, clYellow = 0x00FFFF, clBlue = 0xFF0000, clFuchsia = 0xFF00FF, clAqua = 0xFFFF00,
             clWhite = 0xFFFFFF, clMoneyGreen = 0xC0DCC0, clSkyBlue = 0xF0CAA6, clCream = 0xF0FBFF,
             clMedGray = 0xA4A0A0, clNone = 0x1FFFFFFF, clDefault = 0x20000000;
// Couleurs système (COLOR_xxx | 0xFF000000)
const TColor clScrollBar = static_cast<TColor>(0xFF000000), clBackground = static_cast<TColor>(0xFF000001),
             clActiveCaption = static_cast<TColor>(0xFF000002), clWindow = static_cast<TColor>(0xFF000005),
             clWindowText = static_cast<TColor>(0xFF000008), clHighlight = static_cast<TColor>(0xFF00000D),
             clHighlightText = static_cast<TColor>(0xFF00000E), clBtnFace = static_cast<TColor>(0xFF00000F),
             clBtnShadow = static_cast<TColor>(0xFF000010), clGrayText = static_cast<TColor>(0xFF000011),
             clBtnText = static_cast<TColor>(0xFF000012), clBtnHighlight = static_cast<TColor>(0xFF000014),
             clInfoBk = static_cast<TColor>(0xFF000018);
QColor CouleurQt(TColor Couleur);
TColor CouleurDepuisTexte(const AnsiString &asCouleur); // "clRed", "0x00FF0080"...
// RGB de windows.h : 0x00BBGGRR (statistiques de fluidité et de trafic)
#ifndef RGB // macro de windows.h, si l'en-tête a déjà été inclus
inline int RGB(int r, int g, int b)
{
    return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
}
#endif
//---------------------------------------------------------------------------
struct TRect
{
    int Left = 0, Top = 0, Right = 0, Bottom = 0;
    TRect() = default;
    TRect(int l, int t, int r, int b) : Left(l), Top(t), Right(r), Bottom(b) {}
    int Width() const { return Right - Left; }
    int Height() const { return Bottom - Top; }
    QRect Qt() const { return {Left, Top, Right - Left, Bottom - Top}; }
};
//---------------------------------------------------------------------------
enum TBrushStyle { bsSolid, bsClear };
enum TPenStyle { psSolid, psDash, psDot, psDashDot, psDashDotDot, psClear };
enum TPenMode { pmBlack, pmWhite, pmNop, pmNot, pmCopy }; // seul pmCopy (le défaut) est dessiné
enum TFontStyle { fsBold, fsItalic, fsUnderline, fsStrikeOut };
using TFontStyles = TEnsemble<TFontStyle>;
//---------------------------------------------------------------------------
struct TBrush
{
    TColor Color = clWhite;
    TBrushStyle Style = bsSolid;
};
//---------------------------------------------------------------------------
struct TPen
{
    TColor Color = clBlack;
    int Width = 1;
    TPenStyle Style = psSolid;
    TPenMode Mode = pmCopy;
};
//---------------------------------------------------------------------------
// Familles Qt d'une police de la VCL : les polices « raster » de Windows
// (MS Sans Serif, Small Fonts) ont leurs équivalents TrueType devant
QStringList FamillesQt(const QString &Famille);
//---------------------------------------------------------------------------
struct TFont
{
    AnsiString Name = "MS Sans Serif";
    int Height = -11; // < 0 : hauteur des caractères en pixels (96 ppp)
    TColor Color = clWindowText;
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
    void Acces()
    {
        if (SurAcces)
        {
            SurAcces();
        }
    }

public:
    // Appelé à chaque accès qui peut modifier l'objet (->, affectation) : le
    // contrôle propriétaire s'y fait repeindre, comme sur TBrush::OnChange
    // (TShape->Brush->Color=... ; update() est différé, donc après la modification)
    std::function<void()> SurAcces;
    T *operator->()
    {
        Acces();
        return &Objet;
    }
    const T *operator->() const { return &Objet; }
    operator T *()
    {
        Acces();
        return &Objet;
    }
    T &Ref() { return Objet; }
    const T &Ref() const { return Objet; }
    SousObjet &operator=(const T *t)
    {
        Objet = *t;
        Acces();
        return *this;
    }
    SousObjet &operator=(const T &t)
    {
        Objet = t;
        Acces();
        return *this;
    }
    SousObjet &operator=(const SousObjet &s)
    {
        Objet = s.Objet;
        Acces();
        return *this;
    }
};
//---------------------------------------------------------------------------
// TCanvas : API VCL au-dessus d'un QPainter (fourni par le paintEvent)
//---------------------------------------------------------------------------
class TCanvas
{
    QPainter *Peintre = nullptr;
    QPoint PenPos;
    void PrepareStylo();

public:
    SousObjet<TBrush> Brush;
    SousObjet<TPen> Pen;
    SousObjet<TFont> Font;
    // Dessins en cours (paintEvent des contrôles portés) : Qt interdit d'en
    // imbriquer un autre (Repaint pendant OnDrawCell devient alors update)
    static int NbDessinsEnCours;
    void Commence(QPainter *p)
    {
        if (!Peintre && p)
        {
            NbDessinsEnCours++;
        }
        Peintre = p;
    }
    void Termine()
    {
        if (Peintre)
        {
            NbDessinsEnCours--;
        }
        Peintre = nullptr;
    }
    QPainter *Painter() const { return Peintre; }
    bool Actif() const { return Peintre != nullptr; }
    void FillRect(const TRect &r);
    void Rectangle(int x1, int y1, int x2, int y2);
    void Ellipse(int x1, int y1, int x2, int y2);
    void MoveTo(int x, int y);
    void LineTo(int x, int y);
    void TextOut(int x, int y, const AnsiString &asTexte);
    int TextWidth(const AnsiString &asTexte);
    int TextHeight(const AnsiString &asTexte);
    void DrawFocusRect(const TRect &r);
};
//---------------------------------------------------------------------------
// TBitmap (Graphics::TBitmap) : image, utilisée pour les glyphes. Comme dans
// la VCL, une modification (Assign, GetBitmap d'une liste d'images) déclenche
// OnChange : le bouton dont c'est le Glyph se redessine (AffecteGlyphe).
// La copie ne transmet que l'image, pas OnChange (TBitmap::Assign).
//---------------------------------------------------------------------------
class TBitmap
{
public:
    QPixmap Image;
    std::function<void()> OnChange;
    TBitmap() = default;
    TBitmap(const TBitmap &b) : Image(b.Image) {}
    TBitmap &operator=(const TBitmap &b)
    {
        Image = b.Image;
        Changed();
        return *this;
    }
    void Changed() const
    {
        if (OnChange)
        {
            OnChange();
        }
    }
};
//---------------------------------------------------------------------------
#endif

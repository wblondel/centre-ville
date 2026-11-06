//---------------------------------------------------------------------------
// grille/GrilleSimulation.cpp - Portage de Centre-Ville : carte de simulation
// (comportement de TCustomGrid / TCustomDrawGrid de la VCL)
//---------------------------------------------------------------------------
#include "GrilleSimulation.h"
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QWheelEvent>
//---------------------------------------------------------------------------
GrilleSimulation::GrilleSimulation(QWidget *Parent) : ControleVcl<QAbstractScrollArea>(Parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    viewport()->setMouseTracking(false);
    Options << goRangeSelect;
    Font->Name = "Small Fonts";
    Font->Height = -7;
    ColCount.Definit([this] { return NbColonnes; },
                     [this](const int &n) { // TCustomGrid::SetColCount : la case courante reste dans la grille
                         NbColonnes = qMax(1, n);
                         if (Courante.X >= NbColonnes)
                         {
                             MoveCurrent(NbColonnes - 1, Courante.Y, true);
                         }
                         if (Ancre.X >= NbColonnes)
                         {
                             Ancre.X = NbColonnes - 1;
                         }
                         MetAJourSelection();
                         MetAJourDefilement();
                         viewport()->update();
                     });
    RowCount.Definit([this] { return NbLignes; },
                     [this](const int &n) {
                         NbLignes = qMax(1, n);
                         if (Courante.Y >= NbLignes)
                         {
                             MoveCurrent(Courante.X, NbLignes - 1, true);
                         }
                         if (Ancre.Y >= NbLignes)
                         {
                             Ancre.Y = NbLignes - 1;
                         }
                         MetAJourSelection();
                         MetAJourDefilement();
                         viewport()->update();
                     });
    Col.Definit([this] { return Courante.X; },
                [this](const int &c) {
                    if (c != Courante.X)
                    {
                        FocusCell(c, Courante.Y, true);
                    }
                });
    Row.Definit([this] { return Courante.Y; },
                [this](const int &r) {
                    if (r != Courante.Y)
                    {
                        FocusCell(Courante.X, r, true);
                    }
                });
    LeftCol.Definit([this] { return HautGauche.X; }, [this](const int &c) { DefinitHautGauche(c, HautGauche.Y); });
    TopRow.Definit([this] { return HautGauche.Y; }, [this](const int &r) { DefinitHautGauche(HautGauche.X, r); });
    DefilementAuto.setInterval(60);
    QObject::connect(&DefilementAuto, &QTimer::timeout, this, [this] { // Sélection à la souris hors des cases
                                                                       // entièrement visibles : l'ancre avance d'une
                                                                       // case par tic de la minuterie de 60 ms, et la
                                                                       // vue la suit (TCustomGrid::WMTimer et
                                                                       // TimedScroll). v6.0.12 : la vue avançait d'une
                                                                       // case, puis l'ancre allait sous la souris
                                                                       // (plusieurs cases par tic, selon la distance)
        if (!SelectionEnCours)
        {
            return;
        }
        const QPoint p = DernierePosition;
        TGridCoord a = Ancre;
        if (p.x() < 0)
        {
            if (a.X > 0)
            {
                a.X--;
            }
        }
        else if ((p.x() > LimiteVisible(true)) && (a.X < NbColonnes - 1))
        {
            a.X++;
        }
        if (p.y() < 0)
        {
            if (a.Y > 0)
            {
                a.Y--;
            }
        }
        else if ((p.y() > LimiteVisible(false)) && (a.Y < NbLignes - 1))
        {
            a.Y++;
        }
        if ((a.X != Ancre.X) || (a.Y != Ancre.Y))
        {
            MoveAnchor(a);
        }
    });
    MetAJourSelection();
}
//---------------------------------------------------------------------------
void GrilleSimulation::PrepareVcl()
{ // Propriétés du TDrawGrid dans le .dfm
    ControleVcl<QAbstractScrollArea>::PrepareVcl();
    if (property("vclDefaultColWidth").isValid())
    {
        DefaultColWidth = property("vclDefaultColWidth").toInt();
    }
    if (property("vclDefaultRowHeight").isValid())
    {
        DefaultRowHeight = property("vclDefaultRowHeight").toInt();
    }
    if (property("vclColor").isValid())
    {
        Color = CouleurDepuisTexte(A(property("vclColor").toString()));
    }
    if (property("vclFontName").isValid())
    {
        Font->Name = A(property("vclFontName").toString());
    }
    if (property("vclFontHeight").isValid())
    {
        Font->Height = property("vclFontHeight").toInt();
    }
    if (property("vclOptions").isValid())
    {
        Options = TGridOptions();
        if (property("vclOptions").toString().contains("goRangeSelect"))
        {
            Options << goRangeSelect;
        }
    }
    if (property("vclColCount").isValid())
    {
        ColCount = property("vclColCount").toInt();
    }
    if (property("vclRowCount").isValid())
    {
        RowCount = property("vclRowCount").toInt();
    }
}
//---------------------------------------------------------------------------
// Géométrie
//---------------------------------------------------------------------------
int GrilleSimulation::ColonnesVisibles() const
{
    return qMax(1, viewport()->width() / DefaultColWidth);
}
//---------------------------------------------------------------------------
int GrilleSimulation::LignesVisibles() const
{
    return qMax(1, viewport()->height() / DefaultRowHeight);
}
//---------------------------------------------------------------------------
int GrilleSimulation::DerniereVisible(bool Colonne) const
{ // v6.0.12 : TGridAxisDrawInfo::LastFullVisibleCell
    return Colonne ? qMin(NbColonnes - 1, HautGauche.X + ColonnesVisibles() - 1)
                   : qMin(NbLignes - 1, HautGauche.Y + LignesVisibles() - 1);
}
//---------------------------------------------------------------------------
int GrilleSimulation::LimiteVisible(bool Colonne) const
{ // v6.0.12 : TGridAxisDrawInfo::FullVisBoundary, bord de la dernière case entièrement visible
    return Colonne ? (DerniereVisible(true) - HautGauche.X + 1) * DefaultColWidth
                   : (DerniereVisible(false) - HautGauche.Y + 1) * DefaultRowHeight;
}
//---------------------------------------------------------------------------
TRect GrilleSimulation::CellRect(int ACol, int ARow) const
{ // TCustomGrid::CellRect : rectangle dans le contrôle (bords droit et bas exclus)
    const int x = (ACol - HautGauche.X) * DefaultColWidth;
    const int y = (ARow - HautGauche.Y) * DefaultRowHeight;
    return {x, y, x + DefaultColWidth, y + DefaultRowHeight};
}
//---------------------------------------------------------------------------
QRect GrilleSimulation::RectGrille(const TGridRect &r) const
{
    const TRect a = CellRect(r.Left, r.Top);
    const TRect b = CellRect(r.Right, r.Bottom);
    return {a.Left, a.Top, b.Right - a.Left, b.Bottom - a.Top};
}
//---------------------------------------------------------------------------
TGridCoord GrilleSimulation::CaseEn(const QPoint &p) const
{ // Case sous le point, (-1, -1) hors de la grille. v6.0.12 : sans la variante bornée de
  // l'ancienne minuterie de défilement
    TGridCoord c;
    c.X = HautGauche.X + (p.x() >= 0 ? p.x() / DefaultColWidth : -1);
    c.Y = HautGauche.Y + (p.y() >= 0 ? p.y() / DefaultRowHeight : -1);
    if ((c.X < 0) || (c.X >= NbColonnes) || (c.Y < 0) || (c.Y >= NbLignes))
    {
        c.X = c.Y = -1;
    }
    return c;
}
//---------------------------------------------------------------------------
void GrilleSimulation::MetAJourDefilement()
{ // Défilement par cases entières ; barres seulement si la grille déborde
    const QSize Max = maximumViewportSize();
    const int w = NbColonnes * DefaultColWidth;
    const int h = NbLignes * DefaultRowHeight;
    bool BarreH = w > Max.width();
    bool BarreV = h > Max.height();
    int lh = horizontalScrollBar()->sizeHint().height();
    int lv = verticalScrollBar()->sizeHint().width();
    if (style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, this) != 0)
    {
        lh = lv = 0;
    }
    if (BarreH && !BarreV)
    {
        BarreV = h > Max.height() - lh;
    }
    if (BarreV && !BarreH)
    {
        BarreH = w > Max.width() - lv;
    }
    const int Largeur = Max.width() - (BarreV ? lv : 0);
    const int Hauteur = Max.height() - (BarreH ? lh : 0);
    const int MaxGauche = BarreH ? qMax(0, NbColonnes - qMax(1, Largeur / DefaultColWidth)) : 0;
    const int MaxHaut = BarreV ? qMax(0, NbLignes - qMax(1, Hauteur / DefaultRowHeight)) : 0;
    horizontalScrollBar()->setRange(0, MaxGauche);
    horizontalScrollBar()->setPageStep(qMax(1, Largeur / DefaultColWidth));
    horizontalScrollBar()->setSingleStep(1);
    verticalScrollBar()->setRange(0, MaxHaut);
    verticalScrollBar()->setPageStep(qMax(1, Hauteur / DefaultRowHeight));
    verticalScrollBar()->setSingleStep(1);
    if ((HautGauche.X > MaxGauche) || (HautGauche.Y > MaxHaut))
    {
        DefinitHautGauche(qMin(HautGauche.X, MaxGauche), qMin(HautGauche.Y, MaxHaut));
    }
}
//---------------------------------------------------------------------------
void GrilleSimulation::DefinitHautGauche(int Gauche, int Haut)
{
    Gauche = qBound(0, Gauche, horizontalScrollBar()->maximum());
    Haut = qBound(0, Haut, verticalScrollBar()->maximum());
    if ((Gauche == HautGauche.X) && (Haut == HautGauche.Y))
    {
        return;
    }
    HautGauche.X = Gauche;
    HautGauche.Y = Haut;
    horizontalScrollBar()->setValue(Gauche);
    verticalScrollBar()->setValue(Haut);
    viewport()->update();
}
//---------------------------------------------------------------------------
void GrilleSimulation::scrollContentsBy(int /*dx*/, int /*dy*/)
{
    const int g = horizontalScrollBar()->value();
    const int h = verticalScrollBar()->value();
    if ((g != HautGauche.X) || (h != HautGauche.Y))
    {
        HautGauche.X = g;
        HautGauche.Y = h;
        viewport()->update();
    }
}
//---------------------------------------------------------------------------
void GrilleSimulation::resizeEvent(QResizeEvent *e)
{
    QAbstractScrollArea::resizeEvent(e);
    MetAJourDefilement();
}
//---------------------------------------------------------------------------
void GrilleSimulation::ClampInView(TGridCoord c)
{ // Fait défiler pour que la case soit entièrement visible
    int g = HautGauche.X;
    int h = HautGauche.Y;
    if (c.X < g)
    {
        g = c.X;
    }
    else if (c.X >= g + ColonnesVisibles())
    {
        g = c.X - ColonnesVisibles() + 1;
    }
    if (c.Y < h)
    {
        h = c.Y;
    }
    else if (c.Y >= h + LignesVisibles())
    {
        h = c.Y - LignesVisibles() + 1;
    }
    DefinitHautGauche(g, h);
}
//---------------------------------------------------------------------------
// Sélection
//---------------------------------------------------------------------------
void GrilleSimulation::MetAJourSelection()
{
    Selection.Left = qMin(Courante.X, Ancre.X);
    Selection.Right = qMax(Courante.X, Ancre.X);
    Selection.Top = qMin(Courante.Y, Ancre.Y);
    Selection.Bottom = qMax(Courante.Y, Ancre.Y);
}
//---------------------------------------------------------------------------
void GrilleSimulation::InvalideSelection(const TGridRect &Ancienne)
{
    viewport()->update(RectGrille(Ancienne));
    viewport()->update(RectGrille(Selection));
}
//---------------------------------------------------------------------------
void GrilleSimulation::MoveCurrent(int ACol, int ARow, bool MoveAnchor)
{ // TCustomGrid::MoveCurrent
    ACol = qBound(0, ACol, NbColonnes - 1);
    ARow = qBound(0, ARow, NbLignes - 1);
    const TGridRect Ancienne = Selection;
    const TGridCoord AncienneCourante = Courante;
    Courante.X = ACol;
    Courante.Y = ARow;
    if (MoveAnchor || !Options.Contains(goRangeSelect))
    {
        Ancre = Courante;
    }
    MetAJourSelection();
    ClampInView(Courante);
    InvalidateCell(AncienneCourante.X, AncienneCourante.Y);
    InvalideSelection(Ancienne);
}
//---------------------------------------------------------------------------
void GrilleSimulation::MoveAnchor(TGridCoord NouvelleAncre)
{ // TCustomGrid::MoveAnchor
    if (!Options.Contains(goRangeSelect))
    {
        MoveCurrent(NouvelleAncre.X, NouvelleAncre.Y, true);
        return;
    }
    if ((NouvelleAncre.X == Ancre.X) && (NouvelleAncre.Y == Ancre.Y))
    {
        return;
    }
    const TGridRect Ancienne = Selection;
    Ancre = NouvelleAncre;
    MetAJourSelection();
    ClampInView(Ancre);
    InvalideSelection(Ancienne);
}
//---------------------------------------------------------------------------
void GrilleSimulation::FocusCell(int ACol, int ARow, bool MoveAnchor)
{ // TCustomGrid::FocusCell : déplacement puis Click
    MoveCurrent(ACol, ARow, MoveAnchor);
    if (OnClick)
    {
        OnClick(this);
    }
}
//---------------------------------------------------------------------------
void GrilleSimulation::InvalidateCell(int ACol, int ARow)
{
    if ((ACol < HautGauche.X) || (ARow < HautGauche.Y))
    {
        return;
    }
    viewport()->update(CellRect(ACol, ARow).Qt());
}
//---------------------------------------------------------------------------
void GrilleSimulation::Repaint()
{ // Pendant un autre dessin, Qt n'autorise pas de dessin imbriqué : différé
    if (TCanvas::NbDessinsEnCours != 0)
    {
        viewport()->update();
    }
    else
    {
        viewport()->repaint();
    }
}
//---------------------------------------------------------------------------
// Dessin
//---------------------------------------------------------------------------
void GrilleSimulation::paintEvent(QPaintEvent *e)
{ // TCustomGrid::Paint (DefaultDrawing, gdsClassic, GridLineWidth = 0)
    QPainter p(viewport());
    const QRegion &Zone = e->region();
    Canvas->Commence(&p);
    const bool Focus = hasFocus();
    const int Largeur = viewport()->width();
    const int Hauteur = viewport()->height();
    int y = 0;
    int r = HautGauche.Y;
    for (; (y < Hauteur) && (r < NbLignes); r++, y += DefaultRowHeight)
    {
        int x = 0;
        int c = HautGauche.X;
        for (; (x < Largeur) && (c < NbColonnes); c++, x += DefaultColWidth)
        {
            TRect Where(x, y, x + DefaultColWidth, y + DefaultRowHeight);
            if (!Zone.intersects(Where.Qt()))
            {
                continue; // RectVisible
            }
            TGridDrawState Etat;
            const bool Selectionnee =
                (c >= Selection.Left) && (c <= Selection.Right) && (r >= Selection.Top) && (r <= Selection.Bottom);
            const bool Active = Focus && (c == Courante.X) && (r == Courante.Y);
            if (Selectionnee)
            {
                Etat << gdSelected;
            }
            if (Active)
            {
                Etat << gdFocused;
            }
            Canvas->Brush->Style = bsSolid;
            Canvas->Brush->Color = (Selectionnee && !Active) ? clHighlight : Color;
            Canvas->Font = Font.Ref();
            Canvas->FillRect(Where);
            if (OnDrawCell)
            {
                OnDrawCell(this, c, r, Where, Etat);
            }
            if (Active)
            {
                Canvas->DrawFocusRect(Where);
            }
        }
        if (x < Largeur)
        {
            p.fillRect(QRect(x, y, Largeur - x, DefaultRowHeight), CouleurQt(Color));
        }
    }
    if (y < Hauteur)
    {
        p.fillRect(QRect(0, y, Largeur, Hauteur - y), CouleurQt(Color));
    }
    Canvas->Termine();
}
//---------------------------------------------------------------------------
void GrilleSimulation::focusInEvent(QFocusEvent *e)
{
    QAbstractScrollArea::focusInEvent(e);
    viewport()->update(RectGrille(Selection));
}
//---------------------------------------------------------------------------
void GrilleSimulation::focusOutEvent(QFocusEvent *e)
{
    QAbstractScrollArea::focusOutEvent(e);
    viewport()->update(RectGrille(Selection));
}
//---------------------------------------------------------------------------
// Souris
//---------------------------------------------------------------------------
void GrilleSimulation::mousePressEvent(QMouseEvent *e)
{ // TCustomGrid::MouseDown
    setFocus(Qt::MouseFocusReason);
    if (e->button() != Qt::LeftButton)
    {
        return; // clic droit : pas de changement de case
    }
    const TGridCoord c = CaseEn(e->position().toPoint());
    if (c.X < 0)
    {
        return;
    }
    SelectionEnCours = true;
    DernierePosition = e->position().toPoint();
    DefilementAuto.start();
    if (e->modifiers().testFlag(Qt::ShiftModifier))
    {
        MoveAnchor(c);
    }
    else
    {
        MoveCurrent(c.X, c.Y, true);
    }
}
//---------------------------------------------------------------------------
void GrilleSimulation::mouseMoveEvent(QMouseEvent *e)
{ // Glisser : l'ancre suit la souris, la case courante reste celle du clic
  // (TCustomGrid::MouseMove, gsSelecting). v6.0.12 : comme la VCL, pas au-delà de la case qui
  // suit la dernière case entièrement visible, ni à gauche ou au-dessus des cases : au-delà,
  // la minuterie fait avancer l'ancre (l'ancre allait sous la souris, à plusieurs cases de la
  // vue, et un mouvement à gauche ou au-dessus faisait défiler d'une case)
    if (!SelectionEnCours)
    {
        return;
    }
    DernierePosition = e->position().toPoint();
    const TGridCoord c = CaseEn(DernierePosition);
    if ((DernierePosition.x() >= 0) && (DernierePosition.y() >= 0) && (c.X >= 0) &&
        (c.X <= DerniereVisible(true) + 1) && (c.Y <= DerniereVisible(false) + 1))
    { // v6.0.12
        MoveAnchor(c);
    }
}
//---------------------------------------------------------------------------
void GrilleSimulation::mouseReleaseEvent(QMouseEvent *e)
{ // TCustomGrid::MouseUp : fin de sélection puis Click
    if ((e->button() != Qt::LeftButton) || !SelectionEnCours)
    {
        return;
    }
    mouseMoveEvent(e);
    SelectionEnCours = false;
    DefilementAuto.stop();
    if (OnClick)
    {
        OnClick(this);
    }
}
//---------------------------------------------------------------------------
void GrilleSimulation::mouseDoubleClickEvent(QMouseEvent *e)
{ // ssDouble : DblClick, sans déplacement
    if (e->button() == Qt::LeftButton)
    {
        if (OnDblClick)
        {
            OnDblClick(this);
        }
    }
}
//---------------------------------------------------------------------------
void GrilleSimulation::wheelEvent(QWheelEvent *e)
{ // TCustomGrid::DoMouseWheelDown/Up : la molette déplace la case courante
    CumulMolette += e->angleDelta().y();
    while (CumulMolette <= -120)
    {
        CumulMolette += 120;
        if (Courante.Y < NbLignes - 1)
        {
            FocusCell(Courante.X, Courante.Y + 1, true);
        }
    }
    while (CumulMolette >= 120)
    {
        CumulMolette -= 120;
        if (Courante.Y > 0)
        {
            FocusCell(Courante.X, Courante.Y - 1, true);
        }
    }
    e->accept();
}
//---------------------------------------------------------------------------
void GrilleSimulation::contextMenuEvent(QContextMenuEvent *e)
{
    if (PopupMenu)
    {
        QPoint g = e->globalPos();
        if (e->reason() != QContextMenuEvent::Mouse)
        { // Touche Menu / Maj+F10 : sous la case courante. v6.0.12 : si elle n'est pas
          // entièrement visible, en haut à gauche des cases, comme la VCL (le menu s'ouvrait
          // hors de la grille)
            const TRect r = CellRect(Courante.X, Courante.Y);
            const bool Visible = viewport()->rect().contains(r.Qt());
            g = viewport()->mapToGlobal(Visible ? QPoint(r.Left, r.Bottom) : QPoint(0, 0));
        }
        PopupMenu->Popup(g.x(), g.y());
    }
}
//---------------------------------------------------------------------------
// Clavier
//---------------------------------------------------------------------------
void GrilleSimulation::keyPressEvent(QKeyEvent *e)
{ // TCustomGrid::KeyDown
    bool Ctrl = e->modifiers().testFlag(Qt::ControlModifier), Maj = e->modifiers().testFlag(Qt::ShiftModifier);
#ifdef Q_OS_MACOS
    Ctrl = Ctrl || ((e->modifiers() & Qt::MetaModifier) != 0u);
#endif
    TGridCoord n = Courante;
    int g = HautGauche.X;
    int h = HautGauche.Y;
    bool Gere = true;
    if (Ctrl)
    {
        switch (e->key())
        {
        case Qt::Key_Up:
            h--;
            break;
        case Qt::Key_Down:
            h++;
            break;
        case Qt::Key_Left:
            n.X -= ColonnesVisibles();
            g -= ColonnesVisibles();
            break;
        case Qt::Key_Right:
            n.X += ColonnesVisibles();
            g += ColonnesVisibles();
            break;
        case Qt::Key_PageUp:
            n.Y = HautGauche.Y;
            break;
        case Qt::Key_PageDown:
            n.Y = qMin(NbLignes - 1, HautGauche.Y + LignesVisibles() - 1);
            break;
        case Qt::Key_Home:
            n.X = 0;
            n.Y = 0;
            break;
        case Qt::Key_End:
            n.X = NbColonnes - 1;
            n.Y = NbLignes - 1;
            break;
        default:
            Gere = false;
        }
    }
    else
    {
        switch (e->key())
        {
        case Qt::Key_Up:
            n.Y--;
            break;
        case Qt::Key_Down:
            n.Y++;
            break;
        case Qt::Key_Left:
            n.X--;
            break;
        case Qt::Key_Right:
            n.X++;
            break;
        case Qt::Key_PageDown:
            n.Y += LignesVisibles();
            h += LignesVisibles();
            break;
        case Qt::Key_PageUp:
            n.Y -= LignesVisibles();
            h -= LignesVisibles();
            break;
        case Qt::Key_Home:
            n.X = 0;
            break;
        case Qt::Key_End:
            n.X = NbColonnes - 1;
            break;
        default:
            Gere = false;
        }
    }
    if (!Gere)
    {
        QAbstractScrollArea::keyPressEvent(e);
        return;
    }
    n.X = qBound(0, n.X, NbColonnes - 1);
    n.Y = qBound(0, n.Y, NbLignes - 1);
    if ((g != HautGauche.X) || (h != HautGauche.Y))
    {
        DefinitHautGauche(g, h);
    }
    if ((n.X != Courante.X) || (n.Y != Courante.Y))
    {
        FocusCell(n.X, n.Y, !Maj);
    }
    e->accept();
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// test_interface.cpp - Tests de l'application Qt (phases 4 à 6)
//
// Lance l'interface comme « cvw <carte> » (copie de maps/Démo.cvw dans un
// dossier temporaire, car Marche enregistre le fichier) puis la pilote comme
// un utilisateur : boutons de la barre d'outils, éléments de menu, raccourcis
// clavier. Les boîtes de message (bilan...) reçoivent la réponse par défaut
// de l'utilisateur pressé : « Non ». Vérifie :
//   - le chargement par la ligne de commande, le titre, la taille de la grille ;
//   - les raccourcis selon le contexte (Ctrl+B : passage de bus en édition,
//     détail d'un bus en simulation) ;
//   - Marche / Interruption / Arrêt, par la barre d'outils et par les menus,
//     et la mise à jour de la barre d'état ;
//   - l'enregistrement : enregistrer puis rouvrir et réenregistrer donne le
//     même fichier, identique à la conversion de référence du format ;
//   - Nouveau puis génération aléatoire (fiches Dimensions et Génération) à
//     graine fixe : même réseau que le golden master de la génération ;
//   - lot 1 des fiches secondaires (paramètres et définitions), sur une copie
//     neuve de Démo : chaque fiche est ouverte par son menu, son bouton ou son
//     action, un paramètre y est modifié et validé, puis son effet sur le
//     moteur (cv->...) est vérifié ; Annuler ne change rien ; Paramètres et
//     Carrefours : enregistrer puis rouvrir conserve les valeurs. L'écran
//     « À propos » du démarrage s'anime puis se ferme seul ;
//   - lot 2 (distances et informations), en simulation à graine fixe : les
//     six tableaux de distances ouverts par leurs menus, actions et raccourcis
//     affichent les distances du moteur pour la case courante (Géant, qui a
//     des arrêts, des files et des places de chaque sorte, puis Démo), se redimensionnent
//     d'après le nombre de lignes et se ferment à l'arrêt ; les six fiches
//     d'informations (non modales, au premier plan) suivent tour après tour
//     l'objet de la case courante, l'utilisateur cliquant sur lui quand il se
//     déplace ; à l'arrêt, elles gardent leur contenu jusqu'au clic suivant
//     sur la carte (comme l'original), qui les vide.
// Les fiches modales sont pilotées par des étapes (QuandModale) : l'action
// prévue est exécutée quand la fiche attendue devient la fiche modale active.
// Les captures de la fenêtre principale et des fiches sont écrites dans le
// dossier CV_DOSSIER_CAPTURES (Démo au tour 0 et vers le tour 200, fiche_*.png).
//---------------------------------------------------------------------------
#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileOpenEvent>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <deque>
#include <fstream>
#include <sstream>
#include "application.h"
#include "f_afdistab.h"
#include "f_afdistat.h"
#include "f_afdistfp.h"
#include "f_afdistft.h"
#include "f_afdistp.h"
#include "f_afdistpv.h"
#include "f_carfours.h"
#include "f_deff.h"
#include "f_deffp.h"
#include "f_defn.h"
#include "f_defv.h"
#include "f_defvp.h"
#include "f_dimzone.h"
#include "f_epidemie.h"
#include "f_genres.h"
#include "f_infbus.h"
#include "f_infptn.h"
#include "f_inftaxi.h"
#include "f_inftram.h"
#include "f_infveh.h"
#include "f_infvehlib.h"
#include "f_prmbus.h"
#include "f_prmsim.h"
#include "f_prmtaxi.h"
#include "f_prmtram.h"
#include "f_prmvehlib.h"
#include "f_propos.h"
#include "f_simul.h"
#include "format_reference.h"
#include "compat/alea.h" // après les en-têtes système
//---------------------------------------------------------------------------
extern TfrmGeneration *frmGeneration; // défini dans f_genres.cpp (non déclaré par f_genres.h d'origine)
//---------------------------------------------------------------------------
namespace
 {
  QString DossierCaptures;
  QStringList ArgumentsTest;
  //-------------------------------------------------------------------------
  QByteArray Contenu(const QString &asFichier)
   {
    QFile f(asFichier);
    return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();
   }
  //-------------------------------------------------------------------------
  // f_carfours.cpp : DirPossC4x4[x-xd][y-yd]
  const dirposs DirPossC4x4[4][4]=
   {{aucune,      O,      E, aucune},
    {     S,     SO,     SE,      S},
    {     N,     NO,     NE,      N},
    {aucune,      O,      E, aucune}};
  //-------------------------------------------------------------------------
  // Case libre : sans voie, nature, parking, arrêt, place ni feu
  bool CaseLibre(int x, int y)
   {
    voie &v=cv->v[(x+cv->NbX)%cv->NbX][(y+cv->NbY)%cv->NbY];
    return !v.DirPoss&&!v.SensVoieTram&&!NATURE(&v)&&!v.NumParking&&!v.NumArretBus&&!v.NumArretTram&&
           !v.NumPlaceTaxi&&!v.NumPlacePark&&!v.NumPlaceVehlib&&!v.PassagePietons&&!v.EstDepotBus()&&
           !v.NumFeu[0]&&!v.NumFeu[1]&&!v.NumFeu[2]&&!v.NumFeu[3]&&
           !v.NumFeuP[0]&&!v.NumFeuP[1]&&!v.NumFeuP[2]&&!v.NumFeuP[3];
   }
  //-------------------------------------------------------------------------
  // Zone libre de l à h cases entourée d'une marge libre, en haut à gauche de
  // la carte (visible sans défilement), hors des zones déjà prises
  bool ZoneLibre(int l, int h, int &xz, int &yz, const std::vector<QRect> &Prises={})
   {
    for(int y=1; y+h<24; y++)
     for(int x=1; x+l<36; x++)
      {
       QRect r(x-1, y-1, l+2, h+2);
       bool Ok=true;
       for(const QRect &p : Prises) Ok=Ok&&!p.intersects(r);
       for(int i=-1; Ok&&(i<=l); i++)
        for(int j=-1; Ok&&(j<=h); j++)
         Ok=CaseLibre(x+i, y+j);
       if (Ok) { xz=x; yz=y; return true; }
      }
    return false;
   }
  //-------------------------------------------------------------------------
  int NbFeuxPietonsDefinis()
   {
    int n=0;
    for(int i=0; i<cv->NbFeuxPieton; i++) n+=cv->FeuP[i].Defini();
    return n;
   }
 }
//---------------------------------------------------------------------------
class TestInterface : public QObject
 {
  Q_OBJECT
  QString Carte;
  QStringList Messages; // boîtes de message affichées (titre : texte)
  QTimer Repondeur;
  QMessageBox::StandardButton ReponseQuestions=QMessageBox::No; // réponse aux boîtes Oui/Non
  // Fiches modales à piloter, dans l'ordre : quand Fiche() devient la fiche
  // modale active, Action est exécutée (hors du minuteur, car elle peut ouvrir
  // une autre fiche modale)
  struct Etape { std::function<QWidget *()> Fiche; std::function<void()> Action; };
  std::deque<Etape> Etapes;
  QPointer<QWidget> ModaleSansReponse;
  QElapsedTimer DureeSansReponse;
  void QuandModale(std::function<QWidget *()> Fiche, std::function<void()> Action)
   {
    Etapes.push_back({std::move(Fiche), std::move(Action)});
   }
  void CaptureFiche(QWidget *Fiche, const QString &asNom)
   {
    if (DossierCaptures.isEmpty()) return;
    QDir().mkpath(DossierCaptures);
    QApplication::processEvents();
    QVERIFY(Fiche->grab().save(DossierCaptures+"/"+asNom));
   }
  void Clique(QWidget *w)
   { // Cases à cocher et boutons radio : clic à l'extrémité droite, hors du
     // texte (la VCL y réagit ; Qt, seulement sur la case et le texte)
    if (qobject_cast<QCheckBox *>(w)||qobject_cast<QRadioButton *>(w))
     QTest::mouseClick(w, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(w->width()-3, w->height()/2));
    else
     QTest::mouseClick(w, Qt::LeftButton);
   }
  void Touche(QWidget *w, Qt::Key Touche)
   { // Touche frappée dans le contrôle actif (qui reçoit le clavier)
    w->window()->activateWindow();
    w->setFocus(Qt::OtherFocusReason);
    QTest::keyClick(w, Touche);
   }
  void CliqueChoix(TRadioGroup *g, int i)
   { // Clic sur le i-ème bouton d'un TRadioGroup (<Nom>_<i> créés par dfm2ui)
    QRadioButton *b=g->findChild<QRadioButton *>(g->objectName()+"_"+QString::number(i));
    QVERIFY(b);
    Clique(b);
   }
  void PlaceCurseur(int x, int y)
   { // Case courante (et ancre), puis OnDrawCell de la case : état des actions
    frmSimulation->DrawGridSimul->Col=x;
    frmSimulation->DrawGridSimul->Row=y;
    frmSimulation->DrawGridSimul->Repaint();
   }
  void Selectionne(int x1, int y1, int x2, int y2)
   { // Clic sur (x1, y1) puis Maj+clic sur (x2, y2), comme l'utilisateur
    GrilleSimulation *g=frmSimulation->DrawGridSimul;
    g->LeftCol=0;
    g->TopRow=0;
    auto Centre=[g](int x, int y) { TRect r=g->CellRect(x, y); return QPoint((r.Left+r.Right)/2, (r.Top+r.Bottom)/2); };
    QTest::mouseClick(g->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), Centre(x1, y1));
    QTest::mouseClick(g->viewport(), Qt::LeftButton, Qt::ShiftModifier, Centre(x2, y2));
    g->Repaint();
    QCOMPARE(g->Selection.Left, qMin(x1, x2));
    QCOMPARE(g->Selection.Top, qMin(y1, y2));
    QCOMPARE(g->Selection.Right, qMax(x1, x2));
    QCOMPARE(g->Selection.Bottom, qMax(y1, y2));
   }
  void OuvreFichier(const QString &asFichier)
   { // Ouverture comme depuis le Finder (QFileOpenEvent, macOS)
    OuvertureFichiers Ouverture;
    qApp->installEventFilter(&Ouverture);
    QFileOpenEvent Evenement(asFichier);
    QCoreApplication::sendEvent(qApp, &Evenement);
    qApp->removeEventFilter(&Ouverture);
   }
  QString OuvreDemo(const QString &asNom)
   { // Copie neuve de maps/Démo.cvw, ouverte
    QString f=QFileInfo(Carte).absolutePath()+"/"+asNom;
    QFile::remove(f);
    if (!QFile::copy(ArgumentsTest.value(2), f)) return QString();
    OuvreFichier(f);
    return f;
   }
  void Enregistre(const QString &asFichier)
   {
    frmSimulation->SaveDialog->FileName=A(QDir::toNativeSeparators(asFichier));
    frmSimulation->ActionEnregistrer->Execute();
   }
  void Capture(const QString &asNom)
   {
    if (DossierCaptures.isEmpty()) return;
    QDir().mkpath(DossierCaptures);
    QVERIFY(frmSimulation->grab().save(DossierCaptures+"/"+asNom));
   }
  bool AttendTour(long Tour, int DelaiMs)
   {
    return QTest::qWaitFor([Tour]() { return cv&&(cv->TourCrt>=Tour); }, DelaiMs);
   }
  QString OuvreCarte(const QString &asCarte, const QString &asNom)
   { // Copie neuve de maps/<asCarte>, ouverte
    QString f=QFileInfo(Carte).absolutePath()+"/"+asNom;
    QFile::remove(f);
    if (!QFile::copy(QFileInfo(ArgumentsTest.value(2)).absolutePath()+"/"+asCarte, f)) return QString();
    OuvreFichier(f);
    return f;
   }
  bool Demarre()
   { // Marche par la barre d'outils : vérification, calcul des distances, enregistrement.
     // Graine fixe (srand de AppelleAuChaos) : sur Géant, le moteur d'origine boucle
     // sans fin dans vehlib::ReservePlaceVehlibCible quand plus aucune place de
     // vehlib n'est libre, ce qui arrive selon la graine (123 : avant le tour 50) ;
     // la graine 1 tient plus de 1500 tours (cvsim)
    alea::FixeGraine(1);
    Messages.clear();
    QTest::mouseClick(frmSimulation->ToolButtonMarche, Qt::LeftButton);
    return frmSimulation->SimulationEnCours&&cv->DistancesCalculees;
   }
  bool Arrete()
   { // Arrêt par la barre d'outils : bilan, puis rechargement du réseau
    QTest::mouseClick(frmSimulation->ToolButtonArret, Qt::LeftButton);
    return QTest::qWaitFor([]() { return !frmSimulation->SimulationEnCours&&!frmSimulation->Pause&&(cv->TourCrt==0); }, 10000);
   }
  void CliqueCase(int x, int y)
   { // Clic de l'utilisateur sur la case (x, y), amenée dans la vue
    GrilleSimulation *g=frmSimulation->DrawGridSimul;
    g->LeftCol=qMax(0, x-4);
    g->TopRow=qMax(0, y-4);
    TRect r=g->CellRect(x, y);
    QTest::mouseClick(g->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), QPoint((r.Left+r.Right)/2, (r.Top+r.Bottom)/2));
   }
  void VerifieFichesDistances(const QString &asCarte, bool Captures);
  void SuitObjet(const char *Type, std::function<QString(long)> Ecart, std::function<bool(int &, int &)> Position, int NbTours);
  void Focalise()
   {
    frmSimulation->activateWindow();
    (void)QTest::qWaitForWindowActive(frmSimulation, 2000);
    frmSimulation->DrawGridSimul->setFocus();
    QApplication::processEvents();
   }
  private slots:
  void initTestCase();
  void init();
  void aProposAuDemarrage();
  void ouvertureParLigneDeCommande();
  void raccourcisSelonLeMode();
  void marcheInterruptionArret();
  void enregistrementIdentique();
  void nouveauEtGeneration();
  // Lot 1 des fiches secondaires (phase 6)
  void ficheAPropos();
  void ficheEpidemie();
  void ficheNature();
  void fichesPrioritesEtFeux();
  void fichesPrioritesEtFeuxPietons();
  void fichesParametres();
  void ficheCarrefours();
  // Lot 2 des fiches secondaires (phase 6)
  void fichesDistances();
  void fichesInfos();
  void cleanupTestCase();
 };
//---------------------------------------------------------------------------
void TestInterface::initTestCase()
 {
  // Les boîtes de message reçoivent « Non » (ou OK) et sont mémorisées
  Repondeur.setInterval(20);
  QObject::connect(&Repondeur, &QTimer::timeout, this, [this]()
   {
    QWidget *Modale=QApplication::activeModalWidget();
    if (Modale&&!Etapes.empty()&&(Modale==Etapes.front().Fiche()))
     {
      std::function<void()> Action=Etapes.front().Action;
      Etapes.pop_front();
      ModaleSansReponse=nullptr;
      QTimer::singleShot(0, this, Action);
      return;
     }
    if (frmDimensionsZone&&(Modale==frmDimensionsZone))
     { // Nouveau : dimensions proposées, génération aléatoire, sans les paramètres
      frmDimensionsZone->CheckBoxAfficherParametres->Checked=false;
      frmDimensionsZone->CheckBoxGenererReseau->Checked=true;
      QTest::mouseClick(frmDimensionsZone->ButtonCreer, Qt::LeftButton);
      return;
     }
    if (frmGeneration&&(Modale==frmGeneration))
     {
      QTest::mouseClick(frmGeneration->ButtonOk, Qt::LeftButton);
      return;
     }
    QMessageBox *b=qobject_cast<QMessageBox *>(Modale);
    if (!b)
     { // Fiche modale sans étape prévue : fermée au bout de 10 s (le test échoue sans rester bloqué)
      if (Modale!=ModaleSansReponse) { ModaleSansReponse=Modale; DureeSansReponse.start(); }
      else if (Modale&&DureeSansReponse.hasExpired(10000))
       {
        Messages<<("Fiche modale sans réponse : "+Modale->windowTitle());
        ModaleSansReponse=nullptr;
        Modale->close();
       }
      return;
     }
    Messages<<(b->windowTitle()+" : "+b->text());
    QAbstractButton *r=b->button(ReponseQuestions);
    if (!r) r=b->button(QMessageBox::Ok);
    if (!r) r=b->buttons().value(0);
    if (r) r->click();
   });
  Repondeur.start();
  Carte=ArgumentsTest.value(1);
  QVERIFY2(QFile::exists(Carte), qPrintable(Carte));
  CreeFiches();
  QVERIFY(frmSimulation);
 }
//---------------------------------------------------------------------------
void TestInterface::init()
 {
  Etapes.clear();
  Messages.clear();
  ReponseQuestions=QMessageBox::No;
  // Simulation laissée en cours par un test en échec : arrêtée
  if (frmSimulation->SimulationEnCours||frmSimulation->Pause) (void)Arrete();
  alea::LibereGraine();
  Messages.clear();
 }
//---------------------------------------------------------------------------
void TestInterface::aProposAuDemarrage()
 { // f_simul FormShow : « À propos » affichée (non modale), feu vert puis texte
   // en vert au premier tic du minuteur, fermeture automatique au deuxième (2 s)
  QVERIFY(frmAPropos);
  QVERIFY(frmAPropos->isVisible());
  QVERIFY(!QApplication::activeModalWidget());
  QVERIFY(frmAPropos->FermetureAutomatique);
  QVERIFY(frmAPropos->Timer->Enabled);
  QCOMPARE(Q(frmAPropos->LabelAlignement->Caption), QString("version 64 bits"));
  QVERIFY2(QRegularExpression("^HDPI x\\d,\\d$").match(Q(frmAPropos->LabelHDPI->Caption)).hasMatch(),
           qPrintable(Q(frmAPropos->LabelHDPI->Caption)));
  QVERIFY(frmAPropos->SpeedButtonOk->Glyph.Ref().Image.toImage()==frmAPropos->VirtualImageList->Pixmap(0).toImage());
  QVERIFY(!frmAPropos->SpeedButtonOk->icon().isNull());
  QVERIFY(QTest::qWaitFor([]() { return frmAPropos->LabelTexte->Font->Color==clLime; }, 3000));
  CaptureFiche(frmAPropos, "fiche_propos_demarrage.png");
  QVERIFY(QTest::qWaitFor([]() { return !frmAPropos->isVisible(); }, 4000));
  QVERIFY(!frmAPropos->FermetureAutomatique);
  QVERIFY(!frmAPropos->Timer->Enabled);
 }
//---------------------------------------------------------------------------
void TestInterface::ouvertureParLigneDeCommande()
 { // cvw <carte> : ouverture dans FormShow, sans boîte de dialogue
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+Carte);
  QVERIFY(cv);
  QCOMPARE((int)frmSimulation->DrawGridSimul->ColCount, cv->NbX);
  QCOMPARE((int)frmSimulation->DrawGridSimul->RowCount, cv->NbY);
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
  QVERIFY(!frmSimulation->SimulationModifiee);
  QVERIFY(frmSimulation->ActionMarche->Enabled);
  QVERIFY(!frmSimulation->ActionArret->Enabled);
  QVERIFY(Messages.isEmpty());
  QTest::qWait(200);
  frmSimulation->DrawGridSimul->Repaint();
  Capture("demo_tour0.png");
 }
//---------------------------------------------------------------------------
void TestInterface::raccourcisSelonLeMode()
 { // Ctrl+B : « Passage de bus » (menu contextuel d'édition) avant la simulation
  int x=-1, y=-1;
  for(int i=0; (x<0)&&(i<cv->NbX); i++)
   for(int j=0; j<cv->NbY; j++)
    if (cv->v[i][j].DirPoss&&!cv->v[i][j].PassageBus&&!cv->v[i][j].Priorite[0]&&!cv->v[i][j].NumParking)
     { x=i; y=j; break; }
  QVERIFY(x>=0);
  Focalise();
  frmSimulation->DrawGridSimul->Col=x;
  frmSimulation->DrawGridSimul->Row=y;
  frmSimulation->DrawGridSimul->Repaint(); // OnDrawCell de la case courante : état des actions
  QVERIFY(frmSimulation->ActionPassageBus->Enabled);
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_B, Qt::ControlModifier);
  QVERIFY(cv->v[x][y].PassageBus);
  QVERIFY(frmSimulation->SimulationModifiee);
  QVERIFY(Q(frmSimulation->Caption).endsWith('*'));
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_B, Qt::ControlModifier);
  QVERIFY(!cv->v[x][y].PassageBus);
  QVERIFY(!frmInfosBus->isVisible());
 }
//---------------------------------------------------------------------------
void TestInterface::marcheInterruptionArret()
 {
  // Marche par la barre d'outils : vérification, distances, enregistrement automatique
  QTest::mouseClick(frmSimulation->ToolButtonMarche, Qt::LeftButton);
  QVERIFY2(frmSimulation->SimulationEnCours, qPrintable(Messages.join('\n')));
  QVERIFY(!frmSimulation->SimulationModifiee); // enregistré avant la simulation
  QVERIFY(!frmSimulation->ActionMarche->Enabled);
  QVERIFY(frmSimulation->ActionInterruption->Enabled);
  QVERIFY(frmSimulation->ActionArret->Enabled);
  QVERIFY(frmSimulation->ToolButtonArret->isEnabled());
  QVERIFY(frmSimulation->DrawGridSimul->PopupMenu==frmSimulation->PopupMenuSimulation);
  QVERIFY(AttendTour(20, 30000));
  QVERIFY(Q(frmSimulation->StatusBar->Panels->Items[0]->Text).startsWith("Sur "));
  QVERIFY(Q(frmSimulation->Caption).startsWith("Centre-Ville - "+Carte+" ["));
  // Ctrl+B en simulation : « Détail sur un bus » (menu contextuel de simulation)
  Focalise();
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_B, Qt::ControlModifier);
  QVERIFY(frmInfosBus->isVisible());
  QVERIFY(frmSimulation->ActionDetailBus->Checked);
  Focalise(); // la fiche affichée est devenue active, comme avec Visible=true dans la VCL
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_B, Qt::ControlModifier);
  QVERIFY(!frmInfosBus->isVisible());
  // Interruption par le menu Simulation : bilan, puis pause
  QVERIFY(AttendTour(100, 60000));
  Messages.clear();
  frmSimulation->MenuItemPause->ElementQt()->trigger();
  QVERIFY(QTest::qWaitFor([]() { return frmSimulation->Pause; }, 5000));
  QVERIFY(!frmSimulation->SimulationEnCours);
  QVERIFY(frmSimulation->ActionMarche->Enabled);
  QVERIFY(!frmSimulation->ActionInterruption->Enabled);
  QVERIFY(QTest::qWaitFor([this]() { return !Messages.isEmpty(); }, 5000));
  // (pas de titre sous macOS : QMessageBox suit les règles d'Apple)
  QVERIFY2(Messages.first().contains(QString::fromUtf8("Bilan au tour n°")), qPrintable(Messages.first()));
  QVERIFY2(Messages.first().contains(QRegularExpression("\\d,\\d\\d")), qPrintable(Messages.first())); // séparateur décimal ','
  long TourPause=cv->TourCrt;
  QTest::qWait(300);
  QCOMPARE((long)cv->TourCrt, TourPause);
  // Reprise par la barre d'outils, jusque vers le tour 200 : capture
  QTest::mouseClick(frmSimulation->ToolButtonMarche, Qt::LeftButton);
  QVERIFY(frmSimulation->SimulationEnCours);
  QVERIFY(AttendTour(200, 60000));
  Capture("demo_tour200.png");
  // Arrêt par la barre d'outils : bilan, rechargement du réseau enregistré
  Messages.clear();
  QTest::mouseClick(frmSimulation->ToolButtonArret, Qt::LeftButton);
  QVERIFY(QTest::qWaitFor([]() { return !frmSimulation->SimulationEnCours&&!frmSimulation->Pause&&(cv->TourCrt==0); }, 5000));
  QVERIFY(QTest::qWaitFor([this]() { return !Messages.isEmpty(); }, 5000));
  QVERIFY(Messages.first().contains(QString::fromUtf8("Bilan au tour n°")));
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
  QVERIFY(frmSimulation->ActionMarche->Enabled);
  QVERIFY(!frmSimulation->ActionArret->Enabled);
  QVERIFY(frmSimulation->DrawGridSimul->PopupMenu==frmSimulation->PopupMenuEditionCase);
 }
//---------------------------------------------------------------------------
void TestInterface::enregistrementIdentique()
 { // Enregistrer, rouvrir, réenregistrer sous un autre nom : mêmes octets
  QString Dossier=QFileInfo(Carte).absolutePath();
  QString f1=Dossier+"/enregistrement1.cvw", f2=Dossier+"/enregistrement2.cvw";
  frmSimulation->SaveDialog->FileName=A(QDir::toNativeSeparators(f1));
  frmSimulation->ActionEnregistrer->Execute();
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Enregistrement effectué."));
  // Réouverture comme depuis le Finder (QFileOpenEvent, macOS)
  OuvertureFichiers Ouverture;
  qApp->installEventFilter(&Ouverture);
  QFileOpenEvent Evenement(f1);
  QCoreApplication::sendEvent(qApp, &Evenement);
  qApp->removeEventFilter(&Ouverture);
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+QDir::toNativeSeparators(f1));
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Chargement effectué."));
  frmSimulation->SaveDialog->FileName=A(QDir::toNativeSeparators(f2));
  frmSimulation->ActionEnregistrer->Execute();
  QByteArray o1=Contenu(f1), o2=Contenu(f2);
  QVERIFY(!o1.isEmpty());
  QVERIFY(o1==o2);
  // ... et identique à la conversion de référence de la carte d'origine au format 0x530
  QString Origine=ArgumentsTest.value(2);
  conversion_cvw Reference=ConvertitEn530(Contenu(Origine).toStdString());
  QVERIFY(Reference.Ok);
  QVERIFY(o1.toStdString()==Reference.Octets);
 }
//---------------------------------------------------------------------------
void TestInterface::nouveauEtGeneration()
 { // f_simul : ActionNouveauExecute -> f_dimzone -> f_genres (ButtonOkClick), graine 12345
  alea::FixeGraine(12345);
  Messages.clear();
  QTest::mouseClick(frmSimulation->ToolButtonNouveau, Qt::LeftButton);
  alea::LibereGraine();
  QVERIFY2(Messages.isEmpty(), qPrintable(Messages.join('\n')));
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville"));
  QCOMPARE(cv->NbX, NBXDEF);
  QCOMPARE(cv->NbY, NBYDEF);
  QVERIFY(!frmSimulation->SimulationModifiee);
  QVERIFY(!frmDimensionsZone);
  QVERIFY(!frmGeneration);
  // Réseau généré = golden master « defaut_12345 » (tests/golden/generation)
  std::ifstream f(std::string(CV_DOSSIER_GOLDEN)+"/generation/defaut_12345.txt");
  QVERIFY(f.good());
  std::string Ligne, Attendu, Obtenu;
  bool Carte=false;
  while(std::getline(f, Ligne))
   {
    if (Ligne.rfind("DirPoss", 0)==0) { Carte=true; continue; }
    if (Carte&&(Ligne.rfind("etat=", 0)==0)) break;
    if (Carte) Attendu+=Ligne+"\n";
   }
  char Tampon[4];
  for(int j=0; j<cv->NbY; j++)
   {
    for(int i=0; i<cv->NbX; i++)
     {
      std::snprintf(Tampon, sizeof(Tampon), "%x", (int)cv->v[i][j].DirPoss);
      Obtenu+=Tampon;
     }
    Obtenu+="\n";
   }
  QVERIFY(!Attendu.empty());
  QVERIFY(Obtenu==Attendu);
  frmSimulation->DrawGridSimul->Repaint();
  Capture("generation_12345.png");
 }
//---------------------------------------------------------------------------
// Lot 1 des fiches secondaires
//---------------------------------------------------------------------------
void TestInterface::ficheAPropos()
 { // Aide > À propos : fiche modale ; feu vert, orange puis rouge (minuteur
   // accéléré) ; Échap = clic sur le feu = boîte « Licence »
  QVERIFY(!OuvreDemo("lot1_apropos.cvw").isEmpty());
  frmAPropos->Timer->Interval=20;
  bool Vert=false, Jaune=false, Rouge=false, GlypheRouge=false, Modale=false;
  Messages.clear();
  QuandModale([]() { return frmAPropos; }, [&]()
   {
    Modale=frmAPropos->isModal()&&!frmAPropos->FermetureAutomatique;
    Vert=QTest::qWaitFor([]() { return frmAPropos->LabelTexte->Font->Color==clLime; }, 2000);
    Jaune=QTest::qWaitFor([]() { return frmAPropos->LabelTexte->Font->Color==clYellow; }, 2000);
    Rouge=QTest::qWaitFor([]() { return frmAPropos->LabelTexte->Font->Color==clRed; }, 2000);
    GlypheRouge=frmAPropos->SpeedButtonOk->Glyph.Ref().Image.toImage()==frmAPropos->VirtualImageList->Pixmap(2).toImage();
    frmAPropos->Timer->Enabled=false; // capture au feu rouge
    CaptureFiche(frmAPropos, "fiche_propos.png");
    frmAPropos->Timer->Enabled=true;
    frmAPropos->activateWindow();
    QTest::keyClick(frmAPropos, Qt::Key_Escape); // aucun contrôle actif : FormKeyPress -> SpeedButtonOk->Click() -> LabelClick
    frmAPropos->Close();
   });
  frmSimulation->MenuItemAPropos->ElementQt()->trigger();
  frmAPropos->Timer->Interval=1000;
  QVERIFY(Etapes.empty());
  QVERIFY(Modale);
  QVERIFY(Vert&&Jaune&&Rouge);
  QVERIFY(GlypheRouge);
  QVERIFY(!frmAPropos->isVisible());
  QVERIFY(!frmAPropos->Timer->Enabled);
  QCOMPARE(Messages.size(), 1); // (titre « Licence » absent sous macOS)
  QVERIFY2(Messages.first().contains(QString::fromUtf8(" : Centre-Ville est sous Licence Publique Générale")), qPrintable(Messages.first()));
 }
//---------------------------------------------------------------------------
void TestInterface::ficheEpidemie()
 { // Menu Simulation > Épidémie (AutoCheck) : la fiche modale s'ouvre quand l'action se coche
  QVERIFY(!OuvreDemo("lot1_epidemie.cvw").isEmpty());
  QVERIFY(!frmSimulation->ActionEpidemie->Checked);
  int Infectiosite=cv->EpidemieInfectiosite, PatientZero=cv->EpidemieIterationPatientZero,
      Charge=cv->EpidemieChargeFatale;
  bool Reinfections=cv->EpidemieReinfections, Affichage=false;
  QuandModale([]() { return frmEpidemie; }, [&]()
   {
    Affichage=frmEpidemie->CheckBoxActiver->Checked&&frmEpidemie->GroupBox->Enabled&&
              (frmEpidemie->CSpinEditInfectiosite->Value==Infectiosite)&&
              (frmEpidemie->CSpinEditChargeViraleFatale->Value==Charge);
    frmEpidemie->CSpinEditInfectiosite->Value=Infectiosite+11;
    frmEpidemie->CSpinEditPatientZero->Value=PatientZero+7;
    frmEpidemie->CSpinEditChargeViraleFatale->Value=Charge+3;
    Clique(frmEpidemie->CheckBoxReinfections);
    CaptureFiche(frmEpidemie, "fiche_epidemie.png");
    Clique(frmEpidemie->ButtonOk);
   });
  frmSimulation->Epidemie1->ElementQt()->trigger();
  QVERIFY(Etapes.empty());
  QVERIFY(Affichage);
  QVERIFY(frmSimulation->ActionEpidemie->Checked);
  QVERIFY(!frmEpidemie->isVisible());
  QCOMPARE(cv->EpidemieInfectiosite, Infectiosite+11);
  QCOMPARE(cv->EpidemieIterationPatientZero, PatientZero+7);
  QCOMPARE(cv->EpidemieChargeFatale, Charge+3);
  QCOMPARE(cv->EpidemieReinfections, !Reinfections);
  // Décocher n'ouvre pas la fiche ; recocher puis Annuler (ou Échap) : rien ne change
  frmSimulation->Epidemie1->ElementQt()->trigger();
  QVERIFY(!frmSimulation->ActionEpidemie->Checked);
  QVERIFY(!frmEpidemie->isVisible());
  bool Relu=false;
  QuandModale([]() { return frmEpidemie; }, [&]()
   {
    Relu=(frmEpidemie->CSpinEditInfectiosite->Value==Infectiosite+11)&&(frmEpidemie->CheckBoxReinfections->Checked==!Reinfections);
    frmEpidemie->CSpinEditInfectiosite->Value=Infectiosite+50;
    Clique(frmEpidemie->ButtonAnnuler);
   });
  frmSimulation->Epidemie1->ElementQt()->trigger();
  QVERIFY(Etapes.empty());
  QVERIFY(Relu);
  QCOMPARE(cv->EpidemieInfectiosite, Infectiosite+11);
  frmSimulation->Epidemie1->ElementQt()->trigger();
  QuandModale([]() { return frmEpidemie; }, [&]()
   {
    frmEpidemie->CSpinEditPatientZero->Value=PatientZero+50;
    Touche(frmEpidemie->CSpinEditPatientZero, Qt::Key_Escape);
   });
  frmSimulation->Epidemie1->ElementQt()->trigger();
  QVERIFY(Etapes.empty());
  QCOMPARE(cv->EpidemieIterationPatientZero, PatientZero+7);
  frmSimulation->Epidemie1->ElementQt()->trigger(); // épidémie désactivée pour la suite
  QVERIFY(!frmSimulation->ActionEpidemie->Checked);
 }
//---------------------------------------------------------------------------
void TestInterface::ficheNature()
 { // Sélection de 4 x 2 cases libres, bouton « Nature » : fiche non modale
  QVERIFY(!OuvreDemo("lot1_nature.cvw").isEmpty());
  int xz, yz;
  QVERIFY(ZoneLibre(4, 2, xz, yz));
  Focalise();
  Selectionne(xz, yz, xz+3, yz+1);
  QVERIFY(frmSimulation->ActionDefinirNature->Enabled);
  Clique(frmSimulation->ToolButtonDefinirNature);
  QVERIFY(frmDefNature->isVisible());
  QVERIFY(frmSimulation->ActionDefinirNature->Checked);
  QVERIFY(!QApplication::activeModalWidget());
  QCOMPARE((int)frmDefNature->RadioGroupNature->ItemIndex, -1);
  QVERIFY(!frmDefNature->ButtonAppliquer->Enabled);
  QVERIFY(!frmDefNature->CheckBoxPelouse->Enabled);
  CliqueChoix(frmDefNature->RadioGroupNature, 1); // Trottoirs
  QVERIFY(frmDefNature->ButtonAppliquer->Enabled);
  QVERIFY(frmDefNature->CheckBoxPelouse->Enabled);
  CaptureFiche(frmDefNature, "fiche_nature.png");
  Clique(frmDefNature->ButtonAppliquer);
  for(int i=0; i<4; i++)
   for(int j=0; j<2; j++)
    QCOMPARE((int)NATURE(&cv->v[xz+i][yz+j]), (int)trottoirs);
  QVERIFY(frmSimulation->SimulationModifiee);
  QVERIFY(frmDefNature->isVisible()); // Appliquer laisse la fiche ouverte
  // Pelouse puis Fermer (bouton Cancel) : rien ne change, fiche cachée
  CliqueChoix(frmDefNature->RadioGroupNature, 2); // Immeubles : la pelouse est décochée et grisée
  QVERIFY(!frmDefNature->CheckBoxPelouse->Enabled);
  Clique(frmDefNature->ButtonFermer);
  QVERIFY(!frmDefNature->isVisible());
  QVERIFY(!frmSimulation->ActionDefinirNature->Checked);
  QCOMPARE((int)NATURE(&cv->v[xz][yz]), (int)trottoirs);
  // Réouverture par le menu : fiche réinitialisée (FormShow)
  frmSimulation->MenuItemDefinirNature->ElementQt()->trigger();
  QVERIFY(frmDefNature->isVisible());
  QCOMPARE((int)frmDefNature->RadioGroupNature->ItemIndex, -1);
  CliqueChoix(frmDefNature->RadioGroupNature, 2);
  Touche(frmDefNature->ButtonAppliquer, Qt::Key_Escape); // Échap = Fermer
  QVERIFY(!frmDefNature->isVisible());
  QCOMPARE((int)NATURE(&cv->v[xz+1][yz+1]), (int)trottoirs);
 }
//---------------------------------------------------------------------------
void TestInterface::fichesPrioritesEtFeux()
 { // Case de route : Priorités (non modale), Stop, puis Feu et fiche Feux (non modale)
  QVERIFY(!OuvreDemo("lot1_priorites.cvw").isEmpty());
  int x=-1, y=-1, d=0;
  for(int i=0; (x<0)&&(i<cv->NbX); i++)
   for(int j=0; (x<0)&&(j<cv->NbY); j++)
    {
     voie &v=cv->v[i][j];
     int n=((v.DirPoss&N)==N)+((v.DirPoss&E)==E)+((v.DirPoss&S)==S)+((v.DirPoss&O)==O);
     if ((n>=2)&&!v.SensVoieTram&&!v.NumParking&&!v.EstDepotBus()&&
         !v.Priorite[0]&&!v.Priorite[1]&&!v.Priorite[2]&&!v.Priorite[3])
      { x=i; y=j; }
    }
  QVERIFY(x>=0);
  voie *v=&cv->v[x][y];
  for(int k=nord; (!d)&&(k<=ouest); k++)
   if ((v->DirPoss&PossDir[k])==PossDir[k]) d=k;
  TRadioGroup *Groupes[NBDIR]={frmDefVoie->RadioGroupPrioriteNord, frmDefVoie->RadioGroupPrioriteEst,
                               frmDefVoie->RadioGroupPrioriteSud, frmDefVoie->RadioGroupPrioriteOuest};
  TBitBtn *BoutonsFeu[NBDIR]={frmDefVoie->BitBtnDefFeuN, frmDefVoie->BitBtnDefFeuE,
                              frmDefVoie->BitBtnDefFeuS, frmDefVoie->BitBtnDefFeuO};
  TRadioGroup *Groupe=Groupes[d-1];
  Focalise();
  PlaceCurseur(x, y);
  QVERIFY(frmSimulation->ActionPriorites->Enabled);
  Clique(frmSimulation->ToolButtonPriorites);
  QVERIFY(frmDefVoie->isVisible());
  QVERIFY(frmSimulation->ActionPriorites->Checked);
  QCOMPARE(Q(frmDefVoie->LabelCoordonnees->Caption), QString::asprintf("%02d, %02d", x, y));
  QVERIFY(Groupe->Enabled);
  QCOMPARE((int)Groupe->ItemIndex, (int)a_droite);
  QVERIFY(!frmDefVoie->ButtonAppliquer->Enabled);
  // Stop, OK
  CliqueChoix(Groupe, stop);
  QVERIFY(frmDefVoie->ButtonAppliquer->Enabled);
  CaptureFiche(frmDefVoie, "fiche_priorites.png");
  Clique(frmDefVoie->ButtonOk);
  QVERIFY(!frmDefVoie->isVisible());
  QVERIFY(!frmSimulation->ActionPriorites->Checked);
  QCOMPARE((int)v->Priorite[d-1], (int)stop);
  // Cédez-le-passage, Annuler : rien ne change
  Clique(frmSimulation->ToolButtonPriorites);
  QCOMPARE((int)Groupe->ItemIndex, (int)stop);
  CliqueChoix(Groupe, cedez_le_passage);
  Clique(frmDefVoie->ButtonAnnuler);
  QVERIFY(!frmDefVoie->isVisible());
  QCOMPARE((int)v->Priorite[d-1], (int)stop);
  // Feu, Appliquer : le feu est créé et son bouton de définition s'active
  frmSimulation->MenuItemPriorites->ElementQt()->trigger();
  QVERIFY(frmDefVoie->isVisible());
  QVERIFY(!BoutonsFeu[d-1]->Enabled);
  int NbFeux=0;
  for(int i=0; i<cv->NbFeux; i++) NbFeux+=cv->Feu[i].Defini();
  CliqueChoix(Groupe, feux);
  Clique(frmDefVoie->ButtonAppliquer);
  QCOMPARE((int)v->Priorite[d-1], (int)feux);
  int n=v->NumFeu[d-1];
  QVERIFY(n>0);
  int NbFeuxApres=0;
  for(int i=0; i<cv->NbFeux; i++) NbFeuxApres+=cv->Feu[i].Defini();
  QCOMPARE(NbFeuxApres, NbFeux+1);
  QVERIFY(BoutonsFeu[d-1]->Enabled);
  QVERIFY(!BoutonsFeu[d-1]->icon().isNull());
  // Fiche Feux : durée du vert et état, OK
  Clique(BoutonsFeu[d-1]);
  QVERIFY(frmDefFeux->isVisible());
  QVERIFY(frmDefVoie->isVisible());
  QCOMPARE((int)frmDefFeux->Direction, d);
  QCOMPARE(Q(frmDefFeux->LabelNumero->Caption), QString::number(n));
  QCOMPARE(Q(frmDefFeux->LabelFeuxPietons->Caption), QString("Aucun"));
  frmDefFeux->CSpinEditDureeVert->Value=7;
  CliqueChoix(frmDefFeux->RadioGroupEtat, rouge);
  QVERIFY(frmDefFeux->ButtonAppliquer->Enabled);
  CaptureFiche(frmDefFeux, "fiche_feux.png");
  Clique(frmDefFeux->ButtonOk);
  QVERIFY(!frmDefFeux->isVisible());
  QCOMPARE(cv->Feu[n-1].DureeVert, 7);
  QCOMPARE((int)cv->Feu[n-1].Etat, (int)rouge);
  // Feux, Annuler : rien ne change
  Clique(BoutonsFeu[d-1]);
  QVERIFY(frmDefFeux->isVisible());
  QCOMPARE((int)frmDefFeux->CSpinEditDureeVert->Value, 7);
  frmDefFeux->CSpinEditDureeVert->Value=9;
  CliqueChoix(frmDefFeux->RadioGroupEtat, vert);
  Clique(frmDefFeux->ButtonAnnuler);
  QVERIFY(!frmDefFeux->isVisible());
  QCOMPARE(cv->Feu[n-1].DureeVert, 7);
  QCOMPARE((int)cv->Feu[n-1].Etat, (int)rouge);
  Clique(frmDefVoie->ButtonOk);
  QVERIFY(!frmDefVoie->isVisible());
  QCOMPARE((int)v->Priorite[d-1], (int)feux);
 }
//---------------------------------------------------------------------------
void TestInterface::fichesPrioritesEtFeuxPietons()
 { // Route est-ouest (k voies) entre deux trottoirs, passage piétons :
   // Priorités piétons (non modale) et fiche Feux de piétons (non modale)
  QVERIFY(!OuvreDemo("lot1_pietons.cvw").isEmpty());
  int x=-1, y=-1, k=0;
  for(int i=1; (x<0)&&(i<cv->NbX-1); i++)
   for(int j=1; (x<0)&&(j<cv->NbY-4); j++)
    for(int l=1; (x<0)&&(l<=3); l++)
     {
      bool Ok=CaseLibre(i, j-1)&&CaseLibre(i, j+l);
      for(int m=0; Ok&&(m<l); m++)
       {
        voie &v=cv->v[i][j+m];
        Ok=v.DirPoss&&!(v.DirPoss&(N|S))&&!v.SensVoieTram&&!v.NumParking&&!v.EstDepotBus()&&!v.NumArretBus&&
           !v.Priorite[0]&&!v.Priorite[1]&&!v.Priorite[2]&&!v.Priorite[3];
       }
      if (Ok) { x=i; y=j; k=l; }
     }
  QVERIFY(x>=0);
  int yn=y-1, ys=y+k; // trottoirs nord et sud
  Focalise();
  // Trottoirs (Nature > Trottoir), passage piétons sur chaque voie
  for(int j : {yn, ys})
   {
    PlaceCurseur(x, j);
    QVERIFY(frmSimulation->ActionNatureTrottoir->Enabled);
    frmSimulation->ActionNatureTrottoir->Execute();
    QCOMPARE((int)NATURE(&cv->v[x][j]), (int)trottoirs);
   }
  for(int j=y; j<ys; j++)
   {
    PlaceCurseur(x, j);
    QVERIFY(frmSimulation->ActionPassagePietons->Enabled);
    frmSimulation->ActionPassagePietons->Execute();
    QVERIFY(cv->v[x][j].PassagePietons);
   }
  // Priorités piétons sur le trottoir nord : feu vers le sud possible
  PlaceCurseur(x, yn);
  QVERIFY(frmSimulation->ActionPrioritesPietons->Enabled);
  frmSimulation->MenuItemPrioritesPietons->ElementQt()->trigger();
  QVERIFY(frmDefVoiePietons->isVisible());
  QVERIFY(frmSimulation->ActionPrioritesPietons->Checked);
  QCOMPARE(Q(frmDefVoiePietons->LabelCoordonnees->Caption), QString::asprintf("%02d, %02d", x, yn));
  QVERIFY(frmDefVoiePietons->SpeedButtonS->Enabled);
  QVERIFY(!frmDefVoiePietons->SpeedButtonN->Enabled);
  QVERIFY(!frmDefVoiePietons->SpeedButtonS->Down);
  QVERIFY(frmDefVoiePietons->ImageSS->Visible);
  QVERIFY(!frmDefVoiePietons->ImageNN->Visible);
  QVERIFY(!frmDefVoiePietons->SpeedButtonS->icon().isNull());
  CaptureFiche(frmDefVoiePietons, "fiche_priorites_pietons.png");
  int NbFeuxP=NbFeuxPietonsDefinis();
  // Clic sur le feu sud : feu créé (et son opposé), fiche Feux de piétons ; puis Annuler partout
  Clique(frmDefVoiePietons->SpeedButtonS);
  QVERIFY(frmDefVoiePietons->SpeedButtonS->Down);
  QVERIFY(frmDefFeuxPietons->isVisible());
  int n=cv->v[x][yn].NumFeuP[sud-1];
  QVERIFY(n>0);
  QCOMPARE((int)cv->v[x][ys].NumFeuP[nord-1], n);
  QCOMPARE(Q(frmDefFeuxPietons->LabelCoordonneesFeuPietonsOppose->Caption), QString::asprintf("%02d, %02d", x, ys));
  QCOMPARE(Q(frmDefFeuxPietons->LabelDureeTraversee->Caption), QString::number(cv->FeuP[n-1].DureeTraversee));
  QCOMPARE(NbFeuxPietonsDefinis(), NbFeuxP+1);
  Clique(frmDefFeuxPietons->ButtonAnnuler);
  QVERIFY(!frmDefFeuxPietons->isVisible());
  QVERIFY(frmDefVoiePietons->SpeedButtonS->Down); // le feu existe tant que la priorité n'est pas annulée
  Clique(frmDefVoiePietons->ButtonAnnuler);
  QVERIFY(!frmDefVoiePietons->isVisible());
  QVERIFY(!frmSimulation->ActionPrioritesPietons->Checked);
  QCOMPARE((int)cv->v[x][yn].NumFeuP[sud-1], 0);
  QCOMPARE((int)cv->v[x][ys].NumFeuP[nord-1], 0);
  QCOMPARE(NbFeuxPietonsDefinis(), NbFeuxP);
  // Même chose, état vert et durée du vert, OK partout
  frmSimulation->MenuItemPrioritesPietons->ElementQt()->trigger();
  QVERIFY(frmDefVoiePietons->isVisible());
  Clique(frmDefVoiePietons->SpeedButtonS);
  QVERIFY(frmDefFeuxPietons->isVisible());
  n=cv->v[x][yn].NumFeuP[sud-1];
  QVERIFY(n>0);
  CliqueChoix(frmDefFeuxPietons->RadioGroupEtat, 1); // Vert
  frmDefFeuxPietons->CSpinEditDureeVert->Value=6;
  QVERIFY(frmDefFeuxPietons->ButtonAppliquer->Enabled);
  CaptureFiche(frmDefFeuxPietons, "fiche_feux_pietons.png");
  Messages.clear();
  Clique(frmDefFeuxPietons->ButtonOk); // assistant (aucun feu de circulation lié) : « Non »
  QVERIFY(!frmDefFeuxPietons->isVisible());
  QCOMPARE(Messages.size(), 1);
  QVERIFY2(Messages.first().contains(QString::fromUtf8("Voulez-vous affecter l'état opposé à tous les feux de circulation liés ?")), qPrintable(Messages.first()));
  QCOMPARE((int)cv->FeuP[n-1].Etat, (int)vert);
  QCOMPARE(cv->FeuP[n-1].DureeVert, 6);
  Clique(frmDefVoiePietons->ButtonOk);
  QVERIFY(!frmDefVoiePietons->isVisible());
  QCOMPARE((int)cv->v[x][yn].NumFeuP[sud-1], n);
  QCOMPARE((int)cv->v[x][ys].NumFeuP[nord-1], n);
  QCOMPARE(NbFeuxPietonsDefinis(), NbFeuxP+1);
 }
//---------------------------------------------------------------------------
void TestInterface::fichesParametres()
 { // Bouton Paramètres (modale) et ses fiches filles modales (bus, tram, taxis,
   // vehlibs), après avoir posé un arrêt de bus, un arrêt de tram, une place de
   // taxi et une place de vehlib ; puis Annuler, enregistrer et rouvrir
  QString Fichier=OuvreDemo("lot1_parametres.cvw");
  QVERIFY(!Fichier.isEmpty());
  Focalise();
  // Arrêt de bus et places de taxi et de vehlib sur trois cases de route
  int xb=-1, yb=-1;
  for(int i=0; (xb<0)&&(i<cv->NbX); i++)
   for(int j=0; (xb<0)&&(j<cv->NbY); j++)
    if (cv->AjoutArretBusPossible(i, j)&&!cv->v[i][j].NumParking&&!cv->v[i][j].EstDepotBus()) { xb=i; yb=j; }
  QVERIFY(xb>=0);
  PlaceCurseur(xb, yb);
  QVERIFY(frmSimulation->ActionArretBusTram->Enabled);
  frmSimulation->ActionArretBusTram->Execute();
  int NumArretBus=cv->v[xb][yb].NumArretBus;
  QVERIFY(NumArretBus>0);
  std::vector<std::pair<int, int>> Routes;
  for(int i=0; (Routes.size()<2)&&(i<cv->NbX); i++)
   for(int j=0; (Routes.size()<2)&&(j<cv->NbY); j++)
    if (cv->v[i][j].DirPoss&&!cv->v[i][j].NumParking&&!cv->v[i][j].EstDepotBus()&&!cv->v[i][j].NumArretBus&&
        ((i!=xb)||(j!=yb))&&(Routes.empty()||(Routes[0]!=std::make_pair(i, j))))
     Routes.push_back({i, j});
  QCOMPARE((int)Routes.size(), 2);
  PlaceCurseur(Routes[0].first, Routes[0].second);
  QVERIFY(frmSimulation->ActionPlaceTaxi->Enabled);
  frmSimulation->ActionPlaceTaxi->Execute();
  QVERIFY(cv->NbPlacesTaxi>0);
  PlaceCurseur(Routes[1].first, Routes[1].second);
  QVERIFY(frmSimulation->ActionPlaceVehlib->Enabled);
  frmSimulation->ActionPlaceVehlib->Execute();
  QVERIFY(cv->NbPlacesVehlib>0);
  // Voie de tram d'une case (de l'ouest vers l'est) dans une zone libre, et son arrêt
  int xt, yt;
  QVERIFY(ZoneLibre(3, 1, xt, yt));
  xt++;
  PlaceCurseur(xt-1, yt);
  PlaceCurseur(xt, yt);
  QVERIFY(frmSimulation->ActionDirectionDroiteTram->Enabled);
  frmSimulation->ActionDirectionDroiteTram->Execute();
  QVERIFY(cv->v[xt][yt].SensVoieTram);
  PlaceCurseur(xt, yt);
  QVERIFY(frmSimulation->ActionArretBusTram->Enabled);
  frmSimulation->ActionArretBusTram->Execute();
  int NumArretTram=cv->v[xt][yt].NumArretTram;
  QVERIFY(NumArretTram>0);
  // Paramètres : durées des feux, vitesses, piétons ; puis chaque fiche fille
  int DureeVert=cv->DureeVert, DureeOrange=cv->DureeOrange, NbTours=cv->NbToursParSeconde,
      ProbaPsgPt=cv->ProbaDirPsgPt, NbMaxBus=cv->NbMaxPersonnesBus, NbMaxTram=cv->NbMaxPersonnesTram,
      NbMaxTaxi=cv->NbMaxPersonnesTaxi, TauxVehlib=cv->TauxRemplissageVehlib;
  QVERIFY(cv->NbVitesses!=3);
  bool Affichage=false, CacheeBus=false, Revenue=false, CacheeTram=false, CacheeTaxi=false, CacheeVehlib=false;
  QuandModale([]() { return frmParamSimul; }, [&]()
   {
    Affichage=Q(frmParamSimul->Caption).endsWith(QString(" [%1x%2]").arg(cv->NbX).arg(cv->NbY))&&
              frmParamSimul->BitBtnDefLignesBus->Enabled&&frmParamSimul->BitBtnDefLignesTram->Enabled&&
              frmParamSimul->BitBtnDefTaxis->Enabled&&frmParamSimul->BitBtnDefVehlibs->Enabled&&
              !frmParamSimul->ButtonAppliquer->Enabled&&(frmParamSimul->CSpinEditDureeVert->Value==DureeVert)&&
              !frmParamSimul->BitBtnDefLignesBus->icon().isNull();
    // Saisie perdue au retour des fiches filles : la fiche réaffichée repasse
    // par FormShow, qui recharge les valeurs du moteur (comme dans la VCL)
    frmParamSimul->CSpinEditDureeVert->Value=DureeVert+1;
    QVERIFY(frmParamSimul->ButtonAppliquer->Enabled);
    Clique(frmParamSimul->BitBtnDefLignesBus);
   });
  QuandModale([]() { return frmParamBus; }, [&]()
   { // Ligne 1 : un arrêt, le n° NumArretBus
    CacheeBus=!frmParamSimul->isVisible();
    frmParamBus->CSpinEdit1->Value=1;
    frmParamBus->CSpinEdit11->Value=NumArretBus;
    frmParamBus->CSpinEditNbMaxPersonnesBus->Value=NbMaxBus-10;
    CaptureFiche(frmParamBus, "fiche_parametres_bus.png");
    Clique(frmParamBus->ButtonOk);
   });
  QuandModale([]() { return frmParamSimul; }, [&]()
   {
    Revenue=frmParamSimul->isVisible()&&(frmParamSimul->CSpinEditDureeVert->Value==DureeVert)&&!frmParamBus;
    Clique(frmParamSimul->BitBtnDefLignesTram);
   });
  QuandModale([]() { return frmParamTram; }, [&]()
   {
    CacheeTram=!frmParamSimul->isVisible();
    frmParamTram->CSpinEdit1->Value=1;
    frmParamTram->CSpinEdit11->Value=NumArretTram;
    frmParamTram->CSpinEditNbMaxPersonnesTram->Value=NbMaxTram-20;
    CaptureFiche(frmParamTram, "fiche_parametres_tram.png");
    Clique(frmParamTram->ButtonOk);
   });
  QuandModale([]() { return frmParamSimul; }, [&]() { Clique(frmParamSimul->BitBtnDefTaxis); });
  QuandModale([]() { return frmParamTaxi; }, [&]()
   {
    CacheeTaxi=!frmParamSimul->isVisible();
    frmParamTaxi->CSpinEditNbMaxPersonnesTaxi->Value=NbMaxTaxi+1;
    CaptureFiche(frmParamTaxi, "fiche_parametres_taxis.png");
    Clique(frmParamTaxi->ButtonOk);
   });
  QuandModale([]() { return frmParamSimul; }, [&]() { Clique(frmParamSimul->BitBtnDefVehlibs); });
  QuandModale([]() { return frmParamVehlib; }, [&]()
   {
    CacheeVehlib=!frmParamSimul->isVisible();
    frmParamVehlib->CSpinEditTauxRemplissage->Value=TauxVehlib-20;
    CaptureFiche(frmParamVehlib, "fiche_parametres_vehlibs.png");
    Clique(frmParamVehlib->ButtonOk);
   });
  QuandModale([]() { return frmParamSimul; }, [&]()
   {
    frmParamSimul->CSpinEditDureeVert->Value=DureeVert+2;
    frmParamSimul->CSpinEditDureeOrange->Value=DureeOrange+1;
    frmParamSimul->CSpinEditNbToursParSeconde->Value=NbTours-9;
    frmParamSimul->CSpinEditProbaPsgPt->Value=ProbaPsgPt-10;
    CliqueChoix(frmParamSimul->RadioGroupVitesses, 2);
    CaptureFiche(frmParamSimul, "fiche_parametres.png");
    Clique(frmParamSimul->ButtonOk);
   });
  Clique(frmSimulation->ToolButtonParametres);
  QVERIFY2(Etapes.empty(), qPrintable(Messages.join('\n')));
  QVERIFY(!frmParamSimul);
  QVERIFY(Affichage);
  QVERIFY(CacheeBus&&CacheeTram&&CacheeTaxi&&CacheeVehlib);
  QVERIFY(Revenue);
  QCOMPARE(cv->DureeVert, DureeVert+2);
  QCOMPARE(cv->DureeOrange, DureeOrange+1);
  QCOMPARE(cv->NbToursParSeconde, NbTours-9);
  QCOMPARE(cv->ProbaDirPsgPt, ProbaPsgPt-10);
  QCOMPARE(cv->NbVitesses, 3);
  QCOMPARE(cv->LigneBus[0].NbArrets, 1);
  QCOMPARE(cv->LigneBus[0].NumArret[0], NumArretBus);
  QCOMPARE(cv->NbLignesBus, 1);
  QCOMPARE(cv->NbMaxPersonnesBus, NbMaxBus-10);
  QCOMPARE(cv->LigneTram[0].NbArrets, 1);
  QCOMPARE(cv->LigneTram[0].NumArret[0], NumArretTram);
  QCOMPARE(cv->NbMaxPersonnesTram, NbMaxTram-20);
  QCOMPARE(cv->NbMaxPersonnesTaxi, NbMaxTaxi+1);
  QCOMPARE(cv->TauxRemplissageVehlib, TauxVehlib-20);
  QVERIFY(frmSimulation->SimulationModifiee);
  // Annuler (et Échap dans une fiche fille) : rien ne change
  QuandModale([]() { return frmParamSimul; }, [&]()
   {
    frmParamSimul->CSpinEditDureeVert->Value=DureeVert+5;
    Clique(frmParamSimul->BitBtnDefTaxis);
   });
  QuandModale([]() { return frmParamTaxi; }, [&]()
   {
    frmParamTaxi->CSpinEditNbMaxPersonnesTaxi->Value=NbMaxTaxi+3;
    Touche(frmParamTaxi->CSpinEditNbMaxPersonnesTaxi, Qt::Key_Escape);
   });
  QuandModale([]() { return frmParamSimul; }, [&]() { Clique(frmParamSimul->ButtonAnnuler); });
  frmSimulation->MenuItemParametres->ElementQt()->trigger();
  QVERIFY(Etapes.empty());
  QCOMPARE(cv->DureeVert, DureeVert+2);
  QCOMPARE(cv->NbMaxPersonnesTaxi, NbMaxTaxi+1);
  // Enregistrer, rouvrir : valeurs conservées, et affichées par la fiche
  Enregistre(Fichier);
  QCOMPARE(Q(frmSimulation->StatusBar->Panels->Items[0]->Text), QString::fromUtf8("Enregistrement effectué."));
  QVERIFY(!OuvreDemo("lot1_autre.cvw").isEmpty());
  QCOMPARE(cv->DureeVert, DureeVert);
  OuvreFichier(Fichier);
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+QDir::toNativeSeparators(Fichier));
  QCOMPARE(cv->DureeVert, DureeVert+2);
  QCOMPARE(cv->DureeOrange, DureeOrange+1);
  QCOMPARE(cv->NbToursParSeconde, NbTours-9);
  QCOMPARE(cv->ProbaDirPsgPt, ProbaPsgPt-10);
  QCOMPARE(cv->NbVitesses, 3);
  QCOMPARE(cv->LigneBus[0].NumArret[0], NumArretBus);
  QCOMPARE(cv->NbMaxPersonnesBus, NbMaxBus-10);
  QCOMPARE(cv->LigneTram[0].NumArret[0], NumArretTram);
  QCOMPARE(cv->NbMaxPersonnesTram, NbMaxTram-20);
  QCOMPARE(cv->NbMaxPersonnesTaxi, NbMaxTaxi+1);
  QCOMPARE(cv->TauxRemplissageVehlib, TauxVehlib-20);
  bool Relu=false, ReluBus=false;
  QuandModale([]() { return frmParamSimul; }, [&]()
   {
    Relu=(frmParamSimul->CSpinEditDureeVert->Value==DureeVert+2)&&(frmParamSimul->CSpinEditDureeOrange->Value==DureeOrange+1)&&
         (frmParamSimul->CSpinEditNbToursParSeconde->Value==NbTours-9)&&(frmParamSimul->CSpinEditProbaPsgPt->Value==ProbaPsgPt-10)&&
         (frmParamSimul->RadioGroupVitesses->ItemIndex==2)&&!frmParamSimul->ButtonAppliquer->Enabled;
    Clique(frmParamSimul->BitBtnDefLignesBus);
   });
  QuandModale([]() { return frmParamBus; }, [&]()
   {
    ReluBus=(frmParamBus->CSpinEdit1->Value==1)&&(frmParamBus->CSpinEdit11->Value==NumArretBus)&&
            (frmParamBus->CSpinEditNbMaxPersonnesBus->Value==NbMaxBus-10)&&(Q(frmParamBus->LabelNbBus->Caption)=="1")&&
            !frmParamBus->ButtonAppliquer->Enabled;
    Clique(frmParamBus->ButtonAnnuler);
   });
  QuandModale([]() { return frmParamSimul; }, [&]() { Clique(frmParamSimul->ButtonAnnuler); });
  Clique(frmSimulation->ToolButtonParametres);
  QVERIFY(Etapes.empty());
  QVERIFY(Relu);
  QVERIFY(ReluBus);
 }
//---------------------------------------------------------------------------
void TestInterface::ficheCarrefours()
 { // Sélection de 4 x 4 cases libres, Carrefours (modale) : carrefour à feux ;
   // Annuler ; enregistrer et rouvrir
  QString Fichier=OuvreDemo("lot1_carrefours.cvw");
  QVERIFY(!Fichier.isEmpty());
  int xd, yd, xa, ya;
  QVERIFY(ZoneLibre(4, 4, xd, yd));
  QVERIFY(ZoneLibre(4, 4, xa, ya, {QRect(xd-1, yd-1, 6, 6)}));
  Focalise();
  Selectionne(xd, yd, xd+3, yd+3);
  QVERIFY(frmSimulation->ActionCarrefours->Enabled);
  int NbFeux=0;
  for(int i=0; i<cv->NbFeux; i++) NbFeux+=cv->Feu[i].Defini();
  bool Active=false;
  ReponseQuestions=QMessageBox::Yes; // « Êtes-vous bien sûr(e) de vouloir effacer... »
  Messages.clear();
  QuandModale([]() { return frmCarrefours; }, [&]()
   {
    Active=frmCarrefours->GroupBox44->isEnabled()&&frmCarrefours->SpeedButton44a->isEnabled()&&
           frmCarrefours->SpeedButton44b->isEnabled()&&!frmCarrefours->GroupBox34->isEnabled()&&
           !frmCarrefours->SpeedButton34a1->isEnabled()&&!frmCarrefours->SpeedButton88->isEnabled()&&
           !frmCarrefours->SpeedButton44b->icon().isNull();
    CaptureFiche(frmCarrefours, "fiche_carrefours.png");
    Clique(frmCarrefours->SpeedButton44b); // 4x4 à feux (Tag 4402)
   });
  Clique(frmSimulation->ToolButtonCarrefours);
  ReponseQuestions=QMessageBox::No;
  QVERIFY2(Etapes.empty(), qPrintable(Messages.join('\n')));
  QVERIFY(Active);
  QVERIFY(!frmCarrefours->isVisible());
  QCOMPARE(Messages.size(), 1);
  QVERIFY2(Messages.first().contains(QString::fromUtf8("Êtes-vous bien sûr(e) de vouloir effacer")), qPrintable(Messages.first()));
  auto VerifieCarrefour=[&]()
   {
    for(int i=0; i<4; i++)
     for(int j=0; j<4; j++)
      QCOMPARE((int)cv->v[xd+i][yd+j].DirPoss, (int)DirPossC4x4[i][j]);
    const int xf[NBDIR]={xd+1, xd+3, xd+2, xd}, yf[NBDIR]={yd, yd+1, yd+3, yd+2};
    const direction df[NBDIR]={sud, ouest, nord, est};
    int NumFeux[NBDIR];
    for(int i=0; i<NBDIR; i++)
     {
      QCOMPARE((int)cv->v[xf[i]][yf[i]].Priorite[df[i]-1], (int)feux);
      QVERIFY((NumFeux[i]=cv->v[xf[i]][yf[i]].NumFeu[df[i]-1])>0);
     }
    for(int i=0; i<NBDIR; i++)
     {
      feu &f=cv->Feu[NumFeux[i]-1];
      QCOMPARE((int)f.Etat, 1+2*((1+2+i)%2)); // Type=2
      QCOMPARE(f.NbFeuxDmd, 2);
      QCOMPARE(f.NbFeuxAtt, 2);
      QCOMPARE(f.NumFeuxDmd[0], NumFeux[(i+1)%NBDIR]);
      QCOMPARE(f.NumFeuxDmd[1], NumFeux[(i+3)%NBDIR]);
     }
    int NbFeuxApres=0;
    for(int i=0; i<cv->NbFeux; i++) NbFeuxApres+=cv->Feu[i].Defini();
    QCOMPARE(NbFeuxApres, NbFeux+4);
   };
  VerifieCarrefour();
  if (QTest::currentTestFailed()) return;
  QVERIFY(frmSimulation->SimulationModifiee);
  // Annuler sur une autre zone : rien ne change
  Selectionne(xa, ya, xa+3, ya+3);
  QuandModale([]() { return frmCarrefours; }, [&]() { Clique(frmCarrefours->ButtonAnnuler); });
  frmSimulation->MenuItemCarrefours->ElementQt()->trigger();
  QVERIFY(Etapes.empty());
  for(int i=0; i<4; i++)
   for(int j=0; j<4; j++)
    QVERIFY(CaseLibre(xa+i, ya+j));
  // Enregistrer, rouvrir : carrefour et feux conservés
  Enregistre(Fichier);
  QVERIFY(!OuvreDemo("lot1_autre.cvw").isEmpty());
  QVERIFY(CaseLibre(xd+1, yd+1));
  OuvreFichier(Fichier);
  QCOMPARE(Q(frmSimulation->Caption), QString("Centre-Ville - ")+QDir::toNativeSeparators(Fichier));
  VerifieCarrefour();
 }
//---------------------------------------------------------------------------
// Lot 2 des fiches secondaires : distances et informations
//---------------------------------------------------------------------------
namespace
 {
  // asDir1 des fiches de distances : trois espaces et la flèche de la police
  // Symbol (0xAD haut, 0xAE droite, 0xAF bas, 0xAC gauche)
  QString Fleche(direction d)
   {
    const char16_t Symboles[NBDIR+1]={0, 0xAD, 0xAE, 0xAF, 0xAC};
    return d?QString("   ")+QChar(Symboles[d]):QString();
   }
  //-------------------------------------------------------------------------
  // Tableau attendu d'une fiche de distances pour une case, d'après le moteur
  struct TableDistances
   {
    QString Titre;
    int Nb=0, NbDir=0;
    bool Definie=false;
    std::function<direction(int)> Dir; // direction de la colonne d+1
    std::function<int(int, int)> Distance; // ligne k+1, colonne d+1
   };
  struct FicheDistances
   {
    const char *Nom; // capture fiche_distances_<Nom>.png
    TForm *Fiche;
    TStringGrid *Grille;
    TAction *Action;
    TCheckBox *Couloirs; // case « Avec couloirs de bus » (parkings, files de parkings)
    std::function<TableDistances(voie *, bool)> Table;
    std::function<void()> Ouvre; // menu, raccourci ou action
   };
  TableDistances TableRoute(voie *v)
   { // Colonnes : directions possibles de la case (Dirdp)
    TableDistances t;
    t.NbDir=NbDir[v->DirPoss];
    t.Dir=[v](int d) { return Dirdp[d][v->DirPoss]; };
    return t;
   }
  //-------------------------------------------------------------------------
  // Différences entre deux textes, pour les fiches d'informations
  void Compare(QStringList &Ecarts, const char *Nom, const AnsiString &asObtenu, const QString &Attendu)
   {
    if (Q(asObtenu)!=Attendu) Ecarts<<QString::fromUtf8("%1 : « %2 » au lieu de « %3 »").arg(QString::fromUtf8(Nom), Q(asObtenu), Attendu);
   }
  QString NumeroCoordonnees(int n, int x, int y) { return QString::asprintf("%d (%02d, %02d)", n, x, y); }
  QString Cadre(int n, int x, int y) { return QString(27, ' ')+NumeroCoordonnees(n, x, y)+" "; }
  // Pourcentages (FormatFloat « #.000% » ou « 0.000% » de la VCL)
  const QRegularExpression Pourcentage("^(-?\\d*,\\d{3}%|NAN|-?INF)$");
 }
//---------------------------------------------------------------------------
void TestInterface::VerifieFichesDistances(const QString &asCarte, bool Captures)
 { // Sur une copie de la carte, en simulation : chaque fiche de distances ouverte
   // sur une case de route (de voie de tram pour les arrêts de tram), puis sur une
   // autre case cliquée ; tableau = distances du moteur, hauteur de la fiche
   // selon le nombre de lignes (FormShow), fermeture par la case de la fenêtre
  QString Copie=OuvreCarte(asCarte, "lot2_distances_"+QFileInfo(asCarte).completeBaseName()+".cvw");
  QVERIFY(!Copie.isEmpty());
  Focalise();
  QVERIFY2(Demarre(), qPrintable(Messages.join('\n')));
  QVERIFY(AttendTour(2, 30000));
  TfrmSimulation *s=frmSimulation;
  std::vector<FicheDistances> Fiches=
   {
    {"parkings", frmAffDistParkings, frmAffDistParkings->StringGridDistances, s->ActionDistancesParkings,
     frmAffDistParkings->CheckBoxCouloirsBus,
     [](voie *v, bool c)
      {
       TableDistances t=TableRoute(v);
       t.Titre="Parkings"; t.Nb=cv->NbParkings; t.Definie=v->DistParking;
       t.Distance=[v, c](int k, int d) { return v->DistParking[k][d][c]; };
       return t;
      },
     [s]() { s->MenuItemInfosDistancesEtapes->ElementQt()->trigger(); }},
    {"arrets_bus", frmAffDistArretsBus, frmAffDistArretsBus->StringGridDistances, s->ActionDistancesArretsBus, nullptr,
     [](voie *v, bool)
      {
       TableDistances t=TableRoute(v);
       t.Titre=QString::fromUtf8("Arrêts bus"); t.Nb=cv->NbArretsBus; t.Definie=v->DistArretBus;
       t.Distance=[v](int k, int d) { return v->DistArretBus[k][d]; };
       return t;
      },
     [s]() { s->MenuItemDistancesArretsBus->ElementQt()->trigger(); }}, // menu contextuel de simulation
    {"arrets_tram", frmAffDistArretsTram, frmAffDistArretsTram->StringGridDistances, s->ActionDistancesArretsTram, nullptr,
     [](voie *v, bool)
      {
       TableDistances t;
       t.Titre=QString::fromUtf8("Arrêts tram"); t.Nb=cv->NbArretsTram; t.Definie=v->DistArretTram;
       t.NbDir=v->NbDirPossSensVoieTram(indefinie);
       t.Dir=[v](int d) { return v->DirSensVoieTram(indefinie, d); };
       t.Distance=[v](int k, int d) { return v->DistArretTram[k][d]; };
       return t;
      },
     [this]() { QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_R, Qt::ControlModifier); }},
    {"files_taxi", frmAffDistFilesTaxi, frmAffDistFilesTaxi->StringGridDistances, s->ActionDistancesFilesTaxi, nullptr,
     [](voie *v, bool)
      {
       TableDistances t=TableRoute(v);
       t.Titre="Files taxi"; t.Nb=cv->NbFilesTaxi; t.Definie=v->DistFileTaxi;
       t.Distance=[v](int k, int d) { return v->DistFileTaxi[k][d]; };
       return t;
      },
     [s]() { s->MenuItemInfosDistancesFilesTaxi->ElementQt()->trigger(); }},
    {"files_parkings", frmAffDistFilesPark, frmAffDistFilesPark->StringGridDistances, s->ActionDistancesFilesPark,
     frmAffDistFilesPark->CheckBoxCouloirsBus,
     [](voie *v, bool c)
      {
       TableDistances t=TableRoute(v);
       t.Titre="Files parking"; t.Nb=cv->NbFilesPark; t.Definie=v->DistFilePark;
       t.Distance=[v, c](int k, int d) { return v->DistFilePark[k][d][c]; };
       return t;
      },
     [this]() { QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_K, Qt::ControlModifier); }},
    {"places_vehlib", frmAffDistPlacesVehlib, frmAffDistPlacesVehlib->StringGridDistances, s->ActionDistancesPlacesVehlib, nullptr,
     [](voie *v, bool)
      {
       TableDistances t=TableRoute(v);
       t.Titre="Places vehlib"; t.Nb=cv->NbPlacesVehlib; t.Definie=v->DistPlaceVehlib;
       t.Distance=[v](int k, int d) { return v->DistPlaceVehlib[k][d]; };
       return t;
      },
     [s]() { s->ActionDistancesPlacesVehlib->Execute(); }},
   };
  // Tableau affiché pour la case courante
  auto Verifie=[](const FicheDistances &f) -> QString
   {
    int x=frmSimulation->DrawGridSimul->Col, y=frmSimulation->DrawGridSimul->Row;
    voie *v=&cv->v[x][y];
    TableDistances t=f.Table(v, f.Couloirs&&f.Couloirs->Checked);
    TStringGrid *g=f.Grille;
    QStringList e;
    if (!t.Definie)
     { // EffaceInfos seul
      for(int c=0; c<g->ColCount; c++)
       for(int r=0; r<g->RowCount; r++)
        if (!Q(g->Cells[c][r]).isEmpty()) e<<QString("case [%1][%2] non vide").arg(c).arg(r);
      return e.join('\n');
     }
    Compare(e, "titre", f.Fiche->Caption, QString("%1 (%2, %3)").arg(t.Titre).arg(x).arg(y));
    if (g->RowCount!=1+t.Nb) e<<QString("RowCount %1 au lieu de %2").arg((int)g->RowCount).arg(1+t.Nb);
    Compare(e, "[0][0]", g->Cells[0][0], QString());
    for(int d=0; d<t.NbDir; d++) Compare(e, "en-tête", g->Cells[d+1][0], Fleche(t.Dir(d)));
    for(int k=1; k<=t.Nb; k++)
     {
      Compare(e, "numéro", g->Cells[0][k], QString::asprintf("%5d", k));
      for(int d=0; d<t.NbDir; d++)
       {
        int de=t.Distance(k-1, d);
        Compare(e, "distance", g->Cells[d+1][k], (de==DIST_INDEFINIE)?QString("     "):QString::asprintf("%5d", de));
       }
      for(int c=t.NbDir+1; c<g->ColCount; c++) Compare(e, "colonne vide", g->Cells[c][k], QString());
     }
    return e.join('\n');
   };
  // Case de route à au moins deux directions (de voie de tram pour les arrêts de
  // tram) où le tableau est défini, dans la partie visible de la carte, hors de (xh, yh)
  auto CaseDe=[](const FicheDistances &f, int xh, int yh, int &xc, int &yc)
   {
    int Meilleur=-1;
    for(int y=1; y<22; y++)
     for(int x=1; x<34; x++)
      {
       voie *v=&cv->v[x][y];
       if (((x==xh)&&(y==yh))||v->NumParking) continue;
       TableDistances t=f.Table(v, false);
       if (!(f.Fiche==frmAffDistArretsTram?(v->SensVoieTram>aucune_voie):(v->DirPoss>aucune))) continue;
       int Note=2*t.Definie+(t.NbDir>=2);
       if (Note>Meilleur) { Meilleur=Note; xc=x; yc=y; }
      }
    return Meilleur>=0;
   };
  for(const FicheDistances &f : Fiches)
   {
    int x=0, y=0, x2=0, y2=0;
    if ((f.Fiche==frmAffDistArretsTram)&&!cv->NbArretsTram) continue; // pas de voie de tram (Démo)
    QVERIFY2(CaseDe(f, -1, -1, x, y), f.Nom);
    QVERIFY2(CaseDe(f, x, y, x2, y2), f.Nom);
    Focalise();
    PlaceCurseur(x, y);
    QVERIFY2(f.Action->Enabled, f.Nom);
    QVERIFY(!f.Fiche->isVisible());
    f.Ouvre();
    QVERIFY2(f.Fiche->isVisible(), f.Nom);
    QVERIFY(f.Action->Checked);
    QVERIFY(!f.Fiche->isModal()&&!QApplication::activeModalWidget());
    QVERIFY(f.Fiche->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    QString e=Verifie(f);
    QVERIFY2(e.isEmpty(), qPrintable(QString("%1 (%2, %3) :\n%4").arg(f.Nom).arg(x).arg(y).arg(e)));
    // FormShow : hauteur de la fiche (cadre compris) d'après le nombre de lignes,
    // grille de RowCount lignes de 20 pixels et un trait, sans défilement jusqu'à 10
    TableDistances t=f.Table(&cv->v[x][y], false);
    // (seule f_afdistp compte le panneau de sa case à cocher : 43 et 300 ; f_afdistfp
    // garde 26 et 283, sa grille est donc plus courte de 17 pixels)
    int Panneau=(f.Fiche==frmAffDistParkings)?17:0, Lignes=1+t.Nb;
    int Hauteur=(t.Nb>10)?283+Panneau:26+Panneau+21*Lignes;
    QCOMPARE((int)f.Fiche->Height, Hauteur);
    QCOMPARE(f.Fiche->height(), Hauteur-22);
    QCOMPARE(f.Fiche->width(), (f.Fiche==frmAffDistArretsTram)?156:125); // Constraints 172 / 141, cadre compris
    int Grille=Hauteur-22-(f.Couloirs?17:0)-4; // zone cliente, case à cocher, bord de la grille
    QCOMPARE(f.Grille->viewport()->height(), Grille);
    if (t.Nb>0)
     QCOMPARE(f.Grille->verticalScrollBar()->maximum()>0, 21*Lignes>Grille+1); // dernier trait coupé toléré
    else
     { // RowCount=1 : TCustomGrid::SetRowCount ramène FixedRows à 0, et pour la suite
      QCOMPARE(f.Grille->verticalScrollBar()->maximum(), 0);
      QCOMPARE((int)f.Grille->FixedRows, 0);
     }
    if (Captures) CaptureFiche(f.Fiche, QString("fiche_distances_%1.png").arg(f.Nom));
    if (t.Nb>10)
     { // Clavier (TCustomGrid::KeyDown) : la case courante descend d'une page, la
       // grille défile pour la garder visible ; Ctrl+Début revient en haut
      QCOMPARE((int)f.Grille->Row, 1);
      Touche(f.Grille, Qt::Key_PageDown);
      QVERIFY((int)f.Grille->Row>1);
      QVERIFY((int)f.Grille->TopRow>1);
      QCOMPARE(f.Grille->verticalScrollBar()->value(), (int)f.Grille->TopRow-1);
      QTest::keyClick(f.Grille, Qt::Key_Home, Qt::ControlModifier);
      QCOMPARE((int)f.Grille->Row, 1);
      QCOMPARE((int)f.Grille->TopRow, 1);
     }
    // Couloirs de bus : distances prioritaires
    if (f.Couloirs)
     {
      Clique(f.Couloirs);
      QVERIFY(f.Couloirs->Checked);
      e=Verifie(f);
      QVERIFY2(e.isEmpty(), qPrintable(QString("%1 couloirs :\n%2").arg(f.Nom, e)));
      Clique(f.Couloirs);
      QVERIFY(!f.Couloirs->Checked);
     }
    // Clic sur une autre case : le tableau la suit (DrawGridSimulClick)
    Focalise();
    CliqueCase(x2, y2);
    QCOMPARE((int)frmSimulation->DrawGridSimul->Col, x2);
    QCOMPARE((int)frmSimulation->DrawGridSimul->Row, y2);
    e=Verifie(f);
    QVERIFY2(e.isEmpty(), qPrintable(QString("%1 (%2, %3) :\n%4").arg(f.Nom).arg(x2).arg(y2).arg(e)));
    // Case de la fenêtre : fiche cachée, action décochée (FormHide)
    f.Fiche->Close();
    QVERIFY(!f.Fiche->isVisible());
    QVERIFY(!f.Action->Checked);
   }
  // Fiches ouvertes à l'arrêt : refermées (TimerSimulTimer, tdArret)
  for(const FicheDistances &f : Fiches)
   {
    int x=0, y=0;
    if ((f.Fiche==frmAffDistArretsTram)&&!cv->NbArretsTram) continue;
    QVERIFY(CaseDe(f, -1, -1, x, y));
    Focalise();
    PlaceCurseur(x, y);
    f.Action->Execute();
    QVERIFY2(f.Fiche->isVisible(), f.Nom);
   }
  QVERIFY(Arrete());
  for(const FicheDistances &f : Fiches)
   {
    QVERIFY2(!f.Fiche->isVisible(), f.Nom);
    QVERIFY(!f.Action->Checked);
   }
 }
//---------------------------------------------------------------------------
void TestInterface::fichesDistances()
 { // Géant (arrêts, files et places de chaque sorte, plus de 10 lignes pour les
   // arrêts et les places) puis Démo (4 parkings, aucun arrêt, file ni place :
   // tableaux vides, ce qui laisse ensuite ces grilles sans ligne fixe)
  VerifieFichesDistances(QString::fromUtf8("Géant.cvw"), true);
  if (QTest::currentTestFailed()) return;
  VerifieFichesDistances(QString::fromUtf8("Démo.cvw"), false);
 }
//---------------------------------------------------------------------------
void TestInterface::SuitObjet(const char *Type, std::function<QString(long)> Ecart, std::function<bool(int &, int &)> Position, int NbTours)
 { // À chaque tour, la fiche montre l'objet de la case courante (rafraîchissement
   // demandé par le moteur) ; si l'objet suivi s'est déplacé, l'utilisateur clique
   // sur sa nouvelle case et la fiche le retrouve
  for(int i=0; i<NbTours; i++)
   {
    long Tour=cv->TourCrt;
    QVERIFY(AttendTour(Tour+1, 10000));
    QString e=Ecart(cv->TourCrt-1); // rafraîchissement en fin de tour, avant TourCrt++
    QVERIFY2(e.isEmpty(), qPrintable(QString("%1, tour %2 :\n%3").arg(Type).arg(cv->TourCrt).arg(e)));
    int x, y;
    if (!Position(x, y)) break; // arrivé ou monté dans un véhicule
    if ((x!=frmSimulation->DrawGridSimul->Col)||(y!=frmSimulation->DrawGridSimul->Row))
     {
      Focalise();
      CliqueCase(x, y);
      e=Ecart(cv->TourCrt);
      QVERIFY2(e.isEmpty(), qPrintable(QString("%1, tour %2, après le clic sur (%3, %4) :\n%5").arg(Type).arg(cv->TourCrt).arg(x).arg(y).arg(e)));
     }
   }
 }
//---------------------------------------------------------------------------
void TestInterface::fichesInfos()
 { // Géant en simulation : chaque fiche d'informations ouverte par son menu, son
   // raccourci ou le menu contextuel sur un objet de la case courante, suivie
   // pendant quelques tours, puis fermée par « Vu » ou Échap
  QVERIFY(!OuvreCarte(QString::fromUtf8("Géant.cvw"), "lot2_infos.cvw").isEmpty());
  Focalise();
  QVERIFY2(Demarre(), qPrintable(Messages.join('\n')));
  QVERIFY(AttendTour(15, 30000));
  auto Case=[]() { return &cv->v[frmSimulation->DrawGridSimul->Col][frmSimulation->DrawGridSimul->Row]; };
  auto Coordonnees=[]() { return QString::asprintf("%02d, %02d", (int)frmSimulation->DrawGridSimul->Col, (int)frmSimulation->DrawGridSimul->Row); };
  // Premier objet trouvé sur la carte (Numero(v) > 0), case courante
  auto Trouve=[this](std::function<int(voie *)> Numero) -> int
   {
    for(int y=0; y<cv->NbY; y++)
     for(int x=0; x<cv->NbX; x++)
      if (int n=Numero(&cv->v[x][y]))
       {
        Focalise();
        CliqueCase(x, y);
        return n;
       }
    return 0;
   };
  //-------------------------------------------------------------------------
  // Véhicule (Ctrl+V)
  {
   int n=Trouve([](voie *v) { return (v->NumVehicule>0)&&!v->NumParking?v->NumVehicule:0; });
   QVERIFY(n>0);
   TfrmInfosVehicule *f=frmInfosVehicule;
   auto Ecart=[f, Case, Coordonnees](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     if (v->NumVehicule>0)
      {
       vehicule *Veh=cv->Vehicule+v->NumVehicule-1;
       const char *Priorites[]={"Normal", "Urgences", "Police"};
       if (!f->RadioButtonVehiculeNumero->Checked||f->LabelDirection->isHidden()) e<<QString::fromUtf8("véhicule non affiché");
       Compare(e, "cadre", f->GroupBoxVehicule->Caption, Cadre(v->NumVehicule, Veh->x, Veh->y));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[Veh->Dir]));
       Compare(e, "priorité", f->LabelNivPriorite->Caption, Priorites[Veh->NivPriorite]);
       Compare(e, "sorti au tour", f->LabelTourSortie->Caption, QString::number(Veh->TourDepart));
       Compare(e, "tours", f->LabelTours->Caption, QString::number(Tour-Veh->TourDepart));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(Veh->NbCasesParcourues));
       Compare(e, "vitesse", f->LabelVitesse->Caption, QString::asprintf("%d/%d", 1+(v->NumVehicule-1)%cv->NbVitesses, cv->NbVitesses));
       if (Veh->NumParkingArrivee)
        Compare(e, "parking d'arrivée", f->LabelParkingArrivee->Caption,
                NumeroCoordonnees(Veh->NumParkingArrivee, cv->Parking[Veh->NumParkingArrivee-1].x, cv->Parking[Veh->NumParkingArrivee-1].y));
       for(TLabel *l : {f->LabelParcoursEffectue, f->LabelDetour, f->LabelAttente})
        if (!Pourcentage.match(Q(l->Caption)).hasMatch()) e<<("pourcentage : "+Q(l->Caption));
      }
     else
      if (!f->RadioButtonAucunVehicule->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun véhicule » non affiché");
     return e.join('\n');
    };
   QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_V, Qt::ControlModifier);
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailVehicule->Checked);
   QVERIFY(!f->isModal()&&!QApplication::activeModalWidget());
   QVERIFY(f->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   QVERIFY(Q(f->GroupBoxVehicule->Caption).contains(QString::number(n)+" ("));
   CaptureFiche(f, "fiche_infos_vehicule.png");
   SuitObjet("véhicule", Ecart, [n](int &x, int &y)
    {
     vehicule *Veh=cv->Vehicule+n-1;
     x=Veh->x; y=Veh->y;
     return (x>=0)&&(y>=0)&&(cv->v[x][y].NumVehicule==n);
    }, 8);
   if (QTest::currentTestFailed()) return;
   Clique(f->ButtonOk); // « Vu »
   QVERIFY(!f->isVisible());
   QVERIFY(!frmSimulation->ActionDetailVehicule->Checked);
  }
  //-------------------------------------------------------------------------
  // Bus (menu Informations)
  {
   int n=Trouve([](voie *v) { return v->NumBus; });
   QVERIFY(n>0);
   TfrmInfosBus *f=frmInfosBus;
   auto Ecart=[f, Case, Coordonnees](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     if (v->NumBus>0)
      {
       bus *b=cv->Bus+v->NumBus-1;
       if (!f->RadioButtonBusNumero->Checked||f->LabelDirection->isHidden()) e<<"bus non affiché";
       Compare(e, "cadre", f->GroupBoxBus->Caption, Cadre(v->NumBus, b->x, b->y));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[b->Dir]));
       Compare(e, "ligne", f->LabelNumeroLigne->Caption, QString::number(b->NumLigne));
       Compare(e, "sorti au tour", f->LabelTourSortie->Caption, QString::number(b->TourDepart));
       Compare(e, "personnes", f->LabelNbPersonnes->Caption, QString::number(b->NbPersonnes));
       Compare(e, "tours", f->LabelTours->Caption, QString::number(Tour-b->TourDepart));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(b->NbCasesParcourues));
       Compare(e, "tours depuis l'arrêt", f->LabelToursEntreArrets->Caption, QString::number(Tour-b->TourArriveDrnArret));
       Compare(e, "parcours depuis l'arrêt", f->LabelParcoursEntreArrets->Caption, QString::number(b->NbCasesParcouruesDepuisDrnArret));
       Compare(e, "prochain arrêt", f->LabelProchainArret->Caption, // TourDepartArretSvt du premier bus (code d'origine)
               NumeroCoordonnees(b->NumArretSvt, cv->ArretBus[b->NumArretSvt-1].x, cv->ArretBus[b->NumArretSvt-1].y)+
               QString(" depuis le tour %1").arg(cv->Bus->TourDepartArretSvt));
       Compare(e, "vitesse", f->LabelVitesse->Caption, QString::asprintf("1/%d", cv->NbVitesses));
      }
     else
      if (!f->RadioButtonAucunBus->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun bus » non affiché");
     return e.join('\n');
    };
   frmSimulation->MenuItemInfosBus->ElementQt()->trigger();
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailBus->Checked);
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   CaptureFiche(f, "fiche_infos_bus.png");
   SuitObjet("bus", Ecart, [n](int &x, int &y)
    {
     x=cv->Bus[n-1].x; y=cv->Bus[n-1].y;
     return (x>=0)&&(y>=0)&&(cv->v[x][y].NumBus==n);
    }, 6);
   if (QTest::currentTestFailed()) return;
   Touche(f->ButtonOk, Qt::Key_Escape); // « Vu » est le bouton Cancel
   QVERIFY(!f->isVisible());
   QVERIFY(!frmSimulation->ActionDetailBus->Checked);
  }
  //-------------------------------------------------------------------------
  // Tram (menu contextuel de simulation)
  {
   int n=Trouve([](voie *v) { return v->NumTram; });
   QVERIFY(n>0);
   TfrmInfosTram *f=frmInfosTram;
   auto Ecart=[f, Case, Coordonnees](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     if (v->NumTram>0)
      {
       tram *t=cv->Tram+v->NumTram-1;
       if (!f->RadioButtonTramNumero->Checked||f->LabelDirection->isHidden()) e<<"tram non affiché";
       Compare(e, "cadre", f->GroupBoxTram->Caption, Cadre(v->NumTram, t->x, t->y));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[t->Dir]));
       Compare(e, "ligne", f->LabelNumeroLigne->Caption, QString(QChar('A'+t->NumLigne-1)));
       Compare(e, "personnes", f->LabelNbPersonnes->Caption, QString::number(t->NbPersonnes));
       Compare(e, "tours", f->LabelTours->Caption, QString::number(Tour)); // TourCrt seul (code d'origine)
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(t->NbCasesParcourues));
       Compare(e, "tours depuis l'arrêt", f->LabelToursEntreArrets->Caption, QString::number(Tour-t->TourArriveDrnArret));
       for(TLabel *l : {f->LabelParcoursEffectue, f->LabelDetour, f->LabelAttente})
        if (!Pourcentage.match(Q(l->Caption)).hasMatch()) e<<("pourcentage : "+Q(l->Caption));
      }
     else
      if (!f->RadioButtonAucunTram->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun tram » non affiché");
     return e.join('\n');
    };
   frmSimulation->MenuItemDetailTram->ElementQt()->trigger();
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailTram->Checked);
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   CaptureFiche(f, "fiche_infos_tram.png");
   SuitObjet("tram", Ecart, [n](int &x, int &y)
    {
     x=cv->Tram[n-1].x; y=cv->Tram[n-1].y;
     return (x>=0)&&(y>=0)&&(cv->v[x][y].NumTram==n);
    }, 6);
   if (QTest::currentTestFailed()) return;
   Clique(f->ButtonOk);
   QVERIFY(!f->isVisible());
   QVERIFY(!frmSimulation->ActionDetailTram->Checked);
  }
  //-------------------------------------------------------------------------
  // Taxi (Ctrl+I) et vehlib (Ctrl+E)
  const char *Situations[NBSITUATIONSTAXI]={"En attente", "Départ d'une course", "Dépôt de client(s)", "Retour d'une course"};
  {
   int n=Trouve([](voie *v) { return v->NumTaxi; });
   QVERIFY(n>0);
   TfrmInfosTaxi *f=frmInfosTaxi;
   auto Ecart=[f, Case, Coordonnees, Situations](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     if (v->NumTaxi>0)
      {
       taxi *t=cv->Taxi+v->NumTaxi-1;
       if (!f->RadioButtonTaxiNumero->Checked||f->LabelDirection->isHidden()) e<<"taxi non affiché";
       Compare(e, "cadre", f->GroupBoxTaxi->Caption, Cadre(v->NumTaxi, t->x, t->y));
       Compare(e, "situation", f->LabelSituation->Caption, QString::fromUtf8(Situations[t->Situation]));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[t->Dir]));
       Compare(e, "file", f->LabelNumeroFile->Caption, QString::number(t->NumFile+1));
       Compare(e, "personnes", f->LabelNbPersonnes->Caption, QString::number(t->NbPersonnes));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(t->NbCasesParcourues));
       Compare(e, "tours depuis le départ", f->LabelToursDepuisDepart->Caption, QString::number(Tour-t->TourDepartDrnCourse));
      }
     else
      if (!f->RadioButtonAucunTaxi->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun taxi » non affiché");
     return e.join('\n');
    };
   QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_I, Qt::ControlModifier);
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailTaxi->Checked);
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   CaptureFiche(f, "fiche_infos_taxi.png");
   SuitObjet("taxi", Ecart, [n](int &x, int &y)
    {
     x=cv->Taxi[n-1].x; y=cv->Taxi[n-1].y;
     return (x>=0)&&(y>=0)&&(cv->v[x][y].NumTaxi==n);
    }, 4);
   if (QTest::currentTestFailed()) return;
   Clique(f->ButtonOk);
   QVERIFY(!f->isVisible());
  }
  {
   int n=Trouve([](voie *v) { return v->NumVehlib; });
   QVERIFY(n>0);
   TfrmInfosVehlib *f=frmInfosVehlib;
   auto Ecart=[f, Case, Coordonnees, Situations](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     if (v->NumVehlib>0)
      {
       vehlib *l=cv->Vehlib+v->NumVehlib-1;
       if (!f->RadioButtonVehlibNumero->Checked||f->LabelDirection->isHidden()) e<<"vehlib non affiché";
       Compare(e, "cadre", f->GroupBoxVehlib->Caption, Cadre(v->NumVehlib, l->x, l->y));
       Compare(e, "situation", f->LabelSituation->Caption, QString::fromUtf8(Situations[l->Situation]));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[l->Dir]));
       Compare(e, "place de départ", f->LabelNumeroPlaceDepart->Caption,
               NumeroCoordonnees(l->NumPlaceDepart, cv->PlaceVehlib[l->NumPlaceDepart-1].x, cv->PlaceVehlib[l->NumPlaceDepart-1].y));
       Compare(e, "personnes", f->LabelNbPersonnes->Caption, QString::number(l->NbPersonnes));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(l->NbCasesParcourues));
       Compare(e, "tours depuis le départ", f->LabelToursDepuisDepart->Caption,
               (l->Situation==svDepart)?QString::number(Tour-l->TourDepart):QString("N/D"));
      }
     else
      if (!f->RadioButtonAucunVehlib->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun vehlib » non affiché");
     return e.join('\n');
    };
   QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_E, Qt::ControlModifier);
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailVehlib->Checked);
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   CaptureFiche(f, "fiche_infos_vehlib.png");
   SuitObjet("vehlib", Ecart, [n](int &x, int &y)
    {
     x=cv->Vehlib[n-1].x; y=cv->Vehlib[n-1].y;
     return (x>=0)&&(y>=0)&&(cv->v[x][y].NumVehlib==n);
    }, 4);
   if (QTest::currentTestFailed()) return;
   Clique(f->ButtonOk);
   QVERIFY(!f->isVisible());
  }
  //-------------------------------------------------------------------------
  // Piéton (Ctrl+P), à pied
  {
   int n=Trouve([](voie *v) { return v->NumPieton[0]?v->NumPieton[0]:v->NumPieton[1]; });
   QVERIFY(n>0);
   TfrmInfosPieton *f=frmInfosPieton;
   auto Ecart=[f, Case, Coordonnees](long Tour) -> QString
    {
     voie *v=Case();
     QStringList e;
     Compare(e, "coordonnées", f->LabelCoordonnees->Caption, Coordonnees());
     int Num=v->NumPieton[0]?v->NumPieton[0]:v->NumPieton[1];
     if (Num>0)
      {
       pieton *p=&cv->Pieton[Num-1];
       const char *Positions[2]={"Entrée de la case", "Sortie de la case"};
       if (!f->RadioButtonPietonNumero->Checked||f->LabelDirection->isHidden()) e<<QString::fromUtf8("piéton non affiché");
       Compare(e, "cadre", f->GroupBoxPieton->Caption, Cadre(Num, p->x, p->y));
       Compare(e, "direction", f->LabelDirection->Caption, Q(asDir[p->Dir]));
       Compare(e, "parcours", f->LabelParcours->Caption, QString::number(p->DemiCasesParcourues/2.0, 'f', 1).replace('.', ','));
       Compare(e, "position", f->LabelPosition->Caption, QString::fromUtf8(Positions[!!p->DemiCase]));
       Compare(e, "véhicule", f->LabelBusTramTaxiVehlib->Caption, "Aucun");
      }
     else
      if (!f->RadioButtonAucunPieton->Checked||!f->LabelDirection->isHidden()) e<<QString::fromUtf8("« Aucun piéton » non affiché");
     return e.join('\n');
    };
   QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_P, Qt::ControlModifier);
   QVERIFY(f->isVisible());
   QVERIFY(frmSimulation->ActionDetailPieton->Checked);
   QString e=Ecart(cv->TourCrt);
   QVERIFY2(e.isEmpty(), qPrintable(e));
   CaptureFiche(f, "fiche_infos_pieton.png");
   SuitObjet("piéton", Ecart, [n](int &x, int &y)
    {
     pieton *p=&cv->Pieton[n-1];
     x=p->x; y=p->y;
     return (x>=0)&&(y>=0)&&((cv->v[x][y].NumPieton[0]==n)||(cv->v[x][y].NumPieton[1]==n));
    }, 6);
   if (QTest::currentTestFailed()) return;
   Clique(f->ButtonOk);
   QVERIFY(!f->isVisible());
  }
  //-------------------------------------------------------------------------
  // Arrêt : la fiche ouverte garde son contenu (aucun rafraîchissement dans
  // l'original), puis le clic suivant sur la carte rechargée la vide
  int n=Trouve([](voie *v) { return (v->NumVehicule>0)&&!v->NumParking?v->NumVehicule:0; });
  QVERIFY(n>0);
  int x=frmSimulation->DrawGridSimul->Col, y=frmSimulation->DrawGridSimul->Row;
  QTest::keyClick(frmSimulation->DrawGridSimul, Qt::Key_V, Qt::ControlModifier);
  QVERIFY(frmInfosVehicule->isVisible());
  QVERIFY(frmInfosVehicule->RadioButtonVehiculeNumero->Checked);
  QVERIFY(Arrete());
  QVERIFY(frmInfosVehicule->isVisible());
  QVERIFY(frmSimulation->ActionDetailVehicule->Checked);
  QVERIFY(frmInfosVehicule->RadioButtonVehiculeNumero->Checked);
  QVERIFY(!cv->v[x][y].NumVehicule);
  Focalise();
  CliqueCase(x, y);
  QVERIFY(frmInfosVehicule->RadioButtonAucunVehicule->Checked);
  QVERIFY(frmInfosVehicule->LabelDirection->isHidden());
  QCOMPARE(Q(frmInfosVehicule->GroupBoxVehicule->Caption), QString(27, ' '));
  CaptureFiche(frmInfosVehicule, "fiche_infos_vehicule_arret.png");
  Clique(frmInfosVehicule->ButtonOk);
  QVERIFY(!frmInfosVehicule->isVisible());
 }
//---------------------------------------------------------------------------
void TestInterface::cleanupTestCase()
 {
  alea::LibereGraine();
  Repondeur.stop();
  DetruitFiches();
 }
//---------------------------------------------------------------------------
int main(int argc, char *argv[])
 { // La carte (variable d'environnement CV_CARTE, en Unicode sur tous les
   // systèmes) est copiée puis passée à l'application comme « cvw <copie> »
  QTemporaryDir Temp;
  QString Origine=qEnvironmentVariable("CV_CARTE");
  QString Copie=Temp.path()+"/carte.cvw";
  QFile::copy(Origine, Copie);
  QByteArray CopieLocale=Copie.toLocal8Bit();
  std::vector<char *> ArgsApplication={argv[0], CopieLocale.data()};
  int NbArgsApplication=2;
  // Boîtes de message de Qt et non natives (NSAlert sous macOS) : le test y répond
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
  QApplication App(NbArgsApplication, ArgsApplication.data());
  InitialiseApplication(App);
  // Carte : le chemin tel que passé à l'application (ParamStr(1), affiché tel quel dans le titre)
  ArgumentsTest<<QString::fromLocal8Bit(argv[0])<<Copie<<Origine;
  DossierCaptures=qEnvironmentVariable("CV_DOSSIER_CAPTURES");
  TestInterface t;
  return QTest::qExec(&t, argc, argv);
 }
//---------------------------------------------------------------------------
#include "test_interface.moc"

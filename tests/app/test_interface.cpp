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
//     « À propos » du démarrage s'anime puis se ferme seul.
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
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <deque>
#include <fstream>
#include <sstream>
#include "application.h"
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
void TestInterface::cleanupTestCase()
 {
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

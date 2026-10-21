//---------------------------------------------------------------------------
// test_interface.cpp - Tests de l'application Qt (phases 4 et 5)
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
//     même fichier, identique à la conversion de référence du format.
// Les captures de la fenêtre principale sont écrites dans le dossier
// CV_DOSSIER_CAPTURES (Démo au tour 0 et vers le tour 200).
//---------------------------------------------------------------------------
#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileOpenEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include "application.h"
#include "f_infbus.h"
#include "f_simul.h"
#include "format_reference.h"
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
 }
//---------------------------------------------------------------------------
class TestInterface : public QObject
 {
  Q_OBJECT
  QString Carte;
  QStringList Messages; // boîtes de message affichées (titre : texte)
  QTimer Repondeur;
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
  void ouvertureParLigneDeCommande();
  void raccourcisSelonLeMode();
  void marcheInterruptionArret();
  void enregistrementIdentique();
  void cleanupTestCase();
 };
//---------------------------------------------------------------------------
void TestInterface::initTestCase()
 {
  // Les boîtes de message reçoivent « Non » (ou OK) et sont mémorisées
  Repondeur.setInterval(20);
  QObject::connect(&Repondeur, &QTimer::timeout, this, [this]()
   {
    QMessageBox *b=qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
    if (!b) return;
    Messages<<(b->windowTitle()+" : "+b->text());
    QAbstractButton *r=b->button(QMessageBox::No);
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
  ArgumentsTest<<QString::fromLocal8Bit(argv[0])<<QDir::toNativeSeparators(Copie)<<Origine;
  DossierCaptures=qEnvironmentVariable("CV_DOSSIER_CAPTURES");
  TestInterface t;
  return QTest::qExec(&t, argc, argv);
 }
//---------------------------------------------------------------------------
#include "test_interface.moc"

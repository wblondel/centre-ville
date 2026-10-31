//---------------------------------------------------------------------------
// test_cartes.cpp - Cartes d'exemple de l'AppImage (phase 9)
//
// app/linux/apprun-hooks/centre-ville-cartes.sh, lu par l'AppRun de
// l'AppImage avant cvw, copie au premier lancement les cartes de
// usr/share/centre-ville/cartes dans le dossier des fichiers de Centre-Ville,
// sans écraser un fichier existant (comme l'installeur Windows dans
// %LOCALAPPDATA%\Patquoi.fr\Centre-Ville). Le script est lancé comme par
// l'AppRun (sh, set -e), sur une AppDir et un XDG_DATA_HOME temporaires :
//   - premier lancement : les cartes de maps/ copiées, une carte déjà
//     présente gardée telle quelle ;
//   - lancement suivant : une carte supprimée n'est pas recopiée ;
//   - sous Linux, ce dossier est celui de stRepLocalAppData, où « Ouvrir »
//     et « Enregistrer » démarrent (OpenDialog->InitialDir : interface.demo).
// Pas de sh sous Windows : test sauté.
//
// Sous macOS, cvw fait de même au lancement (CopieCartesExemple de base.cpp,
// main.cpp), depuis ses ressources Qt, car le dossier Cartes du .dmg est en
// lecture seule : la fonction, puis le paquet compilé lancé pour de vrai
// avec un dossier personnel temporaire (CFFIXED_USER_HOME).
//---------------------------------------------------------------------------
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include "base.h"
//---------------------------------------------------------------------------
namespace
 {
  QByteArray Contenu(const QString &asFichier)
   {
    QFile f(asFichier);
    return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();
   }
 }
//---------------------------------------------------------------------------
class TestCartes : public QObject
 {
  Q_OBJECT
  QTemporaryDir Temp;
  QString stCartes, stDonnees, stDossier;
  QStringList Cartes; // cartes de maps/
  int Lance()
   { // AppRun de linuxdeploy : hooks lus par le shell, set -e ; APPDIR posé par l'AppImage
    QProcess p;
    QProcessEnvironment e=QProcessEnvironment::systemEnvironment();
    e.insert("APPDIR", Temp.path()+"/AppDir");
    e.insert("XDG_DATA_HOME", stDonnees);
    p.setProcessEnvironment(e);
    p.start("sh", {"-e", "-c", ". \"$0\"", CV_SCRIPT_CARTES});
    if (!p.waitForFinished(10000)||(p.exitStatus()!=QProcess::NormalExit)) return -1;
    return p.exitCode();
   }
  private slots:
  void initTestCase();
  void premierLancement();
  void lancementSuivant();
  void dossierDeStRepLocalAppData();
  void copieAuPremierLancement();
  void paquetMacOS();
 };
//---------------------------------------------------------------------------
void TestCartes::initTestCase()
 {
#ifdef Q_OS_WIN
  QSKIP("AppImage : Linux seulement (pas de sh sous Windows)");
#endif
  QVERIFY(Temp.isValid());
  stCartes=Temp.path()+"/AppDir/usr/share/centre-ville/cartes";
  stDonnees=Temp.path()+"/donnees";
  stDossier=stDonnees+"/Patquoi.fr/Centre-Ville";
  QVERIFY(QDir().mkpath(stCartes));
  Cartes=QDir(CV_DOSSIER_CARTES).entryList({"*.cvw"}, QDir::Files);
  QCOMPARE(Cartes.size(), 6);
  for(const QString &c : Cartes)
   QVERIFY(QFile::copy(QString(CV_DOSSIER_CARTES)+"/"+c, stCartes+"/"+c));
 }
//---------------------------------------------------------------------------
void TestCartes::premierLancement()
 { // Dossier déjà créé par cvw, une carte modifiée par l'utilisateur
  QVERIFY(QDir().mkpath(stDossier));
  QString Modifiee=stDossier+"/"+Cartes.first();
  QFile f(Modifiee);
  QVERIFY(f.open(QIODevice::WriteOnly));
  f.write("carte modifiée");
  f.close();
  QCOMPARE(Lance(), 0);
  for(const QString &c : Cartes)
   if (c!=Cartes.first())
    QVERIFY2(Contenu(stDossier+"/"+c)==Contenu(stCartes+"/"+c), qPrintable(c));
  QCOMPARE(Contenu(Modifiee), QByteArray("carte modifiée"));
  QCOMPARE(QDir(stDossier).entryList({"*.cvw"}, QDir::Files), Cartes);
 }
//---------------------------------------------------------------------------
void TestCartes::lancementSuivant()
 { // Copie au premier lancement seulement : une carte supprimée reste supprimée
  QVERIFY(QFile::remove(stDossier+"/"+Cartes.last()));
  QCOMPARE(Lance(), 0);
  QVERIFY(!QFile::exists(stDossier+"/"+Cartes.last()));
  QCOMPARE(QDir(stDossier).entryList({"*.cvw"}, QDir::Files).size(), Cartes.size()-1);
 }
//---------------------------------------------------------------------------
void TestCartes::dossierDeStRepLocalAppData()
 {
#ifndef Q_OS_LINUX
  QSKIP("QStandardPaths::GenericDataLocation suit XDG_DATA_HOME sous Linux seulement");
#endif
  qputenv("XDG_DATA_HOME", stDonnees.toLocal8Bit());
  QCOMPARE(Q(stRepLocalAppData()), QDir::toNativeSeparators(stDossier+"/"));
 }
//---------------------------------------------------------------------------
void TestCartes::copieAuPremierLancement()
 { // Source en lecture seule, comme les ressources Qt de l'exécutable
  QTemporaryDir Dossier;
  QVERIFY(Dossier.isValid());
  QString stSource=Dossier.path()+"/cartes", stCible=Dossier.path()+"/Patquoi.fr/Centre-Ville";
  QVERIFY(QDir().mkpath(stSource));
  for(const QString &c : Cartes)
   {
    QVERIFY(QFile::copy(QString(CV_DOSSIER_CARTES)+"/"+c, stSource+"/"+c));
    QVERIFY(QFile::setPermissions(stSource+"/"+c, QFile::ReadOwner|QFile::ReadUser|QFile::ReadGroup|QFile::ReadOther));
   }
  // Sans source (compilation hors paquet) : rien n'est créé
  CopieCartesExemple(Dossier.path()+"/absent", stCible);
  QVERIFY(!QDir(stCible).exists());
  // Premier lancement : une carte de l'utilisateur gardée, les autres copiées et enregistrables
  QVERIFY(QDir().mkpath(stCible));
  QFile f(stCible+"/"+Cartes.first());
  QVERIFY(f.open(QIODevice::WriteOnly));
  f.write("carte modifiée");
  f.close();
  CopieCartesExemple(stSource, stCible);
  QCOMPARE(Contenu(f.fileName()), QByteArray("carte modifiée"));
  QCOMPARE(QDir(stCible).entryList({"*.cvw"}, QDir::Files), Cartes);
  for(const QString &c : Cartes)
   if (c!=Cartes.first())
    {
     QVERIFY2(Contenu(stCible+"/"+c)==Contenu(stSource+"/"+c), qPrintable(c));
     QVERIFY2(QFile::permissions(stCible+"/"+c)&QFile::WriteOwner, qPrintable(c));
    }
  // Lancement suivant : une carte supprimée reste supprimée
  QVERIFY(QFile::remove(stCible+"/"+Cartes.last()));
  CopieCartesExemple(stSource, stCible);
  QVERIFY(!QFile::exists(stCible+"/"+Cartes.last()));
  for(const QString &c : Cartes) QFile::setPermissions(stSource+"/"+c, QFile::ReadOwner|QFile::WriteOwner);
 }
//---------------------------------------------------------------------------
void TestCartes::paquetMacOS()
 {
#ifndef Q_OS_MACOS
  QSKIP("Paquet Centre-Ville.app : macOS seulement");
#else
  QTemporaryDir Accueil;
  QVERIFY(Accueil.isValid());
  QString stCible=Accueil.path()+"/Library/Application Support/Patquoi.fr/Centre-Ville";
  auto Lance=[&]()
   { // Qu'il tourne : la copie précède la création des fiches
    QProcess p;
    QProcessEnvironment e=QProcessEnvironment::systemEnvironment();
    e.insert("CFFIXED_USER_HOME", Accueil.path());
    e.insert("QT_QPA_PLATFORM", "offscreen");
    p.setProcessEnvironment(e);
    p.start(CV_PAQUET_CVW, QStringList());
    QVERIFY2(p.waitForStarted(10000), qPrintable(p.errorString()));
    QVERIFY(QTest::qWaitFor([&]() { return QFile::exists(stCible+"/.cartes-copiees"); }, 30000));
    QTest::qWait(500);
    QCOMPARE(p.state(), QProcess::Running);
    p.kill();
    p.waitForFinished(10000);
   };
  Lance();
  if (QTest::currentTestFailed()) return;
  QCOMPARE(QDir(stCible).entryList({"*.cvw"}, QDir::Files), Cartes);
  for(const QString &c : Cartes)
   QVERIFY2(Contenu(stCible+"/"+c)==Contenu(QString(CV_DOSSIER_CARTES)+"/"+c), qPrintable(c));
  QVERIFY(QFile::remove(stCible+"/"+Cartes.last()));
  Lance();
  if (QTest::currentTestFailed()) return;
  QVERIFY(!QFile::exists(stCible+"/"+Cartes.last()));
#endif
 }
//---------------------------------------------------------------------------
QTEST_MAIN(TestCartes)
#include "test_cartes.moc"

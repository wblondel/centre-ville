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
//---------------------------------------------------------------------------
#include <QDir>
#include <QFile>
#include <QProcess>
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
QTEST_MAIN(TestCartes)
#include "test_cartes.moc"

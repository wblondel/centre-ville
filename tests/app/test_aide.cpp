//---------------------------------------------------------------------------
// test_aide.cpp - Tests de l'aide HTML (phase 7)
//
// AfficheAideContextuelle, AfficheIndexAide et AfficheTutoriel (base.cpp)
// ouvrent leur page par QDesktopServices::openUrl, intercepté ici
// (setUrlHandler("file")). Vérifie :
//   - les 57 entrées de caCorresp, lues dans src/base.cpp (l'original) :
//     HelpContext -> page de redirection (adresse sans ancre, que le système
//     ne peut pas perdre) -> html/index.html#ANCRE à côté de l'exécutable ;
//     chaque ancre existe dans docs/guide/index.html ;
//   - tous les HelpContext des .ui (fiches et composants) ont leur entrée ;
//   - HelpContext inconnu : rien n'est ouvert, comme l'original ;
//   - sommaire et tutoriel : html/index.html et tutocv/index.html ;
//   - l'aide copiée par la compilation à côté de cvw (dans le paquet sous
//     macOS) est identique à docs/guide et docs/tutorial.
//---------------------------------------------------------------------------
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QTest>
#include <QUrl>
#include <QWidget>
#include "base.h"
//---------------------------------------------------------------------------
namespace
 {
  QByteArray Contenu(const QString &asFichier)
   {
    QFile f(asFichier);
    return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();
   }
  //-------------------------------------------------------------------------
  // Ancres (<A NAME="..."> ou id="...") d'une page HTML, à la casse près
  QSet<QString> Ancres(const QString &asPage)
   {
    QSet<QString> Resultat;
    QRegularExpression Motif("<a\\s[^>]*\\bname\\s*=\\s*\"([^\"]+)\"|\\bid\\s*=\\s*\"([^\"]+)\"",
                             QRegularExpression::CaseInsensitiveOption);
    QString stPage=QString::fromUtf8(Contenu(asPage));
    for(auto i=Motif.globalMatch(stPage); i.hasNext(); )
     {
      QRegularExpressionMatch m=i.next();
      Resultat<<(m.captured(1).isEmpty()?m.captured(2):m.captured(1));
     }
    return Resultat;
   }
  //-------------------------------------------------------------------------
  // Fichiers d'un dossier (chemins relatifs, récursivement)
  QStringList Fichiers(const QString &asDossier)
   {
    QStringList Resultat;
    QDir d(asDossier);
    for(QDirIterator i(asDossier, QDir::Files|QDir::Hidden, QDirIterator::Subdirectories); i.hasNext(); )
     Resultat<<d.relativeFilePath(i.next());
    Resultat.sort();
    return Resultat;
   }
 }
//---------------------------------------------------------------------------
class TestAide : public QObject
 {
  Q_OBJECT
  QList<QUrl> Ouvertes; // adresses passées à QDesktopServices::openUrl
  QString stRepExe;     // dossier de l'exécutable des tests (stRepAide)
  QString Cible(const QUrl &Page)
   { // Adresse chargée par la page de redirection (balise meta et lien)
    QString stPage=QString::fromUtf8(Contenu(Page.toLocalFile()));
    QRegularExpressionMatch Meta=QRegularExpression("<meta http-equiv=\"refresh\" content=\"0; url='([^']*)'\">").match(stPage);
    QRegularExpressionMatch Lien=QRegularExpression("<a href=\"([^\"]*)\">").match(stPage);
    if (!Meta.hasMatch()||!Lien.hasMatch()||(Meta.captured(1)!=Lien.captured(1))) return QString();
    return Meta.captured(1).replace("&amp;", "&");
   }
  public slots:
  void Ouvre(const QUrl &Url) { Ouvertes<<Url; }
  private slots:
  void initTestCase();
  void cleanupTestCase();
  void tableDesContextes();
  void contextesDesFiches();
  void contexteInconnu();
  void sommaireEtTutoriel();
  void aideCopieeACoteDeCvw();
 };
//---------------------------------------------------------------------------
void TestAide::initTestCase()
 {
  QDesktopServices::setUrlHandler("file", this, "Ouvre");
  stRepExe=QCoreApplication::applicationDirPath();
 }
//---------------------------------------------------------------------------
void TestAide::cleanupTestCase()
 {
  QDesktopServices::unsetUrlHandler("file");
 }
//---------------------------------------------------------------------------
void TestAide::tableDesContextes()
 { // Table caCorresp de l'original : chaque HelpContext ouvre html/index.html#ANCRE
  QString stSource=QString::fromUtf8(Contenu(CV_SOURCE_BASE));
  int Debut=stSource.indexOf("caCorresp[NBCONTEXTESAIDE]");
  QVERIFY(Debut>0);
  QString stTable=stSource.mid(Debut, stSource.indexOf("};", Debut)-Debut);
  QRegularExpression Entree("\\{\\s*(\\d+)\\s*,\\s*\"([^\"]+)\"\\s*\\}");
  QSet<QString> AncresGuide=Ancres(CV_DOSSIER_DOCS "/guide/index.html");
  QVERIFY(AncresGuide.size()>57);
  int n=0;
  for(auto i=Entree.globalMatch(stTable); i.hasNext(); n++)
   {
    QRegularExpressionMatch m=i.next();
    int HelpContext=m.captured(1).toInt();
    QString stAncre=m.captured(2);
    QVERIFY2(AncresGuide.contains(stAncre), qPrintable("Ancre absente de docs/guide/index.html : "+stAncre));
    QWidget Fiche;
    Fiche.setProperty("vclHelpContext", HelpContext);
    Ouvertes.clear();
    AfficheAideContextuelle(&Fiche);
    QCOMPARE(Ouvertes.size(), 1);
    // Page de redirection : fichier local sans ancre, que le système ouvre
    // tel quel ; elle charge l'adresse de l'aide avec son ancre
    QUrl Page=Ouvertes[0];
    QVERIFY2(Page.isLocalFile()&&!Page.hasFragment(), qPrintable(Page.toString()));
    QVERIFY(QFile::exists(Page.toLocalFile()));
    QUrl Attendue=QUrl::fromLocalFile(stRepExe+"/html/index.html");
    Attendue.setFragment(stAncre);
    QCOMPARE(QUrl(Cible(Page)), Attendue);
   }
  QCOMPARE(n, 57);
 }
//---------------------------------------------------------------------------
void TestAide::contextesDesFiches()
 { // HelpContext des fiches et des composants (.dfm -> .ui) : tous dans la table
  QSet<QString> AncresGuide=Ancres(CV_DOSSIER_DOCS "/guide/index.html");
  QRegularExpression Propriete("<property name=\"vclHelpContext\"[^>]*>\\s*<number>(\\d+)</number>");
  int n=0;
  for(const QString &stUi : QDir(CV_DOSSIER_UI).entryList({"*.ui"}))
   {
    QString stContenu=QString::fromUtf8(Contenu(CV_DOSSIER_UI "/"+stUi));
    for(auto i=Propriete.globalMatch(stContenu); i.hasNext(); n++)
     {
      int HelpContext=i.next().captured(1).toInt();
      QWidget Fiche;
      Fiche.setProperty("vclHelpContext", HelpContext);
      Ouvertes.clear();
      AfficheAideContextuelle(&Fiche);
      QVERIFY2(Ouvertes.size()==1, qPrintable(stUi+" : HelpContext "+QString::number(HelpContext)+" sans page"));
      QUrl Url(Cible(Ouvertes[0]));
      QVERIFY2(AncresGuide.contains(Url.fragment()), qPrintable(Url.toString()));
     }
   }
  QVERIFY(n>=37);
 }
//---------------------------------------------------------------------------
void TestAide::contexteInconnu()
 {
  QWidget Fiche;
  Fiche.setProperty("vclHelpContext", 12345);
  Ouvertes.clear();
  AfficheAideContextuelle(&Fiche);
  QVERIFY(Ouvertes.isEmpty());
 }
//---------------------------------------------------------------------------
void TestAide::sommaireEtTutoriel()
 { // Sans ancre : la page est ouverte directement
  Ouvertes.clear();
  AfficheIndexAide(nullptr);
  AfficheTutoriel();
  QCOMPARE(Ouvertes.size(), 2);
  QCOMPARE(Ouvertes[0], QUrl::fromLocalFile(stRepExe+"/html/index.html"));
  QCOMPARE(Ouvertes[1], QUrl::fromLocalFile(stRepExe+"/tutocv/index.html"));
  QVERIFY(QFile::exists(CV_DOSSIER_DOCS "/guide/index.html"));
  QVERIFY(QFile::exists(CV_DOSSIER_DOCS "/tutorial/index.html"));
 }
//---------------------------------------------------------------------------
void TestAide::aideCopieeACoteDeCvw()
 { // html/ et tutocv/ à côté de cvw (Contents/Resources sous macOS), identiques à docs/
  const struct { const char *Copie, *Source; } Dossiers[]={{"html", "guide"}, {"tutocv", "tutorial"}};
  for(const auto &d : Dossiers)
   {
    QString stCopie=QString(CV_DOSSIER_AIDE_CVW)+"/"+d.Copie, stSource=QString(CV_DOSSIER_DOCS)+"/"+d.Source;
    QVERIFY2(QDir(stCopie).exists(), qPrintable("Dossier absent : "+stCopie));
    QStringList Copies=Fichiers(stCopie);
    QCOMPARE(Copies, Fichiers(stSource));
    QVERIFY(Copies.contains("index.html"));
    for(const QString &f : Copies)
     QVERIFY2(Contenu(stCopie+"/"+f)==Contenu(stSource+"/"+f), qPrintable(stCopie+"/"+f));
   }
 }
//---------------------------------------------------------------------------
QTEST_MAIN(TestAide)
#include "test_aide.moc"

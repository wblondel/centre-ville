//---------------------------------------------------------------------------
// test_aide.cpp - Tests de l'aide HTML (phase 7)
//
// AfficheAideContextuelle, AfficheIndexAide et AfficheTutoriel (base.cpp)
// ouvrent leur page par QDesktopServices::openUrl, intercepté ici
// (setUrlHandler("file")). Vérifie :
//   - les 57 entrées de caCorresp, recopiées de l'original (CorrespOriginal) :
//     HelpContext -> page de redirection (adresse sans ancre, que le système
//     ne peut pas perdre) -> html/index.html#ANCRE à côté de l'exécutable ;
//     chaque ancre existe dans docs/guide/index.html ;
//   - tous les HelpContext des .ui (fiches et composants) ont leur entrée ;
//   - HelpContext inconnu : rien n'est ouvert, comme l'original ;
//   - sommaire et tutoriel : html/index.html et tutocv/index.html ;
//   - l'aide copiée par la compilation à côté de cvw (dans le paquet sous
//     macOS) est identique à docs/guide et docs/tutorial.
//---------------------------------------------------------------------------
#include <array>
#include <cstddef>
#include <iterator>
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
// Table caCorresp de src/base.cpp (5.4.3.2), recopiée pour que le test ne
// dépende plus de src/ : HelpContext -> ancre de html/index.html
struct ElementCorrespOriginal
{
    int Numero;
    const char *szHREF;
};
const auto CorrespOriginal = std::to_array<ElementCorrespOriginal>({
    {.Numero = 0, .szHREF = "ASCV"},        {.Numero = 10, .szHREF = "BR"},
    {.Numero = 20, .szHREF = "EDLS"},       {.Numero = 30, .szHREF = "PDLI"},
    {.Numero = 31, .szHREF = "LMP"},        {.Numero = 3101, .szHREF = "LMF"},
    {.Numero = 3102, .szHREF = "LMA"},      {.Numero = 3103, .szHREF = "LMSI"},
    {.Numero = 3104, .szHREF = "LMST"},     {.Numero = 3105, .szHREF = "LMI"},
    {.Numero = 311, .szHREF = "PDLS"},      {.Numero = 3111, .szHREF = "DDDDLZDS"},
    {.Numero = 3112, .szHREF = "GADUR"},    {.Numero = 3121, .szHREF = "PDRDB"},
    {.Numero = 3122, .szHREF = "PDRDT"},    {.Numero = 3123, .szHREF = "PDT"},
    {.Numero = 3124, .szHREF = "PDV"},      {.Numero = 313, .szHREF = "CDLS"},
    {.Numero = 314, .szHREF = "SDLS"},      {.Numero = 315, .szHREF = "SSLCDR"},
    {.Numero = 316, .szHREF = "PDLADSDE"},  {.Numero = 317, .szHREF = "PDLE"}, // v5.4.1
    {.Numero = 32, .szHREF = "LBDO"},       {.Numero = 33, .szHREF = "LZDS"},
    {.Numero = 3301, .szHREF = "LDEDLZDS"}, {.Numero = 3302, .szHREF = "LMD"},
    {.Numero = 34, .szHREF = "LBDE"},       {.Numero = 40, .szHREF = "CDR"},
    {.Numero = 41, .szHREF = "DDP"},        {.Numero = 415, .szHREF = "DDPP"},
    {.Numero = 42, .szHREF = "DDF"},        {.Numero = 425, .szHREF = "DDFDP"},
    {.Numero = 431, .szHREF = "ODC"},       {.Numero = 4311, .szHREF = "OCUCS"},
    {.Numero = 4312, .szHREF = "OCPCS"},    {.Numero = 432, .szHREF = "C"},
    {.Numero = 44, .szHREF = "BDN"},        {.Numero = 45, .szHREF = "DDLNDUZ"},
    {.Numero = 50, .szHREF = "LDLS"},       {.Numero = 611, .szHREF = "EGDLS"},
    {.Numero = 612, .szHREF = "SDFEDT"},    {.Numero = 62, .szHREF = "SDVDUEAUA"},
    {.Numero = 63, .szHREF = "ADDJAE"},     {.Numero = 641, .szHREF = "ADDJAADB"},
    {.Numero = 642, .szHREF = "ADDJAADT"},  {.Numero = 643, .szHREF = "ADDJAFDT"},
    {.Numero = 644, .szHREF = "ADDJAFDP"},  {.Numero = 645, .szHREF = "ADDJAPDV"},
    {.Numero = 65, .szHREF = "RDUEDLS"},    {.Numero = 66, .szHREF = "ISUV"},
    {.Numero = 671, .szHREF = "ISUB"},      {.Numero = 672, .szHREF = "ISUT"},
    {.Numero = 673, .szHREF = "ISUVDT"},    {.Numero = 674, .szHREF = "ISUVL"},
    {.Numero = 68, .szHREF = "ISUP"},       {.Numero = 69, .szHREF = "RC"},
    {.Numero = 70, .szHREF = "MDE"},
});
//-------------------------------------------------------------------------
QByteArray Contenu(const QString &asFichier)
{
    QFile f(asFichier);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}
//-------------------------------------------------------------------------
// Ancres (<A NAME="..."> ou id="...") d'une page HTML, à la casse près
QSet<QString> Ancres(const QString &asPage)
{
    QSet<QString> Resultat;
    // NOLINTNEXTLINE(modernize-raw-string-literal) : moc lit mal ces chaînes brutes
    const QRegularExpression Motif("<a\\s[^>]*\\bname\\s*=\\s*\"([^\"]+)\"|\\bid\\s*=\\s*\"([^\"]+)\"",
                                   QRegularExpression::CaseInsensitiveOption);
    const QString stPage = QString::fromUtf8(Contenu(asPage));
    for (auto i = Motif.globalMatch(stPage); i.hasNext();)
    {
        const QRegularExpressionMatch m = i.next();
        Resultat << (m.captured(1).isEmpty() ? m.captured(2) : m.captured(1));
    }
    return Resultat;
}
//-------------------------------------------------------------------------
// Fichiers d'un dossier (chemins relatifs, récursivement)
QStringList Fichiers(const QString &asDossier)
{
    QStringList Resultat;
    const QDir d(asDossier);
    for (QDirIterator i(asDossier, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories); i.hasNext();)
    {
        Resultat << d.relativeFilePath(i.next());
    }
    Resultat.sort();
    return Resultat;
}
} // namespace
//---------------------------------------------------------------------------
class TestAide : public QObject // NOLINT(misc-use-internal-linkage) : classe de test, lue par moc
{
    Q_OBJECT
    QList<QUrl> Ouvertes; // adresses passées à QDesktopServices::openUrl
    QString stRepExe;     // dossier de l'exécutable des tests (stRepAide)
    static QString Cible(const QUrl &Page)
    { // Adresse chargée par la page de redirection (balise meta et lien)
        const QString stPage = QString::fromUtf8(Contenu(Page.toLocalFile()));
        const QRegularExpressionMatch Meta =
            // NOLINTNEXTLINE(modernize-raw-string-literal) : moc lit mal ces chaînes brutes
            QRegularExpression("<meta http-equiv=\"refresh\" content=\"0; url='([^']*)'\">").match(stPage);
        const QRegularExpressionMatch Lien = QRegularExpression("<a href=\"([^\"]*)\">").match(stPage);
        if (!Meta.hasMatch() || !Lien.hasMatch() || (Meta.captured(1) != Lien.captured(1)))
        {
            return {};
        }
        return Meta.captured(1).replace("&amp;", "&");
    }
public slots:
    void Ouvre(const QUrl &Url) { Ouvertes << Url; }
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
    stRepExe = QCoreApplication::applicationDirPath();
}
//---------------------------------------------------------------------------
void TestAide::cleanupTestCase() // NOLINT(readability-convert-member-functions-to-static) : fonction de test (slot de
                                 // Qt Test)
{
    QDesktopServices::unsetUrlHandler("file");
}
//---------------------------------------------------------------------------
void TestAide::tableDesContextes()
{ // Table caCorresp de l'original : chaque HelpContext ouvre html/index.html#ANCRE
    const QSet<QString> AncresGuide = Ancres(CV_DOSSIER_DOCS "/guide/index.html");
    QVERIFY(AncresGuide.size() > 57);
    QCOMPARE(std::size(CorrespOriginal), (std::size_t)57); // NBCONTEXTESAIDE
    for (const auto &e : CorrespOriginal)
    {
        const int HelpContext = e.Numero;
        const QString stAncre = e.szHREF;
        QVERIFY2(AncresGuide.contains(stAncre), qPrintable("Ancre absente de docs/guide/index.html : " + stAncre));
        QWidget Fiche;
        Fiche.setProperty("vclHelpContext", HelpContext);
        Ouvertes.clear();
        AfficheAideContextuelle(&Fiche);
        QCOMPARE(Ouvertes.size(), 1);
        // Page de redirection : fichier local sans ancre, que le système ouvre
        // tel quel ; elle charge l'adresse de l'aide avec son ancre
        const QUrl Page = Ouvertes[0];
        QVERIFY2(Page.isLocalFile() && !Page.hasFragment(), qPrintable(Page.toString()));
        QVERIFY(QFile::exists(Page.toLocalFile()));
        QUrl Attendue = QUrl::fromLocalFile(stRepExe + "/html/index.html");
        Attendue.setFragment(stAncre);
        QCOMPARE(QUrl(Cible(Page)), Attendue);
    }
}
//---------------------------------------------------------------------------
void TestAide::contextesDesFiches()
{ // HelpContext des fiches et des composants (.dfm -> .ui) : tous dans la table
    const QSet<QString> AncresGuide = Ancres(CV_DOSSIER_DOCS "/guide/index.html");
    // NOLINTNEXTLINE(modernize-raw-string-literal) : moc lit mal ces chaînes brutes
    const QRegularExpression Propriete("<property name=\"vclHelpContext\"[^>]*>\\s*<number>(\\d+)</number>");
    int n = 0;
    for (const QString &stUi : QDir(CV_DOSSIER_UI).entryList({"*.ui"}))
    {
        const QString stContenu = QString::fromUtf8(Contenu(CV_DOSSIER_UI "/" + stUi));
        for (auto i = Propriete.globalMatch(stContenu); i.hasNext(); n++)
        {
            const int HelpContext = i.next().captured(1).toInt();
            QWidget Fiche;
            Fiche.setProperty("vclHelpContext", HelpContext);
            Ouvertes.clear();
            AfficheAideContextuelle(&Fiche);
            QVERIFY2(Ouvertes.size() == 1,
                     qPrintable(stUi + " : HelpContext " + QString::number(HelpContext) + " sans page"));
            const QUrl Url(Cible(Ouvertes[0]));
            QVERIFY2(AncresGuide.contains(Url.fragment()), qPrintable(Url.toString()));
        }
    }
    QVERIFY(n >= 37);
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
    QCOMPARE(Ouvertes[0], QUrl::fromLocalFile(stRepExe + "/html/index.html"));
    QCOMPARE(Ouvertes[1], QUrl::fromLocalFile(stRepExe + "/tutocv/index.html"));
    QVERIFY(QFile::exists(CV_DOSSIER_DOCS "/guide/index.html"));
    QVERIFY(QFile::exists(CV_DOSSIER_DOCS "/tutorial/index.html"));
}
//---------------------------------------------------------------------------
void TestAide::aideCopieeACoteDeCvw() // NOLINT(readability-convert-member-functions-to-static) : fonction de test (slot
                                      // de Qt Test)
{ // html/ et tutocv/ à côté de cvw (Contents/Resources sous macOS), identiques à docs/
    struct ElementDossiers
    {
        const char *Copie, *Source;
    };
    const auto Dossiers = std::to_array<ElementDossiers>(
        {{.Copie = "html", .Source = "guide"}, {.Copie = "tutocv", .Source = "tutorial"}});
    for (const auto &d : Dossiers)
    {
        const QString stCopie = QString(CV_DOSSIER_AIDE_CVW) + "/" + d.Copie;
        const QString stSource = QString(CV_DOSSIER_DOCS) + "/" + d.Source;
        QVERIFY2(QDir(stCopie).exists(), qPrintable("Dossier absent : " + stCopie));
        const QStringList Copies = Fichiers(stCopie);
        QCOMPARE(Copies, Fichiers(stSource));
        QVERIFY(Copies.contains("index.html"));
        for (const QString &f : Copies)
        {
            QVERIFY2(Contenu(stCopie + "/" + f) == Contenu(stSource + "/" + f), qPrintable(stCopie + "/" + f));
        }
    }
}
//---------------------------------------------------------------------------
QTEST_MAIN(TestAide)
#include "test_aide.moc"

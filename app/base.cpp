//---------------------------------------------------------------------------
// base.cpp - Portage de Centre-Ville : port de src/base.cpp
//---------------------------------------------------------------------------
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <QWidget>
#include <utility>
#include <vector>
//---------------------------------------------------------------------------
#include "base.h"
#include "compat/chaine.h"
#include "vcl.h"
#include "vue.h"
//---------------------------------------------------------------------------
#define NBTYPESMESSAGE  5
#define NBTYPESREPONSE  4
#define NBMAXBOUTONS    3
#define NBCONTEXTESAIDE 57
//---------------------------------------------------------------------------
// portage : MessageBoxW -> QMessageBox (mêmes icônes, boutons et bouton par défaut)
const QMessageBox::Icon Style[NBTYPESMESSAGE] = {QMessageBox::Information, QMessageBox::Warning, QMessageBox::Critical,
                                                 QMessageBox::Question, QMessageBox::Question};
const int NbBoutons[NBTYPESMESSAGE] = {1, 1, 1, 2, 3}; // MB_OK, MB_OK, MB_OK, MB_YESNO, MB_YESNOCANCEL
const int BoutonDefaut[NBTYPESMESSAGE][NBMAXBOUTONS] = {
    {0, 0, 0}, // MB_DEFBUTTONn - 1
    {0, 0, 0}, {0, 0, 0}, {0, 1, 0}, {0, 1, 2},
};
//---------------------------------------------------------------------------
const AnsiString asDebutNomSpinEdit = "CSpinEdit";
//---------------------------------------------------------------------------
const struct
{
    int Numero;
    const char *szHREF;
} // v5.3 : aide type HLP converti en type HTML : correspondances TForm::HelpContext en URL "html\index.html#"
caCorresp[NBCONTEXTESAIDE] = {
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
};
//---------------------------------------------------------------------------
// v5.4 : Pour les fichiers créés par Centre-Ville
//        on utilise le chemin donné par %LocalAppData% pour Windows 7 ou plus
//        sinon le dossier de l'exécutable
// portage : QStandardPaths::GenericDataLocation (%LOCALAPPDATA% sous Windows,
//           donc le même dossier que l'original ; ~/Library/Application Support
//           sous macOS ; ~/.local/share sous Linux) + Patquoi.fr/Centre-Ville
//---------------------------------------------------------------------------
String stRepLocalAppData()
{
    String stResult;
    String stExePath = A(QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/"));
    const QString stRepLocAppDataCvw = "/Patquoi.fr/Centre-Ville/";
    const QString stBase = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (stBase.isEmpty())
    {
        stResult = std::move(stExePath);
    }
    else
    {
        const QString stRep = stBase + stRepLocAppDataCvw;
        if ((!QDir(stRep).exists()) && (!QDir().mkpath(stRep)))
        {
            return stExePath;
        }

        stResult = A(QDir::toNativeSeparators(stRep));
    }
    return stResult;
}
//---------------------------------------------------------------------------
// v6.0.1 : au premier lancement, cartes d'exemple copiées dans le dossier des
// fichiers de Centre-Ville, où « Ouvrir » et « Enregistrer » démarrent, comme le
// font l'installeur Windows et l'AppImage (linux/apprun-hooks/centre-ville-cartes.sh).
// Aucun fichier existant n'est écrasé ; .cartes-copiees marque le premier
// lancement (une carte supprimée ensuite n'est pas recopiée). Écrit seulement si
// tout a été copié : une copie en échec (disque plein) est retentée au lancement suivant
//---------------------------------------------------------------------------
void CopieCartesExemple(const QString &stSource, const QString &stDestination)
{
    const QDir Destination(stDestination);
    const QString stMarque = Destination.filePath(".cartes-copiees");
    const QStringList Cartes = QDir(stSource).entryList({"*.cvw"}, QDir::Files);
    if (Cartes.isEmpty() || QFile::exists(stMarque) || (!QDir().mkpath(stDestination)))
    {
        return;
    }
    bool Ok = true;
    for (const QString &stCarte : Cartes)
    {
        const QString stCopie = Destination.filePath(stCarte);
        if (QFile::exists(stCopie))
        {
            continue;
        }
        if (QFile::copy(QDir(stSource).filePath(stCarte), stCopie))
        {
            // QFile::copy garde les droits de la source : une carte doit rester enregistrable
            QFile::setPermissions(stCopie,
                                  QFile::permissions(stCopie) | QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        }
        else
        {
            Ok = false;
        }
    }
    if (Ok)
    {
        QFile Marque(stMarque);
        (void)Marque.open(QIODevice::WriteOnly);
    }
}
//---------------------------------------------------------------------------
// v5.3. S'active automatiquement au lancement de cvw.exe avec le paramètre ! et génère un fichier cvw.log où se trouve
// cvw.exe à la fermeture de l'application Usage : DebugEcrit("message");
bool Debug = false;
std::vector<AnsiString> *slDebug = nullptr; // portage : TStringList -> vecteur de lignes
//---------------------------------------------------------------------------
void DebugOuvre()
{
    Debug = true;
    slDebug = new std::vector<AnsiString>;
}
//---------------------------------------------------------------------------
void DebugEcrit(const AnsiString &asMessage)
{
    if (Debug && slDebug)
    {
        slDebug->push_back(asMessage);
    }
}
//---------------------------------------------------------------------------
void DebugFerme()
{
    const String stCheminEtFichierLog = stRepLocalAppData() + "cvw.log";
    // On vide d'un coup dans un fichier
    if (slDebug)
    {
        QFile f(Q(stCheminEtFichierLog)); // portage : TStringList::SaveToFile
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            for (const AnsiString &as : *slDebug)
            {
                f.write(as.c_str());
                f.write("\r\n");
            }
        }
        delete slDebug;
        slDebug = nullptr;
    }
    Debug = false;
}
//---------------------------------------------------------------------------
namespace
{
// MessageBox de Windows : Échap ne ferme pas une boîte Oui/Non (pas de bouton Annuler)
class SansEchap : public QObject
{
public:
    bool eventFilter(QObject * /*watched*/, QEvent *e) override
    {
        return ((e->type() == QEvent::KeyPress) || (e->type() == QEvent::ShortcutOverride)) &&
               (static_cast<QKeyEvent *>(e)->key() == Qt::Key_Escape);
    }
};
//-------------------------------------------------------------------------
QWidget *FenetreParente()
{ // Application->Handle : la fenêtre active de l'application
    QWidget *w = QApplication::activeModalWidget();
    if (!w)
    {
        w = QApplication::activeWindow();
    }
    return w;
}
} // namespace
//---------------------------------------------------------------------------
type_reponse AfficheMessage(const AnsiString &asMessage, const AnsiString &asTitre, type_message Type,
                            int OrdreBoutonDefaut, int OrdreBoutonEchap)
{ // v5.4 : Conversion BCB6>BCBX (MessageBoxW + String)
    // portage : QMessageBox ; OrdreBoutonEchap était déjà ignoré par MessageBoxW
    (void)OrdreBoutonEchap;
    QMessageBox Boite(Style[Type], Q(asTitre), Q(asMessage), QMessageBox::NoButton, FenetreParente());
    QAbstractButton *Boutons[NBMAXBOUTONS] = {nullptr, nullptr, nullptr};
    SansEchap Filtre;
    switch (NbBoutons[Type])
    {
    case 1:
        Boutons[0] = Boite.addButton(QMessageBox::Ok);
        Boite.setEscapeButton(Boutons[0]);
        break;
    case 2:
        Boutons[0] = Boite.addButton(QMessageBox::Yes);
        Boutons[1] = Boite.addButton(QMessageBox::No);
        Boite.installEventFilter(&Filtre);
        break;
    default:
        Boutons[0] = Boite.addButton(QMessageBox::Yes);
        Boutons[1] = Boite.addButton(QMessageBox::No);
        Boutons[2] = Boite.addButton(QMessageBox::Cancel);
        Boite.setEscapeButton(Boutons[2]);
        break;
    }
    if (QAbstractButton *b = Boutons[BoutonDefaut[Type][OrdreBoutonDefaut]])
    {
        Boite.setDefaultButton(static_cast<QPushButton *>(b));
    }
    Boite.exec();
    switch (Boite.standardButton(Boite.clickedButton()))
    {
    case QMessageBox::Ok:
        return trOk;
    case QMessageBox::Cancel:
        return trAnnuler;
    case QMessageBox::Yes:
        return trOui;
    case QMessageBox::No:
        return trNon;
    default:
        return (NbBoutons[Type] == 3) ? trAnnuler : trOk;
    }
}
//---------------------------------------------------------------------------
int MessageDlg(const AnsiString &asMessage, TMsgDlgType Type, TMsgDlgButtons Boutons, int HelpCtx)
{ // portage : Dialogs::MessageDlg (titres et libellés de la RTL en français)
    (void)HelpCtx;
    const static char *Titres[] = {"Avertissement", "Erreur", "Information", "Confirmation", ""};
    const static QMessageBox::Icon Icones[] = {
        QMessageBox::Warning,  QMessageBox::Critical, QMessageBox::Information,
        QMessageBox::Question, QMessageBox::NoIcon,
    };
    QMessageBox Boite(Icones[Type], QString::fromUtf8(Titres[Type]), Q(asMessage), QMessageBox::NoButton,
                      FenetreParente());
    struct
    {
        TMsgDlgBtn b;
        QMessageBox::StandardButton q;
        int mr;
    } Correspondances[] = {
        {.b = mbYes, .q = QMessageBox::Yes, .mr = mrYes},
        {.b = mbNo, .q = QMessageBox::No, .mr = mrNo},
        {.b = mbOK, .q = QMessageBox::Ok, .mr = mrOk},
        {.b = mbCancel, .q = QMessageBox::Cancel, .mr = mrCancel},
        {.b = mbAbort, .q = QMessageBox::Abort, .mr = mrAbort},
        {.b = mbRetry, .q = QMessageBox::Retry, .mr = mrRetry},
        {.b = mbIgnore, .q = QMessageBox::Ignore, .mr = mrIgnore},
        {.b = mbAll, .q = QMessageBox::YesToAll, .mr = mrAll},
        {.b = mbNoToAll, .q = QMessageBox::NoToAll, .mr = mrNoToAll},
        {.b = mbYesToAll, .q = QMessageBox::YesToAll, .mr = mrYesToAll},
        {.b = mbHelp, .q = QMessageBox::Help, .mr = mrNone},
        {.b = mbClose, .q = QMessageBox::Close, .mr = mrClose},
    };
    for (const auto &c : Correspondances)
    {
        if (Boutons.Contains(c.b))
        {
            Boite.addButton(c.q);
        }
    }
    // CreateMessageDialog : défaut = OK sinon Oui ; Échap = Annuler sinon Non sinon OK
    if (Boutons.Contains(mbOK))
    {
        Boite.setDefaultButton(QMessageBox::Ok);
    }
    else if (Boutons.Contains(mbYes))
    {
        Boite.setDefaultButton(QMessageBox::Yes);
    }
    if (Boutons.Contains(mbCancel))
    {
        Boite.setEscapeButton(QMessageBox::Cancel);
    }
    else if (Boutons.Contains(mbNo))
    {
        Boite.setEscapeButton(QMessageBox::No);
    }
    else if (Boutons.Contains(mbOK))
    {
        Boite.setEscapeButton(QMessageBox::Ok);
    }
    Boite.exec();
    const QMessageBox::StandardButton r = Boite.standardButton(Boite.clickedButton());
    for (const auto &c : Correspondances)
    {
        if (Boutons.Contains(c.b) && (c.q == r))
        {
            return c.mr;
        }
    }
    return mrCancel;
}
//---------------------------------------------------------------------------
// portage : dossier de l'aide (ExtractFilePath(ParamStr(0)) de l'original) :
// html/ et tutocv/ sont à côté de l'exécutable, comme dans l'installeur
// d'origine ({app}\bin). Sous macOS, ils sont dans Contents/Resources du
// paquet : Contents/MacOS ne doit contenir que du code (signature du paquet).
//---------------------------------------------------------------------------
String stRepAide()
{
    QString stRep = QCoreApplication::applicationDirPath();
#ifdef Q_OS_MACOS
    if (stRep.endsWith("/Contents/MacOS"))
    {
        stRep = QDir::cleanPath(stRep + "/../Resources");
    }
#endif
    return A(stRep + "/");
}
//---------------------------------------------------------------------------
// v5.4 : procédure privée (base.cpp) pour afficher une page HTML
// portage : registre + CreateProcess -> QDesktopServices::openUrl. Le système
// ouvre une adresse file:// comme un document et en perd l'ancre #... (macOS :
// constaté ; xdg-open hors GNOME et KDE la retire). Pour une telle adresse, on
// ouvre une page de redirection, écrite dans le dossier temporaire, qui charge
// l'adresse complète : le navigateur arrive ainsi à l'ancre, comme avec
// l'original qui passait l'adresse au navigateur sur sa ligne de commande.
//---------------------------------------------------------------------------
void AffichePageHtml(const String &stURL)
{
    QUrl Url(Q(stURL));
    if (Url.isLocalFile() && Url.hasFragment())
    {
        QString stAncre = Url.fragment();
        stAncre.replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");
        QFile f(QDir(QDir::tempPath()).filePath("cvw-aide-" + stAncre + ".html"));
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            // ' encodé : l'adresse est entre apostrophes dans content="0; url='...'"
            const QString stCible = Url.toString(QUrl::FullyEncoded).replace("'", "%27").toHtmlEscaped();
            f.write(QString("<!DOCTYPE html>\n"
                            "<html><head><meta charset=\"utf-8\">\n"
                            "<meta http-equiv=\"refresh\" content=\"0; url='%1'\">\n"
                            "<title>Centre-Ville</title></head>\n"
                            "<body><a href=\"%1\">Aide de Centre-Ville</a></body></html>\n")
                        .arg(stCible)
                        .toUtf8());
            f.close();
            Url = QUrl::fromLocalFile(f.fileName());
        }
    }
    QDesktopServices::openUrl(Url);
}
//---------------------------------------------------------------------------
void AfficheAideContextuelle(QWidget *Form)
{ // v5.3 : conversion HLP > HTML.
    const int HelpContext = Form ? Form->window()->property("vclHelpContext").toInt() : 0; // portage :
                                                                                           // TForm::HelpContext
    for (const auto i : caCorresp)
    {
        if (i.Numero == HelpContext) // On recherche la rubrique HelpContext dans le tableau caCorresp
        {                            // v5.4 : conversion BCB6>BCBX (L"open" + String)
            const String stURL =
                A(QUrl::fromLocalFile(Q(stRepAide()) + "html/index.html").toString()) + String("#") + String(i.szHREF);
            AffichePageHtml(stURL); // v5.4 : appel à CreateProcess au lieu de ShellExecute
            break;
        }
    }
}
//---------------------------------------------------------------------------
void AfficheIndexAide(QWidget *Form)
{ // v5.3 : conversion HLP > HTML
    // v5.4 : Conversion BCB6>BCBX (String + L"open")
    (void)Form;
    const String stURL = A(QUrl::fromLocalFile(Q(stRepAide()) + "html/index.html").toString());
    AffichePageHtml(stURL); // v5.4 : appel à CreateProcess au lieu de ShellExecute
}
//---------------------------------------------------------------------------
void AfficheTutoriel() // v5.4 (appelée par TfrmSimulation::ActionTutorielExecute
{
    const String stURL = A(QUrl::fromLocalFile(Q(stRepAide()) + "tutocv/index.html").toString());
    AffichePageHtml(stURL);
}
//---------------------------------------------------------------------------

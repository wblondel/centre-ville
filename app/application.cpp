//---------------------------------------------------------------------------
// application.cpp - Portage de Centre-Ville : démarrage de l'application
// (partie de src/cvw.cpp commune à cvw et aux tests de l'interface)
//---------------------------------------------------------------------------
#include <QApplication>
#include <QDir>
#include <QFileOpenEvent>
#include <QFont>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <clocale>
#include <utility>
#include "application.h"
//---------------------------------------------------------------------------
#include "compat/chaine.h"
#include "f_simul.h"
#include "f_defv.h"
#include "f_defvp.h"
#include "f_deff.h"
#include "f_deffp.h"
#include "f_afdistp.h"
#include "f_afdistab.h"
#include "f_afdistat.h"
#include "f_afdistft.h"
#include "f_afdistfp.h"
#include "f_afdistpv.h"
#include "f_stats.h"
#include "f_statsv.h"
#include "f_rech.h"
#include "f_carfours.h"
#include "f_infveh.h"
#include "f_infptn.h"
#include "f_inftaxi.h"
#include "f_inftram.h"
#include "f_infbus.h"
#include "f_infvehlib.h"
#include "f_defn.h"
#include "f_epidemie.h"
#include "vcl.h"
//---------------------------------------------------------------------------
bool OuvertureFichiers::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FileOpen)
    {
        QString f = dynamic_cast<QFileOpenEvent *>(e)->file();
        if (frmSimulation && frmSimulation->isVisible())
        {
            frmSimulation->OuvreFichierDemande(A(QDir::toNativeSeparators(f)));
        }
        else
        {
            EnAttente = std::move(f);
        }
        return true;
    }
    return QObject::eventFilter(o, e);
}
//---------------------------------------------------------------------------
void InitialiseApplication(QApplication & /*App*/)
{
    // Format de la VCL en français : séparateur décimal ',' (compat/chaine.h)
    static_cast<void>(std::setlocale(LC_NUMERIC, "C")); // NOLINT(concurrency-mt-unsafe) : au démarrage, avant tout
                                                        // autre fil
    FormatSettings.DecimalSeparator = ',';
    FormatSettings.ThousandSeparator = "\xC2\xA0";
    // Boutons des boîtes de dialogue en français (« Oui », « Non », « Annuler »)
    // (qtbase_fr.qm, ou qt_fr.qm où windeployqt fusionne les traductions de Qt)
    static QTranslator Traduction;
    const QString stTraductions = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (Traduction.load(QLocale(QLocale::French), "qtbase", "_", stTraductions) ||
        Traduction.load(QLocale(QLocale::French), "qt", "_", stTraductions))
    {
        QApplication::installTranslator(&Traduction);
    }
    // Polices de Windows absentes ailleurs. MS Sans Serif, police « raster »,
    // n'est pas utilisable par Qt même sous Windows : Microsoft Sans Serif, sa
    // version TrueType aux mêmes mesures (fournie par Windows et macOS), puis
    // (v6.0.16) Arial ou ses équivalents libres aux mêmes largeurs (Linux),
    // enfin la police du système, plus large (libellés coupés)
    QFont::insertSubstitutions("MS Sans Serif",
                               {"Microsoft Sans Serif", "Arial", "Liberation Sans", "Arimo", QFont().defaultFamily()});
    QFont::insertSubstitutions("Small Fonts", {"Microsoft Sans Serif", "Arial", "Liberation Sans", "Arimo",
                                               "DejaVu Sans", QFont().defaultFamily()});
#ifdef Q_OS_WIN
    // Style « windowsvista » (contrôles de Windows 10), plus proche de la VCL que
    // « windows11 », le style par défaut de Qt sous Windows 11, dont les flèches
    // côte à côte cachent la valeur des champs numériques étroits (TCSpinEdit)
    QApplication::setStyle("windowsvista");
#endif
    QApplication::setApplicationName("Centre-Ville");
}
//---------------------------------------------------------------------------
void CreeFiches(const QString &asFichierFinder)
{                                   // Application->CreateForm(...) de cvw.cpp, dans le même ordre
    Application = new TApplication; // Application->Initialize();
    // Application->MainFormOnTaskBar = true; // portage : les fiches secondaires ont la fiche principale pour parent
    Application->Title = "Centre-Ville";
    frmSimulation = new TfrmSimulation(Application); // Application->CreateForm(__classid(TfrmSimulation),
                                                     // &frmSimulation);
    if (!asFichierFinder.isEmpty())
    {
        frmSimulation->OuvreFichierDemande(A(asFichierFinder)); // portage : Finder
    }
    frmSimulation->Show(); // portage : Visible = True dans le .dfm : affichée dès sa création
    frmDefVoie = new TfrmDefVoie(Application);
    frmDefVoiePietons = new TfrmDefVoiePietons(Application);
    frmDefFeux = new TfrmDefFeux(Application);
    frmDefFeuxPietons = new TfrmDefFeuxPietons(Application);
    frmAffDistParkings = new TfrmAffDistParkings(Application);
    frmAffDistArretsBus = new TfrmAffDistArretsBus(Application);
    frmAffDistArretsTram = new TfrmAffDistArretsTram(Application);
    frmAffDistFilesTaxi = new TfrmAffDistFilesTaxi(Application);
    frmAffDistFilesPark = new TfrmAffDistFilesPark(Application);
    frmAffDistPlacesVehlib = new TfrmAffDistPlacesVehlib(Application);
    frmStatistiques = new TfrmStatistiques(Application);
    frmStatsVehicules = new TfrmStatsVehicules(Application);
    frmRecherche = new TfrmRecherche(Application);
    frmCarrefours = new TfrmCarrefours(Application);
    frmInfosVehicule = new TfrmInfosVehicule(Application);
    frmInfosPieton = new TfrmInfosPieton(Application);
    frmInfosTaxi = new TfrmInfosTaxi(Application);
    frmInfosTram = new TfrmInfosTram(Application);
    frmInfosBus = new TfrmInfosBus(Application);
    frmInfosVehlib = new TfrmInfosVehlib(Application);
    frmDefNature = new TfrmDefNature(Application);
    frmEpidemie = new TfrmEpidemie(Application);
}
//---------------------------------------------------------------------------
void DetruitFiches()
{ // Ordre inverse de la création (TComponent::DestroyComponents)
    delete frmEpidemie;
    frmEpidemie = nullptr;
    delete frmDefNature;
    frmDefNature = nullptr;
    delete frmInfosVehlib;
    frmInfosVehlib = nullptr;
    delete frmInfosBus;
    frmInfosBus = nullptr;
    delete frmInfosTram;
    frmInfosTram = nullptr;
    delete frmInfosTaxi;
    frmInfosTaxi = nullptr;
    delete frmInfosPieton;
    frmInfosPieton = nullptr;
    delete frmInfosVehicule;
    frmInfosVehicule = nullptr;
    delete frmCarrefours;
    frmCarrefours = nullptr;
    delete frmRecherche;
    frmRecherche = nullptr;
    delete frmStatsVehicules;
    frmStatsVehicules = nullptr;
    delete frmStatistiques;
    frmStatistiques = nullptr;
    delete frmAffDistPlacesVehlib;
    frmAffDistPlacesVehlib = nullptr;
    delete frmAffDistFilesPark;
    frmAffDistFilesPark = nullptr;
    delete frmAffDistFilesTaxi;
    frmAffDistFilesTaxi = nullptr;
    delete frmAffDistArretsTram;
    frmAffDistArretsTram = nullptr;
    delete frmAffDistArretsBus;
    frmAffDistArretsBus = nullptr;
    delete frmAffDistParkings;
    frmAffDistParkings = nullptr;
    delete frmDefFeuxPietons;
    frmDefFeuxPietons = nullptr;
    delete frmDefFeux;
    frmDefFeux = nullptr;
    delete frmDefVoiePietons;
    frmDefVoiePietons = nullptr;
    delete frmDefVoie;
    frmDefVoie = nullptr;
    delete frmSimulation;
    frmSimulation = nullptr;
    delete Application;
    Application = nullptr;
}
//---------------------------------------------------------------------------

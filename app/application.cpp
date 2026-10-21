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
#include "application.h"
//---------------------------------------------------------------------------
#include "base.h"
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
//---------------------------------------------------------------------------
bool OuvertureFichiers::eventFilter(QObject *o, QEvent *e)
 {
  if (e->type()==QEvent::FileOpen)
   {
    QString f=static_cast<QFileOpenEvent *>(e)->file();
    if (frmSimulation&&frmSimulation->isVisible())
     frmSimulation->OuvreFichierDemande(A(QDir::toNativeSeparators(f)));
    else
     EnAttente=f;
    return true;
   }
  return QObject::eventFilter(o, e);
 }
//---------------------------------------------------------------------------
void InitialiseApplication(QApplication &App)
 {
  // Format de la VCL en français : séparateur décimal ',' (compat/chaine.h)
  std::setlocale(LC_NUMERIC, "C");
  FormatSettings.DecimalSeparator=',';
  FormatSettings.ThousandSeparator="\xC2\xA0";
  // Boutons des boîtes de dialogue en français (« Oui », « Non », « Annuler »)
  static QTranslator Traduction;
  if (Traduction.load(QLocale(QLocale::French), "qtbase", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
   App.installTranslator(&Traduction);
  // Polices de Windows absentes ailleurs
  QFont::insertSubstitution("MS Sans Serif", QFont().defaultFamily());
  QFont::insertSubstitutions("Small Fonts", {"Verdana", "Tahoma", "DejaVu Sans", QFont().defaultFamily()});
  QApplication::setApplicationName("Centre-Ville");
 }
//---------------------------------------------------------------------------
void CreeFiches(const QString &asFichierFinder)
 { // Application->CreateForm(...) de cvw.cpp, dans le même ordre
  Application=new TApplication; // Application->Initialize();
  // Application->MainFormOnTaskBar = true; // portage : les fiches secondaires ont la fiche principale pour parent
  Application->Title = "Centre-Ville";
  frmSimulation=new TfrmSimulation(Application); // Application->CreateForm(__classid(TfrmSimulation), &frmSimulation);
  if (!asFichierFinder.isEmpty()) frmSimulation->OuvreFichierDemande(A(asFichierFinder)); // portage : Finder
  frmSimulation->Show(); // portage : Visible = True dans le .dfm : affichée dès sa création
  frmDefVoie=new TfrmDefVoie(Application);
  frmDefVoiePietons=new TfrmDefVoiePietons(Application);
  frmDefFeux=new TfrmDefFeux(Application);
  frmDefFeuxPietons=new TfrmDefFeuxPietons(Application);
  frmAffDistParkings=new TfrmAffDistParkings(Application);
  frmAffDistArretsBus=new TfrmAffDistArretsBus(Application);
  frmAffDistArretsTram=new TfrmAffDistArretsTram(Application);
  frmAffDistFilesTaxi=new TfrmAffDistFilesTaxi(Application);
  frmAffDistFilesPark=new TfrmAffDistFilesPark(Application);
  frmAffDistPlacesVehlib=new TfrmAffDistPlacesVehlib(Application);
  frmStatistiques=new TfrmStatistiques(Application);
  frmStatsVehicules=new TfrmStatsVehicules(Application);
  frmRecherche=new TfrmRecherche(Application);
  frmCarrefours=new TfrmCarrefours(Application);
  frmInfosVehicule=new TfrmInfosVehicule(Application);
  frmInfosPieton=new TfrmInfosPieton(Application);
  frmInfosTaxi=new TfrmInfosTaxi(Application);
  frmInfosTram=new TfrmInfosTram(Application);
  frmInfosBus=new TfrmInfosBus(Application);
  frmInfosVehlib=new TfrmInfosVehlib(Application);
  frmDefNature=new TfrmDefNature(Application);
  frmEpidemie=new TfrmEpidemie(Application);
 }
//---------------------------------------------------------------------------
void DetruitFiches()
 { // Ordre inverse de la création (TComponent::DestroyComponents)
  delete frmEpidemie; frmEpidemie=NULL;
  delete frmDefNature; frmDefNature=NULL;
  delete frmInfosVehlib; frmInfosVehlib=NULL;
  delete frmInfosBus; frmInfosBus=NULL;
  delete frmInfosTram; frmInfosTram=NULL;
  delete frmInfosTaxi; frmInfosTaxi=NULL;
  delete frmInfosPieton; frmInfosPieton=NULL;
  delete frmInfosVehicule; frmInfosVehicule=NULL;
  delete frmCarrefours; frmCarrefours=NULL;
  delete frmRecherche; frmRecherche=NULL;
  delete frmStatsVehicules; frmStatsVehicules=NULL;
  delete frmStatistiques; frmStatistiques=NULL;
  delete frmAffDistPlacesVehlib; frmAffDistPlacesVehlib=NULL;
  delete frmAffDistFilesPark; frmAffDistFilesPark=NULL;
  delete frmAffDistFilesTaxi; frmAffDistFilesTaxi=NULL;
  delete frmAffDistArretsTram; frmAffDistArretsTram=NULL;
  delete frmAffDistArretsBus; frmAffDistArretsBus=NULL;
  delete frmAffDistParkings; frmAffDistParkings=NULL;
  delete frmDefFeuxPietons; frmDefFeuxPietons=NULL;
  delete frmDefFeux; frmDefFeux=NULL;
  delete frmDefVoiePietons; frmDefVoiePietons=NULL;
  delete frmDefVoie; frmDefVoie=NULL;
  delete frmSimulation; frmSimulation=NULL;
  delete Application; Application=NULL;
 }
//---------------------------------------------------------------------------

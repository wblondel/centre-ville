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
#include <vector>
//---------------------------------------------------------------------------
#include "base.h"
//---------------------------------------------------------------------------
#define NBTYPESMESSAGE  5
#define NBTYPESREPONSE  4
#define NBMAXBOUTONS    3
#define NBCONTEXTESAIDE 57
//---------------------------------------------------------------------------
// portage : MessageBoxW -> QMessageBox (mêmes icônes, boutons et bouton par défaut)
const QMessageBox::Icon Style[NBTYPESMESSAGE]={QMessageBox::Information, QMessageBox::Warning, QMessageBox::Critical, QMessageBox::Question, QMessageBox::Question};
const int NbBoutons[NBTYPESMESSAGE]={1, 1, 1, 2, 3}; // MB_OK, MB_OK, MB_OK, MB_YESNO, MB_YESNOCANCEL
const int BoutonDefaut[NBTYPESMESSAGE][NBMAXBOUTONS]={ {0,0,0}, // MB_DEFBUTTONn - 1
                                                       {0,0,0},
                                                       {0,0,0},
                                                       {0,1,0},
                                                       {0,1,2}};
//---------------------------------------------------------------------------
const AnsiString asDebutNomSpinEdit = "CSpinEdit";
//---------------------------------------------------------------------------
const struct { int Numero; const char *szHREF; } // v5.3 : aide type HLP converti en type HTML : correspondances TForm::HelpContext en URL "html\index.html#"
      caCorresp[NBCONTEXTESAIDE] =
                      {{ 0, "ASCV"},
                       {10, "BR"},
                       {20, "EDLS"},
                       {30, "PDLI"},
                       {31, "LMP"},
                       {3101, "LMF"},
                       {3102, "LMA"},
                       {3103, "LMSI"},
                       {3104, "LMST"},
                       {3105, "LMI"},
                       {311, "PDLS"},
                       {3111, "DDDDLZDS"},
                       {3112, "GADUR"},
                       {3121, "PDRDB"},
                       {3122, "PDRDT"},
                       {3123, "PDT"},
                       {3124, "PDV"},
                       {313, "CDLS"},
                       {314, "SDLS"},
                       {315, "SSLCDR"},
					   {316, "PDLADSDE"},
                       {317, "PDLE"}, // v5.4.1
                       {32, "LBDO"},
                       {33, "LZDS"},
                       {3301, "LDEDLZDS"},
                       {3302, "LMD"},
                       {34, "LBDE"},
                       {40, "CDR"},
                       {41, "DDP"},
                       {415, "DDPP"},
                       {42, "DDF"},
                       {425, "DDFDP"},
                       {431, "ODC"},
                       {4311, "OCUCS"},
                       {4312, "OCPCS"},
                       {432, "C"},
                       {44, "BDN"},
                       {45, "DDLNDUZ"},
                       {50, "LDLS"},
                       {611, "EGDLS"},
                       {612, "SDFEDT"},
                       {62, "SDVDUEAUA"},
                       {63, "ADDJAE"},
                       {641, "ADDJAADB"},
                       {642, "ADDJAADT"},
                       {643, "ADDJAFDT"},
					   {644, "ADDJAFDP"},
                       {645, "ADDJAPDV"},
					   {65, "RDUEDLS"},
                       {66, "ISUV"},
                       {671, "ISUB"},
                       {672, "ISUT"},
                       {673, "ISUVDT"},
                       {674, "ISUVL"},
                       {68, "ISUP"},
                       {69, "RC"},
                       {70, "MDE"}};
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
   String stExePath = A(QDir::toNativeSeparators(QCoreApplication::applicationDirPath()+"/"));
   QString stRepLocAppDataCvw = "/Patquoi.fr/Centre-Ville/";
   QString stBase = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
   if (stBase.isEmpty())
     stResult = stExePath;
   else
	{
	 QString stRep = stBase + stRepLocAppDataCvw;
	 if (!QDir(stRep).exists())
	   if (!QDir().mkpath(stRep))
		 return stExePath;
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
  QDir Destination(stDestination);
  QString stMarque=Destination.filePath(".cartes-copiees");
  QStringList Cartes=QDir(stSource).entryList({"*.cvw"}, QDir::Files);
  if (Cartes.isEmpty()||QFile::exists(stMarque)||(!QDir().mkpath(stDestination))) return;
  bool Ok=true;
  for(const QString &stCarte : Cartes)
   {
    QString stCopie=Destination.filePath(stCarte);
    if (QFile::exists(stCopie)) continue;
    if (QFile::copy(QDir(stSource).filePath(stCarte), stCopie))
     // QFile::copy garde les droits de la source : une carte doit rester enregistrable
     QFile::setPermissions(stCopie, QFile::permissions(stCopie)|QFileDevice::ReadOwner|QFileDevice::WriteOwner);
    else
     Ok=false;
   }
  if (Ok)
   {
    QFile Marque(stMarque);
    (void)Marque.open(QIODevice::WriteOnly);
   }
 }
//---------------------------------------------------------------------------
// v5.3. S'active automatiquement au lancement de cvw.exe avec le paramètre ! et génère un fichier cvw.log où se trouve cvw.exe à la fermeture de l'application
// Usage : DebugEcrit("message");
bool Debug = false;
std::vector<AnsiString> *slDebug = NULL; // portage : TStringList -> vecteur de lignes
//---------------------------------------------------------------------------
void DebugOuvre()
 {
  Debug = true;
  slDebug = new std::vector<AnsiString>;
 }
//---------------------------------------------------------------------------
void DebugEcrit(const AnsiString asMessage)
 {
  if (Debug&&slDebug)
   slDebug->push_back(asMessage);
 }
//---------------------------------------------------------------------------
void DebugFerme()
 {
  String stCheminEtFichierLog = stRepLocalAppData()+"cvw.log";
  // On vide d'un coup dans un fichier
  if (slDebug)
   {
	QFile f(Q(stCheminEtFichierLog)); // portage : TStringList::SaveToFile
	if (f.open(QIODevice::WriteOnly|QIODevice::Truncate))
	 for(const AnsiString &as : *slDebug)
	  {
	   f.write(as.c_str());
	   f.write("\r\n");
	  }
	delete slDebug;
	slDebug = NULL;
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
    bool eventFilter(QObject *, QEvent *e) override
     {
      return ((e->type()==QEvent::KeyPress)||(e->type()==QEvent::ShortcutOverride))&&
             (static_cast<QKeyEvent *>(e)->key()==Qt::Key_Escape);
     }
   };
  //-------------------------------------------------------------------------
  QWidget *FenetreParente()
   { // Application->Handle : la fenêtre active de l'application
    QWidget *w=QApplication::activeModalWidget();
    if (!w) w=QApplication::activeWindow();
    return w;
   }
 }
//---------------------------------------------------------------------------
type_reponse AfficheMessage(const AnsiString asMessage, const AnsiString asTitre, type_message Type, int OrdreBoutonDefaut, int OrdreBoutonEchap)
 {// v5.4 : Conversion BCB6>BCBX (MessageBoxW + String)
  // portage : QMessageBox ; OrdreBoutonEchap était déjà ignoré par MessageBoxW
  (void)OrdreBoutonEchap;
  QMessageBox Boite(Style[Type], Q(asTitre), Q(asMessage), QMessageBox::NoButton, FenetreParente());
  QAbstractButton *Boutons[NBMAXBOUTONS]={nullptr, nullptr, nullptr};
  SansEchap Filtre;
  switch(NbBoutons[Type])
   {
    case 1: Boutons[0]=Boite.addButton(QMessageBox::Ok);
            Boite.setEscapeButton(Boutons[0]);
            break;
    case 2: Boutons[0]=Boite.addButton(QMessageBox::Yes);
            Boutons[1]=Boite.addButton(QMessageBox::No);
            Boite.installEventFilter(&Filtre);
            break;
    default:Boutons[0]=Boite.addButton(QMessageBox::Yes);
            Boutons[1]=Boite.addButton(QMessageBox::No);
            Boutons[2]=Boite.addButton(QMessageBox::Cancel);
            Boite.setEscapeButton(Boutons[2]);
            break;
   }
  if (QAbstractButton *b=Boutons[BoutonDefaut[Type][OrdreBoutonDefaut]])
   Boite.setDefaultButton(static_cast<QPushButton *>(b));
  Boite.exec();
  switch(Boite.standardButton(Boite.clickedButton()))
   {
    case QMessageBox::Ok :     return trOk;
    case QMessageBox::Cancel : return trAnnuler;
    case QMessageBox::Yes :    return trOui;
    case QMessageBox::No:      return trNon;
	default :                  return (NbBoutons[Type]==3)?trAnnuler:trOk;
   }
 }
//---------------------------------------------------------------------------
int MessageDlg(const AnsiString asMessage, TMsgDlgType Type, TMsgDlgButtons Boutons, int HelpCtx)
 { // portage : Dialogs::MessageDlg (titres et libellés de la RTL en français)
  (void)HelpCtx;
  static const char *Titres[]={"Avertissement", "Erreur", "Information", "Confirmation", ""};
  static const QMessageBox::Icon Icones[]={QMessageBox::Warning, QMessageBox::Critical, QMessageBox::Information,
                                           QMessageBox::Question, QMessageBox::NoIcon};
  QMessageBox Boite(Icones[Type], QString::fromUtf8(Titres[Type]), Q(asMessage), QMessageBox::NoButton, FenetreParente());
  struct { TMsgDlgBtn b; QMessageBox::StandardButton q; int mr; } Correspondances[]=
   {{mbYes, QMessageBox::Yes, mrYes}, {mbNo, QMessageBox::No, mrNo}, {mbOK, QMessageBox::Ok, mrOk},
    {mbCancel, QMessageBox::Cancel, mrCancel}, {mbAbort, QMessageBox::Abort, mrAbort},
    {mbRetry, QMessageBox::Retry, mrRetry}, {mbIgnore, QMessageBox::Ignore, mrIgnore},
    {mbAll, QMessageBox::YesToAll, mrAll}, {mbNoToAll, QMessageBox::NoToAll, mrNoToAll},
    {mbYesToAll, QMessageBox::YesToAll, mrYesToAll}, {mbHelp, QMessageBox::Help, mrNone},
    {mbClose, QMessageBox::Close, mrClose}};
  for(auto &c : Correspondances)
   if (Boutons.Contains(c.b)) Boite.addButton(c.q);
  // CreateMessageDialog : défaut = OK sinon Oui ; Échap = Annuler sinon Non sinon OK
  if (Boutons.Contains(mbOK)) Boite.setDefaultButton(QMessageBox::Ok);
  else if (Boutons.Contains(mbYes)) Boite.setDefaultButton(QMessageBox::Yes);
  if (Boutons.Contains(mbCancel)) Boite.setEscapeButton(QMessageBox::Cancel);
  else if (Boutons.Contains(mbNo)) Boite.setEscapeButton(QMessageBox::No);
  else if (Boutons.Contains(mbOK)) Boite.setEscapeButton(QMessageBox::Ok);
  Boite.exec();
  QMessageBox::StandardButton r=Boite.standardButton(Boite.clickedButton());
  for(auto &c : Correspondances)
   if (Boutons.Contains(c.b)&&(c.q==r)) return c.mr;
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
	stRep = QDir::cleanPath(stRep+"/../Resources");
#endif
  return A(stRep+"/");
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
void AffichePageHtml(const String stURL)
 {
  QUrl Url(Q(stURL));
  if (Url.isLocalFile()&&Url.hasFragment())
   {
	QString stAncre = Url.fragment();
	stAncre.replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");
	QFile f(QDir(QDir::tempPath()).filePath("cvw-aide-"+stAncre+".html"));
	if (f.open(QIODevice::WriteOnly|QIODevice::Truncate))
	 {
	  // ' encodé : l'adresse est entre apostrophes dans content="0; url='...'"
	  QString stCible = Url.toString(QUrl::FullyEncoded).replace("'", "%27").toHtmlEscaped();
	  f.write(QString("<!DOCTYPE html>\n"
					  "<html><head><meta charset=\"utf-8\">\n"
					  "<meta http-equiv=\"refresh\" content=\"0; url='%1'\">\n"
					  "<title>Centre-Ville</title></head>\n"
					  "<body><a href=\"%1\">Aide de Centre-Ville</a></body></html>\n").arg(stCible).toUtf8());
	  f.close();
	  Url = QUrl::fromLocalFile(f.fileName());
	 }
   }
  QDesktopServices::openUrl(Url);
 }
//---------------------------------------------------------------------------
void AfficheAideContextuelle(QWidget *Form)
 {// v5.3 : conversion HLP > HTML.
  int HelpContext=Form?Form->window()->property("vclHelpContext").toInt():0; // portage : TForm::HelpContext
  for(int i=0; i<NBCONTEXTESAIDE; i++)
	if (caCorresp[i].Numero==HelpContext) // On recherche la rubrique HelpContext dans le tableau caCorresp
	 { // v5.4 : conversion BCB6>BCBX (L"open" + String)
	  String stURL = A(QUrl::fromLocalFile(Q(stRepAide())+"html/index.html").toString())+
					 String("#")+
					 String(caCorresp[i].szHREF);
	  AffichePageHtml(stURL); // v5.4 : appel à CreateProcess au lieu de ShellExecute
	  break;
	 }
 }
//---------------------------------------------------------------------------
void AfficheIndexAide(QWidget *Form)
 {// v5.3 : conversion HLP > HTML
  // v5.4 : Conversion BCB6>BCBX (String + L"open")
  (void)Form;
  String stURL = A(QUrl::fromLocalFile(Q(stRepAide())+"html/index.html").toString());
  AffichePageHtml(stURL); // v5.4 : appel à CreateProcess au lieu de ShellExecute
 }
//---------------------------------------------------------------------------
void AfficheTutoriel() // v5.4 (appelée par TfrmSimulation::ActionTutorielExecute
 {
  String stURL = A(QUrl::fromLocalFile(Q(stRepAide())+"tutocv/index.html").toString());
  AffichePageHtml(stURL);
 }
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include <QDesktopServices> // portage : ShellExecute -> QDesktopServices::openUrl
#include <QUrl> // portage
//#include <shellapi.hpp> // portage
//---------------------------------------------------------------------------
#include "f_propos.h"
#include "f_simul.h"
//---------------------------------------------------------------------------
#include "ui_f_propos.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_propos_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
// ATTENTION : LE CHANGEMENT DE VERSION DOIT ÊTRE RÉPERCUTÉ sur :
// · Le numéro de version dans les paramètres du projet
// · Le #define VERSION en hexadécimal (simul.h) si le format de fichier a changé
// · La bitmap ici ImageVersion et la sous-version dans LabelSousVersion
//---------------------------------------------------------------------------
TfrmAPropos *frmAPropos;
//---------------------------------------------------------------------------
TfrmAPropos::TfrmAPropos(TComponent* Owner) : TForm(Owner)
 { ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
  #if defined(_WIN64)||defined(__LP64__) // portage : 64 bits aussi hors de Windows (Qt 6 n'existe qu'en 64 bits)
  LabelAlignement->Caption=Format(LabelAlignement->Caption, ARRAYOFCONST((64)));
  #else
  LabelAlignement->Caption=Format(LabelAlignement->Caption, ARRAYOFCONST((32)));
  #endif
  LabelHDPI->Caption = LabelHDPI->Caption + FormatFloat("0.0", Screen->PixelsPerInch/96.0); // v5.4.1
 }
//---------------------------------------------------------------------------
void TfrmAPropos::TimerTimer(TObject *Sender)
 {
  switch((TempsEcoule++)%10)
   {
    case 0: VirtualImageList->GetBitmap(0, Bitmap);
            LabelTexte->Font->Color=clLime;
            break;
    case 4: VirtualImageList->GetBitmap(1, Bitmap);
            LabelTexte->Font->Color=clYellow;
            break;
    case 6: VirtualImageList->GetBitmap(2, Bitmap);
            LabelTexte->Font->Color=clRed;
            break;
   }
  SpeedButtonOk->Glyph=Bitmap;
  if (FermetureAutomatique&&(TempsEcoule==2))
   {
    FermetureAutomatique=false;
    Close();
   }
 }
//---------------------------------------------------------------------------
void TfrmAPropos::FormShow(TObject *Sender)
{
 TempsEcoule=0;
 Bitmap=new TBITMAP;
 VirtualImageList->GetBitmap(0, Bitmap);
 SpeedButtonOk->Glyph=Bitmap;
 Timer->Enabled=true;
}
//---------------------------------------------------------------------------
void TfrmAPropos::FormHide(TObject *Sender)
{
 Timer->Enabled=false;
 delete Bitmap;
}
//---------------------------------------------------------------------------
void TfrmAPropos::LabelURLClick(TObject *Sender)
{ // v5.4 : conversion BCB6>BCBX (string)
 // portage : ShellExecute(NULL, L"open", LabelURL->Caption.c_str(), NULL, NULL, SW_SHOWMAXIMIZED)
 QDesktopServices::openUrl(QUrl(Q(LabelURL->Caption)));
}
//---------------------------------------------------------------------------
void TfrmAPropos::LabelEMailClick(TObject *Sender)
{ // v5.4 : conversion BCB6>BCBX (string)
 // portage : ShellExecute(NULL, L"open", String("mailto:"+...).c_str(), NULL, NULL, SW_SHOWMAXIMIZED)
 QDesktopServices::openUrl(QUrl(Q(String("mailto:"+LabelEMail->Caption+"?subject=Centre-ville"))));
}
//---------------------------------------------------------------------------
void TfrmAPropos::FormCreate(TObject *Sender)
{
 FermetureAutomatique=false;
}
//---------------------------------------------------------------------------
void TfrmAPropos::FormKeyPress(TObject *Sender, char &Key)
{
 if ((Key==13)||(Key==27)) SpeedButtonOk->Click();
}
//---------------------------------------------------------------------------
void TfrmAPropos::LabelClick(TObject *Sender)
{ // v5.2.3. : où que l'on clique, ça apparaît + Texte revu
 AppelleAuChaos(); // v5.2.3 : Déplacé ici
 // v5.4 : conversion BCB6>BCBX (Application-> supprimé + String)
 // portage : MessageBoxW(Application->Handle, ..., MB_ICONINFORMATION) -> AfficheMessage (même boîte, base.cpp)
 AfficheMessage(String("\
Centre-Ville est sous Licence Publique Générale version 2.0.\n\
1998-2018, Patrice FOUQUET\n\n\
Se reporter au fichier licence.txt.\n\
Centre-Ville est fourni SANS AUCUNE GARANTIE."), String("Licence"), tmInformation, 0, 0);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmAPropos::Gestionnaire);
void TfrmAPropos::ChargeFiche()
 {
  Ui::frmAPropos().setupUi(this);
  LieurComposants Lieur(this, "f_propos", Composants);
  COMPOSANTS_frmAPropos(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmAPropos(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------

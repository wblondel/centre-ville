//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include <QDesktopServices> // portage : ShellExecute -> QDesktopServices::openUrl
#include <QImage> // portage
#include <QPixmap> // portage
#include <QUrl> // portage
#include <vector> // portage
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
// portage : le numéro de version est celui de project(VERSION) du
// CMakeLists.txt racine (CV_VERSION_*) ; ImageVersion et LabelSousVersion en
// découlent (ImageDeVersion)
//---------------------------------------------------------------------------
TfrmAPropos *frmAPropos;
//---------------------------------------------------------------------------
// portage : la bitmap ImageVersion (« 5,4 » de la v5.4) est faite de traits
// horizontaux sur 7 bandes de 3 pixels : le trait [x0, x1] d'une bande est
// clair sur sa 1re ligne, et a une ombre décalée d'un pixel à gauche sur les
// deux suivantes. Les chiffres 4 et 5 sont ceux de la bitmap d'origine
// (redessinée à l'identique), les autres sont dessinés de la même façon.
//---------------------------------------------------------------------------
namespace
 {
  struct Trait { int Bande, x0, x1; };
  const std::vector<Trait> Chiffres[10]=
   {
    {{.Bande=0,.x0=1,.x1=11}, {.Bande=1,.x0=1,.x1=4}, {.Bande=1,.x0=8,.x1=11}, {.Bande=2,.x0=1,.x1=4}, {.Bande=2,.x0=8,.x1=11}, {.Bande=3,.x0=1,.x1=4}, {.Bande=3,.x0=8,.x1=11}, {.Bande=4,.x0=1,.x1=4}, {.Bande=4,.x0=8,.x1=11}, {.Bande=5,.x0=1,.x1=4}, {.Bande=5,.x0=8,.x1=11}, {.Bande=6,.x0=4,.x1=8}}, // 0
    {{.Bande=0,.x0=8,.x1=11}, {.Bande=1,.x0=8,.x1=11}, {.Bande=2,.x0=8,.x1=11}, {.Bande=3,.x0=8,.x1=11}, {.Bande=4,.x0=8,.x1=11}, {.Bande=5,.x0=8,.x1=11}, {.Bande=6,.x0=8,.x1=11}},                                       // 1
    {{.Bande=0,.x0=1,.x1=11}, {.Bande=1,.x0=8,.x1=11}, {.Bande=2,.x0=8,.x1=11}, {.Bande=3,.x0=1,.x1=11}, {.Bande=4,.x0=1,.x1=4}, {.Bande=5,.x0=1,.x1=4}, {.Bande=6,.x0=1,.x1=11}},                                         // 2
    {{.Bande=0,.x0=1,.x1=11}, {.Bande=1,.x0=8,.x1=11}, {.Bande=2,.x0=8,.x1=11}, {.Bande=3,.x0=4,.x1=11}, {.Bande=4,.x0=8,.x1=11}, {.Bande=5,.x0=8,.x1=11}, {.Bande=6,.x0=4,.x1=8}},                                        // 3
    {{.Bande=0,.x0=4,.x1=8}, {.Bande=1,.x0=3,.x1=6}, {.Bande=2,.x0=2,.x1=5}, {.Bande=2,.x0=8,.x1=11}, {.Bande=3,.x0=1,.x1=4}, {.Bande=3,.x0=8,.x1=11}, {.Bande=4,.x0=1,.x1=11}, {.Bande=5,.x0=8,.x1=11}, {.Bande=6,.x0=8,.x1=11}},                      // 4 (d'origine)
    {{.Bande=0,.x0=1,.x1=11}, {.Bande=1,.x0=1,.x1=4}, {.Bande=2,.x0=1,.x1=8}, {.Bande=3,.x0=8,.x1=11}, {.Bande=4,.x0=1,.x1=4}, {.Bande=4,.x0=8,.x1=11}, {.Bande=5,.x0=1,.x1=4}, {.Bande=5,.x0=8,.x1=11}, {.Bande=6,.x0=4,.x1=8}},                      // 5 (d'origine)
    {{.Bande=0,.x0=1,.x1=11}, {.Bande=1,.x0=1,.x1=4}, {.Bande=2,.x0=1,.x1=8}, {.Bande=3,.x0=1,.x1=4}, {.Bande=3,.x0=8,.x1=11}, {.Bande=4,.x0=1,.x1=4}, {.Bande=4,.x0=8,.x1=11}, {.Bande=5,.x0=1,.x1=4}, {.Bande=5,.x0=8,.x1=11}, {.Bande=6,.x0=4,.x1=8}},            // 6
    {{.Bande=0,.x0=1,.x1=11}, {.Bande=1,.x0=8,.x1=11}, {.Bande=2,.x0=8,.x1=11}, {.Bande=3,.x0=8,.x1=11}, {.Bande=4,.x0=8,.x1=11}, {.Bande=5,.x0=8,.x1=11}, {.Bande=6,.x0=8,.x1=11}},                                       // 7
    {{.Bande=0,.x0=1,.x1=11}, {.Bande=1,.x0=1,.x1=4}, {.Bande=1,.x0=8,.x1=11}, {.Bande=2,.x0=1,.x1=4}, {.Bande=2,.x0=8,.x1=11}, {.Bande=3,.x0=1,.x1=11}, {.Bande=4,.x0=1,.x1=4}, {.Bande=4,.x0=8,.x1=11}, {.Bande=5,.x0=1,.x1=4}, {.Bande=5,.x0=8,.x1=11}, {.Bande=6,.x0=4,.x1=8}}, // 8
    {{.Bande=0,.x0=1,.x1=11}, {.Bande=1,.x0=1,.x1=4}, {.Bande=1,.x0=8,.x1=11}, {.Bande=2,.x0=1,.x1=4}, {.Bande=2,.x0=8,.x1=11}, {.Bande=3,.x0=1,.x1=11}, {.Bande=4,.x0=8,.x1=11}, {.Bande=5,.x0=8,.x1=11}, {.Bande=6,.x0=4,.x1=8}},                     // 9
   };
  void DessineTrait(QImage &Image, const Trait &t, int x, QRgb Clair, QRgb Ombre)
   {
    int const y=1+(3*t.Bande);
    for(int i=t.x0; i<=t.x1; i++)
     {
      Image.setPixel(x+i, y, Clair);
      Image.setPixel(x+i, y+1, Clair);
      Image.setPixel(x+i-1, y+2, Ombre);
     }
    Image.setPixel(x+t.x0-1, y+1, Ombre);
   }
 }
//---------------------------------------------------------------------------
QPixmap ImageDeVersion(int Majeure, int Mineure)
 {
  QString const stMajeure=QString::number(Majeure);
  QString const stMineure=QString::number(Mineure);
  QImage Image((13*(stMajeure.size()+stMineure.size()))+2, 22, QImage::Format_ARGB32);
  Image.fill(qRgb(0x00, 0x00, 0x00));
  int x=0;
  for(QChar const c : stMajeure) { for(const Trait &t : Chiffres[c.digitValue()]) { DessineTrait(Image, t, x, qRgb(0xB5, 0xE6, 0x1D), qRgb(0x22, 0xB1, 0x4C)); 
}x+=13; }
  DessineTrait(Image, {.Bande=6, .x0=0, .x1=1}, x, qRgb(0xFF, 0xFF, 0x00), qRgb(0x80, 0x80, 0x00)); // virgule
  x+=2;
  for(QChar const c : stMineure) { for(const Trait &t : Chiffres[c.digitValue()]) { DessineTrait(Image, t, x, qRgb(0xB5, 0xE6, 0x1D), qRgb(0x22, 0xB1, 0x4C)); 
}x+=13; }
  return QPixmap::fromImage(Image);
 }
//---------------------------------------------------------------------------
TfrmAPropos::TfrmAPropos(TComponent* Owner) : TForm(Owner)
 { ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
  ImageVersion->setPixmap(ImageDeVersion(CV_VERSION_MAJEURE, CV_VERSION_MINEURE)); // portage : version de CMakeLists.txt
  LabelSousVersion->Caption = AnsiString(",")+IntToStr(CV_VERSION_CORRECTIVE); // portage
  #if defined(_WIN64)||defined(__LP64__) // portage : 64 bits aussi hors de Windows (Qt 6 n'existe qu'en 64 bits)
  LabelAlignement->Caption=Format(LabelAlignement->Caption, ARRAYOFCONST((64)));
  #else
  LabelAlignement->Caption=Format(LabelAlignement->Caption, ARRAYOFCONST((32)));
  #endif
  LabelHDPI->Caption = LabelHDPI->Caption + FormatFloat("0.0", Screen->PixelsPerInch/96.0); // v5.4.1
 }
//---------------------------------------------------------------------------
void TfrmAPropos::TimerTimer(TObject * /*Sender*/)
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
void TfrmAPropos::FormShow(TObject * /*Sender*/)
{
 TempsEcoule=0;
 Bitmap=new TBITMAP;
 VirtualImageList->GetBitmap(0, Bitmap);
 SpeedButtonOk->Glyph=Bitmap;
 Timer->Enabled=true;
}
//---------------------------------------------------------------------------
void TfrmAPropos::FormHide(TObject * /*Sender*/)
{
 Timer->Enabled=false;
 delete Bitmap;
}
//---------------------------------------------------------------------------
void TfrmAPropos::LabelURLClick(TObject * /*Sender*/) const
{ // v5.4 : conversion BCB6>BCBX (string)
 // portage : ShellExecute(NULL, L"open", LabelURL->Caption.c_str(), NULL, NULL, SW_SHOWMAXIMIZED)
 QDesktopServices::openUrl(QUrl(Q(LabelURL->Caption)));
}
//---------------------------------------------------------------------------
void TfrmAPropos::LabelEMailClick(TObject * /*Sender*/) const
{ // v5.4 : conversion BCB6>BCBX (string)
 // portage : ShellExecute(NULL, L"open", String("mailto:"+...).c_str(), NULL, NULL, SW_SHOWMAXIMIZED)
 QDesktopServices::openUrl(QUrl(Q(String("mailto:"+LabelEMail->Caption+"?subject=Centre-ville"))));
}
//---------------------------------------------------------------------------
void TfrmAPropos::FormCreate(TObject * /*Sender*/)
{
 FermetureAutomatique=false;
}
//---------------------------------------------------------------------------
void TfrmAPropos::FormKeyPress(TObject * /*Sender*/, char &Key) const
{
 if ((Key==13)||(Key==27)) { SpeedButtonOk->Click();
}
}
//---------------------------------------------------------------------------
void TfrmAPropos::LabelClick(TObject * /*Sender*/)
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
  if (OnCreate) { OnCreate(this);
}
 }
//---------------------------------------------------------------------------

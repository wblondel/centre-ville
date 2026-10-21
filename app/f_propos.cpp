//---------------------------------------------------------------------------
// f_propos.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_propos.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_propos.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_propos.h"
#include "ui_f_propos.h"
#include "f_propos_composants.h"
//---------------------------------------------------------------------------
TfrmAPropos *frmAPropos;
//---------------------------------------------------------------------------
TfrmAPropos::TfrmAPropos(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  FermetureAutomatique=false; TempsEcoule=0; Bitmap=NULL; // FormCreate
 }
//---------------------------------------------------------------------------
void TfrmAPropos::ChargeFiche()
 {
  Ui::frmAPropos().setupUi(this);
  LieurComposants Lieur(this, "f_propos", Composants);
  COMPOSANTS_frmAPropos(LIE_COMPOSANT)
  FinChargement(Lieur);
  Connecte(OnShow, this, &TfrmAPropos::FormShow);
  Connecte(OnHide, this, &TfrmAPropos::FormHide);
  Connecte(Timer->OnTimer, this, &TfrmAPropos::TimerTimer);
 }
//---------------------------------------------------------------------------
void TfrmAPropos::TimerTimer(TObject *Sender)
 { // Fin de TimerTimer seulement (changement de glyphe et de couleur : phase 6)
  (void)Sender;
  TempsEcoule++;
  if (FermetureAutomatique&&(TempsEcoule==2))
   {
    FermetureAutomatique=false;
    Close();
   }
 }
//---------------------------------------------------------------------------
void TfrmAPropos::FormShow(TObject *Sender)
{
 (void)Sender;
 TempsEcoule=0;
 Timer->Enabled=true;
}
//---------------------------------------------------------------------------
void TfrmAPropos::FormHide(TObject *Sender)
{
 (void)Sender;
 Timer->Enabled=false;
}
//---------------------------------------------------------------------------

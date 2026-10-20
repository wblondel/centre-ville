//---------------------------------------------------------------------------
// cartes.h - Tests du moteur : les 6 cartes livrées dans maps/
//---------------------------------------------------------------------------
#ifndef cartesH
#define cartesH
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
struct carte
 {
  const char *Fichier; // nom du fichier dans maps/ (UTF-8, forme NFC comme dans git)
  const char *Nom; // nom ASCII (fichiers de référence)
 };
//---------------------------------------------------------------------------
const carte Cartes[]=
 {
  {"8x8.cvw", "8x8"},
  {"Carfours.cvw", "Carfours"},
  {"Colimasson.cvw", "Colimasson"},
  {"D\xC3\xA9mo.cvw", "Demo"},
  {"G\xC3\xA9" "ant.cvw", "Geant"},
  {"Spaghettis.cvw", "Spaghettis"},
 };
const int NB_CARTES=sizeof(Cartes)/sizeof(Cartes[0]);
//---------------------------------------------------------------------------
#ifdef CV_DOSSIER_CARTES
inline std::string CheminCarte(const carte &c)
 {
  return std::string(CV_DOSSIER_CARTES)+"/"+c.Fichier;
 }
#endif
//---------------------------------------------------------------------------
#endif

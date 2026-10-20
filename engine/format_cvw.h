//---------------------------------------------------------------------------
// format_cvw.h - Portage de Centre-Ville : garde-fous du format .cvw
//
// Le moteur lit et écrit les fichiers .cvw champ par champ avec
// FileRead/FileWrite(hfile, &champ, sizeof(champ)) : le format dépend donc des
// tailles des types du compilateur. Celles de C++Builder (Win32 et Win64) sont
// figées ici ; tout écart empêche la compilation.
//  - int = 4 octets, bool = 1 octet ;
//  - énumérés forcés sur 1 octet depuis la v5.4, et signés comme le char de
//    C++Builder (char est non signé par défaut sur Linux ARM64) ;
//  - nature = 4 octets (énuméré sans type sous-jacent imposé) ;
//  - aucun long n'est sérialisé (vérifié à la main : 4 octets sous Windows,
//    8 sous Linux et macOS) ;
//  - ordre des octets : petit-boutiste (x86, x86-64, ARM64).
// Le format reste celui de VERSION 0x530.
//---------------------------------------------------------------------------
#ifndef format_cvwH
#define format_cvwH
//---------------------------------------------------------------------------
#include <type_traits>
#include "simul.h"
//---------------------------------------------------------------------------
static_assert(VERSION==0x530, "Le format de fichier ne doit pas changer pendant le portage");
static_assert(sizeof(int)==4, "Format .cvw : int sur 4 octets");
static_assert(sizeof(bool)==1, "Format .cvw : bool sur 1 octet");
static_assert(sizeof(nature)==4, "Format .cvw : nature sur 4 octets");
//---------------------------------------------------------------------------
#define CVW_ENUM_OCTET(e) \
  static_assert(sizeof(e)==1, "Format .cvw : " #e " sur 1 octet"); \
  static_assert(std::is_same<std::underlying_type<e>::type, signed char>::value, #e " : type sous-jacent signed char")
CVW_ENUM_OCTET(priorite);
CVW_ENUM_OCTET(direction);
CVW_ENUM_OCTET(sens);
CVW_ENUM_OCTET(dirposs);
CVW_ENUM_OCTET(diraprendre);
CVW_ENUM_OCTET(etat_feu);
CVW_ENUM_OCTET(dmd_vert);
CVW_ENUM_OCTET(psg_rouge);
CVW_ENUM_OCTET(type_stats);
CVW_ENUM_OCTET(compatibilite);
CVW_ENUM_OCTET(niveau_priorite);
CVW_ENUM_OCTET(position_arret_bus_tram);
CVW_ENUM_OCTET(sens_voie_tram);
CVW_ENUM_OCTET(sens_voie_tram_base);
CVW_ENUM_OCTET(type_distance);
CVW_ENUM_OCTET(type_voie);
CVW_ENUM_OCTET(situation_taxi);
CVW_ENUM_OCTET(situation_vehlib);
CVW_ENUM_OCTET(coin);
#undef CVW_ENUM_OCTET
//---------------------------------------------------------------------------
// Champs publics sérialisés (les champs privés « Numero » sont des int)
#define CVW_CHAMP(classe, champ, taille) \
  static_assert(sizeof(classe::champ)==(taille), "Format .cvw : " #classe "::" #champ)
// centre_ville::Charge/Sauve (en-tête)
CVW_CHAMP(centre_ville, NbFeux, 4);
CVW_CHAMP(centre_ville, NbX, 4);
CVW_CHAMP(centre_ville, NbY, 4);
CVW_CHAMP(centre_ville, NbFeuxPieton, 4);
CVW_CHAMP(centre_ville, NbParkings, 4);
CVW_CHAMP(centre_ville, NbVehicules, 4);
CVW_CHAMP(centre_ville, NbPietons, 4);
CVW_CHAMP(centre_ville, NbArretsBus, 4);
CVW_CHAMP(centre_ville, NbLignesBus, 4);
CVW_CHAMP(centre_ville, NbArretsTram, 4);
CVW_CHAMP(centre_ville, NbLignesTram, 4);
CVW_CHAMP(centre_ville, NbPlacesTaxi, 4);
CVW_CHAMP(centre_ville, NbTaxis, 4);
CVW_CHAMP(centre_ville, NbPlacesPark, 4);
CVW_CHAMP(centre_ville, NbPlacesVehlib, 4);
CVW_CHAMP(centre_ville, NbVehlibs, 4);
CVW_CHAMP(centre_ville, DureeVert, 4);
CVW_CHAMP(centre_ville, DureeOrange, 4);
CVW_CHAMP(centre_ville, DureeRouge, 4);
CVW_CHAMP(centre_ville, AttenteMaxVeh, 4);
CVW_CHAMP(centre_ville, NbToursParSeconde, 4);
CVW_CHAMP(centre_ville, NbToursStats, 4);
CVW_CHAMP(centre_ville, QuadrillageStats, 1);
CVW_CHAMP(centre_ville, TypeStats, 1);
CVW_CHAMP(centre_ville, NbVitesses, 4);
CVW_CHAMP(centre_ville, NbMaxCasesDetectPsgPt, 4);
CVW_CHAMP(centre_ville, ProbaDirPsgPt, 4);
CVW_CHAMP(centre_ville, NbMaxCasesDetectArretBus, 4);
CVW_CHAMP(centre_ville, ProbaDirArretBus, 4);
CVW_CHAMP(centre_ville, ProbaArretBusDmd, 4);
CVW_CHAMP(centre_ville, NbMaxPersonnesBus, 4);
CVW_CHAMP(centre_ville, NbToursStopArretBus, 4);
CVW_CHAMP(centre_ville, NbMaxToursAttenteBus, 4);
CVW_CHAMP(centre_ville, NbMaxCasesDetectArretTram, 4);
CVW_CHAMP(centre_ville, ProbaDirArretTram, 4);
CVW_CHAMP(centre_ville, ProbaArretTramDmd, 4);
CVW_CHAMP(centre_ville, NbMaxPersonnesTram, 4);
CVW_CHAMP(centre_ville, NbToursStopArretTram, 4);
CVW_CHAMP(centre_ville, NbMaxToursAttenteTram, 4);
CVW_CHAMP(centre_ville, NbMaxCasesDetectFileTaxi, 4);
CVW_CHAMP(centre_ville, ProbaDirFileTaxi, 4);
CVW_CHAMP(centre_ville, ProbaArretTaxiDmd, 4);
CVW_CHAMP(centre_ville, NbMaxPersonnesTaxi, 4);
CVW_CHAMP(centre_ville, NbMaxToursAttenteTaxi, 4);
CVW_CHAMP(centre_ville, NbMaxPersonnesVehlib, 4);
CVW_CHAMP(centre_ville, NbMaxCasesDetectFileVehlib, 4);
CVW_CHAMP(centre_ville, ProbaDirFileVehlib, 4);
CVW_CHAMP(centre_ville, TauxRemplissageVehlib, 4);
// feu::Charge/Sauve
CVW_CHAMP(feu, x, 4);
CVW_CHAMP(feu, y, 4);
CVW_CHAMP(feu, Dir, 4);
CVW_CHAMP(feu, Etat, 1);
CVW_CHAMP(feu, NbFeuxDmd, 4);
CVW_CHAMP(feu, NbFeuxAtt, 4);
CVW_CHAMP(feu, TypeDmdVert, 1);
CVW_CHAMP(feu, TypePsgRouge, 1);
CVW_CHAMP(feu, DureeRouge, 4);
CVW_CHAMP(feu, DureeVert, 4);
CVW_CHAMP(feu, DemandeVert, 1);
CVW_CHAMP(feu, RougeDemande, 1);
// feu_pieton::Charge/Sauve
CVW_CHAMP(feu_pieton, x, 4*NBFEUXPIETONSPARNUMERO);
CVW_CHAMP(feu_pieton, y, 4*NBFEUXPIETONSPARNUMERO);
CVW_CHAMP(feu_pieton, Dir, 4*NBFEUXPIETONSPARNUMERO);
CVW_CHAMP(feu_pieton, iFeuDir, 4*NBDIR);
CVW_CHAMP(feu_pieton, Etat, 1);
CVW_CHAMP(feu_pieton, NbFeux, 4);
CVW_CHAMP(feu_pieton, TypeDmdVert, 1);
CVW_CHAMP(feu_pieton, TypePsgRouge, 1);
CVW_CHAMP(feu_pieton, DureeRouge, 4);
CVW_CHAMP(feu_pieton, DureeVert, 4);
CVW_CHAMP(feu_pieton, DemandeVert, 1);
CVW_CHAMP(feu_pieton, RougeDemande, 1);
// parking, depot_bus, place_taxi, place_park, place_vehlib
CVW_CHAMP(parking, x, 4);
CVW_CHAMP(parking, y, 4);
CVW_CHAMP(depot_bus, x, 4);
CVW_CHAMP(depot_bus, y, 4);
CVW_CHAMP(place_taxi, x, 4);
CVW_CHAMP(place_taxi, y, 4);
CVW_CHAMP(place_park, x, 4);
CVW_CHAMP(place_park, y, 4);
CVW_CHAMP(place_vehlib, x, 4);
CVW_CHAMP(place_vehlib, y, 4);
// arret_bus, arret_tram
CVW_CHAMP(arret_bus, x, 4);
CVW_CHAMP(arret_bus, y, 4);
CVW_CHAMP(arret_bus, Ligne, NBMAXLIGNESBUSTRAM);
CVW_CHAMP(arret_bus, NbLignes, 4);
CVW_CHAMP(arret_bus, NumLigne, 4*NBMAXLIGNESBUSTRAM);
CVW_CHAMP(arret_tram, x, 4);
CVW_CHAMP(arret_tram, y, 4);
CVW_CHAMP(arret_tram, Ligne, NBMAXLIGNESBUSTRAM);
CVW_CHAMP(arret_tram, NbLignes, 4);
CVW_CHAMP(arret_tram, NumLigne, 4*NBMAXLIGNESBUSTRAM);
// ligne_bus, ligne_tram
CVW_CHAMP(ligne_bus, NbArrets, 4);
CVW_CHAMP(ligne_bus, NumArret, 4*NBMAXARRETSBUSTRAMPARLIGNE);
CVW_CHAMP(ligne_tram, NbArrets, 4);
CVW_CHAMP(ligne_tram, NumArret, 4*NBMAXARRETSBUSTRAMPARLIGNE);
// voie::Charge/Sauve (x et y sont privés : des int)
CVW_CHAMP(voie, DirPoss, 1);
CVW_CHAMP(voie, SensVoieTram, 1);
CVW_CHAMP(voie, Nature, 4);
CVW_CHAMP(voie, NumParking, 4);
CVW_CHAMP(voie, ToutDroit, 1);
CVW_CHAMP(voie, PassagePietons, 1);
CVW_CHAMP(voie, PassageBus, 1);
CVW_CHAMP(voie, NumArretBus, 4);
CVW_CHAMP(voie, NumArretTram, 4);
CVW_CHAMP(voie, NumPlaceTaxi, 4);
CVW_CHAMP(voie, NumPlacePark, 4);
CVW_CHAMP(voie, NumPlaceVehlib, 4);
CVW_CHAMP(voie, NumSpotVehlib, 4);
CVW_CHAMP(voie, Priorite, NBFEUXPARCASE);
CVW_CHAMP(voie, NumFeu, 4*NBFEUXPARCASE);
CVW_CHAMP(voie, NumFeuP, 4*NBFEUXPARCASE);
#undef CVW_CHAMP
//---------------------------------------------------------------------------
#endif

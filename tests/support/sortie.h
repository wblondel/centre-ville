//---------------------------------------------------------------------------
// sortie.h - Tests du moteur : dossier des fichiers produits par les tests
//---------------------------------------------------------------------------
#ifndef sortieH
#define sortieH
//---------------------------------------------------------------------------
#include <filesystem>
#include <string>
//---------------------------------------------------------------------------
#ifndef CV_DOSSIER_SORTIE
#error "CV_DOSSIER_SORTIE doit être défini"
#endif
//---------------------------------------------------------------------------
// Renvoie le chemin (UTF-8) d'un fichier du dossier de sortie, après avoir
// créé les dossiers nécessaires
inline std::string FichierSortie(const std::string &asNom)
 {
  std::string asChemin=std::string(CV_DOSSIER_SORTIE)+"/"+asNom;
  std::filesystem::path p(std::u8string(asChemin.begin(), asChemin.end()));
  std::filesystem::create_directories(p.parent_path());
  return asChemin;
 }
//---------------------------------------------------------------------------
#endif

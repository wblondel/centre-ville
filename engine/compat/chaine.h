//---------------------------------------------------------------------------
// compat/chaine.h - Portage de Centre-Ville hors de C++Builder
//
// Remplace <dstring.h> pour le moteur : sous-ensemble d'AnsiString,
// Format(..., ARRAYOFCONST((...))) et IntToStr, en C++ standard.
//
// Différence assumée avec C++Builder : les littéraux du dépôt sont en UTF-8
// (et non plus en Windows-1252). Length() compte donc des octets ; le moteur
// ne dépend jamais de la longueur d'une chaîne accentuée.
//---------------------------------------------------------------------------
#ifndef compat_chaineH
#define compat_chaineH
//---------------------------------------------------------------------------
#include <cstddef>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>
//---------------------------------------------------------------------------
class AnsiString
 {
  std::string s;
  public:
  AnsiString() {}
  AnsiString(const char *src) : s(src?src:"") {}
  AnsiString(const std::string &src) : s(src) {}
  AnsiString(char c) : s(1, c) {} // comme C++Builder : chaîne d'un caractère
  const char *c_str() const { return s.c_str(); }
  const std::string &str() const { return s; }
  int Length() const { return (int)s.size(); }
  bool IsEmpty() const { return s.empty(); }
  // Indices à partir de 1, comme la VCL
  char &operator[](int i) { return s[(std::size_t)(i-1)]; }
  char operator[](int i) const { return s[(std::size_t)(i-1)]; }
  AnsiString SubString(int Index, int Count) const;
  int Pos(const AnsiString &asSousChaine) const; // 0 si absente
  AnsiString &operator+=(const AnsiString &as) { s+=as.s; return *this; }
  AnsiString operator+(const AnsiString &as) const { return AnsiString(s+as.s); }
  bool operator==(const AnsiString &as) const { return s==as.s; }
  bool operator!=(const AnsiString &as) const { return s!=as.s; }
  bool operator<(const AnsiString &as) const { return s<as.s; }
  bool operator>(const AnsiString &as) const { return s>as.s; }
  bool operator<=(const AnsiString &as) const { return s<=as.s; }
  bool operator>=(const AnsiString &as) const { return s>=as.s; }
 };
//---------------------------------------------------------------------------
inline AnsiString operator+(const char *gauche, const AnsiString &droite)
 {
  return AnsiString(gauche)+droite;
 }
//---------------------------------------------------------------------------
inline std::ostream &operator<<(std::ostream &os, const AnsiString &as)
 {
  return os<<as.str();
 }
//---------------------------------------------------------------------------
// TVarRec : un élément de tableau ouvert « array of const » (Delphi)
//---------------------------------------------------------------------------
class TVarRec
 {
  public:
  enum genre { vtEntier, vtReel, vtChaine, vtCaractere, vtBooleen, vtPointeur };
  genre Genre;
  long long Entier=0;
  double Reel=0.0;
  std::string Chaine;
  const void *Pointeur=nullptr;
  template <class T,
            typename std::enable_if<std::is_integral<T>::value&&
                                    (!std::is_same<T, char>::value)&&
                                    (!std::is_same<T, bool>::value), int>::type=0>
  TVarRec(T v) : Genre(vtEntier), Entier((long long)v) {}
  TVarRec(char c) : Genre(vtCaractere), Chaine(1, c) {}
  TVarRec(bool b) : Genre(vtBooleen), Entier(b) {}
  TVarRec(double d) : Genre(vtReel), Reel(d) {}
  TVarRec(float f) : Genre(vtReel), Reel(f) {}
  TVarRec(long double d) : Genre(vtReel), Reel((double)d) {} // portage : (long double) de f_simul.cpp (AfficheBilan)
  TVarRec(const char *c) : Genre(vtChaine), Chaine(c?c:"") {}
  TVarRec(const std::string &c) : Genre(vtChaine), Chaine(c) {}
  TVarRec(const AnsiString &as) : Genre(vtChaine), Chaine(as.str()) {}
  TVarRec(const void *p) : Genre(vtPointeur), Pointeur(p) {}
 };
//---------------------------------------------------------------------------
template <class... T> std::vector<TVarRec> TableauDeConstantes(const T &... v)
 {
  return std::vector<TVarRec>{ TVarRec(v)... };
 }
//---------------------------------------------------------------------------
// ARRAYOFCONST((a, b, c)) -> TableauDeConstantes (a, b, c)
#define ARRAYOFCONST(valeurs) TableauDeConstantes valeurs
//---------------------------------------------------------------------------
// portage : séparateurs utilisés par Format (SysUtils::FormatSettings). La VCL
// les prenait dans les paramètres régionaux de Windows (',' et espace
// insécable en français). Le moteur n'utilise que %d et %s : il garde ici les
// valeurs neutres ('.' et ','), comme les tests ; l'application les règle
// comme la VCL en français (app/main.cpp).
//---------------------------------------------------------------------------
struct TFormatSettings
 {
  char DecimalSeparator='.';
  std::string ThousandSeparator=",";
 };
extern TFormatSettings FormatSettings;
//---------------------------------------------------------------------------
// Format façon SysUtils : % [index ":"] ["-"] [largeur] ["." précision] type
// Types : d u x e f g n m s p (insensibles à la casse) et %% ; largeur,
// précision et index peuvent valoir '*'. Les arguments surnuméraires sont
// ignorés ; un argument manquant ou d'un type incompatible lève
// std::invalid_argument (EConvertError dans la VCL).
//---------------------------------------------------------------------------
AnsiString Format(const AnsiString &asFormat, const std::vector<TVarRec> &Args);
//---------------------------------------------------------------------------
AnsiString IntToStr(long long Valeur); // couvre IntToStr(int) et IntToStr(__int64)
//---------------------------------------------------------------------------
#endif

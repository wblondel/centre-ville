//---------------------------------------------------------------------------
// vcl/proprietes.h - Portage de Centre-Ville : propriétés façon C++Builder
//
// Les fiches d'origine lisent et écrivent les propriétés des composants
// (__property de la VCL) : ActionX->Checked^=true, Caption=Caption+"*",
// StatusBar->Panels->Items[0]->Text=..., etc. Propriete<T> reproduit cette
// syntaxe : la lecture passe par une conversion implicite, l'écriture par une
// affectation, chacune appelant l'accesseur fourni par le composant. Le code
// porté garde ainsi les lignes d'origine.
//---------------------------------------------------------------------------
#ifndef vcl_proprietesH
#define vcl_proprietesH
//---------------------------------------------------------------------------
#include <functional>
#include <type_traits>
#include "compat/chaine.h"
//---------------------------------------------------------------------------
template <class T> class Propriete
 {
  std::function<T()> Lit;
  std::function<void(const T &)> Ecrit;
  public:
  Propriete() {}
  Propriete(const Propriete &)=delete;
  void Definit(std::function<T()> Lecture, std::function<void(const T &)> Ecriture)
   {
    Lit=std::move(Lecture);
    Ecrit=std::move(Ecriture);
   }
  T Valeur() const { return Lit(); }
  operator T() const { return Lit(); }
  // Conversion explicite vers un énuméré : (etat_feu)RadioGroupEtat->ItemIndex
  template <class E, typename std::enable_if<std::is_enum<E>::value, int>::type=0>
  explicit operator E() const { return (E)Lit(); }
  Propriete &operator=(const T &v) { Ecrit(v); return *this; }
  // Affectation d'une propriété à une autre : copie de la valeur, pas des accesseurs
  Propriete &operator=(const Propriete &p) { Ecrit(p.Lit()); return *this; }
  // Opérateurs composés (Tag^=1, Checked^=true, Width+=...)
  template <class U> Propriete &operator^=(const U &v) { Ecrit((T)(Lit()^v)); return *this; }
  template <class U> Propriete &operator+=(const U &v) { Ecrit((T)(Lit()+v)); return *this; }
  template <class U> Propriete &operator-=(const U &v) { Ecrit((T)(Lit()-v)); return *this; }
 };
//---------------------------------------------------------------------------
// Propriétés texte : mêmes méthodes qu'AnsiString (Caption[Caption.Length()]...)
//---------------------------------------------------------------------------
template <> class Propriete<AnsiString>
 {
  std::function<AnsiString()> Lit;
  std::function<void(const AnsiString &)> Ecrit;
  public:
  Propriete() {}
  Propriete(const Propriete &)=delete;
  void Definit(std::function<AnsiString()> Lecture, std::function<void(const AnsiString &)> Ecriture)
   {
    Lit=std::move(Lecture);
    Ecrit=std::move(Ecriture);
   }
  AnsiString Valeur() const { return Lit(); }
  operator AnsiString() const { return Lit(); }
  Propriete &operator=(const AnsiString &v) { Ecrit(v); return *this; }
  Propriete &operator=(const char *v) { Ecrit(AnsiString(v)); return *this; }
  Propriete &operator=(const Propriete &p) { Ecrit(p.Lit()); return *this; }
  int Length() const { return Lit().Length(); }
  bool IsEmpty() const { return Lit().IsEmpty(); }
  int Pos(const AnsiString &as) const { return Lit().Pos(as); }
  AnsiString SubString(int Index, int Count) const { return Lit().SubString(Index, Count); }
  char operator[](int i) const { return Lit()[i]; }
  const char *c_str() const = delete; // pointeur sur un temporaire : utiliser Valeur().c_str()
  AnsiString operator+(const AnsiString &as) const { return Lit()+as; }
  AnsiString operator+(const char *s) const { return Lit()+AnsiString(s); }
  bool operator==(const AnsiString &as) const { return Lit()==as; }
  bool operator!=(const AnsiString &as) const { return Lit()!=as; }
 };
//---------------------------------------------------------------------------
inline AnsiString operator+(const char *gauche, const Propriete<AnsiString> &droite)
 {
  return AnsiString(gauche)+droite.Valeur();
 }
//---------------------------------------------------------------------------
// ARRAYOFCONST((Grille->Col, ...)) : la valeur de la propriété (compat/chaine.h)
//---------------------------------------------------------------------------
template <class T> TVarRec VersTVarRec(const Propriete<T> &p) { return TVarRec(p.Valeur()); }
//---------------------------------------------------------------------------
#endif

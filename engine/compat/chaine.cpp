//---------------------------------------------------------------------------
// compat/chaine.cpp - Portage de Centre-Ville hors de C++Builder
//---------------------------------------------------------------------------
#include "chaine.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
//---------------------------------------------------------------------------
AnsiString AnsiString::SubString(int Index, int Count) const
 { // Comme la VCL : Index à partir de 1, bornes tolérées
  if (Index<1) Index=1;
  if ((Count<=0)||(Index>Length())) return AnsiString();
  return AnsiString(s.substr((std::size_t)(Index-1), (std::size_t)Count));
 }
//---------------------------------------------------------------------------
int AnsiString::Pos(const AnsiString &asSousChaine) const
 {
  if (asSousChaine.IsEmpty()) return 0;
  std::size_t p=s.find(asSousChaine.s);
  return (p==std::string::npos)?0:(int)p+1;
 }
//---------------------------------------------------------------------------
AnsiString IntToStr(long long Valeur)
 {
  return AnsiString(std::to_string(Valeur));
 }
//---------------------------------------------------------------------------
namespace
 {
  [[noreturn]] void ErreurFormat(const std::string &asFormat, const char *Raison)
   {
    throw std::invalid_argument(std::string("Format(\"")+asFormat+"\") : "+Raison);
   }
  //-------------------------------------------------------------------------
  std::string Imprime(const char *Gabarit, int Precision, double Valeur)
   {
    char Tampon[512];
    std::snprintf(Tampon, sizeof(Tampon), Gabarit, Precision, Valeur);
    return Tampon;
   }
  //-------------------------------------------------------------------------
  std::string Exponentielle(int Precision, double Valeur)
   { // Delphi : "-d.ddd...E+ddd", Precision = nombre total de chiffres, exposant sur 3 chiffres
    if (Precision<1) Precision=1;
    std::string Brut=Imprime("%.*E", Precision-1, Valeur);
    std::size_t e=Brut.find('E');
    if (e==std::string::npos) return Brut;
    std::string Mantisse=Brut.substr(0, e);
    char Signe=Brut[e+1];
    int Exposant=std::atoi(Brut.c_str()+e+2);
    char Tampon[16];
    std::snprintf(Tampon, sizeof(Tampon), "E%c%03d", Signe, Exposant);
    return Mantisse+Tampon;
   }
  //-------------------------------------------------------------------------
  std::string AvecSeparateurMilliers(const std::string &Nombre)
   {
    std::size_t Debut=(!Nombre.empty()&&(Nombre[0]=='-'))?1:0;
    std::size_t Fin=Nombre.find('.');
    if (Fin==std::string::npos) Fin=Nombre.size();
    std::string Resultat=Nombre.substr(Fin);
    for(std::size_t n=0, i=Fin; i>Debut; i--, n++)
     {
      if (n&&!(n%3)) Resultat=","+Resultat;
      Resultat=Nombre[i-1]+Resultat;
     }
    return Nombre.substr(0, Debut)+Resultat;
   }
 }
//---------------------------------------------------------------------------
AnsiString Format(const AnsiString &asFormat, const std::vector<TVarRec> &Args)
 {
  const std::string &f=asFormat.str();
  std::string Resultat;
  std::size_t i=0, n=f.size(), iArg=0;
  // Lecture d'un nombre ou de '*' (valeur prise dans les arguments)
  auto LitNombre=[&](int &Valeur)->bool
   {
    if ((i<n)&&(f[i]=='*'))
     {
      i++;
      if (iArg>=Args.size()) ErreurFormat(f, "argument manquant pour '*'");
      if (Args[iArg].Genre!=TVarRec::vtEntier) ErreurFormat(f, "'*' attend un entier");
      Valeur=(int)Args[iArg++].Entier;
      return true;
     }
    if ((i<n)&&std::isdigit((unsigned char)f[i]))
     {
      Valeur=0;
      while((i<n)&&std::isdigit((unsigned char)f[i])) Valeur=10*Valeur+(f[i++]-'0');
      return true;
     }
    return false;
   };
  while(i<n)
   {
    if (f[i]!='%') { Resultat+=f[i++]; continue; }
    i++;
    if (i>=n) ErreurFormat(f, "spécificateur incomplet");
    if (f[i]=='%') { Resultat+='%'; i++; continue; }
    bool AGauche=false, LargeurDefinie=false, PrecisionDefinie=false;
    int Largeur=0, Precision=0, Nombre=0;
    std::size_t Debut=i, iArgDebut=iArg;
    // [index ":"]
    if (LitNombre(Nombre)&&(i<n)&&(f[i]==':'))
     {
      iArg=(std::size_t)Nombre;
      i++;
     }
    else
     { // ce n'était pas un index : relecture comme largeur
      i=Debut;
      iArg=iArgDebut;
     }
    if ((i<n)&&(f[i]=='-')) { AGauche=true; i++; }
    LargeurDefinie=LitNombre(Largeur);
    if ((i<n)&&(f[i]=='.'))
     {
      i++;
      PrecisionDefinie=LitNombre(Precision);
      if (!PrecisionDefinie) ErreurFormat(f, "précision attendue après '.'");
     }
    if (i>=n) ErreurFormat(f, "type manquant");
    char Type=(char)std::toupper((unsigned char)f[i++]);
    if (iArg>=Args.size()) ErreurFormat(f, "argument manquant");
    const TVarRec &a=Args[iArg++];
    std::string Texte;
    switch(Type)
     {
      case 'D':
      case 'U':
       {
        if (a.Genre!=TVarRec::vtEntier) ErreurFormat(f, "%d/%u attend un entier");
        long long v=a.Entier;
        bool Negatif=false;
        unsigned long long Absolu;
        if (Type=='U')
         Absolu=(v<0)?(unsigned long long)(unsigned int)v:(unsigned long long)v;
        else
         {
          Negatif=(v<0);
          Absolu=Negatif?(unsigned long long)(-(v+1))+1ULL:(unsigned long long)v;
         }
        std::string Chiffres=std::to_string(Absolu);
        if (PrecisionDefinie&&((int)Chiffres.size()<Precision))
         Chiffres=std::string((std::size_t)Precision-Chiffres.size(), '0')+Chiffres;
        Texte=(Negatif?"-":"")+Chiffres;
        break;
       }
      case 'X':
       {
        if (a.Genre!=TVarRec::vtEntier) ErreurFormat(f, "%x attend un entier");
        unsigned long long v=((a.Entier<0)&&(a.Entier>=-2147483648LL))?
                             (unsigned long long)(unsigned int)a.Entier:
                             (unsigned long long)a.Entier;
        char Tampon[32];
        std::snprintf(Tampon, sizeof(Tampon), "%llX", v);
        Texte=Tampon;
        if (PrecisionDefinie&&((int)Texte.size()<Precision))
         Texte=std::string((std::size_t)Precision-Texte.size(), '0')+Texte;
        break;
       }
      case 'E':
      case 'F':
      case 'G':
      case 'N':
       {
        double v;
        if (a.Genre==TVarRec::vtReel) v=a.Reel;
        else ErreurFormat(f, "%e/%f/%g/%n attend un réel");
        switch(Type)
         {
          case 'E': Texte=Exponentielle(PrecisionDefinie?Precision:15, v); break;
          case 'F': Texte=Imprime("%.*f", PrecisionDefinie?Precision:2, v); break;
          case 'G': Texte=Imprime("%.*G", PrecisionDefinie?Precision:15, v); break;
          case 'N': Texte=AvecSeparateurMilliers(Imprime("%.*f", PrecisionDefinie?Precision:2, v)); break;
         }
        break;
       }
      case 'S':
       {
        if ((a.Genre!=TVarRec::vtChaine)&&(a.Genre!=TVarRec::vtCaractere))
         ErreurFormat(f, "%s attend une chaîne");
        Texte=a.Chaine;
        if (PrecisionDefinie&&((int)Texte.size()>Precision))
         Texte=Texte.substr(0, (std::size_t)Precision);
        break;
       }
      case 'P':
       {
        if (a.Genre!=TVarRec::vtPointeur) ErreurFormat(f, "%p attend un pointeur");
        char Tampon[32];
        std::snprintf(Tampon, sizeof(Tampon), "%0*llX", (int)(2*sizeof(void*)),
                      (unsigned long long)(std::size_t)a.Pointeur);
        Texte=Tampon;
        break;
       }
      default:
       ErreurFormat(f, "type de spécificateur non géré");
     }
    if (LargeurDefinie&&((int)Texte.size()<Largeur))
     {
      std::string Bourrage((std::size_t)Largeur-Texte.size(), ' ');
      Texte=AGauche?Texte+Bourrage:Bourrage+Texte;
     }
    Resultat+=Texte;
   }
  return AnsiString(Resultat);
 }
//---------------------------------------------------------------------------

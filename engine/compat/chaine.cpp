//---------------------------------------------------------------------------
// compat/chaine.cpp - Portage de Centre-Ville hors de C++Builder
//---------------------------------------------------------------------------
#include "chaine.h"

#include <cmath>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <format>
#include <string>
#include <utility>
#include <vector>
//---------------------------------------------------------------------------
// NOLINTNEXTLINE(bugprone-throwing-static-initialization,cert-err58-cpp) : « , » tient sans allocation
TFormatSettings FormatSettings;
//---------------------------------------------------------------------------
AnsiString AnsiString::SubString(int Index, int Count) const
{ // Comme la VCL : Index à partir de 1, bornes tolérées
    Index = std::max(Index, 1);
    if ((Count <= 0) || (Index > Length()))
    {
        return {};
    }
    return {s.substr(static_cast<std::size_t>(Index - 1), static_cast<std::size_t>(Count))};
}
//---------------------------------------------------------------------------
int AnsiString::Pos(const AnsiString &asSousChaine) const
{
    if (asSousChaine.IsEmpty())
    {
        return 0;
    }
    const std::size_t p = s.find(asSousChaine.s);
    return (p == std::string::npos) ? 0 : static_cast<int>(p) + 1;
}
//---------------------------------------------------------------------------
AnsiString IntToStr(long long Valeur)
{
    return {std::to_string(Valeur)};
}
//---------------------------------------------------------------------------
namespace
{
[[noreturn]] void ErreurFormat(const std::string &asFormat, const char *Raison)
{
    throw std::invalid_argument(std::string("Format(\"") + asFormat + "\") : " + Raison);
}
//-------------------------------------------------------------------------
std::string Imprime(char Type, int Precision, double Valeur)
{ // Comme printf("%.*f"), "%.*E" ou "%.*G" dans la locale "C" : std::format ne dépend pas de LC_NUMERIC
  // (QCoreApplication appelle setlocale) ; le '.' est remplacé ensuite par FormatSettings.DecimalSeparator
    switch (Type)
    {
    case 'f':
        return std::format("{:.{}f}", Valeur, Precision);
    case 'E':
        return std::format("{:.{}E}", Valeur, Precision);
    default:
        return std::format("{:.{}G}", Valeur, Precision);
    }
}
//-------------------------------------------------------------------------
std::string Exponentielle(int Precision, double Valeur)
{ // Delphi : "-d.ddd...E+ddd", Precision = nombre total de chiffres, exposant sur 3 chiffres
    Precision = std::max(Precision, 1);
    std::string Brut = Imprime('E', Precision - 1, Valeur);
    const std::size_t e = Brut.find('E');
    if (e == std::string::npos)
    {
        return Brut;
    }
    const std::string Mantisse = Brut.substr(0, e);
    const char Signe = Brut[e + 1];
    const int Exposant = std::stoi(Brut.substr(e + 2));
    return Mantisse + std::format("E{}{:03d}", Signe, Exposant);
}
//-------------------------------------------------------------------------
std::string AvecSeparateurMilliers(const std::string &Nombre)
{
    const std::size_t Debut = (!Nombre.empty() && (Nombre[0] == '-')) ? 1 : 0;
    std::size_t Fin = Nombre.find('.');
    if (Fin == std::string::npos)
    {
        Fin = Nombre.size();
    }
    std::string Resultat = Nombre.substr(Fin);
    for (std::size_t n = 0, i = Fin; i > Debut; i--, n++)
    {
        if ((n != 0U) && ((n % 3) == 0U))
        {
            Resultat.insert(0, 1, ',');
        }
        Resultat.insert(0, 1, Nombre[i - 1]);
    }
    return Nombre.substr(0, Debut) + Resultat;
}
} // namespace
//---------------------------------------------------------------------------
AnsiString Format(const AnsiString &asFormat, const std::vector<TVarRec> &Args)
{
    const std::string &f = asFormat.str();
    std::string Resultat;
    std::size_t i = 0;
    std::size_t n = f.size();
    std::size_t iArg = 0;
    // Lecture d'un nombre ou de '*' (valeur prise dans les arguments)
    const auto LitNombre = [&](int &Valeur) -> bool {
        if ((i < n) && (f[i] == '*'))
        {
            i++;
            if (iArg >= Args.size())
            {
                ErreurFormat(f, "argument manquant pour '*'");
            }
            if (Args[iArg].Genre != TVarRec::vtEntier)
            {
                ErreurFormat(f, "'*' attend un entier");
            }
            Valeur = static_cast<int>(Args[iArg++].Entier);
            return true;
        }
        if ((i < n) && std::isdigit(static_cast<unsigned char>(f[i])))
        {
            Valeur = 0;
            while ((i < n) && std::isdigit(static_cast<unsigned char>(f[i])))
            {
                Valeur = (10 * Valeur) + (f[i++] - '0');
            }
            return true;
        }
        return false;
    };
    while (i < n)
    {
        if (f[i] != '%')
        {
            Resultat += f[i++];
            continue;
        }
        i++;
        if (i >= n)
        {
            ErreurFormat(f, "spécificateur incomplet");
        }
        if (f[i] == '%')
        {
            Resultat += '%';
            i++;
            continue;
        }
        bool AGauche = false;
        bool LargeurDefinie = false;
        bool PrecisionDefinie = false;
        int Largeur = 0;
        int Precision = 0;
        int Nombre = 0;
        const std::size_t Debut = i;
        const std::size_t iArgDebut = iArg;
        // [index ":"]
        if (LitNombre(Nombre) && (i < n) && (f[i] == ':'))
        {
            iArg = static_cast<std::size_t>(Nombre);
            i++;
        }
        else
        { // ce n'était pas un index : relecture comme largeur
            i = Debut;
            iArg = iArgDebut;
        }
        if ((i < n) && (f[i] == '-'))
        {
            AGauche = true;
            i++;
        }
        LargeurDefinie = LitNombre(Largeur);
        if ((i < n) && (f[i] == '.'))
        {
            i++;
            PrecisionDefinie = LitNombre(Precision);
            if (!PrecisionDefinie)
            {
                ErreurFormat(f, "précision attendue après '.'");
            }
        }
        if (i >= n)
        {
            ErreurFormat(f, "type manquant");
        }
        const char Type = static_cast<char>(std::toupper(static_cast<unsigned char>(f[i++])));
        if (iArg >= Args.size())
        {
            ErreurFormat(f, "argument manquant");
        }
        const TVarRec &a = Args[iArg++];
        std::string Texte;
        switch (Type)
        {
        case 'D':
        case 'U':
        {
            if (a.Genre != TVarRec::vtEntier)
            {
                ErreurFormat(f, "%d/%u attend un entier");
            }
            const long long v = a.Entier;
            bool Negatif = false;
            unsigned long long Absolu = 0;
            if (Type == 'U')
            {
                Absolu = (v < 0) ? static_cast<unsigned long long>(static_cast<unsigned int>(v))
                                 : static_cast<unsigned long long>(v);
            }
            else
            {
                Negatif = (v < 0);
                Absolu =
                    Negatif ? static_cast<unsigned long long>(-(v + 1)) + 1ULL : static_cast<unsigned long long>(v);
            }
            std::string Chiffres = std::to_string(Absolu);
            if (PrecisionDefinie && std::cmp_less(Chiffres.size(), Precision))
            {
                Chiffres.insert(0, static_cast<std::size_t>(Precision) - Chiffres.size(), '0');
            }
            Texte = (Negatif ? "-" : "") + Chiffres;
            break;
        }
        case 'X':
        {
            if (a.Genre != TVarRec::vtEntier)
            {
                ErreurFormat(f, "%x attend un entier");
            }
            const unsigned long long v = ((a.Entier < 0) && (a.Entier >= -2147483648LL))
                                             ? static_cast<unsigned long long>(static_cast<unsigned int>(a.Entier))
                                             : static_cast<unsigned long long>(a.Entier);
            Texte = std::format("{:X}", v);
            if (PrecisionDefinie && std::cmp_less(Texte.size(), Precision))
            {
                Texte.insert(0, static_cast<std::size_t>(Precision) - Texte.size(), '0');
            }
            break;
        }
        case 'E':
        case 'F':
        case 'G':
        case 'N':
        {
            double v = NAN;
            if (a.Genre == TVarRec::vtReel)
            {
                v = a.Reel;
            }
            else
            {
                ErreurFormat(f, "%e/%f/%g/%n attend un réel");
            }
            switch (Type)
            {
            case 'E':
                Texte = Exponentielle(PrecisionDefinie ? Precision : 15, v);
                break;
            case 'F':
                Texte = Imprime('f', PrecisionDefinie ? Precision : 2, v);
                break;
            case 'G':
                Texte = Imprime('G', PrecisionDefinie ? Precision : 15, v);
                break;
            case 'N':
                Texte = AvecSeparateurMilliers(Imprime('f', PrecisionDefinie ? Precision : 2, v));
                break;
            default:
                break;
            }
            { // portage : séparateurs de FormatSettings (le texte ne contient que des chiffres, signes, 'E', '.' et
              // ',')
                std::string Local;
                for (const char c : Texte)
                {
                    if (c == '.')
                    {
                        Local += FormatSettings.DecimalSeparator;
                    }
                    else if (c == ',')
                    {
                        Local += FormatSettings.ThousandSeparator;
                    }
                    else
                    {
                        Local += c;
                    }
                }
                Texte = Local;
            }
            break;
        }
        case 'S':
        {
            if ((a.Genre != TVarRec::vtChaine) && (a.Genre != TVarRec::vtCaractere))
            {
                ErreurFormat(f, "%s attend une chaîne");
            }
            Texte = a.Chaine;
            if (PrecisionDefinie && std::cmp_greater(Texte.size(), Precision))
            {
                Texte = Texte.substr(0, static_cast<std::size_t>(Precision));
            }
            break;
        }
        case 'P':
        {
            if (a.Genre != TVarRec::vtPointeur)
            {
                ErreurFormat(f, "%p attend un pointeur");
            }
            Texte = std::format("{:0{}X}",
                                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) : : adresse affichée
                                reinterpret_cast<std::uintptr_t>(a.Pointeur), 2 * sizeof(void *));
            break;
        }
        default:
            ErreurFormat(f, "type de spécificateur non géré");
        }
        if (LargeurDefinie && std::cmp_less(Texte.size(), Largeur))
        {
            const std::string Bourrage(static_cast<std::size_t>(Largeur) - Texte.size(), ' ');
            if (AGauche)
            {
                Texte += Bourrage;
            }
            else
            {
                Texte.insert(0, Bourrage);
            }
        }
        Resultat += Texte;
    }
    return {Resultat};
}
//---------------------------------------------------------------------------

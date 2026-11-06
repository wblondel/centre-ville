//---------------------------------------------------------------------------
// vcl/vcl.cpp - Portage de Centre-Ville : String, SysUtils, TApplication
//---------------------------------------------------------------------------
#include "vcl.h"
#include "compat/chaine.h"
#include "compat/temps.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QGuiApplication>
#include <QScreen>
#include <QStringList>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <format>
#include <string>
#include <utility>
//---------------------------------------------------------------------------
String String::UpperCase() const
{
    return A(Q(*this).toUpper());
}
//---------------------------------------------------------------------------
int ParamCount()
{
    return static_cast<int>(QCoreApplication::arguments().size()) - 1;
}
//---------------------------------------------------------------------------
String ParamStr(int Index)
{
    QStringList l = QCoreApplication::arguments();
    if ((Index < 0) || (Index >= l.size()))
    {
        return {};
    }
    if (Index == 0)
    {
        return A(QCoreApplication::applicationFilePath());
    }
    return A(l[Index]);
}
//---------------------------------------------------------------------------
String FormatDateTime(const AnsiString &asFormat, const TDateTime &dt)
{ // Sous-ensemble utilisé par les fiches : hh, nn, ss, zzz et caractères littéraux
    const auto Jours = static_cast<double>(dt);
    const auto ms = static_cast<long long>(std::floor(((Jours - std::floor(Jours)) * 86400000.0) + 0.5));
    const int h = static_cast<int>((ms / 3600000) % 24);
    const int n = static_cast<int>((ms / 60000) % 60);
    const int s = static_cast<int>((ms / 1000) % 60);
    const int z = static_cast<int>(ms % 1000);
    const std::string &f = asFormat.str();
    std::string r;
    for (std::size_t i = 0; i < f.size();)
    {
        const auto Commence = [&](const char *m) { return f.compare(i, std::char_traits<char>::length(m), m) == 0; };
        if (Commence("hh"))
        {
            r += std::format("{:02d}", h);
            i += 2;
        }
        else if (Commence("nn"))
        {
            r += std::format("{:02d}", n);
            i += 2;
        }
        else if (Commence("ss"))
        {
            r += std::format("{:02d}", s);
            i += 2;
        }
        else if (Commence("zzz"))
        {
            r += std::format("{:03d}", z);
            i += 3;
        }
        else
        {
            r += f[i++];
        }
    }
    return {r};
}
//---------------------------------------------------------------------------
int StrToInt(const AnsiString &asTexte)
{
    bool Ok = false;
    const int n = Q(asTexte).trimmed().toInt(&Ok);
    if (!Ok)
    {
        throw std::invalid_argument("'" + asTexte.str() + "' n'est pas une valeur entière correcte");
    }
    return n;
}
//---------------------------------------------------------------------------
String FormatFloat(const AnsiString &asFormat, long double Valeur)
{ // SysUtils::FormatFloat, une seule section : « 0 » chiffre obligatoire, « # »
  // chiffre facultatif, « . » séparateur décimal (omis sans décimale affichée),
  // texte entre guillemets ou apostrophes et autres caractères (« % »...)
  // recopiés à leur place, signe en tête. Arrondi comme FloatToDecimal : à
  // partir de 17 chiffres significatifs, la moitié vers le haut. NAN, INF et
  // -INF comme la VCL (FloatToTextFmt).
    if (std::isnan(Valeur))
    {
        return {"NAN"};
    }
    if (std::isinf(Valeur))
    {
        return {(Valeur < 0) ? "-INF" : "INF"};
    }
    const std::string &f = asFormat.str();
    // Analyse du format
    int NbAvant = 0;
    int PremierZero = -1;
    int Decimales = 0;
    int DecimalesMin = 0;
    bool Point = false;
    for (std::size_t i = 0; i < f.size(); i++)
    {
        const char c = f[i];
        if ((c == '"') || (c == '\''))
        {
            const std::size_t j = f.find(c, i + 1);
            i = (j == std::string::npos) ? f.size() : j;
        }
        else if ((c == '.') && !Point)
        {
            Point = true;
        }
        else if ((c == '0') || (c == '#'))
        {
            if (!Point)
            {
                if ((c == '0') && (PremierZero < 0))
                {
                    PremierZero = NbAvant;
                }
                NbAvant++;
            }
            else
            {
                Decimales++;
                if (c == '0')
                {
                    DecimalesMin = Decimales;
                }
            }
        }
    }
    const int EntiersMin = (PremierZero < 0) ? 0 : NbAvant - PremierZero;
    // Chiffres : Valeur = 0,Chiffres x 10^(Exposant+1), arrondis à Decimales
    const std::string Brut = std::format("{:.16e}", std::fabs(Valeur)); // d.dddddddddddddddde±xx
    std::string Chiffres = Brut.substr(0, 1) + Brut.substr(2, 16);
    int Exposant = std::stoi(Brut.substr(Brut.find('e') + 1));
    const int Garde = Exposant + 1 + Decimales;
    if (Garde < 0)
    {
        Chiffres.clear();
    }
    else if (std::cmp_less(Garde, Chiffres.size()))
    {
        const bool Haut = Chiffres[static_cast<std::size_t>(Garde)] >= '5';
        Chiffres.resize(static_cast<std::size_t>(Garde));
        if (Haut)
        {
            int i = Garde - 1;
            while ((i >= 0) && (Chiffres[static_cast<std::size_t>(i)] == '9'))
            {
                Chiffres[static_cast<std::size_t>(i--)] = '0';
            }
            if (i < 0)
            {
                Chiffres.insert(0, "1");
                Exposant++;
            }
            else
            {
                Chiffres[static_cast<std::size_t>(i)]++;
            }
        }
    }
    const bool Zero = Chiffres.find_first_not_of('0') == std::string::npos;
    std::string Entier;
    std::string Fraction;
    for (int i = 0; i < Exposant + 1; i++)
    {
        Entier += std::cmp_less(i, Chiffres.size()) ? Chiffres[static_cast<std::size_t>(i)] : '0';
    }
    for (int i = 0; i < Decimales; i++)
    {
        const int j = Exposant + 1 + i;
        Fraction += ((j >= 0) && std::cmp_less(j, Chiffres.size())) ? Chiffres[static_cast<std::size_t>(j)] : '0';
    }
    Entier.erase(0, qMin(Entier.find_first_not_of('0'), Entier.size()));
    if (std::cmp_less(Entier.size(), EntiersMin))
    {
        Entier.insert(0, static_cast<std::size_t>(EntiersMin - static_cast<int>(Entier.size())), '0');
    }
    while (std::cmp_greater(Fraction.size(), DecimalesMin) && (Fraction.back() == '0'))
    {
        Fraction.pop_back();
    }
    // Résultat
    std::string r = ((Valeur < 0) && !Zero) ? "-" : "";
    bool EntierEcrit = false;
    bool PointVu = false;
    std::size_t Decimale = 0;
    for (std::size_t i = 0; i < f.size(); i++)
    {
        const char c = f[i];
        if ((c == '"') || (c == '\''))
        {
            std::size_t j = f.find(c, i + 1);
            if (j == std::string::npos)
            {
                j = f.size();
            }
            r += f.substr(i + 1, j - i - 1);
            i = j;
        }
        else if ((c == '.') && !PointVu)
        {
            PointVu = true;
            if (!EntierEcrit)
            {
                r += Entier;
                EntierEcrit = true;
            }
            if (!Fraction.empty())
            {
                r += FormatSettings.DecimalSeparator;
            }
        }
        else if ((c == '0') || (c == '#'))
        {
            if (!PointVu)
            {
                if (!EntierEcrit)
                {
                    r += Entier;
                    EntierEcrit = true;
                }
            }
            else if (Decimale < Fraction.size())
            {
                r += Fraction[Decimale++];
            }
        }
        else
        {
            r += c;
        }
    }
    if (!EntierEcrit)
    {
        r += Entier;
    }
    return {r};
}
//---------------------------------------------------------------------------
// TScreen
//---------------------------------------------------------------------------
TScreen *Screen()
{ // Créé au premier appel : une exception éventuelle reste interceptable
    static TScreen Ecran;
    return &Ecran;
}
//---------------------------------------------------------------------------
TScreen::TScreen()
{
    PixelsPerInch.Definit(
        [] {
            QScreen *e = QGuiApplication::primaryScreen();
            return e ? qRound(96 * e->devicePixelRatio()) : 96;
        },
        [](const int &) {});
}
//---------------------------------------------------------------------------
// TApplication
//---------------------------------------------------------------------------
TApplication *Application = nullptr;
//---------------------------------------------------------------------------
TApplication::TApplication()
{ // Avec MainFormOnTaskBar, Title n'apparaît nulle part : il est seulement mémorisé
    Title.Definit([this] { return asTitre; }, [this](const AnsiString &as) { asTitre = as; });
}
//---------------------------------------------------------------------------

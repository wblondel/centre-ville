//---------------------------------------------------------------------------
// compat/temps.h - Portage de Centre-Ville hors de C++Builder

// Remplace <systdate.h> pour le moteur : TDateTime (nombre de jours en double,
// partie fractionnaire = heure) et Time() (heure courante, partie date nulle).
// Le moteur ne s'en sert que pour chronométrer (centre_ville::TopChrono et
// StopChrono) : cela n'influe pas sur la simulation.
//---------------------------------------------------------------------------
#ifndef compat_tempsH
#define compat_tempsH
//---------------------------------------------------------------------------
class TDateTime
{
public:
    double Val;
    TDateTime() : Val(0.0) {}
    // NOLINTNEXTLINE(*-explicit-constructor) : conversion implicite voulue, comme la VCL
    TDateTime(double d) : Val(d) {}
    // NOLINTNEXTLINE(*-explicit-constructor) : conversion implicite voulue, comme la VCL
    TDateTime(int i) : Val(static_cast<double>(i)) {}
    // NOLINTNEXTLINE(*-explicit-constructor) : conversion implicite voulue, comme la VCL
    operator double() const { return Val; }
    TDateTime &operator+=(const TDateTime &dt)
    {
        Val += dt.Val;
        return *this;
    }
    TDateTime &operator-=(const TDateTime &dt)
    {
        Val -= dt.Val;
        return *this;
    }
    TDateTime operator+(const TDateTime &dt) const { return {Val + dt.Val}; }
    TDateTime operator-(const TDateTime &dt) const { return {Val - dt.Val}; }
};
//---------------------------------------------------------------------------
TDateTime Time(); // Heure locale courante, en fraction de jour
//---------------------------------------------------------------------------
#endif

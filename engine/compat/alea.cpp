//---------------------------------------------------------------------------
// compat/alea.cpp - Portage de Centre-Ville hors de C++Builder
//---------------------------------------------------------------------------
#include <cstdint>
#include "alea.h"
//---------------------------------------------------------------------------
namespace
{
std::uint32_t GraineCrt = alea::GRAINE_INITIALE; // état laissé par _boot_srand
bool GraineForcee = false;
std::uint32_t ValeurForcee = 0;
//-------------------------------------------------------------------------
void Avance()
{
    GraineCrt = (GraineCrt * alea::MULTIPLICATEUR) + 1U;
}
} // namespace
//---------------------------------------------------------------------------
int alea::Rand()
{
    Avance();
    return static_cast<int>((GraineCrt >> 16) & 0x7FFFU);
}
//---------------------------------------------------------------------------
void alea::Srand(unsigned Graine)
{ // Comme la RTL : la graine est posée, puis le générateur avance d'un pas
    GraineCrt = GraineForcee ? ValeurForcee : static_cast<std::uint32_t>(Graine);
    Avance();
}
//---------------------------------------------------------------------------
unsigned alea::Graine()
{
    return GraineCrt;
}
//---------------------------------------------------------------------------
void alea::FixeGraine(unsigned Graine)
{
    GraineForcee = true;
    ValeurForcee = static_cast<std::uint32_t>(Graine);
    Srand(Graine);
}
//---------------------------------------------------------------------------
void alea::LibereGraine()
{
    GraineForcee = false;
}
//---------------------------------------------------------------------------

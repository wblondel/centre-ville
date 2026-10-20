//---------------------------------------------------------------------------
// compat/alea.cpp - Portage de Centre-Ville hors de C++Builder
//---------------------------------------------------------------------------
#include <cstdint>
#include "alea.h"
//---------------------------------------------------------------------------
namespace
 {
  std::uint32_t GraineCrt=1; // Valeur initiale de la RTL Borland
  bool GraineForcee=false;
  std::uint32_t ValeurForcee=0;
 }
//---------------------------------------------------------------------------
int alea::Rand()
 {
  GraineCrt=GraineCrt*22695477u+1u; // 0x015A4E35
  return (int)((GraineCrt>>16)&0x7FFFu);
 }
//---------------------------------------------------------------------------
void alea::Srand(unsigned Graine)
 {
  GraineCrt=GraineForcee?ValeurForcee:(std::uint32_t)Graine;
 }
//---------------------------------------------------------------------------
unsigned alea::Graine()
 {
  return GraineCrt;
 }
//---------------------------------------------------------------------------
void alea::FixeGraine(unsigned Graine)
 {
  GraineForcee=true;
  ValeurForcee=(std::uint32_t)Graine;
  GraineCrt=ValeurForcee;
 }
//---------------------------------------------------------------------------
void alea::LibereGraine()
 {
  GraineForcee=false;
 }
//---------------------------------------------------------------------------

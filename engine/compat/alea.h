//---------------------------------------------------------------------------
// compat/alea.h - Portage de Centre-Ville hors de C++Builder
//
// rand()/srand() de la RTL Borland (C++Builder), indépendants de la libc :
//   graine = graine * 22695477 + 1   (arithmétique non signée sur 32 bits)
//   rand() = (graine >> 16) & 0x7FFF (bits 30..16)   ;   RAND_MAX = 32767
//   graine initiale = 1 (comme si srand(1) avait été appelé)
// La libc de chaque plateforme a son propre algorithme et son propre RAND_MAX :
// or le moteur utilise rand()%N partout, d'où cette réimplémentation.
//
// À VÉRIFIER contre le binaire d'origine (voir PLAN_PORTAGE.md, phase 0).
//
// Graine injectable (tests, oracle) : FixeGraine(g) force srand() à utiliser g
// quelle que soit la valeur demandée, ce qui revient à remplacer
// srand(time(...)) par srand(g) dans AppelleAuChaos(), sans toucher au moteur.
//
// Ce fichier ne doit être inclus qu'après les en-têtes système : il redéfinit
// rand() et srand() par des macros (le moteur les appelle sans qualification).
//---------------------------------------------------------------------------
#ifndef compat_aleaH
#define compat_aleaH
//---------------------------------------------------------------------------
namespace alea
 {
  const int RAND_MAX_BORLAND=0x7FFF;
  int Rand(); // rand() Borland
  void Srand(unsigned Graine); // srand() Borland (sauf graine forcée)
  unsigned Graine(); // état courant du générateur (tests)
  void FixeGraine(unsigned Graine); // Force la graine utilisée par Srand() et l'applique immédiatement
  void LibereGraine(); // Srand() utilise de nouveau la graine demandée
 }
//---------------------------------------------------------------------------
#define rand()   alea::Rand()
#define srand(g) alea::Srand(g)
//---------------------------------------------------------------------------
#endif

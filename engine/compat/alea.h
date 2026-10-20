//---------------------------------------------------------------------------
// compat/alea.h - Portage de Centre-Ville hors de C++Builder
//
// rand()/srand() de la RTL Embarcadero (C++Builder 10.x) avec laquelle
// cvw 5.4.3.2 a été compilé, indépendants de la libc :
//   pas      : graine = graine * 0x15A4E35 + 1 (22695477, arithmétique 32 bits)
//   rand()   : un pas, puis renvoie (graine >> 16) & 0x7FFF (bits 30..16) ;
//              RAND_MAX = 32767
//   srand(g) : graine = g, PUIS un pas (le premier rand() après srand(g)
//              correspond donc au 2e tirage de la suite Turbo C)
//   état initial (_boot_srand) : graine = 1 puis un pas = 0x015A4E36
// Vérifié par désassemblage des deux exécutables de l'installeur officiel
// (identiques en 32 et 64 bits) : voir engine/README.md, « Vérifications sur
// le binaire d'origine ». La libc de chaque plateforme a son propre algorithme
// et son propre RAND_MAX, or le moteur utilise rand()%N partout.
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
  const unsigned MULTIPLICATEUR=0x015A4E35u; // 22695477
  const unsigned GRAINE_INITIALE=1u*MULTIPLICATEUR+1u; // _boot_srand : 0x015A4E36
  int Rand(); // rand() Borland
  void Srand(unsigned Graine); // srand() Borland (sauf graine forcée)
  unsigned Graine(); // état courant du générateur (tests)
  void FixeGraine(unsigned Graine); // Force la graine utilisée par Srand() et l'applique immédiatement (comme srand)
  void LibereGraine(); // Srand() utilise de nouveau la graine demandée
 }
//---------------------------------------------------------------------------
#define rand()   alea::Rand()
#define srand(g) alea::Srand(g)
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
// allocation.cpp - Tests du moteur : allocations remises à zéro
//
// Le moteur d'origine laissait quelques champs non initialisés (par exemple
// pieton::TourAttBus avant la première attente, ou centre_ville::NbVehlibs
// pour les fichiers antérieurs à la v5.3). Pour que l'export de l'état complet
// et le golden master soient déterministes d'une plateforme à l'autre, les
// exécutables de test remplacent operator new par une version qui renvoie
// une mémoire mise à zéro. Depuis la 6.0.1, le moteur initialise lui-même ces
// champs : cvtests_motif refait les mêmes tests avec une mémoire remplie d'un
// motif (allocation_motif.cpp).
// Compte aussi les blocs vivants, pour vérifier que le moteur libère ce qu'il
// alloue (NombreAllocationsVives, test_corrections.cpp).
//---------------------------------------------------------------------------
#include <atomic>
#include <cstdlib>
#include <new>
//---------------------------------------------------------------------------
static std::atomic<long> AllocationsVives{0};
long NombreAllocationsVives() { return AllocationsVives; }
static void *Alloue(std::size_t n) noexcept
 {
  void *p=std::calloc(n?n:1, 1);
  if (p) AllocationsVives++;
  return p;
 }
static void Libere(void *p) noexcept
 {
  if (p) AllocationsVives--;
  std::free(p);
 }
//---------------------------------------------------------------------------
void *operator new(std::size_t n)
 {
  if (void *p=Alloue(n)) return p;
  throw std::bad_alloc();
 }
//---------------------------------------------------------------------------
void *operator new[](std::size_t n)
 {
  if (void *p=Alloue(n)) return p;
  throw std::bad_alloc();
 }
//---------------------------------------------------------------------------
void *operator new(std::size_t n, const std::nothrow_t &) noexcept
 {
  return Alloue(n);
 }
//---------------------------------------------------------------------------
void *operator new[](std::size_t n, const std::nothrow_t &) noexcept
 {
  return Alloue(n);
 }
//---------------------------------------------------------------------------
void operator delete(void *p) noexcept { Libere(p); }
void operator delete[](void *p) noexcept { Libere(p); }
void operator delete(void *p, std::size_t) noexcept { Libere(p); }
void operator delete[](void *p, std::size_t) noexcept { Libere(p); }
void operator delete(void *p, const std::nothrow_t &) noexcept { Libere(p); }
void operator delete[](void *p, const std::nothrow_t &) noexcept { Libere(p); }
//---------------------------------------------------------------------------

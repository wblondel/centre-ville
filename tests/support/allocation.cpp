//---------------------------------------------------------------------------
// allocation.cpp - Tests du moteur : allocations remises à zéro
//
// Le moteur d'origine laisse quelques champs non initialisés (par exemple
// pieton::TourAttBus avant la première attente, ou centre_ville::NbVehlibs
// pour les fichiers antérieurs à la v5.3). Pour que l'export de l'état complet
// et le golden master soient déterministes d'une plateforme à l'autre, les
// exécutables de test remplacent operator new par une version qui renvoie
// une mémoire mise à zéro. Le moteur lui-même n'est pas modifié.
//---------------------------------------------------------------------------
#include <cstdlib>
#include <new>
//---------------------------------------------------------------------------
void *operator new(std::size_t n)
 {
  if (void *p=std::calloc(n?n:1, 1)) return p;
  throw std::bad_alloc();
 }
//---------------------------------------------------------------------------
void *operator new[](std::size_t n)
 {
  if (void *p=std::calloc(n?n:1, 1)) return p;
  throw std::bad_alloc();
 }
//---------------------------------------------------------------------------
void *operator new(std::size_t n, const std::nothrow_t &) noexcept
 {
  return std::calloc(n?n:1, 1);
 }
//---------------------------------------------------------------------------
void *operator new[](std::size_t n, const std::nothrow_t &) noexcept
 {
  return std::calloc(n?n:1, 1);
 }
//---------------------------------------------------------------------------
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }
void operator delete(void *p, const std::nothrow_t &) noexcept { std::free(p); }
void operator delete[](void *p, const std::nothrow_t &) noexcept { std::free(p); }
//---------------------------------------------------------------------------

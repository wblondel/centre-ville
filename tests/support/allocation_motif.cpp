//---------------------------------------------------------------------------
// allocation_motif.cpp - Tests du moteur : allocations remplies d'un motif
//
// Contraire d'allocation.cpp : operator new renvoie une mémoire remplie de
// l'octet 0xA5, pour qu'un champ que le moteur n'initialise pas se voie (un
// entier vaut 0xA5A5A5A5, un pointeur n'est pas nul). Utilisé par
// cvtests_motif (test_memoire.cpp) : depuis la 6.0.1, le moteur initialise
// lui-même les champs que l'original laissait indéterminés.
//---------------------------------------------------------------------------
#include <cstdlib>
#include <cstring>
#include <new>
//---------------------------------------------------------------------------
namespace
 {
  void *Alloue(std::size_t n)
   {
    void *p=std::malloc(n?n:1);
    if (p) std::memset(p, 0xA5, n?n:1);
    return p;
   }
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
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }
void operator delete(void *p, const std::nothrow_t &) noexcept { std::free(p); }
void operator delete[](void *p, const std::nothrow_t &) noexcept { std::free(p); }
//---------------------------------------------------------------------------

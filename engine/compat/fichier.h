//---------------------------------------------------------------------------
// compat/fichier.h - Portage de Centre-Ville hors de C++Builder
//
// FileOpen/FileCreate/FileRead/FileWrite/FileSeek/FileClose de SysUtils,
// avec la même sémantique que la VCL :
//  - les poignées sont des int strictement positifs ;
//  - FileOpen/FileCreate renvoient -1 en cas d'échec (INVALID_HANDLE_VALUE) ;
//  - FileRead/FileWrite renvoient le nombre d'octets lus/écrits (0 en fin de
//    fichier) ou -1 en cas d'erreur ;
//  - les écritures passent par un tampon : une erreur n'est connue qu'en le
//    vidant. Après un échec, les écritures suivantes échouent aussi (le
//    fichier est déjà incomplet), et FileFlush, absente de SysUtils, vide le
//    tampon et dit si toutes les écritures ont abouti (v6.0.1) ;
//  - les fichiers sont toujours ouverts en binaire ;
//  - les noms de fichiers sont en UTF-8 (convertis en UTF-16 sous Windows).
//---------------------------------------------------------------------------
#ifndef compat_fichierH
#define compat_fichierH
//---------------------------------------------------------------------------
#include "chaine.h"
//---------------------------------------------------------------------------
// Modes de FileOpen (valeurs de SysUtils)
const unsigned fmOpenRead       = 0x0000;
const unsigned fmOpenWrite      = 0x0001;
const unsigned fmOpenReadWrite  = 0x0002;
const unsigned fmShareCompat    = 0x0000;
const unsigned fmShareExclusive = 0x0010;
const unsigned fmShareDenyWrite = 0x0020;
const unsigned fmShareDenyRead  = 0x0030;
const unsigned fmShareDenyNone  = 0x0040;
//---------------------------------------------------------------------------
int FileOpen(const AnsiString &asNomFichier, unsigned Mode);
int FileCreate(const AnsiString &asNomFichier);
int FileRead(int Handle, void *Tampon, unsigned NbOctets);
int FileWrite(int Handle, const void *Tampon, unsigned NbOctets);
int FileSeek(int Handle, int Decalage, int Origine); // Origine : 0 début, 1 courant, 2 fin
bool FileFlush(int Handle); // false si une écriture a échoué depuis l'ouverture (v6.0.1)
void FileClose(int Handle);
//---------------------------------------------------------------------------
#endif

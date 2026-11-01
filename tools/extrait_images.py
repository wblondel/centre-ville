#!/usr/bin/env python3
"""Extraction des images embarquées dans les fiches VCL (.dfm) de Centre-Ville.

Phase 3 de PLAN_PORTAGE.md (outillage) :
  - TImageCollection : chaque élément (Image.Data, PNG) est écrit sous
    images/<nom de collection>.png, le « \\ » du nom devenant un dossier
    (ex. 'Simulation\\S000' -> images/Simulation/S000.png) ;
  - Glyph.Data (TBitBtn, TSpeedButton) et Picture.Data (TImage) : écrits sous
    images/<fiche>/<composant>.png ; les BMP sont convertis en PNG avec canal
    alpha selon la règle de la VCL (couleur transparente = pixel en bas à
    gauche) pour les glyphes et les TImage « Transparent = True » ; le BMP
    d'origine est conservé à côté ;
  - TVirtualImageList : table d'index (indice -> image) écrite dans
    index_images.json et index_images.h, pour que les constantes IDB_* de
    f_simul.cpp restent valables sans changement ;
  - images.qrc : fichier de ressources Qt listant toutes les images ;
  - rapport_images.txt : contrôle des IDB_* et contrôle croisé avec res/png/.

Usage :
  python3 tools/extrait_images.py [--src src] [--res res/png] [--sortie app/ressources]
  python3 tools/extrait_images.py --verifie   # échoue si la sortie n'est pas à jour

Aucune dépendance hors bibliothèque standard.
"""
import argparse
import json
import os
import re
import struct
import sys
import zlib
from collections import OrderedDict

# ---------------------------------------------------------------------------
# Lecture des .dfm (format texte)
# ---------------------------------------------------------------------------


class ErreurDfm(Exception):
    pass


class Objet:
    def __init__(self, nom, classe, genre):
        self.nom = nom
        self.classe = classe
        self.genre = genre  # object / inherited / inline
        self.props = OrderedDict()
        self.enfants = []

    def parcours(self):
        yield self
        for e in self.enfants:
            yield from e.parcours()


class Binaire(bytes):
    """Valeur { ... } d'une propriété."""


class Identifiant(str):
    """Identifiant non quoté (True, clBtnFace, poScreenCenter, ...)."""


_JETON = re.compile(r"""
    (?P<espace>\s+)
  | (?P<chaine>(?:'(?:[^']|'')*'|\#\d+)+)
  | (?P<nombre>[-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?)
  | (?P<ident>[A-Za-z_][A-Za-z0-9_]*(?:\.[A-Za-z_][A-Za-z0-9_]*)*)
  | (?P<symbole>[=:<>(){}\[\],+])
""", re.VERBOSE)


def _decode_chaine(s):
    resultat = []
    for m in re.finditer(r"'((?:[^']|'')*)'|#(\d+)", s):
        if m.group(1) is not None:
            resultat.append(m.group(1).replace("''", "'"))
        else:
            resultat.append(chr(int(m.group(2))))
    return "".join(resultat)


class LecteurDfm:
    def __init__(self, texte, nom_fichier):
        self.texte = texte
        self.nom_fichier = nom_fichier
        self.jetons = []
        pos = 0
        n = len(texte)
        while pos < n:
            if texte[pos] == '{':  # données binaires hexadécimales
                fin = texte.index('}', pos)
                hexa = re.sub(r"\s+", "", texte[pos + 1:fin])
                self.jetons.append(("binaire", Binaire(bytes.fromhex(hexa))))
                pos = fin + 1
                continue
            m = _JETON.match(texte, pos)
            if not m:
                raise ErreurDfm("%s : caractère inattendu %r à la position %d" % (nom_fichier, texte[pos], pos))
            genre = m.lastgroup
            if genre != "espace":
                self.jetons.append((genre, m.group(genre)))
            pos = m.end()
        self.i = 0

    def _voit(self, genre=None, valeur=None):
        if self.i >= len(self.jetons):
            return False
        g, v = self.jetons[self.i]
        return (genre is None or g == genre) and (valeur is None or v == valeur)

    def _prend(self, genre=None, valeur=None):
        if not self._voit(genre, valeur):
            attendu = valeur or genre
            obtenu = self.jetons[self.i] if self.i < len(self.jetons) else "fin"
            raise ErreurDfm(f"{self.nom_fichier} : {attendu!r} attendu, {obtenu!r} obtenu")
        j = self.jetons[self.i]
        self.i += 1
        return j[1]

    def lit(self):
        racine = self._objet()
        return racine

    def _objet(self):
        genre = self._prend("ident")
        if genre not in ("object", "inherited", "inline"):
            raise ErreurDfm(f"{self.nom_fichier} : 'object' attendu, {genre!r} obtenu")
        nom = self._prend("ident")
        classe = None
        if self._voit("symbole", ":"):
            self._prend()
            classe = self._prend("ident")
        if self._voit("symbole", "["):  # index d'héritage [n]
            while not self._voit("symbole", "]"):
                self.i += 1
            self._prend()
        obj = Objet(nom, classe, genre)
        while not self._voit("ident", "end"):
            if self._voit("ident") and self.jetons[self.i][1] in ("object", "inherited", "inline"):
                obj.enfants.append(self._objet())
            else:
                prop = self._prend("ident")
                self._prend("symbole", "=")
                obj.props[prop] = self._valeur()
        self._prend("ident", "end")
        return obj

    def _valeur(self):
        g, v = self.jetons[self.i]
        if g == "chaine":
            self.i += 1
            texte = _decode_chaine(v)
            while self._voit("symbole", "+"):
                self.i += 1
                texte += _decode_chaine(self._prend("chaine"))
            return texte
        if g == "nombre":
            self.i += 1
            return float(v) if ("." in v or "e" in v.lower()) else int(v)
        if g == "ident":
            self.i += 1
            return Identifiant(v)
        if g == "binaire":
            self.i += 1
            return v
        if v == "(":
            self.i += 1
            valeurs = []
            while not self._voit("symbole", ")"):
                valeurs.append(self._valeur())
            self.i += 1
            return valeurs
        if v == "[":
            self.i += 1
            valeurs = []
            while not self._voit("symbole", "]"):
                if self._voit("symbole", ","):
                    self.i += 1
                    continue
                valeurs.append(self._valeur())
            self.i += 1
            return set(valeurs)
        if v == "<":
            self.i += 1
            elements = []
            while not self._voit("symbole", ">"):
                self._prend("ident", "item")
                if self._voit("symbole", "["):  # item [n]
                    while not self._voit("symbole", "]"):
                        self.i += 1
                    self.i += 1
                props = OrderedDict()
                while not self._voit("ident", "end"):
                    prop = self._prend("ident")
                    self._prend("symbole", "=")
                    props[prop] = self._valeur()
                self._prend("ident", "end")
                elements.append(props)
            self.i += 1
            return elements
        raise ErreurDfm(f"{self.nom_fichier} : valeur inattendue {v!r}")


def lit_dfm(chemin):
    with open(chemin, encoding="utf-8") as f:
        return LecteurDfm(f.read(), os.path.basename(chemin)).lit()


# ---------------------------------------------------------------------------
# Images : PNG, BMP -> PNG
# ---------------------------------------------------------------------------

def taille_png(donnees):
    if donnees[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("pas un PNG")
    largeur, hauteur = struct.unpack(">II", donnees[16:24])
    return largeur, hauteur


def ecrit_png_rgba(largeur, hauteur, pixels):
    """pixels : liste de lignes (haut -> bas) de tuples (r, g, b, a)."""
    brut = bytearray()
    for ligne in pixels:
        brut.append(0)  # filtre None
        for r, g, b, a in ligne:
            brut += bytes((r, g, b, a))

    def bloc(type_bloc, contenu):
        return (struct.pack(">I", len(contenu)) + type_bloc + contenu +
                struct.pack(">I", zlib.crc32(type_bloc + contenu) & 0xFFFFFFFF))

    ihdr = struct.pack(">IIBBBBB", largeur, hauteur, 8, 6, 0, 0, 0)
    return (b"\x89PNG\r\n\x1a\n" + bloc(b"IHDR", ihdr) +
            bloc(b"IDAT", zlib.compress(bytes(brut), 9)) + bloc(b"IEND", b""))


def decode_bmp(bmp):
    """Décode un BMP Windows non compressé (1, 4, 8, 24 ou 32 bits).
    Renvoie (largeur, hauteur, lignes de tuples (r, g, b)) de haut en bas."""
    if bmp[:2] != b"BM":
        raise ValueError("pas un BMP")
    decalage = struct.unpack("<I", bmp[10:14])[0]
    taille_entete = struct.unpack("<I", bmp[14:18])[0]
    largeur, hauteur = struct.unpack("<ii", bmp[18:26])
    bpp, compression = struct.unpack("<HI", bmp[28:34])
    if compression != 0:
        raise ValueError("BMP compressé non pris en charge")
    nb_couleurs = struct.unpack("<I", bmp[46:50])[0] if taille_entete >= 40 else 0
    palette = []
    if bpp <= 8:
        n = nb_couleurs or (1 << bpp)
        debut = 14 + taille_entete
        for k in range(n):
            b, g, r, _ = bmp[debut + 4 * k:debut + 4 * k + 4]
            palette.append((r, g, b))
    de_bas_en_haut = hauteur > 0
    hauteur = abs(hauteur)
    pas = ((largeur * bpp + 31) // 32) * 4
    lignes = []
    for y in range(hauteur):
        ligne_fichier = (hauteur - 1 - y) if de_bas_en_haut else y
        octets = bmp[decalage + ligne_fichier * pas:decalage + (ligne_fichier + 1) * pas]
        ligne = []
        for x in range(largeur):
            if bpp == 24:
                b, g, r = octets[3 * x:3 * x + 3]
                ligne.append((r, g, b))
            elif bpp == 32:
                b, g, r, _ = octets[4 * x:4 * x + 4]
                ligne.append((r, g, b))
            else:
                bit = x * bpp
                valeur = (octets[bit // 8] >> (8 - bpp - bit % 8)) & ((1 << bpp) - 1)
                ligne.append(palette[valeur])
        lignes.append(ligne)
    return largeur, hauteur, lignes


def bmp_vers_png(bmp, transparent):
    largeur, hauteur, lignes = decode_bmp(bmp)
    cle = lignes[-1][0] if transparent else None  # pixel en bas à gauche (règle de la VCL)
    pixels = [[(r, g, b, 0 if (r, g, b) == cle else 255) for (r, g, b) in ligne] for ligne in lignes]
    return ecrit_png_rgba(largeur, hauteur, pixels), largeur, hauteur


# ---------------------------------------------------------------------------
# Extraction
# ---------------------------------------------------------------------------

class Extraction:
    def __init__(self):
        self.fichiers = OrderedDict()   # chemin relatif -> octets
        self.collections = OrderedDict()  # fiche -> objet -> [ {nom, fichier, largeur, hauteur} ]
        self.listes = OrderedDict()     # fiche -> objet -> {largeur, hauteur, collection, images: [...]}
        self.composants = []            # glyphes et images de composants
        self.messages = []

    def ajoute(self, chemin, octets):
        if chemin in self.fichiers and self.fichiers[chemin] != octets:
            self.messages.append(f"ATTENTION : deux images différentes pour {chemin}")
        self.fichiers[chemin] = octets


def nom_fichier_sur(nom):
    return re.sub(r"[^A-Za-z0-9_.\-]", "_", nom)


def extrait(src):
    ex = Extraction()
    for chemin in sorted(f for f in os.listdir(src) if f.endswith(".dfm")):
        fiche = os.path.splitext(chemin)[0]
        racine = lit_dfm(os.path.join(src, chemin))
        for obj in racine.parcours():
            # TImageCollection
            if obj.classe == "TImageCollection":
                elements = []
                for item in obj.props.get("Images", []):
                    nom = item["Name"]
                    sources = item.get("SourceImages", [])
                    for _k, source in enumerate(sources):
                        png = bytes(source["Image.Data"])
                        largeur, hauteur = taille_png(png)
                        base = "images/" + "/".join(nom_fichier_sur(p) for p in nom.split("\\"))
                        fichier = base + (".png" if len(sources) == 1 else "@%dx%d.png" % (largeur, hauteur))
                        ex.ajoute(fichier, png)
                        elements.append({"nom": nom, "fichier": fichier, "largeur": largeur, "hauteur": hauteur})
                ex.collections.setdefault(fiche, OrderedDict())[obj.nom] = elements
            # TVirtualImageList
            if obj.classe == "TVirtualImageList":
                images = []
                for item in obj.props.get("Images", []):
                    images.append({"nom": item.get("Name"), "collection": item.get("CollectionName"),
                                   "indice_collection": item.get("CollectionIndex"),
                                   "desactive": bool(item.get("Disabled", False) == "True")})
                ex.listes.setdefault(fiche, OrderedDict())[obj.nom] = {
                    "largeur": obj.props.get("Width", 16), "hauteur": obj.props.get("Height", 16),
                    "collection": str(obj.props.get("ImageCollection", "")), "images": images}
            # Glyph.Data (TBitBtn, TSpeedButton) : entier de taille + BMP
            if "Glyph.Data" in obj.props:
                donnees = bytes(obj.props["Glyph.Data"])
                bmp = donnees[4:]
                base = f"images/{fiche}/{nom_fichier_sur(obj.nom)}"
                png, largeur, hauteur = bmp_vers_png(bmp, transparent=True)
                ex.ajoute(base + ".bmp", bmp)
                ex.ajoute(base + ".png", png)
                ex.composants.append({"fiche": fiche, "composant": obj.nom, "classe": obj.classe,
                                      "propriete": "Glyph", "fichier": base + ".png",
                                      "largeur": largeur, "hauteur": hauteur,
                                      "glyphes": obj.props.get("NumGlyphs", 1)})
            # Picture.Data (TImage) : nom de classe (chaîne courte) + données de la classe
            if "Picture.Data" in obj.props:
                donnees = bytes(obj.props["Picture.Data"])
                n = donnees[0]
                classe_image = donnees[1:1 + n].decode("ascii")
                reste = donnees[1 + n:]
                base = f"images/{fiche}/{nom_fichier_sur(obj.nom)}"
                if classe_image == "TPngImage":
                    largeur, hauteur = taille_png(reste)
                    ex.ajoute(base + ".png", reste)
                elif classe_image == "TBitmap":
                    bmp = reste[4:]
                    transparent = str(obj.props.get("Transparent", "False")) == "True"
                    png, largeur, hauteur = bmp_vers_png(bmp, transparent)
                    ex.ajoute(base + ".bmp", bmp)
                    ex.ajoute(base + ".png", png)
                else:
                    ex.messages.append(f"ATTENTION : {fiche}.{obj.nom} : Picture de classe {classe_image} non prise en charge")
                    continue
                ex.composants.append({"fiche": fiche, "composant": obj.nom, "classe": obj.classe,
                                      "propriete": "Picture", "fichier": base + ".png",
                                      "largeur": largeur, "hauteur": hauteur, "format": classe_image})
    return ex


# ---------------------------------------------------------------------------
# Index, contrôles, sorties
# ---------------------------------------------------------------------------

def fichier_de_collection(ex, fiche, nom_collection, nom_image):
    for elt in ex.collections.get(fiche, {}).get(nom_collection, []):
        if elt["nom"] == nom_image:
            return elt["fichier"]
    return None


def constantes_idb(src):
    constantes = OrderedDict()
    with open(os.path.join(src, "f_simul.cpp"), encoding="utf-8") as f:
        for ligne in f:
            m = re.match(r"\s*#define\s+(IDB_\w+)\s+(\d+)", ligne)
            if m:
                constantes[m.group(1)] = int(m.group(2))
    return constantes


def controle_idb(ex, src, rapport):
    """Vérifie que chaque IDB_* désigne une image de VirtualImageList8x8 et que
    la liste suit l'ordre des noms (indice i -> Simulation\\S<i>)."""
    ok = True
    liste = ex.listes["f_simul"]["VirtualImageList8x8"]["images"]
    rapport.append("VirtualImageList8x8 : %d images (indice i -> Simulation\\S<i>)" % len(liste))
    for i, img in enumerate(liste):
        attendu = "Simulation\\S%03d" % i
        if img["collection"] != attendu:
            ok = False
            rapport.append("  ÉCART : indice %d -> %s (attendu %s)" % (i, img["collection"], attendu))
    # Glyphes numérotés à partir de 1 : indice i -> G<i+1> (AffecteGlyphe(NumGlyphe, ...))
    for nom_liste, prefixe, premier in (("VirtualImageList16x16", "Boutons\\B%03d", 0),
                                        ("VirtualImageListGlyphes", "Glyphes\\G%02d", 1)):
        images = ex.listes["f_simul"][nom_liste]["images"]
        rapport.append("%s : %d images (indice i -> %s)" % (nom_liste, len(images), prefixe.replace("%03d", "<i>").replace("%02d", "<i+1>" if premier else "<i>")))
        for i, img in enumerate(images):
            if img["collection"] != prefixe % (i + premier):
                ok = False
                rapport.append("  ÉCART : indice %d -> %s (attendu %s)" % (i, img["collection"], prefixe % (i + premier)))
    rapport.append("")
    rapport.append("Constantes IDB_* de f_simul.cpp (indices de VirtualImageList8x8) :")
    for nom, valeur in constantes_idb(src).items():
        if 0 <= valeur < len(liste):
            rapport.append("  %-14s %3d -> %s" % (nom, valeur, liste[valeur]["collection"]))
        else:
            ok = False
            rapport.append("  %-14s %3d -> HORS LISTE (%d images)" % (nom, valeur, len(liste)))
    return ok


def png_pixels(donnees):
    """Décode un PNG non entrelacé (8 bits : palette, gris, RGB, RGBA) en
    lignes de tuples RGBA, pour comparer deux PNG au pixel près."""
    pos = 8
    ihdr = None
    palette = []
    transparence = b""
    idat = bytearray()
    while pos < len(donnees):
        longueur = struct.unpack(">I", donnees[pos:pos + 4])[0]
        genre = donnees[pos + 4:pos + 8]
        contenu = donnees[pos + 8:pos + 8 + longueur]
        pos += 12 + longueur
        if genre == b"IHDR":
            ihdr = struct.unpack(">IIBBBBB", contenu)
        elif genre == b"PLTE":
            palette = [tuple(contenu[i:i + 3]) for i in range(0, len(contenu), 3)]
        elif genre == b"tRNS":
            transparence = contenu
        elif genre == b"IDAT":
            idat += contenu
    largeur, hauteur, profondeur, couleur, _, _, entrelace = ihdr
    if entrelace or profondeur not in (1, 2, 4, 8):
        return None
    canaux = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}[couleur]
    bits_par_pixel = canaux * profondeur
    pas = (largeur * bits_par_pixel + 7) // 8
    octets_par_pixel = max(1, bits_par_pixel // 8)
    brut = zlib.decompress(bytes(idat))
    lignes = []
    precedente = bytearray(pas)
    for y in range(hauteur):
        filtre = brut[y * (pas + 1)]
        ligne = bytearray(brut[y * (pas + 1) + 1:(y + 1) * (pas + 1)])
        for i in range(pas):
            a = ligne[i - octets_par_pixel] if i >= octets_par_pixel else 0
            b = precedente[i]
            c = precedente[i - octets_par_pixel] if i >= octets_par_pixel else 0
            if filtre == 1:
                ligne[i] = (ligne[i] + a) & 0xFF
            elif filtre == 2:
                ligne[i] = (ligne[i] + b) & 0xFF
            elif filtre == 3:
                ligne[i] = (ligne[i] + (a + b) // 2) & 0xFF
            elif filtre == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                ligne[i] = (ligne[i] + (a if pa <= pb and pa <= pc else (b if pb <= pc else c))) & 0xFF
        precedente = ligne
        pixels = []
        for x in range(largeur):
            if profondeur < 8:
                bit = x * profondeur
                v = (ligne[bit // 8] >> (8 - profondeur - bit % 8)) & ((1 << profondeur) - 1)
                valeurs = [v]
            else:
                valeurs = list(ligne[x * canaux:(x + 1) * canaux])
            if couleur == 3:
                r, g, b = palette[valeurs[0]]
                alpha = transparence[valeurs[0]] if valeurs[0] < len(transparence) else 255
                pixels.append((r, g, b, alpha))
            elif couleur == 0:
                pixels.append((valeurs[0],) * 3 + (255,))
            elif couleur == 4:
                pixels.append((valeurs[0],) * 3 + (valeurs[1],))
            elif couleur == 2:
                pixels.append((*tuple(valeurs), 255))
            else:
                pixels.append(tuple(valeurs))
        lignes.append(pixels)
    return lignes


def controle_res_png(ex, res, racine, rapport):
    """Compare les images de toutes les collections aux fichiers de res/png/
    (recherchés par nom, dans tous les sous-dossiers)."""
    affichage = os.path.relpath(res, racine)
    if not os.path.isdir(res):
        rapport.append(f"Dossier {affichage} absent : contrôle croisé non effectué.")
        return True
    references = {}
    for dossier, _, fichiers in os.walk(res):
        for f in sorted(fichiers):
            if f.lower().endswith(".png"):
                references.setdefault(os.path.splitext(f)[0], []).append(os.path.join(dossier, f))
    rapport.append(f"Contrôle croisé des collections avec {affichage} :")
    ok = True
    noms_vus = set()
    for fiche, collections in ex.collections.items():
        for nom_collection, elements in collections.items():
            identiques = memes_pixels = differents = absents = 0
            details = []
            for elt in elements:
                cle = elt["nom"].split("\\")[-1]
                noms_vus.add(cle)
                png = ex.fichiers[elt["fichier"]]
                candidats = references.get(cle, [])
                if not candidats:
                    absents += 1
                    details.append("    absente : {}".format(elt["nom"]))
                    continue
                with open(candidats[0], "rb") as f:
                    ref = f.read()
                if ref == png:
                    identiques += 1
                elif png_pixels(ref) == png_pixels(png):
                    memes_pixels += 1
                else:
                    differents += 1
                    ok = False
                    details.append("    DIFFÉRENTE : {} / {}".format(elt["nom"], os.path.relpath(candidats[0], racine)))
            rapport.append("  %s.dfm, %s : %d identiques à l'octet près, %d au pixel près, %d différentes, %d absentes"
                           % (fiche, nom_collection, identiques, memes_pixels, differents, absents))
            rapport.extend(details)
    en_trop = sorted(set(references) - noms_vus)
    if en_trop:
        rapport.append("  Présentes dans {} mais dans aucune collection : {}".format(affichage, ", ".join(en_trop)))
    return ok


def entete_cpp(ex):
    lignes = ["//---------------------------------------------------------------------------",
         "// index_images.h - GÉNÉRÉ par tools/extrait_images.py : ne pas modifier",
         "//",
         "// Indices des TVirtualImageList de f_simul.dfm -> images des ressources Qt.",
         "// Les constantes IDB_* de f_simul.cpp s'appliquent telles quelles à",
         "// VirtualImageList8x8 (malgré son nom, des images 16x16).",
         "//---------------------------------------------------------------------------",
         "#ifndef index_imagesH", "#define index_imagesH", ""]
    for fiche, listes in ex.listes.items():
        for nom_liste, liste in listes.items():
            lignes.append("// %s.dfm, %s : %d images %dx%d" % (fiche, nom_liste, len(liste["images"]),
                                                          liste["largeur"], liste["hauteur"]))
            lignes.append("const int NB_%s_%s=%d;" % (fiche.upper(), nom_liste, len(liste["images"])))
            lignes.append(f"const char *const {fiche}_{nom_liste}[NB_{fiche.upper()}_{nom_liste}]=")
            lignes.append(" {")
            for i, img in enumerate(liste["images"]):
                fichier = fichier_de_collection(ex, fiche, liste["collection"], img["collection"])
                lignes.append('  ":/%s", // %d : %s' % (fichier, i, img["collection"].replace("\\", "/")))
            lignes.append(" };")
            lignes.append("")
    lignes.append("#endif")
    return "\n".join(lignes) + "\n"


def qrc(ex):
    lignes = ['<!DOCTYPE RCC>', '<!-- GÉNÉRÉ par tools/extrait_images.py : ne pas modifier -->', '<RCC version="1.0">',
         '  <qresource prefix="/">']
    for chemin in ex.fichiers:
        if chemin.endswith(".png"):
            lignes.append(f"    <file>{chemin}</file>")
    lignes += ["  </qresource>", "</RCC>"]
    return "\n".join(lignes) + "\n"


def index_json(ex):
    index = OrderedDict()
    index["genere_par"] = "tools/extrait_images.py"
    index["collections"] = ex.collections
    index["listes"] = OrderedDict()
    for fiche, listes in ex.listes.items():
        index["listes"][fiche] = OrderedDict()
        for nom_liste, liste in listes.items():
            entree = OrderedDict()
            entree["largeur"] = liste["largeur"]
            entree["hauteur"] = liste["hauteur"]
            entree["collection"] = liste["collection"]
            entree["images"] = [OrderedDict([("indice", i), ("nom", img["nom"]), ("collection", img["collection"]),
                                             ("fichier", fichier_de_collection(ex, fiche, liste["collection"], img["collection"]))])
                                for i, img in enumerate(liste["images"])]
            index["listes"][fiche][nom_liste] = entree
    index["composants"] = ex.composants
    return json.dumps(index, ensure_ascii=False, indent=1) + "\n"


def main():
    racine = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--src", default=os.path.join(racine, "src"))
    p.add_argument("--res", default=os.path.join(racine, "res", "png"))
    p.add_argument("--sortie", default=os.path.join(racine, "app", "ressources"))
    p.add_argument("--verifie", action="store_true", help="n'écrit rien ; échoue si la sortie n'est pas à jour")
    args = p.parse_args()

    ex = extrait(args.src)
    rapport = ["Rapport de tools/extrait_images.py (GÉNÉRÉ : ne pas modifier)", ""]
    nb_collection = sum(len(e) for c in ex.collections.values() for e in c.values())
    rapport.append("%d images de collections, %d images de composants (glyphes/TImage), %d fichiers écrits"
                   % (nb_collection, len(ex.composants), len(ex.fichiers)))
    for fiche, collections in ex.collections.items():
        for nom, elements in collections.items():
            rapport.append("  %s.dfm, %s : %d images" % (fiche, nom, len(elements)))
    rapport.append("")
    ok_idb = controle_idb(ex, args.src, rapport)
    rapport.append("")
    ok_res = controle_res_png(ex, args.res, racine, rapport)
    rapport.extend(ex.messages)

    sorties = OrderedDict(ex.fichiers)
    sorties["index_images.json"] = index_json(ex).encode("utf-8")
    sorties["index_images.h"] = entete_cpp(ex).encode("utf-8")
    sorties["images.qrc"] = qrc(ex).encode("utf-8")
    sorties["rapport_images.txt"] = ("\n".join(rapport) + "\n").encode("utf-8")

    a_jour = True
    for chemin, octets in sorties.items():
        complet = os.path.join(args.sortie, chemin)
        if args.verifie:
            try:
                with open(complet, "rb") as f:
                    if f.read() != octets:
                        a_jour = False
                        print("différent : " + chemin)
            except OSError:
                a_jour = False
                print("absent : " + chemin)
        else:
            os.makedirs(os.path.dirname(complet), exist_ok=True)
            with open(complet, "wb") as f:
                f.write(octets)
    print("\n".join(rapport))
    if not (ok_idb and ok_res):
        print("ÉCHEC des contrôles (voir ci-dessus)", file=sys.stderr)
        return 1
    if args.verifie and not a_jour:
        print("La sortie n'est pas à jour : relancer tools/extrait_images.py", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())

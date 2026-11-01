#!/usr/bin/env python3
"""Recense les conditions dont la borne d'un indice est testée après son usage.

Audit de la 6.0.1 (engine/README.md, « Audit des lectures hors tableau ») :
même motif que voie::DefinitNature, corrigé en 6.0.0. Pour chaque boucle
for (variables incrémentées) ou while, et avec --if pour chaque if, la
condition est découpée en termes reliés par && ; un terme de la forme
« v < borne » (ou <=, >, >=) est signalé si un terme qui le précède utilise
déjà v (indice de tableau, argument d'un appel, case voisine...). Les
commentaires et les chaînes sont ignorés. À trier à la main : une
comparaison sans accès (DirCrt!=d) ou une borne basse (n>-1) est signalée
aussi.

Usage :
    tools/audit_bornes.py [--if] FICHIER...
"""
import re
import sys


def sans_commentaires(s):
    """Remplace commentaires et contenus de chaînes par des blancs (lignes gardées)."""
    out = []
    i, n = 0, len(s)
    while i < n:
        if s.startswith('//', i):
            j = s.find('\n', i)
            j = n if j < 0 else j
            out.append(' ' * (j - i))
            i = j
        elif s.startswith('/*', i):
            j = s.find('*/', i)
            j = n if j < 0 else j + 2
            out.append(re.sub(r'[^\n]', ' ', s[i:j]))
            i = j
        elif s[i] == '"':
            j = i + 1
            while j < n and s[j] != '"':
                j += 2 if s[j] == '\\' else 1
            out.append('"' + ' ' * (j - i - 1) + '"')
            i = j + 1
        else:
            out.append(s[i])
            i += 1
    return ''.join(out)


def parenthese(s, i):
    """Indice de la parenthèse fermante correspondant à s[i]=='('."""
    p = 0
    for j in range(i, len(s)):
        if s[j] == '(':
            p += 1
        elif s[j] == ')':
            p -= 1
            if p == 0:
                return j
    return -1


def decoupe(c, separateur):
    """Découpe c au niveau 0 des parenthèses et crochets."""
    termes, p, deb, i = [], 0, 0, 0
    while i < len(c):
        if c[i] in '([':
            p += 1
        elif c[i] in ')]':
            p -= 1
        elif p == 0 and c.startswith(separateur, i):
            termes.append(c[deb:i])
            deb = i + len(separateur)
            i = deb
            continue
        i += 1
    termes.append(c[deb:])
    return termes


def sans_parentheses(x):
    x = x.strip()
    while x.startswith('(') and parenthese(x, 0) == len(x) - 1:
        x = x[1:-1].strip()
    return x


def audite(nom, mots):
    with open(nom, encoding='utf-8') as f:
        s = sans_commentaires(f.read())
    for m in re.finditer(r'\b({})\s*\('.format('|'.join(mots)), s):
        i = m.end() - 1
        entete = s[i + 1:parenthese(s, i)]
        if m.group(1) == 'for':
            parties = decoupe(entete, ';')
            if len(parties) != 3:
                continue
            condition, increment = parties[1], parties[2]
            variables = set(re.findall(r'([A-Za-z_]\w*)\s*(?:\+\+|--|\+=|-=)', increment)) | \
                set(re.findall(r'(?:\+\+|--)\s*([A-Za-z_]\w*)', increment))
        else:
            condition, variables = entete, None
        termes = [sans_parentheses(x) for x in decoupe(condition, '&&')]
        for k, terme in enumerate(termes):
            b = re.fullmatch(r'([A-Za-z_]\w*)\s*(<=|<|>=|>)\s*(.+)', terme)
            if not b or k == 0:
                continue
            v = b.group(1)
            if variables is not None and v not in variables:
                continue
            avant = [a for a in termes[:k] if re.search(rf'\b{v}\b', a)]
            if avant:
                print('%s:%d  borne « %s » après : %s'
                      % (nom, s.count('\n', 0, m.start()) + 1, terme, ' && '.join(avant)))


def main():
    args = sys.argv[1:]
    mots = ['for', 'while']
    if args and args[0] == '--if':
        mots.append('if')
        args = args[1:]
    if not args:
        print(__doc__)
        return 2
    for nom in args:
        audite(nom, mots)
    return 0


if __name__ == '__main__':
    sys.exit(main())

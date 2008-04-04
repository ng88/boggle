#!/bin/sh

#
# Mets en forme le dico
# 
# Vire les mots de longueur < 3
# Vire les mots contenant [^a-z]
# Trie et vire les doublons
#
#

backup=$1.$RANDOM.old

cp $1 $backup

sort $backup | uniq > $1
sed -i -e '/^[a-z]$/d' -e '/^[a-z][a-z]$/d' -e '/[^a-z]/d' $1



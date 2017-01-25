#!/bin/bash

# This script is used to convert .fbx animations for a model into Urho3d .ani format.
# Fbx animations from Mixamo.com are ready to convert with this script, just take care to check "in place",
# animations when possible to avoid model motions.

# Place in "fbx_anims" folder all fbx animation.
# All animations will be converted in "export/urho_anims"

FBX_ANIMS='fbx_anims'
URHO_ANIMS='export/urho_anims'

for f in $FBX_ANIMS/*.fbx; do
  output="${f##*/}"
  output=`echo "$output" | cut -d'.' -f1`
  output=$URHO_ANIMS/$output.ani
  echo $output
  ./AssetImporter anim $f $output
done

cd $URHO_ANIMS
rename 's/_mixamo.com//g' *.ani
rename 's/_inPlace//g' *.ani

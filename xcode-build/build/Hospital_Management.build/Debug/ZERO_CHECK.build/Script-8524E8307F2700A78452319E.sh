#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/matthewhvizdos/Desktop/COE892/Hospital_Management/xcode-build
  make -f /Users/matthewhvizdos/Desktop/COE892/Hospital_Management/xcode-build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/matthewhvizdos/Desktop/COE892/Hospital_Management/xcode-build
  make -f /Users/matthewhvizdos/Desktop/COE892/Hospital_Management/xcode-build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/matthewhvizdos/Desktop/COE892/Hospital_Management/xcode-build
  make -f /Users/matthewhvizdos/Desktop/COE892/Hospital_Management/xcode-build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/matthewhvizdos/Desktop/COE892/Hospital_Management/xcode-build
  make -f /Users/matthewhvizdos/Desktop/COE892/Hospital_Management/xcode-build/CMakeScripts/ReRunCMake.make
fi


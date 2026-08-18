#!/bin/bash

source ~/amesim_env.sh

# Absolute directory containing this script
SIL_KIT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

# Paths
REGISTRY="$SIL_KIT_DIR/sil-kit/build/Release/sil-kit-registry"
FMU_IMPORTER="$SIL_KIT_DIR/sil-kit-fmu-importer/FmuImporter/_build/crossplatform-x64-Debug/FmuImporter"
PUBLISHER="$SIL_KIT_DIR/sil-kit/build/Release/SilKitDemoPublisher"
SUBSCRIBER="$SIL_KIT_DIR/sil-kit/build/Demos/communication/PubSub/SilKitDemoSubscriber"
SYSTEM_CONTROLLER="$SIL_KIT_DIR/sil-kit/build/Release/sil-kit-system-controller"

FMU="$SIL_KIT_DIR/fmus/plant/InvertedPendulum.fmu"
CONFIG="$SIL_KIT_DIR/fmus/plant/InvertedPendulum.config.yaml"
CID="$SIL_KIT_DIR/fmus/plant/InvertedPendulum.cid.yaml"

# --------------------------------------------------
# 1. SIL Kit Registry
# --------------------------------------------------

gnome-terminal --title="SIL Kit Registry" -- bash -c "
echo '=== SIL Kit Registry ==='
echo
'$REGISTRY'
echo
echo 'Registry stopped. Press Enter to close.'
read
"

# Give Registry time to start
sleep 1

# --------------------------------------------------
# 2. FMU Importer
# --------------------------------------------------

gnome-terminal --title="FMU Importer - InvertedPendulum" -- bash -c "
echo '=== FMU Importer ==='
echo
'$FMU_IMPORTER' \
  -f '$FMU' \
  -c '$CONFIG' \
  -i '$CID' \
  -p 'InvertedPendulum'
echo
echo 'FMU Importer stopped. Press Enter to close.'
read
"

# --------------------------------------------------
# 3. Publisher
# --------------------------------------------------

gnome-terminal --title="PID Controller" -- bash -c "
echo '=== PID Controller ==='
echo
'$PUBLISHER'
echo
echo 'PID Controller stopped. Press Enter to close.'
read
"

# --------------------------------------------------
# 4. Subscriber
# --------------------------------------------------

gnome-terminal --title="Plotter" -- bash -c "
echo '=== Plotter ==='
echo
'$SUBSCRIBER'
echo
echo 'Plotter stopped. Press Enter to close.'
read
"

# --------------------------------------------------
# 5. System Controller
# --------------------------------------------------

gnome-terminal --title="SIL Kit System Controller" -- bash -c "
echo '=== SIL Kit System Controller ==='
echo
'$SYSTEM_CONTROLLER' Plotter Controller InvertedPendulum
echo
echo 'System Controller stopped. Press Enter to close.'
read
"

echo "All SIL Kit windows started."

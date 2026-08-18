#!/bin/bash
source ~/amesim_env.sh

SIL_KIT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

REGISTRY_PID=""
FMU_PID=""
PUBLISHER_PID=""
SUBSCRIBER_PID=""
CONTROLLER_PID=""

cleanup() {
    echo "Stopping processes..."

    kill "$CONTROLLER_PID" \
         sleep 1 \
         "$REGISTRY_PID" 2>/dev/null

    wait 2>/dev/null
    echo "All processes stopped."
}

trap cleanup EXIT INT TERM

"$SIL_KIT_DIR/sil-kit/build/Release/sil-kit-registry" &
REGISTRY_PID=$!

sleep 1

"$SIL_KIT_DIR/sil-kit-fmu-importer/FmuImporter/_build/crossplatform-x64-Debug/FmuImporter" \
  -f "$SIL_KIT_DIR/fmus/plant/InvertedPendulum.fmu" \
  -c "$SIL_KIT_DIR/fmus/plant/InvertedPendulum.config.yaml" \
  -i "$SIL_KIT_DIR/fmus/plant/InvertedPendulum.cid.yaml" \
  -p InvertedPendulum &
FMU_PID=$!

sleep 1

"$SIL_KIT_DIR/sil-kit/build/Release/SilKitDemoPublisher" --fast --sim-step-duration 10000 &
PUBLISHER_PID=$!

"$SIL_KIT_DIR/sil-kit/build/Demos/communication/PubSub/SilKitDemoSubscriber" --fast &
SUBSCRIBER_PID=$!

sleep 1

"$SIL_KIT_DIR/sil-kit/build/Release/sil-kit-system-controller" \
 Plotter Controller InvertedPendulum&
CONTROLLER_PID=$!

echo "All processes started."
echo "Press Ctrl+C to stop everything."

wait
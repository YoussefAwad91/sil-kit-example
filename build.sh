source ~/amesim_env.sh

SIL_KIT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

cmake --build "$SIL_KIT_DIR/sil-kit/build"

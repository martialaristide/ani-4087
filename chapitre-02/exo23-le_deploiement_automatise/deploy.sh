#!/usr/bin/env bash
# deploy.sh - enchaine construire -> empaqueter -> signer -> installer
# S'arrete a la premiere etape qui echoue et dit laquelle.

set -u  # variable non definie = erreur immediate (mais pas -e, on gere chaque etape a la main)

# --- Configuration : a adapter via variables d'environnement si besoin ---
: "${ANDROID_SDK_ROOT:=$HOME/AppData/Local/Android/Sdk}"
: "${BUILD_TOOLS_VERSION:=34.0.0}"
: "${NDK_VERSION:=28.2.13676358}"
: "${KEYSTORE_PATH:=$HOME/cles-signature/ani-4087/ani4087_v3.jks}"
: "${KEYSTORE_ALIAS:=ani4087key}"
: "${KEYSTORE_PASS_FILE:=$HOME/cles-signature/ani-4087/mot_de_passe.txt}"
: "${PACKAGE_NAME:=com.ani4087.exo16}"
: "${PROJECT_DIR:=$(pwd)}"
: "${APK_DIR:=$PROJECT_DIR/Build/Bin/Release-Android/Salle/android-build-arm64-v8a}"
: "${APK_NAME:=Salle-Release.apk}"

export PATH="$ANDROID_SDK_ROOT/build-tools/$BUILD_TOOLS_VERSION:$ANDROID_SDK_ROOT/platform-tools:$PATH"

echo_step() { echo ""; echo "=== $1 ==="; }
fail_step() {
    echo ""
    echo "ECHEC a l'etape : $1"
    echo "Raison : $2"
    exit 1
}

# --- 1. Construire ---
echo_step "1/4 Construire"
cd "$PROJECT_DIR" || fail_step "Construire" "dossier de projet introuvable : $PROJECT_DIR"
jenga build --platform Android-arm64 --config Release
if [ $? -ne 0 ]; then
    fail_step "Construire" "jenga build a retourne une erreur (voir sortie ci-dessus)"
fi

# --- 2. Empaqueter (injection lib native + asset, alignement) ---
echo_step "2/4 Empaqueter"
cd "$APK_DIR" || fail_step "Empaqueter" "dossier de sortie APK introuvable : $APK_DIR"

if [ ! -f "$APK_NAME" ]; then
    fail_step "Empaqueter" "APK non trouve apres le build : $APK_NAME"
fi

NDK_LIBCPP="$ANDROID_SDK_ROOT/ndk/$NDK_VERSION/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so"
if [ ! -f "$NDK_LIBCPP" ]; then
    fail_step "Empaqueter" "libc++_shared.so introuvable dans le NDK : $NDK_LIBCPP (verifiez NDK_VERSION)"
fi

mkdir -p lib/arm64-v8a
cp "$NDK_LIBCPP" lib/arm64-v8a/ || fail_step "Empaqueter" "echec de copie de libc++_shared.so"

if [ -f "$PROJECT_DIR/assets/donnees.txt" ]; then
    cp "$PROJECT_DIR/assets/donnees.txt" .
fi

python -c "
import zipfile, os, sys
src = '$APK_NAME'
dst = '${APK_NAME}.fixed'
try:
    with zipfile.ZipFile(src, 'r') as zin, zipfile.ZipFile(dst, 'w') as zout:
        for item in zin.infolist():
            if item.filename.startswith('META-INF/'):
                continue
            zout.writestr(item, zin.read(item.filename), compress_type=zipfile.ZIP_DEFLATED)
        zout.write('lib/arm64-v8a/libc++_shared.so', 'lib/arm64-v8a/libc++_shared.so', compress_type=zipfile.ZIP_STORED)
        if os.path.exists('donnees.txt'):
            zout.write('donnees.txt', 'assets/donnees.txt', compress_type=zipfile.ZIP_DEFLATED)
except Exception as e:
    print('Erreur Python: ' + str(e))
    sys.exit(1)
"
if [ $? -ne 0 ]; then
    fail_step "Empaqueter" "echec de l'injection Python dans l'APK"
fi
mv "${APK_NAME}.fixed" "$APK_NAME"

zipalign.exe -f -p 4 "$APK_NAME" "${APK_NAME}.aligned"
if [ $? -ne 0 ]; then
    fail_step "Empaqueter" "echec de zipalign (verifiez que build-tools $BUILD_TOOLS_VERSION est installe)"
fi
mv "${APK_NAME}.aligned" "$APK_NAME"

# --- 3. Signer ---
echo_step "3/4 Signer"
if [ ! -f "$KEYSTORE_PATH" ]; then
    fail_step "Signer" "keystore introuvable : $KEYSTORE_PATH"
fi
if [ ! -f "$KEYSTORE_PASS_FILE" ]; then
    fail_step "Signer" "fichier de mot de passe introuvable : $KEYSTORE_PASS_FILE"
fi

apksigner.bat sign --ks "$KEYSTORE_PATH" --ks-key-alias "$KEYSTORE_ALIAS" \
    --ks-pass "pass:$(cat "$KEYSTORE_PASS_FILE")" \
    --v1-signing-enabled true --v2-signing-enabled true --v3-signing-enabled true "$APK_NAME"
if [ $? -ne 0 ]; then
    fail_step "Signer" "apksigner sign a echoue"
fi

apksigner.bat verify "$APK_NAME"
if [ $? -ne 0 ]; then
    fail_step "Signer" "apksigner verify a echoue (signature invalide)"
fi

# --- 4. Installer ---
echo_step "4/4 Installer"
if ! command -v adb >/dev/null 2>&1; then
    fail_step "Installer" "adb introuvable dans le PATH"
fi

DEVICE_COUNT=$(adb devices | grep -w "device" | wc -l)
if [ "$DEVICE_COUNT" -eq 0 ]; then
    fail_step "Installer" "aucun appareil detecte par adb (verifiez la connexion et l'autorisation USB)"
fi

adb install -r "$APK_NAME"
if [ $? -ne 0 ]; then
    fail_step "Installer" "adb install a echoue"
fi

echo ""
echo "=== SUCCES : les 4 etapes se sont terminees sans erreur ==="

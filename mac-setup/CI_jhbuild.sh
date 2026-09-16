#!/usr/bin/env bash

set -e
set -o pipefail

### Step 1: install jhbuild

LOCKFILE="$(dirname "$0")"/jhbuild-version.lock
MODULEFILE="$(dirname "$0")"/xournalpp.modules
GTK_OSX_PATCHFILE="$(dirname "$0")"/gtk-osx.patch
GTK_MODULES="meta-gtk-osx-gtk3 gtksourceview3"

get_lockfile_entry() {
    local key="$1"
    sed -e "/$key/!d" -e 's/^[^=]*=//' "$LOCKFILE"
}

shallow_clone_into_commit() {
    local repo="$1"
    local lockfile_entry="$2"
    local dir="$3"
    local commit=$(get_lockfile_entry $lockfile_entry)

    echo "Cloning commit $commit of $repo into $dir"

    [ -d "$dir" ] && rm -rf "$dir"
    mkdir -p "$dir"

    (cd "$dir" && git init -b main)
    (cd "$dir" && git remote add origin "$repo")
    (cd "$dir" && git fetch --depth 1 origin "$commit")
    (cd "$dir" && git checkout FETCH_HEAD)
}

download_jhbuild_sources() {
    shallow_clone_into_commit "https://gitlab.gnome.org/GNOME/gtk-osx.git" "gtk-osx" ~/gtk-osx-custom

    JHBUILD_BRANCH=$(sed -e '/^JHBUILD_RELEASE_VERSION=/!d' -e 's/^[^=]*=//' -e 's/^\"\([^\"]*\)\"$/\1/' ~/gtk-osx-custom/gtk-osx-setup.sh)
    echo "Cloning jhbuild version $JHBUILD_BRANCH"
    git clone --depth 1 -b $JHBUILD_BRANCH https://gitlab.gnome.org/GNOME/jhbuild.git "$HOME/Source/jhbuild"

    shallow_clone_into_commit "https://github.com/xournalpp/xournalpp-pipeline-dependencies" "xournalpp-pipeline-dependencies" ~/xournalpp-pipeline-dependencies
}

echo "::group::Download jhbuild sources"
download_jhbuild_sources
echo "::endgroup::"

install_jhbuild() {
    rm -rf ~/gtk ~/.new_local ~/.config/jhbuildrc ~/.config/jhbuildrc-custom ~/Source ~/.cache/jhbuild

    # gtk-osx-setup.sh expects cargo at ~/.new_local/bin/cargo. Homebrew's
    # cargo-c formula provides cargo-c but not the cargo executable itself,
    # so explicitly bootstrap Rust and expose cargo at the expected path.
    if ! command -v cargo >/dev/null 2>&1; then
        echo "cargo is required by gtk-osx-setup.sh but was not found"
        exit 1
    fi
    mkdir -p ~/.new_local/bin
    ln -sf "$(command -v cargo)" ~/.new_local/bin/cargo

    bash ~/gtk-osx-custom/gtk-osx-setup.sh

    install -m644 ~/gtk-osx-custom/jhbuildrc-gtk-osx ~/.config/jhbuildrc
    install -m644 ~/gtk-osx-custom/jhbuildrc-gtk-osx-custom-example ~/.config/jhbuildrc-custom
}

configure_jhbuild_envvars() {
    if ! [ -f ~/.zshrc ] || ! grep '"$HOME"/.new_local/bin' ~/.zshrc; then
        echo 'export PATH="$HOME"/.new_local/bin:"$PATH"' >> ~/.zshrc
    fi

    export PATH="$HOME/.new_local/bin:$PATH"
}

setup_custom_modulesets() {
    sed -i '' -e "s/^setup_sdk()/setup_sdk(target=\"$(get_lockfile_entry deployment_target)\")/" ~/.config/jhbuildrc-custom

    cat <<EOF >> ~/.config/jhbuildrc-custom

### BEGIN xournalpp macOS CI
use_local_modulesets = True
moduleset = "gtk-osx.modules"
modulesets_dir = os.path.expanduser("~/gtk-osx-custom/modulesets-stable")

module_cmakeargs['freetype-no-harfbuzz'] = ' -DFT_DISABLE_BROTLI=TRUE '
module_cmakeargs['freetype'] = ' -DFT_DISABLE_BROTLI=TRUE '
module_makeargs['portaudio'] = ' -j1 '

repos['ftp.gnu.org'] = 'https://ftpmirror.gnu.org/gnu/'

### END
EOF

    echo "interact = False" >> ~/.config/jhbuildrc
    echo "exit_on_error = True" >> ~/.config/jhbuildrc
    echo "shallow_clone = True" >> ~/.config/jhbuildrc
    echo "use_local_modulesets = True" >> ~/.config/jhbuildrc
    echo "disable_Werror = False" >> ~/.config/jhbuildrc

    cp "$MODULEFILE" "$HOME/gtk-osx-custom/modulesets-stable/"
    export MODULEFILE="$(basename $MODULEFILE)"

    echo "verbose=off" >> ~/.wgetrc
}

echo "::group::Setup jhbuild"
install_jhbuild
configure_jhbuild_envvars
setup_custom_modulesets
echo "::endgroup::"

### Step 2: Download modules' sources
download() {
    jhbuild update $GTK_MODULES
    jhbuild -m "$MODULEFILE" update meta-xournalpp-deps
    jhbuild -m bootstrap.modules update meta-bootstrap
    echo "Downloaded all jhbuild modules' sources"
}
echo "::group::Download modules' sources"
download
echo "::endgroup::"

### Step 3: bootstrap
bootstrap_jhbuild() {
    jhbuild -m bootstrap.modules build --no-network meta-bootstrap
}
echo "::group::Bootstrap jhbuild"
bootstrap_jhbuild
echo "::endgroup::"

### Step 4: build gtk (~15 minutes on a Mac Mini M1 w/ 8 cores)
build_gtk() {
    jhbuild build --no-network $GTK_MODULES
    echo "Finished building gtk"
}
echo "::group::Build gtk"
build_gtk
echo "::endgroup::"

### Step 5: build xournalpp deps
build_xournalpp_deps() {
    jhbuild -m "$MODULEFILE" build --no-network meta-xournalpp-deps
}
echo "::group::Build deps"
build_xournalpp_deps
echo "::endgroup::"

### Step 6: build binary blob
build_binary_blob() {
    jhbuild run python3 ~/xournalpp-pipeline-dependencies/gtk/package-gtk-bin.py -o xournalpp-binary-blob.tar.gz
}
echo "::group::Build blob"
build_binary_blob
echo "::endgroup::"

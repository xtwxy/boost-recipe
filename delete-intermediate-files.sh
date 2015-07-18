# !/bin/sh

PRG=$(readlink -f "$0")
PRG_DIR=$(dirname "$PRG")

find . -name "*~" -exec rm -rf {} \;

list=$(find . -maxdepth 1 -type d)

for f in $list; do
    cd "$f"
    if [[ "$?" -eq "0" ]] && [[ -f "Makefile" ]] && [[ "$f" != "$(echo "$f" | sed -ne '/^\.\/\./p')" ]]; then
        echo "Cleaning '$(pwd)'..."
        make clean
        rm -f $f*
        rm -f src/*.o
        echo "done."
    fi
    cd "$PRG_DIR"
done


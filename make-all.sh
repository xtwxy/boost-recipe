# !/bin/sh

PRG=$(readlink -f "$0")
PRG_DIR=$(dirname "$PRG")

list=$(find . -maxdepth 1 -type d)

for f in $list; do
    cd "$f"
    if [[ "$?" -eq "0" ]] && [[ -f "Makefile" ]] && [[ "$f" != "$(echo "$f" | sed -ne '/^\.\/\./p')" ]]; then
        echo "Making '$(pwd)'..."
        make
        if [[ "$?" -ne "0" ]]; then
            exit -1;
        fi
        echo "done."
    fi
    cd "$PRG_DIR"
done


#!/bin/sh
# Linux server: activate an already uploaded, verified static release.
# No build tools, Node.js, Docker, sudo, network access or release deletion.
set -eu
[ "$#" -eq 2 ] || { echo "Usage: sh activate-release.sh /absolute/site-root release-id" >&2; exit 2; }
case "$1" in /*) ;; *) echo "Site root must be absolute" >&2; exit 2 ;; esac
case "$2" in ''|.|..|*[!A-Za-z0-9._-]*) echo "Invalid release ID" >&2; exit 2 ;; esac
command -v rsync >/dev/null 2>&1 || { echo "rsync is required on the deployment host" >&2; exit 2; }
root=$(CDPATH= cd -- "$1" && pwd -P)
[ "$root" != / ] || { echo "Refusing filesystem root" >&2; exit 2; }
[ -d "$root/releases" ] && [ ! -L "$root/releases" ] || { echo "Missing real releases directory" >&2; exit 2; }
release="$root/releases/$2"
[ -d "$release" ] && [ ! -L "$release" ] || { echo "Missing real release directory" >&2; exit 2; }
[ "$(CDPATH= cd -- "$release" && pwd -P)" = "$release" ] || { echo "Release escapes site root" >&2; exit 2; }
for file in index.html docs/index.html 404.html pagefind/pagefind-entry.json; do
    [ -f "$release/$file" ] || { echo "Incomplete release: $file" >&2; exit 2; }
done
[ -d "$release/_astro" ] || { echo "Missing hashed Astro assets" >&2; exit 2; }
for directory in "$root/shared" "$root/shared/_astro"; do
    [ ! -L "$directory" ] || { echo "Shared asset directory must not be a symlink" >&2; exit 2; }
done
if [ -e "$root/current" ] && [ ! -L "$root/current" ]; then
    echo "current must be a symlink, never a real directory" >&2; exit 2
fi
mkdir "$root/.deploy-lock" || { echo "Another activation holds the lock" >&2; exit 2; }
staged="$root/.current-$$"
cleanup() {
    code=$?
    if [ -L "$staged" ]; then unlink "$staged"; fi
    rmdir "$root/.deploy-lock"
    exit "$code"
}
trap cleanup 0
trap 'exit 1' 1 2 15
[ ! -e "$staged" ] && [ ! -L "$staged" ] || { echo "Staging link exists" >&2; exit 2; }
# Retain older hashed assets for clients that still hold the previous HTML.
mkdir -p "$root/shared/_astro"
rsync -a --checksum --delay-updates "$release/_astro/" "$root/shared/_astro/"
ln -s "releases/$2" "$staged"
# GNU mv -T atomically replaces the symlink rather than following its target.
mv -Tf -- "$staged" "$root/current"
printf 'Activated %s\n' "$2"

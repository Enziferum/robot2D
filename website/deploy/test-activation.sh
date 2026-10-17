#!/bin/sh
# Linux-only fixture test; never contacts a host or touches a real deployment.
set -eu
script=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)/activate-release.sh
fixture=$(mktemp -d /tmp/robot2d-deploy-test.XXXXXXXX)
cleanup() {
    case "$fixture" in /tmp/robot2d-deploy-test.*) rm -rf -- "$fixture" ;; *) exit 2 ;; esac
}
trap cleanup 0
mkdir -p "$fixture/releases"
for id in first second incomplete; do mkdir -p "$fixture/releases/$id/docs" "$fixture/releases/$id/pagefind" "$fixture/releases/$id/_astro"; done
for id in first second; do
    printf "%s" "$id" > "$fixture/releases/$id/_astro/$id.js"
    for file in index.html docs/index.html 404.html pagefind/pagefind-entry.json; do printf '%s' "$id" > "$fixture/releases/$id/$file"; done
done
sh "$script" "$fixture" first
[ "$(readlink "$fixture/current")" = releases/first ]
sh "$script" "$fixture" second
[ "$(cat "$fixture/current/index.html")" = second ]
[ "$(cat "$fixture/shared/_astro/first.js")" = first ]
[ "$(cat "$fixture/shared/_astro/second.js")" = second ]
sh "$script" "$fixture" first
[ "$(cat "$fixture/current/index.html")" = first ]
for invalid in ../outside incomplete; do
    if sh "$script" "$fixture" "$invalid"; then echo "Invalid activation accepted" >&2; exit 1; fi
    [ "$(readlink "$fixture/current")" = releases/first ]
done
mkdir "$fixture/.deploy-lock"
if sh "$script" "$fixture" second; then echo "Lock ignored" >&2; exit 1; fi
rmdir "$fixture/.deploy-lock"
unlink "$fixture/current"
mkdir "$fixture/current"
if sh "$script" "$fixture" second; then echo "Real current directory replaced" >&2; exit 1; fi
[ -d "$fixture/current" ]
[ ! -L "$fixture/current" ]
echo 'Activation, rollback, invalid paths, incomplete upload, locking and real-directory protection passed.'

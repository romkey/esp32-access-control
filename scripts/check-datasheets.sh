#!/bin/sh
# Fails if any datasheet in datasheets/ lacks its license file:
# datasheets/SCD4x.pdf needs datasheets/SCD4x-license.txt. Run by pre-commit.
cd "$(dirname "$0")/.." || exit 1
[ -d datasheets ] || exit 0
status=0
for sheet in datasheets/*; do
  [ -f "$sheet" ] || continue
  case "$sheet" in
    *-license.txt | datasheets/README.md) continue ;;
  esac
  license="${sheet%.*}-license.txt"
  if [ ! -f "$license" ]; then
    echo "$sheet has no $license (see datasheets/README.md)" >&2
    status=1
  fi
done
exit $status

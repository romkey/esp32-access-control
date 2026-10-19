#!/bin/sh
# Prepares a release: works out the next version from the latest vX.Y.Z tag,
# moves CHANGELOG.md's Unreleased entries under it, labels UPGRADING.md's
# "## Unreleased:" entries with it (if that file exists), and sets every
# project_version (and .claude-plugin/plugin.json, if present) to it.
# Prints the new version. Doesn't commit or tag.
#
#   scripts/bump-version.sh patch    # 1.4.2 -> 1.4.3 (default)
#   scripts/bump-version.sh minor    # 1.4.2 -> 1.5.0
#   scripts/bump-version.sh major    # 1.4.2 -> 2.0.0
#
# Run from the repo root. Used by the release workflow's "Run workflow"
# button and by the /esphome-rules:release skill.
set -eu
bump=${1:-patch}
changelog=${CHANGELOG:-CHANGELOG.md}
here=$(dirname "$0")

latest=$(git tag --list 'v*' --sort=-v:refname \
  | grep -E '^v[0-9]+\.[0-9]+\.[0-9]+$' | head -n 1 || true)
latest=${latest:-v0.0.0}
IFS=. read -r major minor patch <<EOF
${latest#v}
EOF
case "$bump" in
  major) major=$((major + 1)); minor=0; patch=0 ;;
  minor) minor=$((minor + 1)); patch=0 ;;
  patch) patch=$((patch + 1)) ;;
  *) echo "bump must be patch, minor or major, not '$bump'" >&2; exit 1 ;;
esac
version="$major.$minor.$patch"

if git rev-parse -q --verify "refs/tags/v$version" >/dev/null; then
  echo "tag v$version already exists" >&2
  exit 1
fi

# Refuse to release nothing; changelog-section.sh fails on an empty section.
CHANGELOG=$changelog "$here/changelog-section.sh" Unreleased >/dev/null

# Keep an empty Unreleased section above the new version's.
today=$(date -u +%Y-%m-%d)
perl -0pi -e "s/^## \[Unreleased\]\n/## [Unreleased]\n\n## [$version] - $today\n/m" "$changelog"

for dir in examples firmware packages; do
  [ -d "$dir" ] || continue
  grep -rlE --include='*.yaml' --include='*.yml' 'project_version:' "$dir" \
    | while read -r file; do
        perl -pi -e "s/(project_version:\s*)\"[^\"]*\"/\${1}\"$version\"/" "$file"
      done
done

if [ -f UPGRADING.md ]; then
  perl -pi -e "s/^## Unreleased: /## v$version: /" UPGRADING.md
fi

if [ -f .claude-plugin/plugin.json ]; then
  jq --arg v "$version" '.version = $v' .claude-plugin/plugin.json > .claude-plugin/plugin.json.new
  mv .claude-plugin/plugin.json.new .claude-plugin/plugin.json
fi

echo "$version"

// Public robot2D history starts with a full 0.8.0 version tag.
export function parsePublicVersion(tag) {
  const match =
    /^(?:v)?(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-([0-9A-Za-z]+(?:[.-][0-9A-Za-z]+)*))?(?:\+[0-9A-Za-z]+(?:[.-][0-9A-Za-z]+)*)?$/.exec(
      tag || '',
    );
  if (!match) return null;
  const parts = match.slice(1, 4).map(Number);
  if (!parts.every(Number.isSafeInteger)) return null;
  if (parts[0] === 0 && parts[1] < 8) return null;
  return { parts, prerelease: Boolean(match[4]) };
}
export function publicReleases(releases) {
  return releases
    .filter((release) => !release.draft && parsePublicVersion(release.tag))
    .map((release) => ({
      ...release,
      prerelease:
        Boolean(release.prerelease) ||
        parsePublicVersion(release.tag).prerelease,
    }))
    .sort((a, b) => b.publishedAt.localeCompare(a.publishedAt));
}

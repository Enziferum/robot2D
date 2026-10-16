import { writeFile } from 'node:fs/promises';

// Explicit maintainer command: visitors and ordinary builds never call GitHub.
const endpoint =
  'https://api.github.com/repos/Enziferum/robot2D/releases?per_page=100';
const headers = {
  Accept: 'application/vnd.github+json',
  'User-Agent': 'robot2D-website',
  'X-GitHub-Api-Version': '2022-11-28',
};
if (process.env.GITHUB_TOKEN)
  headers.Authorization = `Bearer ${process.env.GITHUB_TOKEN}`;
const response = await fetch(endpoint, {
  headers,
  signal: AbortSignal.timeout(30000),
});
if (!response.ok)
  throw new Error(`GitHub release sync failed: HTTP ${response.status}`);
const input = await response.json();
if (!Array.isArray(input)) throw new Error('Expected a GitHub releases array');
if (input.length >= 100)
  throw new Error(
    'Release history requires pagination; refusing a truncated snapshot',
  );
const safeURL = (value) => {
  const url = new URL(value);
  if (
    url.origin !== 'https://github.com' ||
    !url.pathname.startsWith('/Enziferum/robot2D/releases/')
  )
    throw new Error('Unexpected release URL');
  return url.href;
};
const releases = input
  .filter((release) => !release.draft)
  .map((release) => {
    if (
      !release.tag_name ||
      !release.published_at ||
      !Number.isFinite(Date.parse(release.published_at))
    )
      throw new Error('Invalid published release');
    return {
      tag: release.tag_name,
      name: release.name || release.tag_name,
      publishedAt: release.published_at,
      prerelease: Boolean(release.prerelease),
      url: safeURL(release.html_url),
      assets: release.assets.map((asset) => ({
        name: asset.name,
        url: safeURL(asset.browser_download_url),
      })),
    };
  })
  .sort((a, b) => b.publishedAt.localeCompare(a.publishedAt));
if (!releases.length)
  throw new Error('No published releases; existing snapshot was preserved');
const snapshot = {
  fetchedAt: new Date().toISOString(),
  source: endpoint,
  releases,
};
await writeFile(
  new URL('../src/data/releases.json', import.meta.url),
  JSON.stringify(snapshot, null, 2) + '\n',
);
console.log(
  `Saved ${releases.length} published releases. Review and commit the snapshot.`,
);

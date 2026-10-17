import test from 'node:test';
import assert from 'node:assert/strict';
import {
  parsePublicVersion,
  publicReleases,
} from '../scripts/release-policy.mjs';

test('legacy releases never enter the public catalogue', () => {
  for (const tag of ['v0.7', 'v0.7.99', 'v0.3', 'v0.2', 'v0.0.8'])
    assert.equal(parsePublicVersion(tag), null, tag);
});
test('0.8.0 and later full versions are eligible, including double-digit versions', () => {
  for (const tag of ['v0.8.0', '0.8.1', 'v0.10.0', 'v1.0.0', 'v10.0.0'])
    assert.ok(parsePublicVersion(tag), tag);
});
test('a prerelease is labelled even if the upstream flag is missing', () => {
  const result = publicReleases([
    { tag: 'v0.8.0-rc.1', publishedAt: '2026-10-17T00:00:00Z' },
  ]);
  assert.equal(result[0].prerelease, true);
});
test('drafts, invalid tags and abbreviated versions are excluded', () => {
  assert.deepEqual(
    publicReleases([
      { tag: 'v0.8.0', draft: true },
      { tag: 'nightly' },
      { tag: 'v0.8' },
      { tag: 'v00.8.0' },
    ]),
    [],
  );
});
test('an empty public history is valid', () =>
  assert.deepEqual(publicReleases([]), []));
test('eligible releases retain real asset links and are sorted by publication date', () => {
  const assets = [
    {
      name: 'editor.zip',
      url: 'https://github.com/Enziferum/robot2D/releases/download/v0.8.0/editor.zip',
    },
  ];
  const result = publicReleases([
    { tag: 'v0.8.0', publishedAt: '2026-10-16', assets },
    { tag: 'v0.7.9', publishedAt: '2026-10-18' },
    { tag: 'v0.8.1', publishedAt: '2026-10-17', assets: [] },
  ]);
  assert.deepEqual(
    result.map((value) => value.tag),
    ['v0.8.1', 'v0.8.0'],
  );
  assert.deepEqual(result[1].assets, assets);
});

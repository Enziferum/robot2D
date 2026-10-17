import input from '../data/releases.json';
import { publicReleases } from '../../scripts/release-policy.mjs';
export interface ReleaseAsset {
  name: string;
  url: string;
}
export interface Release {
  tag: string;
  name: string;
  publishedAt: string;
  prerelease: boolean;
  url: string;
  assets: ReleaseAsset[];
}
export const snapshot: {
  fetchedAt: string;
  source: string;
  releases: Release[];
} = { ...input, releases: publicReleases(input.releases) };

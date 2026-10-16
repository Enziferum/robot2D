import { readFileSync } from 'node:fs';
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

const version = readFileSync(
  new URL('../CMakeLists.txt', import.meta.url),
  'utf8',
).match(/project\(robot2D\s+VERSION\s+([\d.]+)\)/)?.[1];
if (!version) throw new Error('Cannot read version from root CMakeLists.txt');
const base = process.env.SITE_BASE || '/';
export default defineConfig({
  ...(process.env.SITE_URL ? { site: process.env.SITE_URL } : {}),
  base,
  vite: {
    define: { 'import.meta.env.ROBOT2D_VERSION': JSON.stringify(version) },
  },
  trailingSlash: 'always',
  output: 'static',
  integrations: [
    starlight({
      title: 'robot2D',
      favicon: '/brand/robot2d-logo-02-32.png',
      logo: { src: './public/brand/robot2d-logo-02-512.png' },
      disable404Route: true,
      description: 'Build small worlds. Learn how they work.',
      social: [
        {
          icon: 'github',
          label: 'GitHub',
          href: 'https://github.com/Enziferum/robot2D',
        },
      ],
      customCss: ['./src/styles/docs.css'],
      components: { ThemeSelect: './src/components/DocsTheme.astro' },
      sidebar: [
        {
          label: 'Start here',
          items: [
            { label: 'Overview', slug: 'docs' },
            { label: 'Build from source', slug: 'docs/getting-started' },
            { label: 'Your first scene', slug: 'docs/first-scene' },
            { label: 'C# scripting', slug: 'docs/scripting' },
            { label: 'Project status', slug: 'docs/project-status' },
          ],
        },
      ],
    }),
  ],
});

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
        {
          label: 'Core / C++',
          items: [
            { label: 'Overview', slug: 'docs/core' },
            { label: '01 — Your first sprite', slug: 'docs/core/first-sprite' },
            {
              label: '02 — Keyboard movement',
              slug: 'docs/core/keyboard-movement',
            },
            {
              label: '03 — Coordinates and bounds',
              slug: 'docs/core/sprite-bounds',
            },
            {
              label: '04 — Atlas animation',
              slug: 'docs/core/sprite-animation',
            },
            { label: '05 — Camera and mouse', slug: 'docs/core/camera-view' },
            {
              label: '06 — Sprite collisions',
              slug: 'docs/core/sprite-collisions',
            },
            {
              label: 'Application and events',
              slug: 'docs/core/application-events',
            },
            {
              label: 'Graphics and resources',
              slug: 'docs/core/graphics-resources',
            },
            { label: 'ECS concepts', slug: 'docs/core/ecs' },
          ],
        },
        {
          label: 'Learn / Robot Lab',
          items: [
            { label: 'Learning path', slug: 'docs/learn' },
            { label: 'Reading a scene', slug: 'docs/learn/reading-a-scene' },
          ],
        },
      ],
    }),
  ],
});

import { readFile, readdir, stat } from 'node:fs/promises';
import { resolve, relative } from 'node:path';
import { fileURLToPath } from 'node:url';
import { parse } from 'parse5';

const dist = fileURLToPath(new URL('../dist/', import.meta.url));
const base =
  `/${(process.env.SITE_BASE || '/').replace(/^\/+|\/+$/g, '')}`.replace(
    /\/$/,
    '',
  ) + '/';
const origin = 'https://local.invalid';
const failures = [];
const documents = new Map();
const walkFiles = async (directory) => {
  const result = [];
  for (const entry of await readdir(directory, { withFileTypes: true })) {
    const full = resolve(directory, entry.name);
    if (entry.isDirectory()) result.push(...(await walkFiles(full)));
    else result.push(full);
  }
  return result;
};
const walk = (node, visit) => {
  visit(node);
  for (const child of node.childNodes || []) walk(child, visit);
};
const files = await walkFiles(dist);
for (const file of files.filter((file) => file.endsWith('.html'))) {
  const html = await readFile(file, 'utf8');
  if (/\bv0\.[0-7](?:\.\d+)?\b/.test(html))
    failures.push(`${file}: pre-0.8 release leaked into public HTML`);
  const document = parse(html);
  const ids = new Set();
  const links = [];
  let headings = 0;
  walk(document, (node) => {
    const attrs = Object.fromEntries(
      (node.attrs || []).map((attr) => [attr.name, attr.value]),
    );
    if (attrs.id) ids.add(attrs.id);
    if (node.tagName === 'h1') headings++;
    if (node.tagName === 'img' && attrs.alt === undefined)
      failures.push(`${file}: image without alt`);
    for (const key of ['href', 'src'])
      if (attrs[key] !== undefined) links.push(attrs[key]);
    if (attrs.srcset)
      links.push(
        ...attrs.srcset.split(',').map((value) => value.trim().split(/\s+/)[0]),
      );
  });
  if (headings !== 1)
    failures.push(`${file}: expected one h1, found ${headings}`);
  documents.set(file, { ids, links });
}
let checked = 0;
for (const [file, { links }] of documents) {
  const route = relative(dist, file)
    .replaceAll('\\', '/')
    .replace(/index\.html$/, '');
  const from = new URL(base + route, origin);
  for (const link of links) {
    if (!link || link === '#') {
      failures.push(`${route}: empty or placeholder link`);
      continue;
    }
    if (/^(mailto:|tel:|data:)/.test(link)) continue;
    const url = new URL(link, from);
    if (url.origin !== origin) {
      if (url.protocol !== 'https:')
        failures.push(`${route}: unexpected external protocol ${url.protocol}`);
      continue;
    }
    if (!url.pathname.startsWith(base)) {
      failures.push(`${route}: escapes configured base: ${link}`);
      continue;
    }
    const pathname = decodeURIComponent(url.pathname.slice(base.length));
    let target = resolve(dist, pathname);
    if (relative(dist, target).startsWith('..')) {
      failures.push(`${route}: path escapes dist`);
      continue;
    }
    try {
      if ((await stat(target)).isDirectory())
        target = resolve(target, 'index.html');
      await stat(target);
    } catch {
      failures.push(`${route}: missing ${link}`);
      continue;
    }
    if (
      url.hash &&
      documents.has(target) &&
      !documents.get(target).ids.has(decodeURIComponent(url.hash.slice(1)))
    )
      failures.push(`${route}: missing anchor ${link}`);
    checked++;
  }
}
for (const route of [
  '',
  'features/',
  'showcase/',
  'downloads/',
  'releases/',
  'roadmap/',
  'docs/',
  'docs/getting-started/',
  'docs/first-scene/',
  'docs/scripting/',
  'docs/project-status/',
  'docs/core/',
  'docs/core/application-events/',
  'docs/core/graphics-resources/',
  'docs/core/ecs/',
  'docs/learn/',
  'docs/learn/reading-a-scene/',
]) {
  if (!documents.has(resolve(dist, route, 'index.html')))
    failures.push(`Missing required route: ${route}`);
}
if (!files.some((file) => file.endsWith('pagefind.js')))
  failures.push('Missing Pagefind search bundle');
if (failures.length) {
  console.error(failures.join('\n'));
  process.exitCode = 1;
} else
  console.log(
    `Verified ${documents.size} HTML pages and ${checked} local links/resources (base ${base}), including anchors and the search bundle.`,
  );

# Contributing documentation

Public guides are written in English. Keep each article focused on one reader
question and an observable result. Publish working examples and state limitations.

- Core guides explain C++ contracts verified against the implementation. Pin source
  links to a commit when describing behavior that depends on a development version.
- Robot Lab lessons connect scenes and C# scripts to the engine systems they use.
  Use [the lesson template](content-templates/lesson.md) for setup, source code,
  expected results, diagnostics, an exercise and asset licensing.
- Pages live in `src/content/docs/docs/core/` and `src/content/docs/docs/learn/`.
  Register them in the Starlight sidebar. Relative links resolve from the published
  page URL, rather than the source file's directory.
- Import example sources with MDX `?raw` so the displayed code matches the compiled
  example. Native examples live in `public/examples/`; enable
  `RB2D_BUILD_CORE_EXAMPLES=ON` to build them in the repository checkout.
- Add regression checks for example behavior. OpenGL checks require
  `RB2D_CORE_EXAMPLE_GRAPHICS_TESTS=ON` and a working display/context. Compilation
  and headless tests do not replace visible rendering, keyboard or DPI acceptance.
- Before publishing, run `pnpm test`, `pnpm check`, `pnpm format:check`,
  `pnpm build` and `pnpm verify`. Check navigation, search and the lesson manually.

Use real engine screenshots for Showcase. Describe development builds accurately;
do not present planned features or incomplete examples as released functionality.
Deployment instructions are in [deploy/README.md](deploy/README.md) and
[deploy/github-pages.md](deploy/github-pages.md).

## Example integration

Lesson 01 imports source files with MDX `?raw`. Its sources and PNG are in
`public/examples/first-sprite/`, and its graphics check is in
`tests/core-first-sprite.cpp`. Native CI builds the examples on Windows and Linux;
the Linux graphics job runs them under Xvfb and treats skipped graphics checks
as errors.

Lesson 02 (`keyboard-movement`) reuses Lesson 01's application and PNG. Its five
headless scenarios exercise input state and `update(deltaTime)`. The Engine
baseline `4132acf` uses a fixed step; its timestep must not be described as render
delta. Code blocks use `ec.config.mjs` for the shared SlateTeal palette, headings
and copy button.

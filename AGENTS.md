# AGENTS

This repository contains the **Tiled** website, built with **Jekyll**.

## Project overview
- Static site built with Jekyll (4.x).
- Plugins: `jekyll-seo-tag`, `jekyll-redirect-from`.
- Deployed via GitHub Pages from the `gh-pages` branch of `mapeditor/tiled`. GitHub Pages still builds server-side with its own pinned Jekyll 3.10, so site content must stay compatible with both Jekyll 3.10 and 4.x.
- Repository root also includes generated site output in `_site/`.

## Key directories
- `_posts/` — News posts and release announcements (dated Markdown/HTML).
- `_layouts/` — Jekyll layouts (`default.html`, `post.html`).
- `_includes/` — Shared HTML snippets (header, footer, social icons, etc.).
- `_data/` — YAML data used by templates (features, showcases, sponsors, libraries).
- `css/`, `img/`, `vendor/` — Static assets.
- `img/sponsors` — Sponsor banners.
- `_source/` — Source assets and helper scripts (e.g. `serve.sh`).
- `_drafts/` — Unpublished drafts (served locally with `--drafts`).

## Important files
- `_config.yml` — Jekyll configuration (plugins, metadata).
- `Gemfile` / `Gemfile.lock` — Ruby dependencies (Jekyll plus a couple of plugins).
- `default.nix` — Nix shell that installs deps and runs the dev server.
- `index.html`, `download.html`, `docs.html`, etc. — Top-level pages.

## Local development

Manual:
- `bundle install`
- `bundle exec jekyll serve --drafts --livereload`

Nix:
- `nix-shell` (runs the same `bundle exec jekyll serve --drafts --livereload` via `default.nix`).

Site will be served locally (typically at `http://127.0.0.1:4000`).

## Content editing quick tips
- **News posts** go in `_posts/` with `YYYY-MM-DD-title.md`.
- **Static pages** live at repo root (e.g. `download.html`, `docs.html`, `donate.html`).
- **Reusable UI** is under `_includes/`.
- **Site chrome** is in `_layouts/`.

## Build output
- `_site/` is the generated output. It is derived content and should not be edited directly.

## Gotchas
- GitHub Pages server-side builds use Jekyll 3.10. Local dev uses Jekyll 4.x. Avoid Jekyll-4-only Liquid features in templates so both build the same output.
- Use `--drafts` if you want to preview `_drafts/` content locally.

## Adding a New Sponsor Banner

- If the sponsor is from OpenCollective, take the banner, url and name from their OpenCollective profile. The url can be found on their profile page with data-cy="social-link-0" attribute.
- The banner image needs to be edited to have a 16:9 aspect ratio with a resolution of no more than 320x180. You can do this using ImageMagick or a custom Python script.
- The banner image should be compressed using the compress-png.sh script available in the PATH (it modifies the image in-place).
- Place your sponsor banner in the `img/sponsors` directory.
- Update the `_data/sponsors.yml` file to include the new sponsor. The sponsors are listed in the following order:
    - Game development related companies
    - Non-social networking and non-gambling companies
    - Social networking related companies
    - Gambling related companies
- Commit both the sponsor banner image and the `_data/sponsors.yml` update together in a single commit with the message "Added sponsor banner". If inactive sponsors are also removed in the same commit, append that to the message, e.g. "Added sponsor banner, removed 1 inactive OpenCollective sponsor".

## Cleaning Up Sponsors

- Run the script in `_source/check-sponsors.py` to find no longer active OpenCollective sponsors, and `_source/check-patreon-sponsors.py` for Patreon (needs `PATREON_ACCESS_TOKEN`, see `.env`).
- Remove sponsors that are missing or who have been inactive for more than a month from `_data/sponsors.yml`. The OpenCollective script reports these as LAPSED, using a 45-day window (one monthly billing cycle plus slack).
- A sponsor may pay for several banner slots at once, which is why some names appear more than once. The script counts one slot per order that charged within the window, so a "count mismatch" means slots were added or dropped and entries need to be added or removed to match.
- Do not go by the OpenCollective order status: long-running orders are often marked CANCELLED while they keep billing monthly. Payments are the source of truth.
- Delete their banner images, provided they are not used by any still active sponsor.
- In the commit message, just note "Removed N inactive OpenCollective sponsor(s)".

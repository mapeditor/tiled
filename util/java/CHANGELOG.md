# Changelog

## [1.5.0] - 2026-09-06

### Added
- Support for modern TMX features: object templates, infinite maps with
  chunked layers, class properties, Wang sets, editor settings, tileset
  transformations, embedded and cropped tile images, group layers, parallax,
  tint colors, zstd-compressed and CSV-encoded layer data and SVG tilesets
  (by Taz Tatsuno, #4325)
- The writer now always stamps the current format version and supports CSV
  encoding, zstd compression and chunked infinite layers (by Taz Tatsuno, #4325)
- New oblique renderer, rewritten hexagonal renderer, flip flag rendering,
  group layer opacity and corrected rotation pivot (by Taz Tatsuno, #4325)
- Added `ROTATED_HEXAGONAL_120_FLAG` (#4315)
- The `zstd-jni` and `jsvg` dependencies can be excluded by consumers that
  use neither zstd compression nor SVG tilesets (by Taz Tatsuno, #4325)

### Changed
- Migrated from `javax.xml.bind` to `jakarta.xml.bind` for Java 21 support,
  which raises the minimum supported Java version to 11 (by Taz Tatsuno, #4326)
- Optimized image scaling in `MapObject.getImage` (by Taz Tatsuno, #4327)

### Fixed
- Fixed "Can't find ReflectionNavigator class" error when running tmxviewer
  (by Taz Tatsuno, #4328)
- XML escaping, XXE-hardened parsing, errors on truncated or invalid tile
  data, correct unsigned gid handling and locale-independent number
  formatting (by Taz Tatsuno, #4325)

## [1.4.3] - 2025-06-02

### Fixed
- Updated dependencies to fix a bug when running under Java 21.
- Fixed javadoc generation with recent Java versions
- Fixed intermittent test failures

---

Previous releases are not listed here. Future changes will be documented in this file.

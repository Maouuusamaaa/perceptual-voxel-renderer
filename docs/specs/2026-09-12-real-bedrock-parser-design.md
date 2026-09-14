# Real Minecraft Bedrock World Parser — Design

Date: 2026-09-12
Status: Proposed — awaiting user review

## Goal

Replace the current synthetic `.mcworld` terrain fixture path with a real Bedrock world ingestion backend while preserving the existing PVR architecture. The importer must consume a normal exported `.mcworld`, decode `level.dat` and the `db/` LevelDB database, extract renderable chunk/subchunk block data, and convert it into PVR's renderer-neutral normalized records and `WorldTruth`.

This is an interoperability milestone, not an attempt to modify or hook Minecraft's official renderer.

## Scope

First real-world compatibility target:

- `.mcworld` ZIP container.
- `level.dat` Bedrock little-endian NBT sufficient for world metadata/dimension information.
- Current LevelDB-backed Bedrock worlds.
- Bedrock chunk-key classification, especially `SubChunkPrefix` records (`0x2F`).
- Modern paletted subchunk decoding, beginning with the currently documented v8/v9 forms and retaining an isolated path for other encountered versions.
- Legacy LevelDB terrain records where practical, without letting legacy handling contaminate the modern decoder.
- Deterministic conversion to PVR normalized chunk/section/block-state records.
- Structured diagnostics for unsupported but non-critical records.
- Fail-closed behavior for malformed critical terrain records.

Out of scope for this milestone:

- Modifying Minecraft APKs or RenderDragon.
- Gameplay simulation, redstone, entities, commands, scripts, particles, animation, and exact vanilla lighting.
- Writing back to Minecraft worlds.
- Pre-LevelDB `chunks.dat` worlds.
- Full block-model fidelity. The PVR material mapping remains intentionally simplified at first.

## Recommended architecture

Use a hybrid backend boundary:

```text
.mcworld
  |
  +--> ZIP extraction / safe staging
  |        |
  |        +--> level.dat --> Bedrock NBT metadata decoder
  |        |
  |        +--> db/ -------> Bedrock storage backend
  |                              |
  |                              +--> raw key/value records
  |                              |
  |                              +--> chunk key classifier
  |                              |
  |                              +--> subchunk decoder
  |
  +--------------------------------------+
                                         |
                                         v
                              MinecraftWorldReader
                                         |
                                         v
                              normalized PVR-neutral data
                                         |
                                         v
                              MinecraftWorldAdapter
                                         |
                                         v
                                    WorldTruth
```

The renderer never sees LevelDB keys, NBT nodes, palette versions, or Minecraft-specific storage structures.

## Storage backend decision

Do not write a new LevelDB implementation inside PVR unless the selected external backend cannot be integrated safely.

The preferred backend is the current `bedrock-leveldb`/`bedrock-world` implementation from BE-Community-Dev, isolated behind a small PVR adapter. Current upstream documentation reports native LevelDB manifest/table/WAL reads, Bedrock key helpers, raw terrain records, and modern subchunk parsing in `bedrock-world`. The storage crate remains deliberately raw while the world crate owns chunk/subchunk semantics.

PVR must not expose upstream Rust types through its public C++ engine API. The boundary is an internal compatibility service that returns PVR-owned normalized structs.

If Rust integration proves too costly for the current build environment, the fallback is a narrowly scoped C++ port of only the required read-only algorithms, not a second general-purpose LevelDB implementation.

## Data flow

1. Receive `.mcworld` path.
2. Validate ZIP structure and extract into a private temporary directory.
3. Locate `level.dat` and `db/`.
4. Parse `level.dat` for format/version/dimension metadata.
5. Open `db/` read-only with `create_if_missing=false`.
6. Enumerate chunk positions using key-only scans where possible; do not materialize unrelated records.
7. For a requested chunk, fetch relevant `SubChunkPrefix` records using exact/prefix reads.
8. Decode subchunk version, palette, packed block indices, and section Y.
9. Convert palette entries to stable Minecraft source block-state identifiers.
10. Produce `MinecraftChunkRecord` / `MinecraftSectionRecord` / `MinecraftBlockStateRecord` values.
11. Pass those records to the existing material mapper and `WorldTruth` adapter.
12. Preserve diagnostics and source provenance for unsupported records.

The initial implementation should support viewport-oriented loading rather than requiring a full-world decode. This aligns with PVR's perceptual streaming architecture.

## Chunk and subchunk loading model

The reader exposes two levels:

- `index`: inexpensive discovery of renderable chunk positions.
- `load_chunk`: decode only the requested chunk's relevant terrain records.

The viewer/session layer requests chunks according to camera/perceptual priority. A full-world import remains available later as an offline operation but is not the first runtime path.

This prevents a large Bedrock world from becoming a mandatory all-at-once RAM allocation.

## Block-state normalization

A normalized block-state record contains at minimum:

- source dimension identifier
- chunk X/Z
- section Y
- local X/Y/Z
- source palette identity
- normalized source block-state identity/string where available
- opaque/air classification

The adapter then maps this to the existing PVR `MaterialID`.

Unknown Minecraft block states must not silently become a known material. They receive the explicit fallback material and a diagnostic counter/list.

## Dimension handling

The normalized data model must retain dimension identity. The first viewer supports one selected dimension per session, defaulting to the Overworld when unambiguous.

Nether/End records must not be merged into Overworld coordinates. If dimension discovery cannot be resolved safely, the import fails with a structured diagnostic rather than guessing.

## Error policy

Fatal:

- invalid `.mcworld` archive
- missing `level.dat`
- missing `db/` for a LevelDB world
- unreadable LevelDB metadata
- malformed critical chunk/subchunk payload needed for the requested render region

Recoverable:

- unsupported record family unrelated to terrain
- unknown block-state mapping
- unsupported optional metadata
- entity/block-entity records not required for terrain rendering

Every recoverable condition gets a stable diagnostic category and source location.

## Security and robustness

The importer is read-only.

ZIP extraction must reject absolute paths, `..` traversal, and excessive expansion relative to the input archive. Temporary extraction is isolated and cleaned after the session.

LevelDB must be opened read-only with creation/repair disabled.

No Minecraft world file is modified.

## Testing strategy

Use TDD at each decoder boundary.

1. Synthetic unit fixtures for ZIP, NBT, key classification, palette packing, and malformed payloads.
2. A small real exported Bedrock `.mcworld` fixture for end-to-end compatibility.
3. Deterministic snapshot-style normalized records from selected chunks.
4. Regression tests for negative chunk coordinates and multiple section Y values.
5. Tests for at least one modern palette format and one legacy-compatible format if present in the fixture/backend.
6. Large-world test remains opt-in to avoid bloating normal CI.
7. Existing PVR tests must continue to pass.

The real fixture must be treated as an interoperability contract: if its world contents change, the expected normalized snapshot must be deliberately regenerated rather than silently accepted.

## Performance strategy

Do not scan every LevelDB value merely to find renderable chunks. Prefer key-only/prefix discovery and exact reads for requested chunks.

Decode subchunks only when they enter the PVR streaming/representation pipeline.

Keep the Bedrock storage cache separate from PVR representation/GPU caches.

Expose import timings:

- ZIP staging time
- level.dat parse time
- chunk index time
- requested chunk read time
- subchunk decode time
- normalized block count
- diagnostic count

These metrics will later be compared against fixed-distance and perceptual rendering behavior.

## Acceptance criteria

The milestone is accepted only when all of the following are true:

- A real exported `.mcworld` is parsed without synthetic `pvr/terrain.bin` data.
- `level.dat` and LevelDB are read through the new backend.
- At least one real terrain chunk is decoded into normalized records.
- The same chunk produces deterministic normalized output across repeated reads.
- Negative coordinates and section Y are preserved correctly.
- Unknown materials produce explicit fallback diagnostics.
- The existing viewer can consume the resulting `WorldTruth` without Minecraft-specific renderer code.
- Existing PVR tests remain green.
- No claim is made that Minecraft's official RenderDragon renderer has been replaced.

## Source-format basis

The current public `bedrock-world` documentation describes Bedrock LevelDB opening, little-endian `level.dat`, chunk/subchunk parsing, and modern subchunk palette versions. `bedrock-leveldb` documents native LevelDB table/WAL reads and Bedrock chunk-key helpers. These are used as implementation references; PVR's normalized data model remains independent of those crates.

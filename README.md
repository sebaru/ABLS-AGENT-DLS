# abls-agent-dls

Runtime agent for the Abls-Habitat Distributed Logic System (DLS).

## Current implementation status

- Runtime based on ABLS-AGENT-LIBS
- Facility fixed to `dls`
- Prefix initialized from `agent_tech_id`
- DLS runtime modules for logic, inputs, outputs, timers, messages and archives
- Main lifecycle implemented with:
  - `Agent_init(...)`
  - `Agent_loop(...)`
  - `Agent_end(...)`

## Build

```sh
./install_deps.sh
./build.sh
```

## Packaging RPM

```sh
./build_rpm.sh
```

Produces the runtime RPM package in `build/`.

## Packaging DEB

```sh
./build_apt.sh --dist bookworm
./build_apt.sh --dist trixie
```

Default target suite is detected from host OS codename (`/etc/os-release`), with `bookworm` fallback.

Useful options:

- `--version-suffix <s>`: override Debian version suffix (example `~trixie`)
- `--no-dist-suffix`: disable automatic `~<suite>` suffix

Produces runtime DEB package and copies normalized artifacts to:

- `build/deb/<suite>/<arch>/`

`build_apt.sh` builds only the native host architecture.

Package signatures are centralized in ABLS-PKGS (both DEB repository metadata and RPM package/repository signatures).

## Release bump + publication

```sh
./bump.sh 1.2.3
```

The release flow:

- tags `v1.2.3` from `trunk`
- merges `trunk` into `main`
- builds RPM + DEB packages
- creates packages locally; publication is handled separately by ABLS-PKGS

## Container build

```sh
podman build -t abls-agent-dls:dev \
  --build-arg ABLS_LIBS_DEVEL_RPM_URL=<url> \
  --build-arg ABLS_AGENT_LIBS_DEVEL_RPM_URL=<url> \
  --build-arg ABLS_LIBS_RPM_URL=<url> \
  --build-arg ABLS_AGENT_LIBS_RPM_URL=<url> \
  -f Containerfile .
```

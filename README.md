# abls-agent-dls

DLS runtime agent for Abls-Habitat.

## Current implementation status

- Runtime skeleton based on ABLS-AGENT-LIBS
- State moved into `agent->vars` through `struct ABLS_DLS_VARS`
- Service is singleton: `abls-agent-dls.service`
- Prefix initialized from `agent_tech_id`
- Config bootstrap via `Json_read_config` using precedence:
  1. Environment variables (`ABLS_*`)
  2. `/etc/abls-agent.conf`
  3. Defaults in code

## Build

```sh
./install_deps.sh
./build.sh
```

## Packaging RPM

```sh
./build_rpm.sh
```

Produces runtime RPM package in `build/`.

## Packaging DEB

```sh
./build_apt.sh --dist bookworm --no-sign
./build_apt.sh --dist trixie --no-sign
```

Produces runtime DEB package and copies normalized artifacts to:

- `build/deb/<suite>/<arch>/`

## Release bump + publication

```sh
./bump.sh 1.2.3
```

The release flow:

- tags `v1.2.3` from `trunk`
- merges `trunk` into `main`
- builds RPM + DEB packages
- copies RPM to `../ABLS-PKGS/public/rpms/<arch>/`
- copies DEB to `../ABLS-PKGS/deb-packages/<suite>/`

---
title: 'ASAR Integrity'
description: 'An experimental feature that ensures the validity of ASAR contents at runtime.'
slug: asar-integrity
hide_title: false
---

ASAR integrity is an experimental feature that validates the contents of your app's
[ASAR archives](./asar-archives.md) at runtime.

## Version support

Currently, ASAR integrity checking is supported on:

* macOS as of `electron>=16.0.0`
* Windows as of `electron>=30.0.0`

In order to enable ASAR integrity checking, you also need to ensure that your `app.asar` file
was generated by a version of the `@electron/asar` npm package that supports ASAR integrity.

Support was introduced in `asar@3.1.0`. Note that this package has since migrated over to `@electron/asar`.
All versions of `@electron/asar` support ASAR integrity.

## How it works

Each ASAR archive contains a JSON string header. The header format includes an `integrity` object
that contain a hex encoded hash of the entire archive as well as an array of hex encoded hashes for each
block of `blockSize` bytes.

```json
{
  "algorithm": "SHA256",
  "hash": "...",
  "blockSize": 1024,
  "blocks": ["...", "..."]
}
```

Separately, you need to define a hex encoded hash of the entire ASAR header when packaging your Electron app.

When ASAR integrity is enabled, your Electron app will verify the header hash of the ASAR archive on runtime.
If no hash is present or if there is a mismatch in the hashes, the app will forcefully terminate.

## Enabling ASAR integrity in the binary

ASAR integrity checking is currently disabled by default in Electron and can
be enabled on build time by toggling the `EnableEmbeddedAsarIntegrityValidation`
[Electron fuse](fuses.md).

When enabling this fuse, you typically also want to enable the `onlyLoadAppFromAsar` fuse.
Otherwise, the validity checking can be bypassed via the Electron app code search path.

```js @ts-nocheck
const { flipFuses, FuseVersion, FuseV1Options } = require('@electron/fuses')

flipFuses(
  // E.g. /a/b/Foo.app
  pathToPackagedApp,
  {
    version: FuseVersion.V1,
    [FuseV1Options.EnableEmbeddedAsarIntegrityValidation]: true,
    [FuseV1Options.OnlyLoadAppFromAsar]: true
  }
)
```

:::tip Fuses in Electron Forge

With Electron Forge, you can configure your app's fuses with
[@electron-forge/plugin-fuses](https://www.electronforge.io/config/plugins/fuses)
in your Forge configuration file.

:::

## Providing the header hash

ASAR integrity validates the contents of the ASAR archive against the header hash that you provide
on package time. The process of providing this packaged hash is different for macOS and Windows.

### Using Electron tooling

Electron Forge and Electron Packager do this setup automatically for you with no additional
configuration. The minimum required versions for ASAR integrity are:

* `@electron/packager@18.3.1`
* `@electron/forge@7.4.0`

### Using other build systems

#### macOS

When packaging for macOS, you must populate a valid `ElectronAsarIntegrity` dictionary block
in your packaged app's `Info.plist`. An example is included below.

```xml title='Info.plist'
<key>ElectronAsarIntegrity</key>
<dict>
  <key>Resources/app.asar</key>
  <dict>
    <key>algorithm</key>
    <string>SHA256</string>
    <key>hash</key>
    <string>9d1f61ea03c4bb62b4416387a521101b81151da0cfbe18c9f8c8b818c5cebfac</string>
  </dict>
</dict>
```

Valid `algorithm` values are currently `SHA256` only. The `hash` is a hash of the ASAR header using the given algorithm.
The `@electron/asar` package exposes a `getRawHeader` method whose result can then be hashed to generate this value
(e.g. using the [`node:crypto`](https://nodejs.org/api/crypto.html) module).

### Windows

When packaging for Windows, you must populate a valid [resource](https://learn.microsoft.com/en-us/windows/win32/menurc/resources)
entry of type `Integrity` and name `ElectronAsar`. The value of this resource should be a JSON encoded dictionary
in the form included below:

```json
[
  {
    "file": "resources\\app.asar",
    "alg": "sha256",
    "value": "9d1f61ea03c4bb62b4416387a521101b81151da0cfbe18c9f8c8b818c5cebfac"
  }
]
```

:::info

For an implementation example, see [`src/resedit.ts`](https://github.com/electron/packager/blob/main/src/resedit.ts)
in the Electron Packager code.

:::
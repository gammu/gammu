# Windows SQL dependencies

The full installer bundles the unmodified x64 MariaDB Connector/C MSI's
dynamic library and plugins. `mariadb.ps1` verifies the MSI before performing
an administrative extraction (it does not install the connector globally).
The minimal installer skips this dependency.

## Updating Connector/C

Renovate proposes stable Git tag updates to `mariadb.json`. Windows binaries
can be published later than tags; do not merge until the win64 MSI exists.

1. With the proposed version in `mariadb.json`, run in PowerShell:
   `./.github/actions/windows-installer/mariadb.ps1 -ChecksumOnly`.
2. Review the download from MariaDB's official service and copy the printed
   SHA-256 into `mariadb.json`. The helper hashes the downloaded MSI; it does
   not treat a Git commit digest as a binary checksum.
3. Require both Windows database jobs and the installed-package tests to pass.
   Review new DLL dependencies with `dumpbin /dependents` if packaging fails.

The version and checksum are deliberately separate from the test server
versions. A checksum mismatch fails before MSI extraction. Updates are not
automerged. Notices and a link to the exact source tag ship with the installer.

The installed-package test hides the connector extraction directory and clears
the executable search path before injecting SMS messages. It tests uncached
MySQL authentication and accounts requiring SSL as well as ordinary accounts.
This does not assert certificate-verification behavior or add TLS settings.

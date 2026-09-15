param([switch]$ChecksumOnly)
$ErrorActionPreference = 'Stop'
$dependency = Get-Content "$PSScriptRoot/mariadb.json" -Raw | ConvertFrom-Json
$version = $dependency.version
if ($version -notmatch '^\d+\.\d+\.\d+$') { throw 'Invalid Connector/C version' }
$filename = "mariadb-connector-c-$version-win64.msi"
# The download service uses numeric IDs. Discover the exact versioned file
# from the official listing instead of pinning an ID independently of version.
$listing = Invoke-WebRequest "https://dlm.mariadb.com/browse/c_connector/$version/"
$pattern = 'https://dlm\.mariadb\.com/\d+/Connectors/c/connector-c-' + [regex]::Escape($version) + '/' + [regex]::Escape($filename) + '(?=")'
$urls = @([regex]::Matches($listing.Content, $pattern).Value | Select-Object -Unique)
if ($urls.Count -ne 1) { throw "Expected one official download for $filename" }
$archive = Join-Path ([IO.Path]::GetTempPath()) $filename
Invoke-WebRequest $urls[0] -OutFile $archive
$digest = (Get-FileHash $archive -Algorithm SHA256).Hash.ToLowerInvariant()
if ($ChecksumOnly) {
    # Review this official download before copying its digest into mariadb.json.
    Write-Output "$filename SHA256: $digest"
    exit 0
}
if ($digest -ne $dependency.sha256) { throw "Connector/C checksum mismatch: $digest" }
$destination = Join-Path $env:RUNNER_TEMP 'gammu-mariadb-connector'
$process = Start-Process msiexec.exe -Wait -PassThru -ArgumentList @('/a', "`"$archive`"", '/qn', "TARGETDIR=`"$destination`"")
if ($process.ExitCode -ne 0) { throw "Connector extraction failed: $($process.ExitCode)" }
$libraries = @(Get-ChildItem $destination -Recurse -Filter libmariadb.lib)
if ($libraries.Count -ne 1) { throw 'Expected one Connector/C import library' }
$root = $libraries[0].Directory.Parent.FullName
$notices = Join-Path $destination 'notices'
New-Item $notices -ItemType Directory -Force | Out-Null
Invoke-WebRequest "https://raw.githubusercontent.com/MariaDB/mariadb-connector-c/v$version/COPYING.LIB" -OutFile "$notices/COPYING.LIB"
@"
MariaDB Connector/C $version, dynamically linked, unmodified official Windows build.
Source: https://github.com/MariaDB/mariadb-connector-c/tree/v$version
Source archive: https://github.com/MariaDB/mariadb-connector-c/archive/refs/tags/v$version.tar.gz
Binary: $($urls[0])
SHA256: $digest
License: LGPL-2.1-or-later; see COPYING.LIB.
"@ | Set-Content "$notices/README.txt"
"MYSQL_ROOT=$($root.Replace('\', '/'))" >> $env:GITHUB_ENV
"MYSQL_LICENSE_DIR=$($notices.Replace('\', '/'))" >> $env:GITHUB_ENV
"MYSQL_EXTRACTION_DIR=$destination" >> $env:GITHUB_ENV

param([switch]$Minimal)
$ErrorActionPreference = 'Stop'
$install = Join-Path $env:RUNNER_TEMP 'gammu-installed'
$packages = @(Get-ChildItem _build -Filter 'Gammu-*.exe')
if ($packages.Count -ne 1) { throw 'Expected one NSIS installer' }
$process = Start-Process $packages[0].FullName -ArgumentList @('/S', "/D=$install") -Wait -PassThru
if ($process.ExitCode -ne 0) { throw 'Installation failed' }
$inject = Join-Path $install 'bin/gammu-smsd-inject.exe'
$savedPath = $env:PATH
$env:PATH = "$env:SystemRoot/system32;$env:SystemRoot"
Remove-Item Env:MARIADB_PLUGIN_DIR -ErrorAction SilentlyContinue
if ($Minimal) {
    if (Get-ChildItem $install -Recurse -Filter '*mariadb*.dll') { throw 'Connector bundled in minimal installer' }
    & $inject -v
    if ($LASTEXITCODE -ne 0) { throw 'Minimal executable failed' }
    exit 0
}
foreach ($dll in @('libmariadb.dll', 'caching_sha2_password.dll')) {
    if (-not (Test-Path "$install/bin/$dll" -PathType Leaf)) {
        throw "Required Connector/C DLL missing from installer: $dll"
    }
}
# Ensure no runtime dependency can be resolved from the extracted SDK.
Rename-Item $env:MYSQL_EXTRACTION_DIR "$($env:MYSQL_EXTRACTION_DIR)-hidden"
try {
    $config = Join-Path $install 'test.smsdrc'
    @"
[gammu]
model = dummy
connection = none
port = $install/dummy
[smsd]
service = sql
driver = native_mysql
host = 127.0.0.1
database = smsd
user = smsd
password = smsd
logfile = stderr
"@ | Set-Content $config
    New-Item "$install/dummy" -ItemType Directory | Out-Null
    foreach ($transport in @('NONE', 'SSL')) {
        # Reset the password to invalidate MySQL's authentication cache, so the
        # installed caching_sha2_password plugin must handle a full login.
        & $env:MYSQL_BIN --no-defaults --host=127.0.0.1 --user=root --execute="ALTER USER 'smsd'@'%' IDENTIFIED BY 'smsd' REQUIRE $transport; DELETE FROM smsd.outbox;"
        if ($LASTEXITCODE -ne 0) { throw 'Could not reset test account' }
        & $inject -c $config TEXT 123456 -text "Installed connector $transport"
        if ($LASTEXITCODE -ne 0) { throw "Installed injection failed ($transport), exit code $LASTEXITCODE" }
        $count = & $env:MYSQL_BIN --no-defaults --host=127.0.0.1 --user=root --batch --skip-column-names --execute='SELECT COUNT(*) FROM smsd.outbox'
        if ($LASTEXITCODE -ne 0 -or $count -ne '1') { throw 'Injected SMS missing from database' }
    }
} finally {
    $env:PATH = $savedPath
    Rename-Item "$($env:MYSQL_EXTRACTION_DIR)-hidden" $env:MYSQL_EXTRACTION_DIR
}

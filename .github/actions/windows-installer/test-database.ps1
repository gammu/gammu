param([ValidateSet('mysql', 'mariadb')][string]$Server)
$ErrorActionPreference = 'Stop'
$directory = Join-Path $env:RUNNER_TEMP 'gammu-database'
New-Item $directory -ItemType Directory -Force | Out-Null
if ($Server -eq 'mysql') {
    $version = '8.4.8'
    $url = "https://cdn.mysql.com/archives/mysql-8.4/mysql-$version-winx64.zip"
} else {
    $version = '11.4.8'
    $listing = Invoke-WebRequest "https://dlm.mariadb.com/browse/mariadb_server/$version/?flat=1"
    $pattern = 'https://dlm\.mariadb\.com/\d+/[^"<> ]*/mariadb-' + [regex]::Escape($version) + '-winx64\.zip(?=")'
    $urls = @([regex]::Matches($listing.Content, $pattern).Value | Select-Object -Unique)
    if ($urls.Count -ne 1) { throw 'Expected one MariaDB server ZIP' }
    $url = $urls[0]
}
Invoke-WebRequest $url -OutFile "$directory/server.zip"
Expand-Archive "$directory/server.zip" "$directory/server"
$serverExe = @(Get-ChildItem "$directory/server" -Recurse -Filter mysqld.exe)[0].FullName
if (-not $serverExe) { throw 'Database server executable missing' }
$bin = Split-Path $serverExe
$client = Join-Path $bin 'mysql.exe'
$data = "$directory/data"
if ($Server -eq 'mysql') {
    & $serverExe --no-defaults --initialize-insecure "--datadir=$data" --console
} else {
    & "$bin/mariadb-install-db.exe" "--datadir=$data"
}
if ($LASTEXITCODE -ne 0) { throw 'Database initialization failed' }
# Use the same certificates for both server families; the encrypted-transport
# check tests the packaged connector, not certificate-verification policy.
$openssl = 'C:/Program Files/Git/usr/bin/openssl.exe'
& $openssl req -x509 -newkey rsa:2048 -nodes -keyout "$directory/key.pem" -out "$directory/cert.pem" -days 1 -subj '/CN=localhost'
if ($LASTEXITCODE -ne 0) { throw 'Certificate generation failed' }
Get-Service -Name 'MySQL*', 'MariaDB*' -ErrorAction SilentlyContinue | Stop-Service -Force
# A console server can terminate when this Actions step closes its console.
# Run it through the service manager so later test steps use the same database.
$configuration = "$directory/my.ini"
$configDirectory = $directory.Replace('\', '/')
@"
[mysqld]
datadir="$configDirectory/data"
port=3306
skip-log-bin
bind-address=127.0.0.1
ssl-ca="$configDirectory/cert.pem"
ssl-cert="$configDirectory/cert.pem"
ssl-key="$configDirectory/key.pem"
log-error="$configDirectory/server-error.log"
"@ | Set-Content $configuration
& $serverExe --install-manual GammuSQLTest "--defaults-file=$configuration"
if ($LASTEXITCODE -ne 0) { throw 'Database service registration failed' }
try {
    Start-Service GammuSQLTest
} catch {
    Get-Content "$directory/server-error.log" -ErrorAction SilentlyContinue
    throw
}
$ready = $false
for ($attempt = 0; $attempt -lt 60; $attempt++) {
    & $client --no-defaults --host=127.0.0.1 --user=root --execute='SELECT 1' 2>$null
    if ($LASTEXITCODE -eq 0) { $ready = $true; break }
    if ((Get-Service GammuSQLTest).Status -ne 'Running') { break }
    Start-Sleep -Seconds 1
}
if (-not $ready) {
    Get-Content "$directory/server*.log"
    throw 'Database did not become ready'
}
& $client --no-defaults --host=127.0.0.1 --user=root --execute="CREATE DATABASE smsd; CREATE USER 'smsd'@'%' IDENTIFIED BY 'smsd'; GRANT ALL ON smsd.* TO 'smsd'@'%';"
if ($LASTEXITCODE -ne 0) { throw 'Database test setup failed' }
"MYSQL_BIN=$($client.Replace('\', '/'))" >> $env:GITHUB_ENV

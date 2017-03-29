# Wrapper for CTest to execute every test through OpenCPPCoverage.

# Unfortunately there doesn't seem to be way to hook into the process than
# pretending to be valgrind memory tester.

# Usage:

# cmake \
#   -DMEMORYCHECK_COMMAND=windows-coverage.bat \
#   -DMEMORYCHECK_COMMAND_OPTIONS=--separator \
#   -DMEMORYCHECK_TYPE=Valgrind
# ctest -D NightlyMemCheck

$rootDir = "c:\\projects\\gammu"
$coverageTemplate = "c:\\projects\\gammu\\cobertura{0}.xml"

$outArgs = @()
$afterSeparator = $false
$logFile = ""

# Parse params passed by CTest
for ( $i = 0; $i -lt $args.count; $i++ ) {
    if ($args[$i] -eq "--separator") { 
        $afterSeparator = $true
        continue
    }
    if ($afterSeparator) {
        $outArgs += $args[ $i ]
    }
    if ($args[$i].StartsWith("--log-file=")) {
        $logFile = $args[$i].Substring(11)
    }
}

if ($logFile -eq "") {
    throw "Missing --logfile parameter!"
}

# Create empty file (CTest expects to find it)
$logFileObj = New-Item $logFile -type file

# Figure out test number (it's included in log name)
$testNum = [io.path]::GetFileNameWithoutExtension($logFile).split('.')[1]

# Storage for coverage log
$coverageFile = [System.String]::Format($coverageTemplate, $testNum)

# Coverage command
$args = @(
    '--quiet',
    '--export_type', [System.String]::Format('cobertura:{0}', $coverageFile),
    '--modules', $rootDir,
    '--sources', $rootDir,
    '--'
) + $outArgs

# Execute with code coverage
Start-Process 'OpenCppCoverage.exe' -ArgumentList $args -Wait

# Propagate error code
if ($LASTEXITCODE -ne 1) {
    throw [System.String]::Format("Failed to run OpenCppCoverage, ExitCode: {0}.", $LASTEXITCODE)
}

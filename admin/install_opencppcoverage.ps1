#todo: convert this into a chocolatey package...

$downloadUrl = "http://download-codeplex.sec.s-msft.com/Download/Release?ProjectName=opencppcoverage&DownloadId=1594912&FileTime=131138681432830000&Build=21031"

$installerPath = [System.IO.Path]::Combine($Env:USERPROFILE, "Downloads", "OpenCppCoverageSetup.exe")
$installPath = [System.IO.Path]::Combine(${Env:ProgramFiles}, "OpenCppCoverage")
$openCppCoverageExe = [System.IO.Path]::Combine($installPath, "OpenCppCoverage.exe")

if(-Not (Test-Path $installerPath))
{
    Write-Host -ForegroundColor White ("Downloading OpenCppCoverage from: " + $downloadUrl)
    Start-FileDownload $downloadUrl -FileName $installerPath
}

Write-Host -ForegroundColor White "Installing OpenCppCoverage..."

$installProcess = (Start-Process $installerPath -ArgumentList '/VERYSILENT' -PassThru -Wait)
if($installProcess.ExitCode -ne 0)
{
    throw [System.String]::Format("Failed to install OpenCppCoverage, ExitCode: {0}.", $installProcess.ExitCode)
}

$env:Path="$env:Path;$installPath"

# Redirect from stderr
(& $openCppCoverageExe -h) 2>&1 | Select -First 1 | Write-Host -ForegroundColor White

# OpenCppCoverage returns 1 with no passed options...
if($LASTEXITCODE -ne 1)
{
    throw [System.String]::Format("Failed to check the OpenCppCoverage version, ExitCode: {0}.", $LASTEXITCODE)
}

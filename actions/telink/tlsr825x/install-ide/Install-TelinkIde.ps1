# Action to install the Telink Eclipse based IDE.
#
# TODO:
# - Hard-code more paths since we cannot guarentee anything if things move!
Param(
    [Parameter(Mandatory = $true)]
    [string] $TelinkIdePath,
    [Parameter(Mandatory = $true)]
    [string] $TelinkIdeUri,
    [Parameter(Mandatory = $true)]
    [string] $TelinkIdeZip,
    [Parameter(Mandatory = $true)]
    [string] $TelinkIdeHash
)

# Override the built-in cmdlet with a custom version that is not noisy!
function Write-Error {
    Write-Host -Foreground red @Args
}

# Abort the script if any command fails.
$PSNativeCommandUseErrorActionPreference = $true
$ErrorActionPreference = 'Stop'

# Always display information messages.
# Debug is enabled by setting the "common parameter" -Debug
$InformationPreference = 'Continue'

Write-Debug "TelinkIdePath: ${TelinkIdePath}"
Write-Debug "TelinkIdeUri: ${TelinkIdeUri}"
Write-Debug "TelinkIdeZip: ${TelinkIdeZip}"
Write-Debug "TelinkIdeHash: ${TelinkIdeHash}"

Write-Information "Creating target directory '${TelinkIdePath}..."
if (!(Test-Path ${TelinkIdePath} -PathType container)) {
    New-Item -Path ${TelinkIdePath} -ItemType "directory"
}

Write-Information "Downloading the IDE..."
if (!(Test-Path "${env:TEMP}" -PathType container)) {
    New-Item -Path "${env:TEMP}" -ItemType "directory"
}
Invoke-RestMethod -Method GET -FollowRelLink -Uri "${TelinkIdeUri}" -OutFile "${env:TEMP}\${TelinkIdeZip}"

Write-Information "Validating the downloaded IDE..."
$Hash=Get-FileHash -Path "${env:TEMP}\${TelinkIdeZip}" -Algorithm sha256
if ($Hash.hash.ToString() -ne "${TelinkIdeHash}")
{
    Write-Error "IDE hash has changed implying IDE update!"
    Write-Error "Expected sha256: ${TelinkIdeHash}"
    Write-Error "Actual sha256:   $(Out-String -InputObject $Hash.hash)"
    exit 1
}

Write-Information "Unzipping the IDE..."
Expand-Archive -Path "${env:TEMP}\${TelinkIdeZip}" -DestinationPath "${env:TEMP}"
Remove-Item -Path "${env:TEMP}\${TelinkIdeZip}"
if ($DebugPreference -eq 'Continue')
{
    Get-ChildItem -Path "${env:TEMP}" -Recurse -Name
}

# TODO: Want to hard-code the executable name.
# TODO: Use the & operator and splat the arguments (or set as list?).
Write-Information "Installing the IDE..."
$exes=Get-ChildItem -Path "${env:TEMP}\IDE\telink*.exe"
# Run the installer, waiting for it to complte, redirecting output etc.
$proc=Start-Process `
    -FilePath $exes[0].FullName `
    -NoNewWindow `
    -RedirectStandardOutput "${env:TEMP}\stdout.txt" `
    -RedirectStandardError "${env:TEMP}\stderr.txt" `
    -Wait `
    -PassThru `
    -ArgumentList `
      "/VERYSILENT", `
      "/SUPPRESSMSGBOXES", `
      "/DIR=""${TelinkIdePath}""", `
      "/NOICONS", `
      "/LOG=""${env:TEMP}\Install-TelinkIde.log"""

Write-Information "Installer exit code: $($proc.ExitCode.ToString())."
Write-Information "STDOUT from the installer..."
Get-Content -Path ${env:TEMP}\stdout.txt
Write-Information "STDERR from the installer..."
Get-Content -Path "${env:TEMP}\stderr.txt"
Remove-Item -Path "${env:TEMP}\stdout.txt"
Remove-Item -Path "${env:TEMP}\stderr.txt"

if ($DebugPreference -eq 'Continue')
{
    Write-Debug "Directories/files in '${TelinkIdePath}'..."
    Get-ChildItem -Path "${TelinkIdePath}" -Recurse -Name
}

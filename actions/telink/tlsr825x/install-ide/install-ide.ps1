# Script to install the Telink Eclipse based IDE.
Param(
  [Parameter(Require=true)]
  string $telink_ide_path="${env:GITHUB_WORKSPACE}\_ide"
)

# Make information messages appear.
$InformationPreference = 'Continue'
$DebugPreference = 'Continue'

Write-Information "IDE1: ${IDE_URL}"
Write-Information "IDE2: ${env:IDE_URL}"

Write-Information "Downloading the IDE to temp. directory..."
Invoke-RestMethod -Method GET -FollowRelLink -Uri "${env:IDE_URI}" -OutFile "${env:TEMP}\${env:IDE_ZIP}"

Write-Information "Validating the IDE using SHA256 hash..."
$Hash=Get-FileHash -Path "${env:TEMP}\${env:IDE_SIP}" -Algorithm sha256
if ($Hash.hash.ToString() -ne $env:IDE_HASH)
{
    Write-Warning "IDE hash has changed implying IDE update!"
    Write-Warning "Expected sha256: ${env:IDE_HASH}"
    Write-Warning "Actual sha256:   $(Out-String -InputObject $Hash.hash)"
}

Write-Information "Unzipping the IDE ZIP file..."
Expand-Archive -Path "${env:TEMP}\${env:IDE_ZIP}" -DestinationPath "${env:TEMP}"
Remove-Item -Path "${env:TEMP}\${env:IDE_ZIP}"
Get-ChildItem -Path "${telink_ide_path}" -Recurse -Name

Write-Information "Determine the IDE version..."
$exes=Get-ChildItem -Path ${telink_ide_path}\telink*.exe | Select FullName
if ($exes.count > 1) {
    Write-Error "Multiple executables were found that could be IDE installers" -ErrorAction:Continue
    Get-ChildItem -Path "${telink_ide_path}\telink*.exe" | Select FullName
    exit 1
}
$version="unknown"
if ($exes[0].FullName -match '(v\d+\.\d+\.\d+)') {
    $version=$Matches[0]
}
Write-Information -Message "Telink IDE version: ${version}"

# We do not use the "call operator" ( '&' ) here because it does not appear
# to work!
Write-Information "Installling the IDE..."
Get-ChildItem -Path "${env:TEMP}\IDE\telink*.exe"
$exes=Get-ChildItem -Path "${env:TEMP}\IDE\telink*.exe"
# Run the installer, waiting for it to complte, redirecting output etc.
$proc=Start-Process `
    -FilePath $exes[0].FullName `
    -ArgumentList `
    "/VERYSILENT", `
    "/SUPPRESSMSGBOXES", `
    "/DIR=""${telink_ide_path}""", `
    "/NOICONS", `
    "/LOG=""${logfile}""" `
    -NoNewWindow `
    -RedirectStandardOutput "${env:TEMP}\stdout.txt" `
    -RedirectStandardError "${env:TEMP}\stderr.txt" `
    -Wait `
    -PassThru

Write-Debug "Installer exit code: $($proc.ExitCode.ToString())."
Write-Debug "STDOUT from the installer..."
Get-Content -Path ${env:TEMP}\stdout.txt

Write-Debug "STDERR from the installer..."
Get-Content -Path ${env:TEMP}\stderr.txt

Remove-Item -Path ${env:TEMP}\stdout.txt
Remove-Item -Path ${env:TEMP}\stderr.txt

# List the top-level to really confirm that we were installed.
Write-Information "Directories/files in ${telink_ide_path}..." -InformationAction:Continue
Get-ChildItem -Path ${telink_ide_path} -Recurse -Name

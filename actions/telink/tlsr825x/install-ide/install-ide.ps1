# Script to install the Telink Eclipse based IDE.
Param(
  [Parameter(Require=true)]
  string $telink_ide_path="${env:GITHUB_WORKSPACE}\_ide"
)

# Make information messages appear.
InformationPreference = 'Continue'


inputs:
  telink_ide_path:
    description: 'Directory into which to install the IDE'
    default: $GITHUB_WORKSPACE\_ide

runs:
  using: "composite"
  steps:
    - name: Set-up environment variables.
      shell: pwsh
      env:
        # Download locations, filenames and SHA256 hashes.
        IDE_URI: "https://www.dropbox.com/s/ths9rev0tvhhl96/Telink_IDE.zip?dl=1"
        IDE_ZIP: Telink_IDE.zip
        IDE_HASH: "D47595AAFAE1B711A04DC83BAB08467160EA2A3559EA976576C2F8A44F47BF1F"
        IDE_EXE: TelinkSDKv1.3.1.exe
      run: |
        echo "_TEMP=${env:GITHUB_WORKSPACE}\_temp" >> $env:GITHUB_ENV
        echo "IDE_URI=${env:IDE_URI}" >> ${env:GITHUB_ENV}
        echo "IDE_ZIP=${env:IDE_ZIP}" >> ${env:GITHUB_ENV}
        echo "IDE_HASH=${env:IDE_HASH}" >> ${env:GITHUB_ENV}

Write-Information "Downloading the IDE to temp. directory..."
Invoke-RestMethod -Method GET -FollowRelLink -Uri "${env:IDE_URI}" -OutFile "${env:TEMP}\${env:IDE_ZIP}"

Write-Information "Validating the IDE using SHA256 hash..."
$Hash=Get-FileHash -Path "${env:_TEMP}\${env:IDE_SIP}" -Algorithm sha256
if ($Hash.hash.ToString() -ne $env:IDE_HASH)
{
    Write-Warning "IDE hash has changed implying IDE update!"
    Write-Warning "Expected sha256: ${env:IDE_HASH}"
    Write-Warning "Actual sha256:   $(Out-String -InputObject $Hash.hash)"
}

Write-Information "Unzipping the IDE ZIP file..."
Expand-Archive -Path "${env:_TEMP}\${env:IDE_ZIP}" -DestinationPath "${env:_TEMP}"
Remove-Item -Path "${env:_TEMP}\${env:IDE_ZIP}"
Get-ChildItem -Path "${{ inputs.telink_ide_path }}" -Recurse -Name

Write-Information "Determine the IDE version..."
$exes=Get-ChildItem -Path ${{ inputs.telink_ide_path }}\telink*.exe | Select FullName
if ($exes.count > 1) {
    Write-Error "Multiple executables were found that could be IDE installers" -ErrorAction:Continue
    Get-ChildItem -Path "${{ inputs.telink_ide_path }}\telink*.exe" | Select FullName
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
Get-ChildItem -Path "${env:_TEMP}\IDE\telink*.exe"
$exes=Get-ChildItem -Path "${env:_TEMP}\IDE\telink*.exe"
# Run the installer, waiting for it to complte, redirecting output etc.
$proc=Start-Process `
    -FilePath $exes[0].FullName `
    -ArgumentList `
    "/VERYSILENT", `
    "/SUPPRESSMSGBOXES", `
    "/DIR=""${{ inputs.telink_ide_path }}""", `
    "/NOICONS", `
    "/LOG=""${logfile}""" `
    -NoNewWindow `
    -RedirectStandardOutput "${env:_TEMP}\stdout.txt" `
    -RedirectStandardError "${env:_TEMP}\stderr.txt" `
    -Wait `
    -PassThru
Write-Information "Installer exit code: $($proc.ExitCode.ToString())." -InformationAction:Continue
Write-Information "STDOUT from the installer..." -InformationAction:Continue
Get-Content -Path ${env:_TEMP}\stdout.txt
Write-Information "STDERR from the installer..." -InformationAction:Continue
Get-Content -Path ${env:_TEMP}\stderr.txt
Remove-Item -Path ${env:_TEMP}\stdout.txt
Remove-Item -Path ${env:_TEMP}\stderr.txt

        # List the top-level to really confirm that we were installed.
        Write-Information "Directories/files in ${{ inputs.telink_ide_path }}..." -InformationAction:Continue
        Get-ChildItem =Path ${{ inputs.telink_ide_path }} -Recurse -Name

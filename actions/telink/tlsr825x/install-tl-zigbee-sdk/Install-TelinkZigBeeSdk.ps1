# Script to install the Telink Zigbee SDK.
#
# TODO: Add proper help, parameter descriptions etc.
# description: 'Install the Zigbee SDK required to build Telink tlsr825x projects'

Param(
    [Parameter(Mandatory = $true)]
    [string] $TelinkZigBeeSdkPath,
    [Parameter(Mandatory = $true)]
    [string] $SdkUri,
    [Parameter(Mandatory = $true)]
    [string] $SdkZip,
    [Parameter(Mandatory = $true)]
    [string] $SdkHash
)

# Override the built-in cmdlet with a custom version that is not noisy!
function Write-Error($message) {
    [Console]::ForegroundColor = 'red'
    [Console]::Error.WriteLine($message)
    [Console]::ResetColor()
}

# Abort the script if any command fails.
$PSNativeCommandUseErrorActionPreference = $true
$ErrorActionPreference = 'Stop'

# Always display information messages.
# Debug is enabled by setting the "common parameter" -Debug
$InformationPreference = 'Continue'

Write-Debug "TelinkSigBeeSdkPath: ${TelinkSigBeeSdkPath}"
Write-Debug "SdkUri: ${SdkUri}"
Write-Debug "SdkZip: ${SdkZip}"
Write-Debug "SdkHash: ${SdkHash}"
     
Write-Information "Creating target directory '${TelinkZigBeeSdkPath}'..."    
if (!(Test-Path "${TelinkZigBeeSdkPath}" -PathType container)) {
    New-Item -Path "${TelinkZigBeeSdkPath}" -ItemType "directory"
}

Write-Information "Downloading the Telink ZigBee SDK..."
if (!(Test-Path "${env:TEMP}" -PathType container)) {
    New-Item -Path "${env:TEMP}" -ItemType "directory"
}
Invoke-RestMethod -Method GET -FollowRelLink -Uri "${SdkUri}" -OutFile "${env:TEMP}\${SdkZip}"

Write-Information "Validating the downloaded SDK..."
$Hash=Get-FileHash -Path "${env:TEMP}\${SdkZip}" -Algorithm sha256
if ($Hash.hash.ToString() -ne $SdkHash)
{
    Write-Error "Zigbee SDK hash has changed implying Zigbee SDK update!"
    Write-Error "Expected sha256: ${SdkHash}"
    Write-Error "Actual sha256:   $(Out-String -InputObject $Hash.hash)"
    exit 1
}

Write-Information "Unziping the SDK..."
Expand-Archive -Path "${env:TEMP}\${SdkZip}" -DestinationPath "${TelinkZigBeeSdkPath}"
Remove-Item -Path "${env:TEMP}\${SdkZip}"

if ($DebugPreference == 'Continue')
{
    Get-ChildItem -Path "${TelinkZigBeeSdkPath}" -Recurse -Name
}

# Script to install the Telink Zigbee SDK.
#
# TODO: Add proper help, parameter descriptions etc.
# description: 'Install the Zigbee SDK required to build Telink tlsr825x projects'

Param(
    [Parameter(Mandatory = $true)]
    [string] $TelinkZigBeeSdkPath,
    [Parameter(Mandatory = $true)]
    [string] $TelinkZigBeeSdkUri,
    [Parameter(Mandatory = $true)]
    [string] $TelinkZigBeeSdkZip,
    [Parameter(Mandatory = $true)]
    [string] $TelinkZigBeeSdkHash
)

# Common initialization
. "${PSScriptRoot}\..\..\..\common\utils\Initialize-Script.ps1"

Write-Information "Creating target directory '${TelinkZigBeeSdkPath}'..."
if (!(Test-Path "${TelinkZigBeeSdkPath}" -PathType container))
{
    New-Item -Path "${TelinkZigBeeSdkPath}" -ItemType "directory"
}

Write-Information "Downloading the Telink ZigBee SDK..."
if (!(Test-Path "${env:TEMP}" -PathType container))
{
    New-Item -Path "${env:TEMP}" -ItemType "directory"
}
Invoke-RestMethod -Method GET -FollowRelLink -Uri "${TelinkZigBeeSdkUri}" -OutFile "${env:TEMP}\${TelinkZigBeeSdkZip}"

if (${TelinkZigBeeSdkHash}.length -eq 64))
{
    Write-Information "Validating the downloaded SDK..."
    $Hash=Get-FileHash -Path "${env:TEMP}\${TelinkZigBeeSdkZip}" -Algorithm sha256
    if ($Hash.hash.ToString() -ne "${TelinkZigBeeSdkHash}")
    {
        Write-Error "Zigbee SDK hash has changed implying Zigbee SDK update!"
        Write-Error "Expected sha256: ${TelinkZigBeeSdkHash}"
        Write-Error "Actual sha256:   $(Out-String -InputObject $Hash.hash)"
        exit 1
    }
}
else
{
    Write-Warning "Do not validate the downloaded SDK as hash not 64 hex bytes..."
}

Write-Information "Unzipping the SDK..."
Expand-Archive -Force -Path "${env:TEMP}\${TelinkZigBeeSdkZip}" -DestinationPath "${TelinkZigBeeSdkPath}"
Remove-Item -Path "${env:TEMP}\${TelinkZigBeeSdkZip}"

# Some ZIP files add an extra directory :-(.
if (Test-Path -PathType Container -Path "${TelinkZigBeeSdkPath}\*\tl_zigbee_sdk")
{
    Move-Item -Path "${TelinkZigBeeSdkPath}\*\*" -Destination ${TelinkZigBeeSdkPath}
}

if ($DebugPreference -eq 'Continue')
{
    Get-ChildItem -Path "${TelinkZigBeeSdkPath}" -Recurse -Name
}

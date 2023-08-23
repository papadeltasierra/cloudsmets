Param(
  [Parameter(mandatory = $true)]
  [string] $Tag
)

# Common initialization.
. "${PSScriptRoot}\..\utils\Initialize-Script.ps1"

# Override the built-in cmdlet with a custom version that is not noisy!
function Write-Error {
    Write-Host -Foreground red @Args
}

$InformationPreference = 'Continue'

Write-Debug "Tag: '${Tag}'..."

# Leading zeros are not permitted but a single zero is.
$valid = ${Tag} -match '^(?:0|[1-9]\d?)\.(?:0|[1-9]\d?)\.(?:0|[1-9]\d?)(\-dev\.(?:0|[1-9]\d?))?$'
if (! $valid)
{
    Write-Error `
      -Message "Tag '${Tag}' does not match should match SemVer format '1.2.3[-dev.45]'."
    exit 1
}
Write-Information "Tag '${Tag}' matches SemVer requirements."

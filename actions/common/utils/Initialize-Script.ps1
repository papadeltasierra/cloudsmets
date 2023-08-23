# Common functionality used by all Telink powershell scripts.

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

# If appropriate, display the parameter for the script.
$PSBoundParameters
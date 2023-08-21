Param(
    [Parameter(Mandatory = $true)]
    [string] $TelinkIdePath,
    [Parameter(Mandatory = $true)]
    [string] $TelinkZigbeeSdkPath,
    [Parameter(Mandatory = $true)]
    [string] $Project,
    [Parameter(Mandatory = $true)]
    [string] $Target
)

# Override the built-in cmdlet with a custom version that is not noisy!
function Write-Error($message) {
    [Console]::ForegroundColor = 'red'
    [Console]::Error.WriteLine($message)
    [Console]::ResetColor()
}

# Set debugging, or not.
$DebugPreference = 'Continue'
$InformationPreference = 'Continue'

Write-Debug "TelinkIdePath: ${TelinkIdePath}"
Write-Debug "TelinkZigBeeSdkPath: ${TelinkZigBeeSdkPath}"
Write-Debug "Target: ${Target}"

$Project = 'tlsr8258\build\tlsr_tc32'
$Extensions = @('bin', 'elf', 'lst')

# We have to patch the '.project' file to ensure that the linked resources are
# found and this requires a 'Linux format' directory name!
Write-Information "Patching '.project' file..."
$TelinkZigbeeSdkPathLinux=${TelinkZigbeeSdkPath} -replace '\\', '/'
Write-Debug "TelinkZigbeeSdkPathLinux: ${TelinkZigbeeSdkPathLinux}"

$DotProjectPath="${env:GITHUB_WORKSPACE}\${Project}\.project"
Write-Debug "ProjectPath: ${ProjectPath}"

# Patch the file...
$Content=$(Get-Content -Path ${DotProjectPath} -raw) -replace `
    '<value>\$%7BTELINK_ZIGBEE_SDK_WS%7D</value>', `
    "<value>file:/${TelinkZigbeeSdkPathLinux}</value>"

# Write the patched info back.
Set-Content -Path ${DotProjectPath} -Value ${Content}

if ($DebugPreference -eq 'Continue')
{
    Get-Content -Path ${DotProjectPath}
}

Write-Information "Running build process '${TelinkIdePath}\eclipsec.exe'..."
# Time out the build after 5 minutes in case something hangs.
$proc=Start-Process `
    -FilePath "${TelinkIdePath}\eclipsec.exe" `
    -RedirectStandardOutput "${env:TEMP}\stdout.txt" `
    -RedirectStandardError "${env:TEMP}\stderr.txt" `
    -PassThru `
    -NoNewWindow `
    -ArgumentList `
        "-vm", "${TelinkIdePath}\jre\bin\client", `
        "-noSplash", `
        "-application", "org.eclipse.cdt.managedbuilder.core.headlessbuild", `
        "-import", "${env:GITHUB_WORKSPACE}\${Project}", `
        "-cleanBuild", "tlsr_tc32/${Target}", `
        "--launcher.suppressErrors"

#if ($LASTEXITCODE -ne 0)
#{
#    Write-Error "eclipsec exited with error status code: ${LASTEXITCODE}."
#    exit 1
#}

# Time out of 5 minutes.
$timedout = $null
Wait-Process -InputObject $proc -Timeout 300 -ErrorAction SilentlyContinue -ErrorVariable timedout

if ($timedout)
{
    Write-Error "'eclipsec' timed-out before completing."
    $proc.Kill()
}

# Display output, especially any errors!
if ($DebugPreference -eq 'Continue')
{
    Write-Debug "STDOUT from the installer..."
    Get-Content -Path "${env:TEMP}\stdout.txt"

    Write-Debug "STDERR from the installer..."
    Get-Content -Path "${env:TEMP}\stderr.txt"
}

# Capture first line of errors to test later.
Get-Content -Path "${env:TEMP}\stderr.txt" -First 1
$stderr=Get-Content  -Path "${env:TEMP}\stderr.txt" -First 1

# Delete the output files.

Remove-Item -Path "${env:TEMP}\stdout.txt"
Remove-Item -Path "${env:TEMP}\stderr.txt"

# Builds worked it all of the following are true:
# - ExitCode is zero (0) (checked above)
# - stderr log output is empty
# - an 'elf' image has been created

# Appears that the compiler exits with status 1 for success!
# if ($env:eclipsec_rc -ne 0)
# {
#   Write-Error "eclipsec exited with $proc.ExitCode."
#   exit 1
# }
# elseif ($stderr -ne "")

# eclipsec throws warnings via STDERR so we have to remove them, then
# remove whitespace and only THEN can we see if $stderr is null or empty!
if ($null -eq $stderr)
{
    $stderr=$stderr -replace '(?m)^.*The option was ignored\.$', ''
    $stderr=$stderr -replace '\s', ''
}
if (($null -eq $stderr) -and ($stderr -ne ''))
{
    Write-Error "eclipsec reported errors."
    exit 1
}

# See if we have all the expected files.
Write-Information "Check expected firmware files are present..."
Get-ChildItem -Path . -Recurse -Name -Filter '*.elf'
$ImageRoot = "${Project}\${Target}"
Write-Debug "ImageRoot: $ImageRoot"

$missing = $False
foreach ($ext in $extensions)
{
    if (! $(Test-Path -Path ${image_root}\${Target}.$ext))
    {
        Write-Error "Expected output file '${ImageRoot}\${Target}.$ext' not found."
        $missing = $true
    }
}

if ($missing)
{
    Write-Error "Build has failed."
    exit 1
}

# Get here and we have succeeded.
Write-Information "eclipsec build succeeded!"

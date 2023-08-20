Param(
    [Parameter(Required = true)]
    string $TelinkIdePath
    [Parameter(Required=true)]
    string $TelinkZigBeeSDK
    [Parameter(Required=true)]
    string $Target
)

# Set debugging, or not.
$DebugPreference = Continue
$InformationPreference = Continue

runs:
  using: "composite"
  steps:
    - name: Set environment variables
      shell: pwsh
      env:
        # !!PDS: This may be badly named!
$Project = "tlsr8258\build\tlsr_tc32"
$Extensions = @('bin', 'elf', 'lst')

Write-Information "IDE: ${{ inputs.telink_ide_path }}" -InformationAction Continue

$image_base="${TelinkZigBeeSdk}\${Project}\${Target\\${Target}"

# Eclipsec is hard-coded.
        $eclipsec=Get-ChildItem -Path "${{ inputs.telink_ide_path }}\eclipsec.exe" -Recurse | Select FullName
        if ($eclipsec.count > 1)
        {
          Write-Error "Multiple instances of 'eclipsec' were found." -ErrorAction:Continue
          Get-ChildItem -Path "${{ inputs.telink_ide_path }}\eclipsec.exe" -Recurse | Select FullName
          exit 1
        }
        # Passing result to the next step.
        Write-Information "'$($eclipsec[0].FullName)' was found." -InformationAction Continue
        echo "eclipsec=$($eclipsec[0].FullName)" >> $env:GITHUB_ENV

# We have to patch the '.project' file to ensure that the linked resources are
# found and this requires a 'Linux format' directory name!
$TelinkZigBeeSdkLinux="${TelinkZigBeeSdk}" -replace '\\', '/'
Write-Debug "TelinkZigBeeSdkLinux: ${TelinkZigBeeSdkLinux"

$ProjectPath="${env:GITHUB_WORKSPACE}\tlsr8258\build\tlsr_tc32\.project"
Write-Debug "ProjecTPath: ${ProjectPath}"

# Patch the file...
$Content=$(Get-Content -Path ${ProjectPath} -raw) -replace `
    '<value>\$%7BTELINK_ZIGBEE_SDK_WS%7D</value>', `
    "<value>file:/${TelinkZigBeeSdkLinux}</value>"

# Write the patched info back.
Set-Content -Path ${PROJECT_PATH} -Value ${Content}

if $DebugPreference -eq Continue
{
    Get-Content -Path ${PROJECT_PATH}
}

    - name: Build TLSR8258 ZigBee image
      shell: pwsh
      run: |
Write-Information "Running build process '$env:eclipsec'..." -InformationAction Continue
# Testing time-out after 5 minutes.
$proc=Start-Process `
    -FilePath $env:eclipsec `
    -ArgumentList `
    "-vm", """${{ inputs.telink_ide_path }}\jre\bin\client""", `
    "-noSplash", `
    "-application", "org.eclipse.cdt.managedbuilder.core.headlessbuild", `
    "-import", """${env:GITHUB_WORKSPACE}\${env:PROJECT}""", `
    "-cleanBuild", """tlsr_tc32/${{ inputs.target }}""", `
    "--launcher.suppressErrors" `
    -NoNewWindow `
    -RedirectStandardOutput $env:TEMP\stdout.txt `
    -RedirectStandardError $env:TEMP\stderr.txt `
    -PassThru

# Time out of 5 minutes.
$timedout = $null
Wait-Process -InputObject $proc -Timeout 300 -ErrorAction SilentlyContinue -ErrorVariable timedout

if ($timedout)
{
    Write-Information "'eclipsec' timed-out before completing." -InformationAction Continue
    $proc.Kill()
}

# Display output, especially any errors!
Write-Debug "STDOUT from the installer..."
Get-Content -Path $env:TEMP\stdout.txt

Write-Debug "STDERR from the installer..."
Get-Content -Path $env:TEMP\stderr.txt

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
#   Write-Error "eclipsec exited with $proc.ExitCode." -ErrorAction:Continue
#   exit 1
# }
# elseif ($stderr -ne "")

# Eclipsec throws warnings via STDERR so we have to remove them, then
# remove whitespace and only THEN can we see if $stderr is null!
if ($stderr -ne $null)
{
    $stderr=$stderr -replace '(?m)^.*The option was ignored\.$', ''
    $stderr=$stderr -replace '\s', ''
}
if (($stderr -ne $null) -and ($stderr -ne ''))
{
    Write-Error "eclipsec reported errors."
    exit 1
}

# See if we have all the expected files.
$image_root = "${{ inputs.zigbee_sdk }}\${env:PROJECT}\${{ inputs.target }}"
$extensions = "${env:EXTENSIONS}".split(",")
$missing = $False

foreach ($ext in $extensions)
{
    if (! $(Test-Path -Path ${image_root}\${{ inputs.target }}.$ext))
    {
    Write-Error "Expected output file '${image_root}\${{ inputs.target }}.$ext' not found." -ErrorAction:Continue
    $missing=$True
    }
}

if ($missing)
{
    Write-Error "Build has failed." -ErrorAction:Continue
    exit 1
}

        # Get here and we have succeeded.
        Write-Information "eclipsec build succeeded." -InformationAction:Continue

    - uses: actions/upload-artifact@v3
      with:
        name: tlsr825x-firmware
        path: ${{ env._image_files }}
        retention-days: 1

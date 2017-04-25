param(
    [Parameter(Mandatory=$true)][string]$VMSwitchName,
    [Parameter(Mandatory=$true)][string]$TestsFolder,
    [Parameter()][string]$vtestPath = ".\vtest.exe",
    [Parameter()][string]$ExtensionName = "vRouter forwarding extension"
)

function RestartExtension {
    Disable-VMSwitchExtension -Name $ExtensionName -VMSwitchName $VMSwitchName -ErrorVariable err | out-null
    If ($err) {
        Write-Host "Error while disabling the extension"
        exit 1
    }
    Enable-VMSwitchExtension -Name $ExtensionName -VMSwitchName $VMSwitchName -ErrorVariable err | out-null
    If ($err) {
        Write-Host "Error while enabling the extension"
        exit 2
    }
}

$nr = 0
Get-ChildItem -Path $TestsFolder -Filter *.xml -Recurse | % {
    RestartExtension
    Invoke-Expression "$($vtestPath) $($_.FullName)"
    If ($LASTEXITCODE -ne 0) {
        Write-Host "Error while running vtest"
        exit 3
    }
    $nr = $nr + 1
}
RestartExtension
Write-Host "vtest: all($($nr)) tests passed"

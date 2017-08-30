Param ([Parameter(Mandatory = $true)] [string] $VMSwitchName,
       [Parameter(Mandatory = $true)] [string] $TestsFolder,
       [Parameter(Mandatory = $false)] [string] $VtestPath = "vtest.exe",
       [Parameter(Mandatory = $false)] [string] $ExtensionName = "vRouter forwarding extension")

function RestartExtension {
    Disable-VMSwitchExtension -Name $ExtensionName -VMSwitchName $VMSwitchName -ErrorVariable Err | out-null
    If ($Err) {
        throw "Error while disabling the extension"
    }
    Enable-VMSwitchExtension -Name $ExtensionName -VMSwitchName $VMSwitchName -ErrorVariable Err | out-null
    If ($Err) {
        throw "Error while enabling the extension"
    }
}

$Tests = Get-ChildItem -Path $TestsFolder -Filter *.xml -Recurse
$Tests | % {
    RestartExtension
    Invoke-Expression "$($VtestPath) $($_.FullName)"
    If ($LASTEXITCODE -ne 0) {
        throw "Error while running vtest"
    }
}

RestartExtension
Write-Host "vtest: all($($($Tests | measure).Count)) tests passed"

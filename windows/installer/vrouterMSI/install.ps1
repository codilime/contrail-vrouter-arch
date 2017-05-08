param(
    [Parameter()][string]$targetDir = "C:\Program Files\Juniper Networks\vRouter"
)

Invoke-Expression "netcfg -l '$($targetDir)\vRouter.inf' -c s -i vRouter"

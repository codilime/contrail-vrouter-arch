param(
    [Parameter()][string]$targetDir = "C:\Program Files\Juniper Networks\vRouter"
)

Invoke-Expression "Import-Certificate -CertStoreLocation Cert:\LocalMachine\Root\ '$($targetDir)\vRouter.cer'"
Invoke-Expression "Import-Certificate -CertStoreLocation Cert:\LocalMachine\TrustedPublisher '$($targetDir)\vRouter.cer'"
Invoke-Expression "netcfg -l '$($targetDir)\vRouter.inf' -c s -i vRouter"

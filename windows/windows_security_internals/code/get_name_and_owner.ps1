function Get-NameAndOwner {
    [CmdLetBinding]
    param(
        [parameter(Mandatory, ValueFromPipeline)]
        $Entry,
        [parameter(Mandatory)]
        $Root
    )

    begin {
        $curr_owner = Get_NtSid -Owner
    }

    process {
        $sd = Get-NtSecurityDescriptor -Path $Entry.Name -Root $Root -TypeName $Entry.NtTypeName -ErrorAction SilentlyContinue
        if ($null -ne $sd -and $sd.Owner.Sid -ne $curr_owner) {
            [PSCustomObject] @{
                Name = $Entry.Name
                NtTypeName = $Entry.NtTypeName
                Owner = $sd.Owner.Sid.Name
                SecurityDescriptor = $sd
            }
        }
    }
}
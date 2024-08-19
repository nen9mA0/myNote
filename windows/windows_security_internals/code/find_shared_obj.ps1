$ss = Get-NtHandle -ObjectType Section -GroupByAddress | Where-Object ShareCount -eq 2
$mask = Get-NtAccessMask -SectionAccess MapWrite
$ss = $ss | Where-Object {Test-NtAccessMask $_.AccessIntersection $mask}
foreach($s in $ss) {
    $count = ($s.ProcessIds | Where-Object{Test-NtProcess -ProcessId $_ -Access DupHandle}).Count
    if ($count -eq 1) {
        $s.Handles | Select ProcessId, ProcessName, Handle
    }
}
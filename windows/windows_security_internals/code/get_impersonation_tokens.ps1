function Get-ImpersonationTokens {
    $hs = Get-NtHandle -ObjectType ImpersonationTokens
    foreach($h in $hs) {
        try {
            Use-NtObject($token = Copy-NtObject -Handle $h)
            {
                if(Test-NtTokenImpersonation -Token $token) {
                    Copy-NtObject -Object $token
                }
            }
        } catch {
            
        }
    }
}
# 第6章 READING AND ASSIGNING SECURITY DESCRIPTORS
#           Assigning Security Descriptors
#               Assigning a Security Descriptor During Resource Creation
#                   Setting Only the Parent Security Descriptor

# 创建父安全描述符
function New-ParentSD($AceFlags = 0, $Control = 0) {
    $owner = Get-NtSid -KnownSid BuiltinAdministrators                  # 这里使用Administrator作为安全描述符的Owner和Group
    $parent = New-NtSecurityDescriptor -Type Directory -Owner $owner -Group $owner
    Add-NtSecurityDescriptorAce $parent -Name "Everyone" -Access GenericAll
    Add-NtSecurityDescriptorAce $parent -Name "Users" -Access GenericAll -Flags $AceFlags
    Add-NtSecurityDescriptorControl $parent -Control $Control
    Edit-NtSecurityDescriptor $parent -MapGeneric
    return $parent
}

# 
function Test-NewSD(
    $AceFlags = 0,
    $Control = 0,
    $Creator = $null,
    [switch]$Container) {
        $parent = New-ParentSD -AceFlags $AceFlags -Control $Control    # 使用上面定义的函数创建父安全描述符
        Write-Output "-= Parent SID =-"
        Format-NtSecurityDescriptor $parent -Summary                    # 打印父安全描述符信息

        if ($Creator -ne $null) {                                       # 若指定了创建者安全描述符，则打印该描述符信息
            Write-Output "`r`n-= Creator SD =-"
            Format-NtSecurityDescriptor $Creator -Summary
        }

        $auto_inherit_flags = @()
        if (Test-NtSecurityDescriptor $parent -DaclAutoInherited) {     # 根据父安全描述符是否有DaclAutoInherit或SaclAutoInherit设置新描述符的Auto_Inherit
            $auto_inherit_flags += "DaclAutoInherit"
        }
        if (Test-NtSecurityDescriptor $parent -SaclAutoInherited) {
            $auto_inherit_flags += "SaclAutoInherit"
        }
        if ($auto_inherit_flags.Count -eq 0) {
            $auto_inherit_flags += "None"
        }

        $token = Get-NtToken -Effective -Pseudo                         # 使用刚创建的父安全描述符和传入的创建者安全描述符，创建新安全描述符并打印信息
        $sd = New-NtSecurityDescriptor -Token $token -Parent $parent -Creator $Creator -Type Mutant -Container:$Container -AutoInherit $auto_inherit_flags
        Write-Output "`r`n-= New SD =-"
        Format-NtSecurityDescriptor $sd -Summary
}
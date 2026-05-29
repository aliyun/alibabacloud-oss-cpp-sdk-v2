# Run from the project root directory as follows:
# powershell -ExecutionPolicy Bypass -File scripts\check_format.ps1 sdk tests

param(
    [Parameter(Mandatory=$true, Position=0, ValueFromRemainingArguments=$true)]
    [string[]]$Dirs
)

Write-Host "Running clang-format..."

$retCode = 0

foreach ($dir in $Dirs) {
    if (-not (Test-Path $dir -PathType Container)) {
        Write-Host "$dir is not a directory"
        continue
    }
    Get-ChildItem -Path $dir -Recurse -Include *.h,*.hpp,*.c,*.cpp -File | Where-Object {
        $_.FullName -notmatch 'sdk[\\/]src[\\/]thirdparty[\\/]' -and
        $_.FullName -notmatch 'tests[\\/]external[\\/]'
    } | ForEach-Object {
        & clang-format -i --dry-run --Werror --style=file $_.FullName
        if ($LASTEXITCODE -ne 0) { $retCode = 1 }
    }
    Write-Host "~~~ $dir directory checked ~~~"
}

if ($retCode -eq 0) {
    Write-Host "Everything up to standard" -ForegroundColor Green
} else {
    Write-Host "Not up to formatting standard" -ForegroundColor Red
    Write-Host "Try running run_format.ps1 to format all files."
}

exit $retCode

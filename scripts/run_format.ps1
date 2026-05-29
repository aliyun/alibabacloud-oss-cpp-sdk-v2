# Run from the project root directory as follows:
# powershell -ExecutionPolicy Bypass -File scripts\run_format.ps1 sdk tests

param(
    [Parameter(Mandatory=$true, Position=0, ValueFromRemainingArguments=$true)]
    [string[]]$Dirs
)

Write-Host "Running clang-format..."

foreach ($dir in $Dirs) {
    if (-not (Test-Path $dir -PathType Container)) {
        Write-Host "$dir is not a directory"
        continue
    }
    Get-ChildItem -Path $dir -Recurse -Include *.h,*.hpp,*.c,*.cpp -File | Where-Object {
        $_.FullName -notmatch 'sdk[\\/]src[\\/]thirdparty[\\/]' -and
        $_.FullName -notmatch 'tests[\\/]external[\\/]'
    } | ForEach-Object {
        Write-Host "format $($_.FullName)"
        & clang-format -i --style=file $_.FullName
    }
    Write-Host "~~~ $dir directory formatted ~~~"
}

Write-Host "Done. All files were formatted (if required)."

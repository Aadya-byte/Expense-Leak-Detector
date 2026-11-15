param(
    [switch]$NoRun
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $projectRoot
$stagingExecutable = Join-Path $projectRoot 'expense_leak_detector_gui.static.tmp.exe'
$distributionFolder = Join-Path $projectRoot 'dist\Expense Leak Detector Portable'
$distributionExecutable = Join-Path $distributionFolder 'expense_leak_detector_gui.exe'

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    throw 'g++ was not found. Install MinGW-w64 and add g++ to PATH.'
}
if (-not (Get-Command objdump -ErrorAction SilentlyContinue)) {
    throw 'objdump was not found. Install MinGW-w64 and add its bin folder to PATH.'
}

if (Test-Path -LiteralPath $stagingExecutable) {
    Remove-Item -LiteralPath $stagingExecutable
}

& g++ -std=c++11 -Wall -Wextra -pedantic -mwindows `
  -IPerson1 -IPerson2 -IPerson3 -IPerson4 -IPerson5 -IPerson6 `
  Person1\Expense.cpp Person1\ExpenseNode.cpp Person1\ExpenseManager.cpp `
  Person2\FileManager.cpp Person2\RecordParser.cpp Person2\ReportExporter.cpp `
  Person3\HashEntry.cpp Person3\HashTable.cpp Person3\LeakAnalyzer.cpp `
  Person4\Search.cpp Person4\Sort.cpp `
  Person5\SavingsAdvisor.cpp Person5\UndoAction.cpp Person5\UndoStack.cpp `
  Person6\GuiLogic.cpp Person6\GUI.cpp `
  -static -static-libgcc -static-libstdc++ `
  -lcomctl32 -lgdi32 -luser32 -o $stagingExecutable

if ($LASTEXITCODE -ne 0) {
    throw "GUI build failed with exit code $LASTEXITCODE."
}

$importOutput = & objdump -p $stagingExecutable
if ($LASTEXITCODE -ne 0) {
    throw 'Could not inspect the built executable imports.'
}
$imports = @($importOutput | ForEach-Object {
    if ($_ -match 'DLL Name:\s*(\S+)') { $Matches[1] }
})
if ($imports.Count -eq 0) {
    throw 'No executable DLL imports were found; refusing to package an unverified build.'
}
$compilerRuntimeImports = @($imports | Where-Object {
    $_ -match '^(libgcc_s|libstdc\+\+-6|libwinpthread-1)\.dll$'
})
if ($compilerRuntimeImports.Count -gt 0) {
    throw "Static build still imports compiler runtime DLLs: $($compilerRuntimeImports -join ', ')"
}
$unexpectedImports = @($imports | Where-Object {
    $_ -notmatch '^(COMCTL32|GDI32|KERNEL32|USER32)\.dll$' -and
    $_ -notmatch '^(api-ms-win-|ext-ms-win-).+\.dll$'
})
if ($unexpectedImports.Count -gt 0) {
    throw "Unexpected non-Windows DLL imports: $($unexpectedImports -join ', ')"
}

New-Item -ItemType Directory -Path $distributionFolder -Force | Out-Null
Copy-Item -LiteralPath $stagingExecutable -Destination $distributionExecutable -Force

$sourceExpenseFile = Join-Path $projectRoot 'expense.txt'
$distributionExpenseFile = Join-Path $distributionFolder 'expense.txt'
if ((Test-Path -LiteralPath $sourceExpenseFile) -and
    -not (Test-Path -LiteralPath $distributionExpenseFile)) {
    Copy-Item -LiteralPath $sourceExpenseFile -Destination $distributionExpenseFile
}

Remove-Item -LiteralPath $stagingExecutable
Write-Host "Built portable GUI: $distributionExecutable"
Write-Host 'Imported DLLs:'
$imports | ForEach-Object { Write-Host "  $_" }

if (-not $NoRun) {
    & $distributionExecutable
    if ($LASTEXITCODE -ne 0) {
        throw "GUI exited with code $LASTEXITCODE."
    }
}

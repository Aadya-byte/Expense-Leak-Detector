$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $projectRoot

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    throw 'g++ was not found. Install MinGW-w64 and add g++ to PATH.'
}

& g++ -std=c++11 -Wall -Wextra -pedantic -mwindows `
  -IPerson1 -IPerson2 -IPerson3 -IPerson4 -IPerson5 -IPerson6 `
  Person1\Expense.cpp Person1\ExpenseNode.cpp Person1\ExpenseManager.cpp `
  Person2\FileManager.cpp Person2\RecordParser.cpp Person2\ReportExporter.cpp `
  Person3\HashEntry.cpp Person3\HashTable.cpp Person3\LeakAnalyzer.cpp `
  Person4\Search.cpp Person4\Sort.cpp `
  Person5\SavingsAdvisor.cpp Person5\UndoAction.cpp Person5\UndoStack.cpp `
  Person6\GuiLogic.cpp Person6\GUI.cpp `
  -lcomctl32 -lgdi32 -luser32 -o expense_leak_detector_gui.exe

if ($LASTEXITCODE -ne 0) {
    throw "GUI build failed with exit code $LASTEXITCODE."
}

Write-Host 'Built expense_leak_detector_gui.exe'
& .\expense_leak_detector_gui.exe
if ($LASTEXITCODE -ne 0) {
    throw "GUI exited with code $LASTEXITCODE."
}

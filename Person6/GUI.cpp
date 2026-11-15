// ============================================================================
// Expense Leak Detector - native Windows GUI (Person 6)
//
// A graphical replacement for Person6/main.cpp + UI.cpp. It uses exactly the
// same shared modules (ExpenseManager, FileManager, LeakAnalyzer, Search,
// Sort, SavingsAdvisor, UndoStack, ReportExporter) and the same expense.txt
// file, so data saved by the console build opens here and vice versa.
//
// Only the Win32 API + common controls are used: no extra libraries, no
// resource files required, and no STL containers or algorithms.
// Amounts are typed in rupees and stored as integer paise.
// ============================================================================

// Project headers first: windows.h defines a DELETE macro that would break
// ActionType::DELETE if it were included before UndoAction.h.
#include "ExpenseManager.h"
#include "FileManager.h"
#include "LeakAnalyzer.h"
#include "Search.h"
#include "Sort.h"
#include "SavingsAdvisor.h"
#include "UndoStack.h"
#include "UndoAction.h"
#include "Expense.h"
#include "GuiLogic.h"

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <new>
#include <sstream>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>
#undef DELETE   // let ActionType::DELETE mean the enum value again

namespace {

// ----------------------------------------------------------------------------
// Look and feel
// ----------------------------------------------------------------------------
const COLORREF C_BG        = RGB(244, 246, 249);
const COLORREF C_HEADER    = RGB(15, 76, 92);
const COLORREF C_CARD      = RGB(255, 255, 255);
const COLORREF C_BORDER    = RGB(220, 225, 232);
const COLORREF C_TEXT      = RGB(31, 41, 55);
const COLORREF C_MUTED     = RGB(107, 114, 128);
const COLORREF C_ACCENT    = RGB(13, 148, 136);
const COLORREF C_ACCENT_DK = RGB(15, 118, 110);
const COLORREF C_WARN      = RGB(217, 119, 6);
const COLORREF C_DANGER    = RGB(185, 28, 28);
const COLORREF C_LEAK_ROW  = RGB(254, 240, 214);
const COLORREF C_BAND_ROW  = RGB(248, 250, 252);

const COLORREF kBarColors[8] = {
    RGB(13, 148, 136), RGB(99, 102, 241), RGB(245, 158, 11), RGB(244, 63, 94),
    RGB(14, 165, 233), RGB(34, 197, 94),  RGB(168, 85, 247), RGB(249, 115, 22)
};

const char* const kMonthNames[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

// Button looks (stored in the button's GWLP_USERDATA).
enum { BTN_SECONDARY = 0, BTN_PRIMARY = 1, BTN_DANGER = 2 };

// ----------------------------------------------------------------------------
// Control ids
// ----------------------------------------------------------------------------
enum {
    IDC_ADD = 101, IDC_EDIT, IDC_DELETE, IDC_UNDO,
    IDC_LEAK, IDC_FORECAST, IDC_EXPORT,
    IDC_FILTER = 120, IDC_FINDID, IDC_FINDGO, IDC_SORTKEY, IDC_SORTDIR,
    IDC_LIST = 130,
    IDC_LABEL = 140
};

enum {
    ID_D_OK = 1, ID_D_CANCEL = 2, ID_D_CALC = 3,
    ID_D_TYPE = 30, ID_D_FREQ = 31, ID_D_HINT = 90, ID_D_ERR = 91
};

// ----------------------------------------------------------------------------
// Global application state
// ----------------------------------------------------------------------------
HINSTANCE g_inst = NULL;
int g_dpi = 96;

ExpenseManager* g_manager = NULL;
FileManager* g_fileManager = NULL;
LeakAnalyzer* g_leak = NULL;
Search* g_search = NULL;
Sort* g_sort = NULL;
SavingsAdvisor* g_advisor = NULL;
UndoStack* g_undo = NULL;

HWND g_main = NULL;
HWND g_list = NULL;
HWND g_filterEdit = NULL;
HWND g_findEdit = NULL;
HWND g_sortCombo = NULL;
HWND g_dirButton = NULL;

HFONT g_fBody = NULL, g_fBold = NULL, g_fSmall = NULL, g_fTitle = NULL,
      g_fBig = NULL, g_fHead = NULL, g_fMono = NULL;
HBRUSH g_brBg = NULL, g_brWhite = NULL;

// Non-owning view of the expenses currently shown in the table.
Expense** g_view = NULL;
int g_viewCount = 0;

char g_filter[64] = "";
int g_sortKey = 0;         // 0 = ID, 1 = Date, 2 = Amount
bool g_sortDesc = false;

// Dashboard numbers
long long g_totalPaise = 0;
long long g_monthPaise = 0;
int g_recordCount = 0;
int g_patternCount = 0;
int g_curMonth = 1, g_curYear = 2026;
CategoryTotal g_cats[32];
int g_catCount = 0;

// Layout rectangles (client coordinates)
RECT g_cardRect[4];
RECT g_chartRect;
RECT g_statusRect;

char g_status[300] = "Ready.";
bool g_statusError = false;

int S(int v) { return MulDiv(v, g_dpi, 96); }

// ----------------------------------------------------------------------------
// Small helpers
// ----------------------------------------------------------------------------
HFONT makeFont(int points, int weight, const char* face) {
    return CreateFontA(-MulDiv(points, g_dpi, 72), 0, 0, 0, weight, FALSE, FALSE,
                       FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                       DEFAULT_PITCH | FF_DONTCARE, face);
}

void setStatus(bool isError, const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::vsnprintf(g_status, sizeof(g_status), format, args);
    va_end(args);
    g_statusError = isError;
    if (g_main) InvalidateRect(g_main, &g_statusRect, FALSE);
}

void currentDate() {
    const std::time_t now = std::time(NULL);
    const std::tm* local = std::localtime(&now);
    if (local) {
        g_curMonth = local->tm_mon + 1;
        g_curYear = local->tm_year + 1900;
    }
}

void plainRupees(char* out, int capacity, long long paise) {
    std::snprintf(out, static_cast<std::size_t>(capacity), "%lld.%02d",
                  paise / 100, static_cast<int>(paise % 100));
}

void todayText(char* out, int capacity) {
    const std::time_t now = std::time(NULL);
    const std::tm* local = std::localtime(&now);
    if (!local) { std::snprintf(out, static_cast<std::size_t>(capacity), "01-01-2026"); return; }
    std::snprintf(out, static_cast<std::size_t>(capacity), "%02d-%02d-%04d",
                  local->tm_mday, local->tm_mon + 1, local->tm_year + 1900);
}

HWND makeControl(HWND parent, const char* cls, const char* text, DWORD style,
                 int x, int y, int w, int h, int id, DWORD exStyle = 0,
                 HFONT font = NULL) {
    HWND c = CreateWindowExA(exStyle, cls, text, WS_CHILD | WS_VISIBLE | style,
                             S(x), S(y), S(w), S(h), parent,
                             reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                             g_inst, NULL);
    SendMessageA(c, WM_SETFONT, reinterpret_cast<WPARAM>(font ? font : g_fBody), TRUE);
    return c;
}

HWND makeButton(HWND parent, const char* text, int id, int look,
                int x, int y, int w, int h) {
    HWND b = makeControl(parent, "BUTTON", text, WS_TABSTOP | BS_OWNERDRAW,
                         x, y, w, h, id);
    SetWindowLongPtrA(b, GWLP_USERDATA, static_cast<LONG_PTR>(look));
    return b;
}

void drawOwnerButton(const DRAWITEMSTRUCT* d) {
    const int look = static_cast<int>(GetWindowLongPtrA(d->hwndItem, GWLP_USERDATA));
    const bool pressed = (d->itemState & ODS_SELECTED) != 0;
    const bool disabled = (d->itemState & ODS_DISABLED) != 0;

    COLORREF fill = C_CARD, border = C_BORDER, text = C_TEXT;
    if (look == BTN_PRIMARY) {
        fill = pressed ? C_ACCENT_DK : C_ACCENT;
        border = C_ACCENT_DK;
        text = RGB(255, 255, 255);
    } else if (look == BTN_DANGER) {
        fill = pressed ? RGB(254, 226, 226) : C_CARD;
        border = RGB(240, 180, 180);
        text = C_DANGER;
    } else if (pressed) {
        fill = RGB(232, 236, 241);
    }
    if (disabled) {
        fill = RGB(236, 238, 242);
        border = C_BORDER;
        text = RGB(160, 165, 175);
    }

    RECT r = d->rcItem;
    HBRUSH bg = CreateSolidBrush(C_BG);
    FillRect(d->hDC, &r, bg);
    DeleteObject(bg);

    HBRUSH br = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ oldBr = SelectObject(d->hDC, br);
    HGDIOBJ oldPen = SelectObject(d->hDC, pen);
    RoundRect(d->hDC, r.left, r.top, r.right, r.bottom, S(10), S(10));
    SelectObject(d->hDC, oldBr);
    SelectObject(d->hDC, oldPen);
    DeleteObject(br);
    DeleteObject(pen);

    char caption[64];
    GetWindowTextA(d->hwndItem, caption, sizeof(caption));
    HGDIOBJ oldFont = SelectObject(d->hDC, g_fBold);
    SetBkMode(d->hDC, TRANSPARENT);
    SetTextColor(d->hDC, text);
    DrawTextA(d->hDC, caption, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(d->hDC, oldFont);

    if (d->itemState & ODS_FOCUS) {
        RECT f = d->rcItem;
        InflateRect(&f, -S(4), -S(4));
        DrawFocusRect(d->hDC, &f);
    }
}

void drawLabel(HDC dc, const char* text, int x, int y, int w, int h,
               HFONT font, COLORREF color, UINT format) {
    RECT r = { x, y, x + w, y + h };
    HGDIOBJ old = SelectObject(dc, font);
    SetTextColor(dc, color);
    DrawTextA(dc, text, -1, &r, format | DT_NOPREFIX);
    SelectObject(dc, old);
}

void fillRound(HDC dc, const RECT& r, COLORREF fill, COLORREF border, int radius) {
    HBRUSH br = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ oldBr = SelectObject(dc, br);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    RoundRect(dc, r.left, r.top, r.right, r.bottom, radius, radius);
    SelectObject(dc, oldBr);
    SelectObject(dc, oldPen);
    DeleteObject(br);
    DeleteObject(pen);
}

void fillSolid(HDC dc, const RECT& r, COLORREF c) {
    HBRUSH br = CreateSolidBrush(c);
    FillRect(dc, &r, br);
    DeleteObject(br);
}

// ----------------------------------------------------------------------------
// Data: view building, statistics, persistence
// ----------------------------------------------------------------------------
int leakCountFor(const Expense* e) {
    if (!e || !g_leak) return 0;
    char key[160];
    std::snprintf(key, sizeof(key), "%s:%s", e->getCategory(), e->getMerchant());
    const int patterns = g_leak->getPatternCount();
    for (int i = 0; i < patterns; ++i) {
        const HashEntry* entry = g_leak->getPatternAt(i);
        if (entry && std::strcmp(entry->getKey(), key) == 0)
            return entry->getTransactionCount();
    }
    return 0;
}

bool rowMatchesFilter(const Expense* e) {
    if (g_filter[0] == '\0') return true;
    return GuiLogic::containsIgnoreCase(e->getMerchant(), g_filter) ||
           GuiLogic::containsIgnoreCase(e->getCategory(), g_filter) ||
           GuiLogic::containsIgnoreCase(e->getDate(), g_filter) ||
           GuiLogic::containsIgnoreCase(e->getExpenseType(), g_filter) ||
           GuiLogic::containsIgnoreCase(e->getFrequency(), g_filter);
}

// Rebuilds the table's pointer array: copy, sort with the Person 4 module,
// filter, then apply the ascending/descending choice.
void rebuildView() {
    delete[] g_view;
    g_view = NULL;
    g_viewCount = 0;

    const int count = g_manager->getCount();
    g_view = new (std::nothrow) Expense*[count > 0 ? count : 1];
    if (!g_view) return;
    int copied = g_manager->copyExpensePointers(g_view, count);
    if (copied < 0) copied = 0;

    if (g_sortKey == 1) g_sort->sortByDate(g_view, copied);
    else if (g_sortKey == 2) g_sort->sortByAmount(g_view, copied);
    else g_sort->sortById(g_view, copied);

    int kept = 0;
    for (int i = 0; i < copied; ++i) {
        if (rowMatchesFilter(g_view[i])) g_view[kept++] = g_view[i];
    }
    if (g_sortDesc) {
        for (int i = 0, j = kept - 1; i < j; ++i, --j) {
            Expense* tmp = g_view[i];
            g_view[i] = g_view[j];
            g_view[j] = tmp;
        }
    }
    g_viewCount = kept;
}

void recomputeStats() {
    g_leak->analyzeExpenses(*g_manager);
    g_patternCount = g_leak->getPatternCount();
    g_recordCount = g_manager->getCount();

    g_totalPaise = 0;
    const ExpenseNode* node = g_manager->getHead();
    while (node) {
        g_totalPaise += node->getExpense().getAmountPaise();
        node = node->getNext();
    }
    g_monthPaise = (g_curYear >= 1900 && g_curYear <= 2100)
        ? g_advisor->calculateMonthlyTotal(*g_manager, g_curMonth, g_curYear) : 0;
    g_catCount = GuiLogic::buildCategoryTotals(*g_manager, g_cats, 32);
}

void updateColumnHeaders() {
    static const char* const names[8] = {
        "ID", "Date", "Amount", "Category", "Merchant", "Type", "Frequency", "Leak signal"
    };
    for (int i = 0; i < 8; ++i) {
        char text[40];
        if (i <= 2 && i == g_sortKey)
            std::snprintf(text, sizeof(text), "%s %s", names[i], g_sortDesc ? "v" : "^");
        else
            std::snprintf(text, sizeof(text), "%s", names[i]);
        LVCOLUMNA col;
        std::memset(&col, 0, sizeof(col));
        col.mask = LVCF_TEXT;
        col.pszText = text;
        SendMessageA(g_list, LVM_SETCOLUMNA, static_cast<WPARAM>(i),
                     reinterpret_cast<LPARAM>(&col));
    }
    SetWindowTextA(g_dirButton, g_sortDesc ? "Descending" : "Ascending");
    SendMessageA(g_sortCombo, CB_SETCURSEL, static_cast<WPARAM>(g_sortKey), 0);
}

void refreshView() {
    rebuildView();
    SendMessageA(g_list, LVM_SETITEMCOUNT, static_cast<WPARAM>(g_viewCount), 0);
    InvalidateRect(g_list, NULL, FALSE);
    updateColumnHeaders();
}

void refreshAll() {
    recomputeStats();
    refreshView();
    if (g_main) InvalidateRect(g_main, NULL, FALSE);
}

int selectedExpenseId() {
    const int index = static_cast<int>(SendMessageA(g_list, LVM_GETNEXTITEM,
                                       static_cast<WPARAM>(-1), LVNI_SELECTED));
    if (index < 0 || index >= g_viewCount) return -1;
    return g_view[index]->getId();
}

bool selectById(int id) {
    for (int i = 0; i < g_viewCount; ++i) {
        if (g_view[i]->getId() != id) continue;
        LVITEMA clear;
        std::memset(&clear, 0, sizeof(clear));
        clear.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        clear.state = 0;
        SendMessageA(g_list, LVM_SETITEMSTATE, static_cast<WPARAM>(-1),
                     reinterpret_cast<LPARAM>(&clear));
        LVITEMA pick;
        std::memset(&pick, 0, sizeof(pick));
        pick.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        pick.state = LVIS_SELECTED | LVIS_FOCUSED;
        SendMessageA(g_list, LVM_SETITEMSTATE, static_cast<WPARAM>(i),
                     reinterpret_cast<LPARAM>(&pick));
        SendMessageA(g_list, LVM_ENSUREVISIBLE, static_cast<WPARAM>(i), FALSE);
        return true;
    }
    return false;
}

void saveData() {
    if (!g_fileManager->saveExpenses(*g_manager)) {
        char msg[700];
        std::snprintf(msg, sizeof(msg),
                      "The change was applied but could not be saved:\n\n%s",
                      g_fileManager->getLastError());
        MessageBoxA(g_main, msg, "Save problem", MB_OK | MB_ICONWARNING);
    }
}

// ----------------------------------------------------------------------------
// Modal dialog framework (own window class, own message loop)
// ----------------------------------------------------------------------------
struct DlgState {
    int kind;                 // 1 expense form, 2 forecast, 3 export, 4 text report
    bool done;
    bool ok;
    HWND hwnd;
    HWND edit[4];
    HWND combo[3];
    HWND err;
    HWND val[3];
    HWND savingsLabel;
    // expense form result
    char date[16];
    char merchant[64];
    char category[40];
    char type[20];
    char freq[20];
    long long paise;
    // month/year dialogs
    int month;
    int year;
    int percent;
};

const char* const kTypes[2] = { "ONE_TIME", "RECURRING" };
const char* const kFreqs[4] = { "NONE", "DAILY", "WEEKLY", "MONTHLY" };

void showDialogError(DlgState* st, const char* message, HWND focus) {
    SetWindowTextA(st->err, message);
    if (focus) {
        SetFocus(focus);
        SendMessageA(focus, EM_SETSEL, 0, static_cast<LPARAM>(-1));
    }
}

void updateFrequencyState(DlgState* st) {
    const int type = static_cast<int>(SendMessageA(st->combo[1], CB_GETCURSEL, 0, 0));
    if (type <= 0) {
        SendMessageA(st->combo[2], CB_SETCURSEL, 0, 0);
        EnableWindow(st->combo[2], FALSE);
    } else {
        EnableWindow(st->combo[2], TRUE);
        if (SendMessageA(st->combo[2], CB_GETCURSEL, 0, 0) <= 0)
            SendMessageA(st->combo[2], CB_SETCURSEL, 2, 0);   // WEEKLY
    }
}

bool readExpenseForm(DlgState* st) {
    char date[40], amount[80], merchant[160], category[160];
    GetWindowTextA(st->edit[0], date, sizeof(date));
    GetWindowTextA(st->edit[1], amount, sizeof(amount));
    GetWindowTextA(st->edit[2], merchant, sizeof(merchant));
    GetWindowTextA(st->combo[0], category, sizeof(category));
    GuiLogic::trim(date);
    GuiLogic::trim(amount);
    GuiLogic::trim(merchant);
    GuiLogic::trim(category);

    Expense probe;
    if (!probe.setDate(date)) {
        showDialogError(st, "Date must be a real date written as DD-MM-YYYY (for example 14-11-2026).", st->edit[0]);
        return false;
    }
    long long paise = 0;
    if (!GuiLogic::parseRupees(amount, paise)) {
        showDialogError(st, "Amount must be greater than 0 with at most 2 decimals (for example 125.50).", st->edit[1]);
        return false;
    }
    if (!probe.setMerchant(merchant)) {
        showDialogError(st, "Merchant is required (up to 50 characters, no | character).", st->edit[2]);
        return false;
    }
    if (!probe.setCategory(category)) {
        showDialogError(st, "Category is required (up to 30 characters, no | character).", st->combo[0]);
        return false;
    }
    int typeIndex = static_cast<int>(SendMessageA(st->combo[1], CB_GETCURSEL, 0, 0));
    int freqIndex = static_cast<int>(SendMessageA(st->combo[2], CB_GETCURSEL, 0, 0));
    if (typeIndex < 0 || typeIndex > 1) typeIndex = 0;
    if (typeIndex == 0) freqIndex = 0;
    if (freqIndex < 0 || freqIndex > 3) freqIndex = 0;
    if (typeIndex == 1 && freqIndex == 0) {
        showDialogError(st, "A recurring expense needs a frequency: DAILY, WEEKLY or MONTHLY.", st->combo[2]);
        return false;
    }

    std::snprintf(st->date, sizeof(st->date), "%s", date);
    std::snprintf(st->merchant, sizeof(st->merchant), "%s", merchant);
    std::snprintf(st->category, sizeof(st->category), "%s", category);
    std::snprintf(st->type, sizeof(st->type), "%s", kTypes[typeIndex]);
    std::snprintf(st->freq, sizeof(st->freq), "%s", kFreqs[freqIndex]);
    st->paise = paise;
    return true;
}

bool readMonthYear(DlgState* st, bool needPercent) {
    char buffer[32];
    char* end = NULL;

    GetWindowTextA(st->edit[0], buffer, sizeof(buffer));
    GuiLogic::trim(buffer);
    long month = std::strtol(buffer, &end, 10);
    if (buffer[0] == '\0' || *end != '\0' || month < 1 || month > 12) {
        showDialogError(st, "Month must be a number from 1 to 12.", st->edit[0]);
        return false;
    }
    GetWindowTextA(st->edit[1], buffer, sizeof(buffer));
    GuiLogic::trim(buffer);
    long year = std::strtol(buffer, &end, 10);
    const long minYear = needPercent ? 1900 : 1;
    const long maxYear = needPercent ? 2100 : 9999;
    if (buffer[0] == '\0' || *end != '\0' || year < minYear || year > maxYear) {
        char msg[80];
        std::snprintf(msg, sizeof(msg), "Year must be a number from %ld to %ld.", minYear, maxYear);
        showDialogError(st, msg, st->edit[1]);
        return false;
    }
    long percent = 0;
    if (needPercent) {
        GetWindowTextA(st->edit[2], buffer, sizeof(buffer));
        GuiLogic::trim(buffer);
        percent = std::strtol(buffer, &end, 10);
        if (buffer[0] == '\0' || *end != '\0' || percent < 0 || percent > 100) {
            showDialogError(st, "Reduction must be a whole number from 0 to 100.", st->edit[2]);
            return false;
        }
    }
    st->month = static_cast<int>(month);
    st->year = static_cast<int>(year);
    st->percent = static_cast<int>(percent);
    return true;
}

void calculateForecast(DlgState* st) {
    if (!readMonthYear(st, true)) return;
    SetWindowTextA(st->err, "");
    char text[64];
    GuiLogic::formatMoney(text, sizeof(text),
        g_advisor->calculateMonthlyTotal(*g_manager, st->month, st->year));
    SetWindowTextA(st->val[0], text);
    GuiLogic::formatMoney(text, sizeof(text),
        g_advisor->calculateMonthlyForecast(*g_manager, st->month, st->year));
    SetWindowTextA(st->val[1], text);
    GuiLogic::formatMoney(text, sizeof(text),
        g_advisor->calculatePotentialSavings(*g_manager, st->month, st->year, st->percent));
    SetWindowTextA(st->val[2], text);
    char label[64];
    std::snprintf(label, sizeof(label), "Potential savings at %d%% less", st->percent);
    SetWindowTextA(st->savingsLabel, label);
}

LRESULT CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CREATE) {
        const CREATESTRUCTA* cs = reinterpret_cast<const CREATESTRUCTA*>(lp);
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return 0;
    }
    DlgState* st = reinterpret_cast<DlgState*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    if (!st) return DefWindowProcA(hwnd, msg, wp, lp);

    switch (msg) {
    case WM_DRAWITEM: {
        const DRAWITEMSTRUCT* d = reinterpret_cast<const DRAWITEMSTRUCT*>(lp);
        if (d->CtlType == ODT_BUTTON) { drawOwnerButton(d); return TRUE; }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc = reinterpret_cast<HDC>(wp);
        HWND control = reinterpret_cast<HWND>(lp);
        if (st->kind == 4 && control == st->edit[0]) {
            SetTextColor(dc, C_TEXT);
            SetBkColor(dc, RGB(255, 255, 255));
            return reinterpret_cast<LRESULT>(g_brWhite);
        }
        const int id = GetDlgCtrlID(control);
        SetBkColor(dc, C_BG);
        SetBkMode(dc, TRANSPARENT);
        if (id == ID_D_ERR) SetTextColor(dc, C_DANGER);
        else if (id == ID_D_HINT) SetTextColor(dc, C_MUTED);
        else SetTextColor(dc, C_TEXT);
        return reinterpret_cast<LRESULT>(g_brBg);
    }
    case WM_COMMAND: {
        const int id = LOWORD(wp);
        const int code = HIWORD(wp);
        if (id == ID_D_TYPE && code == CBN_SELCHANGE) {
            updateFrequencyState(st);
            return 0;
        }
        if (id == ID_D_CANCEL) { st->ok = false; st->done = true; return 0; }
        if (id == ID_D_CALC && st->kind == 2) { calculateForecast(st); return 0; }
        if (id == ID_D_OK) {
            if (st->kind == 1) { if (readExpenseForm(st)) { st->ok = true; st->done = true; } }
            else if (st->kind == 3) { if (readMonthYear(st, false)) { st->ok = true; st->done = true; } }
            else { st->ok = true; st->done = true; }
            return 0;
        }
        break;
    }
    case WM_CLOSE:
        st->ok = false;
        st->done = true;
        return 0;
    default:
        break;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

HWND createDialogWindow(HWND owner, const char* title, int clientW, int clientH, DlgState* st) {
    RECT r = { 0, 0, S(clientW), S(clientH) };
    const DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;
    AdjustWindowRectEx(&r, style, FALSE, WS_EX_DLGMODALFRAME);
    const int w = r.right - r.left;
    const int h = r.bottom - r.top;
    RECT o;
    GetWindowRect(owner, &o);
    int x = o.left + ((o.right - o.left) - w) / 2;
    int y = o.top + ((o.bottom - o.top) - h) / 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    return CreateWindowExA(WS_EX_DLGMODALFRAME, "ELDDialog", title, style,
                           x, y, w, h, owner, NULL, g_inst, st);
}

bool comboIsDropped(HWND h) {
    char cls[32];
    HWND target = h;
    if (GetClassNameA(target, cls, sizeof(cls)) == 0 || lstrcmpiA(cls, "ComboBox") != 0) {
        target = GetParent(h);
        if (!target || GetClassNameA(target, cls, sizeof(cls)) == 0 ||
            lstrcmpiA(cls, "ComboBox") != 0)
            return false;
    }
    return SendMessageA(target, CB_GETDROPPEDSTATE, 0, 0) != 0;
}

void runModal(HWND owner, DlgState& st, HWND firstFocus) {
    EnableWindow(owner, FALSE);
    ShowWindow(st.hwnd, SW_SHOW);
    UpdateWindow(st.hwnd);
    if (firstFocus) SetFocus(firstFocus);

    bool quit = false;
    int quitCode = 0;
    MSG msg;
    while (!st.done) {
        const BOOL got = GetMessageA(&msg, NULL, 0, 0);
        if (got == 0) { quit = true; quitCode = static_cast<int>(msg.wParam); break; }
        if (got < 0) break;

        if (msg.message == WM_KEYDOWN &&
            (msg.hwnd == st.hwnd || IsChild(st.hwnd, msg.hwnd)) &&
            !comboIsDropped(msg.hwnd)) {
            if (msg.wParam == VK_ESCAPE) {
                SendMessageA(st.hwnd, WM_COMMAND, MAKEWPARAM(ID_D_CANCEL, BN_CLICKED), 0);
                continue;
            }
            if (msg.wParam == VK_RETURN) {
                int target = (st.kind == 2) ? ID_D_CALC : ID_D_OK;
                if (GetDlgCtrlID(msg.hwnd) == ID_D_CANCEL) target = ID_D_CANCEL;
                SendMessageA(st.hwnd, WM_COMMAND, MAKEWPARAM(target, BN_CLICKED), 0);
                continue;
            }
        }
        if (!IsDialogMessageA(st.hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    EnableWindow(owner, TRUE);
    DestroyWindow(st.hwnd);
    st.hwnd = NULL;
    SetActiveWindow(owner);
    if (quit) PostQuitMessage(quitCode);
}

void addCategoryChoices(HWND combo) {
    static const char* const defaults[7] = {
        "Food", "Shopping", "Transport", "Bills", "Entertainment", "Health", "Other"
    };
    CategoryTotal existing[32];
    const int n = GuiLogic::buildCategoryTotals(*g_manager, existing, 32);
    for (int i = 0; i < n; ++i)
        SendMessageA(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(existing[i].name));
    for (int d = 0; d < 7; ++d) {
        bool present = false;
        for (int i = 0; i < n; ++i) {
            if (GuiLogic::containsIgnoreCase(existing[i].name, defaults[d]) &&
                std::strlen(existing[i].name) == std::strlen(defaults[d])) { present = true; break; }
        }
        if (!present)
            SendMessageA(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(defaults[d]));
    }
}

// Add / edit form. `existing` is NULL for a new expense.
bool showExpenseForm(HWND owner, const Expense* existing, DlgState& st) {
    std::memset(&st, 0, sizeof(st));
    st.kind = 1;
    char title[64];
    if (existing) std::snprintf(title, sizeof(title), "Edit expense #%d", existing->getId());
    else std::snprintf(title, sizeof(title), "Add expense");

    st.hwnd = createDialogWindow(owner, title, 460, 440, &st);
    if (!st.hwnd) return false;
    HWND d = st.hwnd;

    makeControl(d, "STATIC", title, SS_LEFT, 24, 16, 412, 30, IDC_LABEL, 0, g_fHead);

    static const char* const labels[6] = {
        "Date (DD-MM-YYYY)", "Amount (Rs.)", "Merchant", "Category", "Type", "Frequency"
    };
    for (int i = 0; i < 6; ++i)
        makeControl(d, "STATIC", labels[i], SS_LEFT, 24, 70 + i * 46 + 6, 112, 22, IDC_LABEL + 1 + i);

    st.edit[0] = makeControl(d, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, 144, 70, 292, 28, 20);
    st.edit[1] = makeControl(d, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, 144, 116, 292, 28, 21);
    st.edit[2] = makeControl(d, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, 144, 162, 292, 28, 22);
    st.combo[0] = makeControl(d, "COMBOBOX", "", WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL, 144, 208, 292, 220, 23);
    st.combo[1] = makeControl(d, "COMBOBOX", "", WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, 144, 254, 292, 120, ID_D_TYPE);
    st.combo[2] = makeControl(d, "COMBOBOX", "", WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, 144, 300, 292, 140, ID_D_FREQ);
    SendMessageA(st.edit[0], EM_LIMITTEXT, 10, 0);
    SendMessageA(st.edit[2], EM_LIMITTEXT, 50, 0);
    SendMessageA(st.combo[0], CB_LIMITTEXT, 30, 0);

    addCategoryChoices(st.combo[0]);
    for (int i = 0; i < 2; ++i)
        SendMessageA(st.combo[1], CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(kTypes[i]));
    for (int i = 0; i < 4; ++i)
        SendMessageA(st.combo[2], CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(kFreqs[i]));

    if (existing) {
        char amount[40];
        plainRupees(amount, sizeof(amount), existing->getAmountPaise());
        SetWindowTextA(st.edit[0], existing->getDate());
        SetWindowTextA(st.edit[1], amount);
        SetWindowTextA(st.edit[2], existing->getMerchant());
        SetWindowTextA(st.combo[0], existing->getCategory());
        SendMessageA(st.combo[1], CB_SETCURSEL,
                     std::strcmp(existing->getExpenseType(), "RECURRING") == 0 ? 1 : 0, 0);
        int f = 0;
        for (int i = 0; i < 4; ++i)
            if (std::strcmp(existing->getFrequency(), kFreqs[i]) == 0) f = i;
        SendMessageA(st.combo[2], CB_SETCURSEL, static_cast<WPARAM>(f), 0);
    } else {
        char today[16];
        todayText(today, sizeof(today));
        SetWindowTextA(st.edit[0], today);
        SendMessageA(st.combo[1], CB_SETCURSEL, 0, 0);
        SendMessageA(st.combo[2], CB_SETCURSEL, 0, 0);
    }
    updateFrequencyState(&st);

    st.err = makeControl(d, "STATIC", "", SS_LEFT, 24, 348, 412, 36, ID_D_ERR);
    makeButton(d, existing ? "Save changes" : "Add expense", ID_D_OK, BTN_PRIMARY, 226, 392, 120, 36);
    makeButton(d, "Cancel", ID_D_CANCEL, BTN_SECONDARY, 356, 392, 80, 36);

    runModal(owner, st, st.edit[0]);
    return st.ok;
}

void showForecastDialog(HWND owner) {
    DlgState st;
    std::memset(&st, 0, sizeof(st));
    st.kind = 2;
    st.hwnd = createDialogWindow(owner, "Forecast and savings", 480, 460, &st);
    if (!st.hwnd) return;
    HWND d = st.hwnd;

    makeControl(d, "STATIC", "Forecast and savings", SS_LEFT, 24, 16, 432, 30, IDC_LABEL, 0, g_fHead);
    makeControl(d, "STATIC", "Month (1-12)", SS_LEFT, 24, 76, 140, 22, IDC_LABEL + 1);
    makeControl(d, "STATIC", "Year", SS_LEFT, 24, 120, 140, 22, IDC_LABEL + 2);
    makeControl(d, "STATIC", "Cut spending by (%)", SS_LEFT, 24, 164, 140, 22, IDC_LABEL + 3);
    st.edit[0] = makeControl(d, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, 180, 70, 120, 28, 20);
    st.edit[1] = makeControl(d, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, 180, 114, 120, 28, 21);
    st.edit[2] = makeControl(d, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, 180, 158, 120, 28, 22);
    char text[16];
    std::snprintf(text, sizeof(text), "%d", g_curMonth);
    SetWindowTextA(st.edit[0], text);
    std::snprintf(text, sizeof(text), "%d", g_curYear);
    SetWindowTextA(st.edit[1], text);
    SetWindowTextA(st.edit[2], "10");

    makeButton(d, "Calculate", ID_D_CALC, BTN_PRIMARY, 180, 204, 120, 36);
    st.err = makeControl(d, "STATIC", "", SS_LEFT, 24, 248, 432, 22, ID_D_ERR);

    makeControl(d, "STATIC", "Actual spending", SS_LEFT, 24, 284, 200, 20, IDC_LABEL + 4, 0, g_fSmall);
    st.val[0] = makeControl(d, "STATIC", "-", SS_LEFT, 24, 304, 432, 30, IDC_LABEL + 5, 0, g_fHead);
    makeControl(d, "STATIC", "Projected month-end spending", SS_LEFT, 24, 340, 300, 20, IDC_LABEL + 6, 0, g_fSmall);
    st.val[1] = makeControl(d, "STATIC", "-", SS_LEFT, 24, 360, 432, 30, IDC_LABEL + 7, 0, g_fHead);
    st.savingsLabel = makeControl(d, "STATIC", "Potential savings", SS_LEFT, 24, 396, 300, 20, IDC_LABEL + 8, 0, g_fSmall);
    st.val[2] = makeControl(d, "STATIC", "-", SS_LEFT, 24, 416, 250, 30, IDC_LABEL + 9, 0, g_fHead);
    makeButton(d, "Close", ID_D_OK, BTN_SECONDARY, 376, 410, 80, 36);

    calculateForecast(&st);
    runModal(owner, st, st.edit[2]);
}

bool showExportDialog(HWND owner, DlgState& st) {
    std::memset(&st, 0, sizeof(st));
    st.kind = 3;
    st.hwnd = createDialogWindow(owner, "Export monthly report", 440, 300, &st);
    if (!st.hwnd) return false;
    HWND d = st.hwnd;

    makeControl(d, "STATIC", "Export monthly report", SS_LEFT, 24, 16, 392, 30, IDC_LABEL, 0, g_fHead);
    makeControl(d, "STATIC",
                "Saves that month's transactions, spending-leak analysis and forecast "
                "to a text file next to your expense file.",
                SS_LEFT, 24, 54, 392, 44, ID_D_HINT);
    makeControl(d, "STATIC", "Month (1-12)", SS_LEFT, 24, 116, 120, 22, IDC_LABEL + 1);
    makeControl(d, "STATIC", "Year", SS_LEFT, 24, 160, 120, 22, IDC_LABEL + 2);
    st.edit[0] = makeControl(d, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, 150, 110, 120, 28, 20);
    st.edit[1] = makeControl(d, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, 150, 154, 120, 28, 21);
    char text[16];
    std::snprintf(text, sizeof(text), "%d", g_curMonth);
    SetWindowTextA(st.edit[0], text);
    std::snprintf(text, sizeof(text), "%d", g_curYear);
    SetWindowTextA(st.edit[1], text);

    st.err = makeControl(d, "STATIC", "", SS_LEFT, 24, 198, 392, 28, ID_D_ERR);
    makeButton(d, "Export", ID_D_OK, BTN_PRIMARY, 236, 244, 100, 36);
    makeButton(d, "Cancel", ID_D_CANCEL, BTN_SECONDARY, 346, 244, 70, 36);

    runModal(owner, st, st.edit[0]);
    return st.ok;
}

void showTextDialog(HWND owner, const char* title, const char* body) {
    DlgState st;
    std::memset(&st, 0, sizeof(st));
    st.kind = 4;
    st.hwnd = createDialogWindow(owner, title, 780, 580, &st);
    if (!st.hwnd) return;
    HWND d = st.hwnd;

    st.edit[0] = makeControl(d, "EDIT", "",
                             WS_TABSTOP | WS_BORDER | WS_VSCROLL | WS_HSCROLL |
                             ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                             16, 16, 748, 504, 20, 0, g_fMono);
    SendMessageA(st.edit[0], EM_SETLIMITTEXT, 1 << 22, 0);

    // The edit control wants \r\n line endings.
    const std::size_t length = std::strlen(body);
    char* converted = new (std::nothrow) char[length * 2 + 1];
    if (converted) {
        std::size_t out = 0;
        for (std::size_t i = 0; i < length; ++i) {
            if (body[i] == '\n') converted[out++] = '\r';
            converted[out++] = body[i];
        }
        converted[out] = '\0';
        SetWindowTextA(st.edit[0], converted);
        delete[] converted;
    } else {
        SetWindowTextA(st.edit[0], body);
    }
    makeButton(d, "Close", ID_D_OK, BTN_PRIMARY, 664, 530, 100, 36);

    runModal(owner, st, NULL);
}

// ----------------------------------------------------------------------------
// Actions
// ----------------------------------------------------------------------------
void doAdd() {
    DlgState st;
    if (!showExpenseForm(g_main, NULL, st)) return;

    int id = 0;
    if (!g_manager->addExpense(st.date, st.paise, st.merchant, st.category,
                               st.type, st.freq, id)) {
        MessageBoxA(g_main,
                    "The expense could not be added. Please check the date, amount, "
                    "text lengths and the type/frequency combination.",
                    "Could not add expense", MB_OK | MB_ICONERROR);
        return;
    }
    saveData();
    const Expense* saved = g_manager->findExpenseById(id);
    bool undoOk = true;
    if (saved) {
        UndoAction action(ActionType::ADD, id);
        action.setAfterSnapshot(*saved);
        undoOk = g_undo->push(action);
    }
    refreshAll();
    selectById(id);
    char money[48];
    GuiLogic::formatMoney(money, sizeof(money), st.paise);
    setStatus(false, "Added expense #%d (%s at %s).%s", id, money, st.merchant,
              undoOk ? "" : " Undo history is full.");
}

void doEdit() {
    const int id = selectedExpenseId();
    if (id < 0) {
        MessageBoxA(g_main, "Select an expense in the table first.", "Edit expense",
                    MB_OK | MB_ICONINFORMATION);
        return;
    }
    const Expense* current = g_manager->findExpenseById(id);
    if (!current) return;
    const Expense before(*current);

    DlgState st;
    if (!showExpenseForm(g_main, &before, st)) return;

    const Expense updated(id, st.date, st.paise, st.merchant, st.category, st.type, st.freq);
    if (!g_manager->editExpense(id, updated)) {
        MessageBoxA(g_main, "The edit was rejected. Please check the values.",
                    "Could not edit expense", MB_OK | MB_ICONERROR);
        return;
    }
    saveData();
    const Expense* after = g_manager->findExpenseById(id);
    bool undoOk = true;
    if (after) {
        UndoAction action(ActionType::EDIT, id, before, *after);
        undoOk = g_undo->push(action);
    }
    refreshAll();
    selectById(id);
    setStatus(false, "Updated expense #%d.%s", id, undoOk ? "" : " Undo history is full.");
}

void doDelete() {
    const int id = selectedExpenseId();
    if (id < 0) {
        MessageBoxA(g_main, "Select an expense in the table first.", "Delete expense",
                    MB_OK | MB_ICONINFORMATION);
        return;
    }
    const Expense* found = g_manager->findExpenseById(id);
    if (!found) return;
    const Expense before(*found);

    char money[48], question[300];
    GuiLogic::formatMoney(money, sizeof(money), before.getAmountPaise());
    std::snprintf(question, sizeof(question),
                  "Delete expense #%d?\n\n%s  %s  (%s, %s)\n\nYou can bring it back with Undo.",
                  id, before.getDate(), money, before.getMerchant(), before.getCategory());
    if (MessageBoxA(g_main, question, "Delete expense", MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;

    if (!g_manager->deleteExpense(id)) {
        MessageBoxA(g_main, "Delete failed.", "Delete expense", MB_OK | MB_ICONERROR);
        return;
    }
    saveData();
    UndoAction action(ActionType::DELETE, id);
    action.setBeforeSnapshot(before);
    const bool undoOk = g_undo->push(action);
    refreshAll();
    setStatus(false, "Deleted expense #%d.%s", id, undoOk ? " Press Undo to restore it." : " Undo history is full.");
}

void doUndo() {
    UndoAction action;
    if (!g_undo->pop(action)) {
        setStatus(true, "Nothing to undo.");
        return;
    }
    const int id = static_cast<int>(action.getExpenseId());
    bool ok = false;
    const char* what = "change";
    if (action.getType() == ActionType::ADD) {
        ok = g_manager->deleteExpense(id);
        what = "added expense removed";
    } else if (action.getType() == ActionType::DELETE) {
        Expense before;
        if (action.getBeforeSnapshot(before)) ok = g_manager->addLoadedExpense(before);
        what = "deleted expense restored";
    } else if (action.getType() == ActionType::EDIT) {
        Expense before;
        if (action.getBeforeSnapshot(before)) ok = g_manager->editExpense(id, before);
        what = "edit reverted";
    }
    if (!ok) {
        g_undo->push(action);
        setStatus(true, "Undo failed; the action was kept on the undo stack.");
        return;
    }
    saveData();
    refreshAll();
    selectById(id);
    setStatus(false, "Undo: %s (#%d).", what, id);
}

void doFind() {
    char text[32];
    GetWindowTextA(g_findEdit, text, sizeof(text));
    GuiLogic::trim(text);
    char* end = NULL;
    const long id = std::strtol(text, &end, 10);
    if (text[0] == '\0' || *end != '\0' || id <= 0 || id > 2147483647L) {
        setStatus(true, "Enter a positive whole number to find an expense by ID.");
        return;
    }
    const int count = g_manager->getCount();
    if (count <= 0) { setStatus(true, "There are no expenses to search."); return; }

    Expense** items = new (std::nothrow) Expense*[count];
    if (!items) { setStatus(true, "Not enough memory to search."); return; }
    int copied = g_manager->copyExpensePointers(items, count);
    if (copied < 0) copied = 0;
    g_sort->sortById(items, copied);
    const Expense* hit = g_search->binarySearchById(items, copied, static_cast<int>(id));
    delete[] items;

    if (!hit) { setStatus(true, "No expense found with ID %ld.", id); return; }

    if (!selectById(static_cast<int>(id))) {
        // Hidden by the filter: clear it so the match becomes visible.
        SetWindowTextA(g_filterEdit, "");
        g_filter[0] = '\0';
        refreshView();
        selectById(static_cast<int>(id));
    }
    SetFocus(g_list);
    setStatus(false, "Found expense #%ld: %s, %s, %s.", id, hit->getDate(),
              hit->getMerchant(), hit->getCategory());
}

void doLeak() {
    g_leak->analyzeExpenses(*g_manager);
    std::ostringstream report;
    g_leak->printLeakReport(report);
    // The temporary string lives until the end of this full expression.
    showTextDialog(g_main, "Spending leak report", report.str().c_str());
}

void doExport() {
    DlgState st;
    if (!showExportDialog(g_main, st)) return;

    char path[260], error[200];
    path[0] = '\0';
    error[0] = '\0';
    if (!GuiLogic::exportMonthlyReport(*g_manager, *g_advisor, g_fileManager->getFilePath(),
                                       st.month, st.year, path, sizeof(path),
                                       error, sizeof(error))) {
        MessageBoxA(g_main, error[0] ? error : "Export failed.", "Export failed",
                    MB_OK | MB_ICONERROR);
        setStatus(true, "Export failed: %s", error);
        return;
    }
    char message[420];
    std::snprintf(message, sizeof(message),
                  "Report saved:\n\n%s\n\nIt includes the transactions, leak analysis and forecast for %s %d.",
                  path, kMonthNames[st.month - 1], st.year);
    MessageBoxA(g_main, message, "Report exported", MB_OK | MB_ICONINFORMATION);
    setStatus(false, "Monthly report exported to %s", path);
}

// ----------------------------------------------------------------------------
// Main window: layout, painting, messages
// ----------------------------------------------------------------------------
void layoutMain(HWND hwnd) {
    if (!g_list) return;
    RECT rc;
    GetClientRect(hwnd, &rc);
    const int W = rc.right;
    const int H = rc.bottom;
    const int M = S(20);
    const int gap = S(14);

    const int cardsTop = S(86);
    const int cardH = S(84);
    const int cardW = (W - 2 * M - 3 * gap) / 4;
    for (int i = 0; i < 4; ++i) {
        g_cardRect[i].left = M + i * (cardW + gap);
        g_cardRect[i].top = cardsTop;
        g_cardRect[i].right = g_cardRect[i].left + cardW;
        g_cardRect[i].bottom = cardsTop + cardH;
    }

    const int row1 = cardsTop + cardH + S(16);
    const int bh = S(36);
    int x = M;
    const int actionIds[4] = { IDC_ADD, IDC_EDIT, IDC_DELETE, IDC_UNDO };
    const int actionW[4] = { S(130), S(84), S(96), S(84) };
    for (int i = 0; i < 4; ++i) {
        MoveWindow(GetDlgItem(hwnd, actionIds[i]), x, row1, actionW[i], bh, TRUE);
        x += actionW[i] + S(8);
    }
    const int rightIds[3] = { IDC_EXPORT, IDC_FORECAST, IDC_LEAK };
    const int rightW[3] = { S(130), S(110), S(130) };
    int rx = W - M;
    for (int i = 0; i < 3; ++i) {
        rx -= rightW[i];
        MoveWindow(GetDlgItem(hwnd, rightIds[i]), rx, row1, rightW[i], bh, TRUE);
        rx -= S(8);
    }

    const int row2 = row1 + bh + S(12);
    const int eh = S(28);
    x = M;
    MoveWindow(GetDlgItem(hwnd, IDC_LABEL + 0), x, row2 + S(5), S(44), S(22), TRUE);
    x += S(46);
    MoveWindow(g_filterEdit, x, row2, S(230), eh, TRUE);
    x += S(230) + S(26);
    MoveWindow(GetDlgItem(hwnd, IDC_LABEL + 1), x, row2 + S(5), S(54), S(22), TRUE);
    x += S(56);
    MoveWindow(g_findEdit, x, row2, S(70), eh, TRUE);
    x += S(70) + S(6);
    MoveWindow(GetDlgItem(hwnd, IDC_FINDGO), x, row2, S(56), eh, TRUE);
    x += S(56) + S(26);
    MoveWindow(GetDlgItem(hwnd, IDC_LABEL + 2), x, row2 + S(5), S(56), S(22), TRUE);
    x += S(58);
    MoveWindow(g_sortCombo, x, row2, S(100), S(160), TRUE);
    x += S(100) + S(8);
    MoveWindow(g_dirButton, x, row2, S(110), eh, TRUE);

    const int listTop = row2 + eh + S(14);
    const int statusH = S(32);
    const int listBottom = H - statusH;
    const int chartW = S(330);
    int listW = W - 2 * M - chartW - gap;
    if (listW < S(300)) listW = S(300);
    MoveWindow(g_list, M, listTop, listW, listBottom - listTop, TRUE);

    g_chartRect.left = M + listW + gap;
    g_chartRect.top = listTop;
    g_chartRect.right = W - M;
    g_chartRect.bottom = listBottom;
    g_statusRect.left = M;
    g_statusRect.top = H - statusH;
    g_statusRect.right = W - M;
    g_statusRect.bottom = H;
}

void drawCard(HDC dc, const RECT& r, const char* label, const char* value, COLORREF accent) {
    fillRound(dc, r, C_CARD, C_BORDER, S(12));
    RECT bar = { r.left + S(18), r.top + S(14), r.left + S(46), r.top + S(18) };
    fillSolid(dc, bar, accent);
    drawLabel(dc, label, r.left + S(18), r.top + S(24), r.right - r.left - S(36), S(18),
              g_fSmall, C_MUTED, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
    drawLabel(dc, value, r.left + S(18), r.top + S(42), r.right - r.left - S(36), S(34),
              g_fBig, C_TEXT, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void drawChart(HDC dc) {
    const RECT& r = g_chartRect;
    if (r.right <= r.left || r.bottom <= r.top) return;
    fillRound(dc, r, C_CARD, C_BORDER, S(12));
    const int pad = S(18);
    const int innerW = r.right - r.left - 2 * pad;
    drawLabel(dc, "Spending by category", r.left + pad, r.top + S(14), innerW, S(24),
              g_fBold, C_TEXT, DT_LEFT | DT_SINGLELINE);

    if (g_catCount == 0) {
        drawLabel(dc, "Nothing to chart yet. Add an expense to get started.",
                  r.left + pad, r.top + S(56), innerW, S(40),
                  g_fSmall, C_MUTED, DT_LEFT | DT_WORDBREAK);
    } else {
        const int top = r.top + S(52);
        const int pitch = S(42);
        const int footer = S(46);
        int fit = (r.bottom - footer - top) / pitch;
        if (fit < 1) fit = 1;
        int shown = g_catCount < fit ? g_catCount : fit;
        if (shown > 8) shown = 8;
        const long long maxValue = g_cats[0].paise > 0 ? g_cats[0].paise : 1;

        for (int i = 0; i < shown; ++i) {
            const int y = top + i * pitch;
            char money[48];
            GuiLogic::formatMoney(money, sizeof(money), g_cats[i].paise);
            drawLabel(dc, g_cats[i].name, r.left + pad, y, innerW * 55 / 100, S(18),
                      g_fSmall, C_TEXT, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
            drawLabel(dc, money, r.left + pad + innerW * 55 / 100, y, innerW * 45 / 100, S(18),
                      g_fSmall, C_MUTED, DT_RIGHT | DT_SINGLELINE);

            RECT track = { r.left + pad, y + S(22), r.left + pad + innerW, y + S(30) };
            fillRound(dc, track, RGB(233, 236, 241), RGB(233, 236, 241), S(8));
            int fillW = static_cast<int>(static_cast<long long>(innerW) * g_cats[i].paise / maxValue);
            if (fillW < S(8)) fillW = S(8);
            RECT bar = { track.left, track.top, track.left + fillW, track.bottom };
            fillRound(dc, bar, kBarColors[i % 8], kBarColors[i % 8], S(8));
        }
        if (g_catCount > shown) {
            char more[48];
            std::snprintf(more, sizeof(more), "+ %d more categories", g_catCount - shown);
            drawLabel(dc, more, r.left + pad, top + shown * pitch, innerW, S(18),
                      g_fSmall, C_MUTED, DT_LEFT | DT_SINGLELINE);
        }
    }

    // Legend for highlighted rows.
    RECT swatch = { r.left + pad, r.bottom - S(32), r.left + pad + S(14), r.bottom - S(18) };
    fillRound(dc, swatch, C_LEAK_ROW, C_WARN, S(4));
    drawLabel(dc, "Highlighted rows: repeated merchant (possible leak)",
              swatch.right + S(8), r.bottom - S(34), innerW - S(24), S(20),
              g_fSmall, C_MUTED, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void paintMain(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    RECT rc;
    GetClientRect(hwnd, &rc);
    const int W = rc.right;
    const int H = rc.bottom;

    HDC dc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, W > 0 ? W : 1, H > 0 ? H : 1);
    HGDIOBJ oldBmp = SelectObject(dc, bmp);
    SetBkMode(dc, TRANSPARENT);

    fillSolid(dc, rc, C_BG);

    // Header band
    RECT header = { 0, 0, W, S(70) };
    fillSolid(dc, header, C_HEADER);
    drawLabel(dc, "Expense Leak Detector", S(24), S(10), W / 2, S(32), g_fTitle,
              RGB(255, 255, 255), DT_LEFT | DT_SINGLELINE);
    drawLabel(dc, "See where your money quietly leaks away", S(26), S(43), W / 2, S(20),
              g_fSmall, RGB(178, 214, 222), DT_LEFT | DT_SINGLELINE);
    const char* path = g_fileManager ? g_fileManager->getFilePath() : "";
    drawLabel(dc, path, W / 2, S(26), W / 2 - S(24), S(20), g_fSmall,
              RGB(178, 214, 222), DT_RIGHT | DT_SINGLELINE | DT_PATH_ELLIPSIS);

    // Summary cards
    char value[64], label[64];
    GuiLogic::formatMoney(value, sizeof(value), g_totalPaise);
    drawCard(dc, g_cardRect[0], "TOTAL SPENT", value, C_ACCENT);

    std::snprintf(value, sizeof(value), "%d", g_recordCount);
    drawCard(dc, g_cardRect[1], "TRANSACTIONS", value, RGB(99, 102, 241));

    std::snprintf(label, sizeof(label), "THIS MONTH (%s %d)", kMonthNames[g_curMonth - 1], g_curYear);
    GuiLogic::formatMoney(value, sizeof(value), g_monthPaise);
    drawCard(dc, g_cardRect[2], label, value, RGB(14, 165, 233));

    std::snprintf(value, sizeof(value), "%d", g_patternCount);
    drawCard(dc, g_cardRect[3], "LEAK PATTERNS FOUND", value, g_patternCount > 0 ? C_WARN : C_ACCENT);

    drawChart(dc);

    // Status line
    drawLabel(dc, g_status, g_statusRect.left, g_statusRect.top + S(6),
              g_statusRect.right - g_statusRect.left, S(22), g_fSmall,
              g_statusError ? C_DANGER : C_MUTED,
              DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);

    BitBlt(hdc, 0, 0, W, H, dc, 0, 0, SRCCOPY);
    SelectObject(dc, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dc);
    EndPaint(hwnd, &ps);
}

void createMainControls(HWND hwnd) {
    makeButton(hwnd, "+  Add expense", IDC_ADD, BTN_PRIMARY, 0, 0, 10, 10);
    makeButton(hwnd, "Edit", IDC_EDIT, BTN_SECONDARY, 0, 0, 10, 10);
    makeButton(hwnd, "Delete", IDC_DELETE, BTN_DANGER, 0, 0, 10, 10);
    makeButton(hwnd, "Undo", IDC_UNDO, BTN_SECONDARY, 0, 0, 10, 10);
    makeButton(hwnd, "Export report", IDC_EXPORT, BTN_SECONDARY, 0, 0, 10, 10);
    makeButton(hwnd, "Forecast", IDC_FORECAST, BTN_SECONDARY, 0, 0, 10, 10);
    makeButton(hwnd, "Leak report", IDC_LEAK, BTN_SECONDARY, 0, 0, 10, 10);

    makeControl(hwnd, "STATIC", "Filter", SS_LEFT, 0, 0, 10, 10, IDC_LABEL + 0);
    makeControl(hwnd, "STATIC", "Find ID", SS_LEFT, 0, 0, 10, 10, IDC_LABEL + 1);
    makeControl(hwnd, "STATIC", "Sort by", SS_LEFT, 0, 0, 10, 10, IDC_LABEL + 2);

    g_filterEdit = makeControl(hwnd, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
                               0, 0, 10, 10, IDC_FILTER);
    SendMessageA(g_filterEdit, EM_LIMITTEXT, 60, 0);
    g_findEdit = makeControl(hwnd, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
                             0, 0, 10, 10, IDC_FINDID);
    SendMessageA(g_findEdit, EM_LIMITTEXT, 10, 0);
    makeButton(hwnd, "Go", IDC_FINDGO, BTN_SECONDARY, 0, 0, 10, 10);

    g_sortCombo = makeControl(hwnd, "COMBOBOX", "", WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                              0, 0, 10, 10, IDC_SORTKEY);
    SendMessageA(g_sortCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("ID"));
    SendMessageA(g_sortCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Date"));
    SendMessageA(g_sortCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Amount"));
    SendMessageA(g_sortCombo, CB_SETCURSEL, 0, 0);
    g_dirButton = makeButton(hwnd, "Ascending", IDC_SORTDIR, BTN_SECONDARY, 0, 0, 10, 10);

    g_list = makeControl(hwnd, "SysListView32", "",
                         WS_TABSTOP | WS_BORDER | LVS_REPORT | LVS_OWNERDATA |
                         LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                         0, 0, 10, 10, IDC_LIST);
    SendMessageA(g_list, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                 LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    static const int widths[8] = { 64, 100, 110, 130, 190, 100, 96, 120 };
    for (int i = 0; i < 8; ++i) {
        char blank[2] = "";
        LVCOLUMNA col;
        std::memset(&col, 0, sizeof(col));
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
        col.fmt = (i == 2) ? LVCFMT_RIGHT : LVCFMT_LEFT;
        col.cx = S(widths[i]);
        col.pszText = blank;
        col.iSubItem = i;
        SendMessageA(g_list, LVM_INSERTCOLUMNA, static_cast<WPARAM>(i),
                     reinterpret_cast<LPARAM>(&col));
    }
}

void handleListDisplay(NMLVDISPINFOA* info) {
    const int row = info->item.iItem;
    if (row < 0 || row >= g_viewCount || !(info->item.mask & LVIF_TEXT)) return;
    const Expense* e = g_view[row];
    char text[96];
    text[0] = '\0';
    switch (info->item.iSubItem) {
    case 0: std::snprintf(text, sizeof(text), "%d", e->getId()); break;
    case 1: std::snprintf(text, sizeof(text), "%s", e->getDate()); break;
    case 2: GuiLogic::formatMoney(text, sizeof(text), e->getAmountPaise()); break;
    case 3: std::snprintf(text, sizeof(text), "%s", e->getCategory()); break;
    case 4: std::snprintf(text, sizeof(text), "%s", e->getMerchant()); break;
    case 5: std::snprintf(text, sizeof(text), "%s", e->getExpenseType()); break;
    case 6: std::snprintf(text, sizeof(text), "%s", e->getFrequency()); break;
    case 7: {
        const int n = leakCountFor(e);
        if (n >= 3) std::snprintf(text, sizeof(text), "Frequent x%d", n);
        else if (n >= 2) std::snprintf(text, sizeof(text), "Repeated x%d", n);
        else std::snprintf(text, sizeof(text), "-");
        break;
    }
    default: break;
    }
    if (info->item.pszText && info->item.cchTextMax > 0) {
        std::strncpy(info->item.pszText, text, static_cast<std::size_t>(info->item.cchTextMax) - 1);
        info->item.pszText[info->item.cchTextMax - 1] = '\0';
    }
}

LRESULT handleListCustomDraw(LPNMLVCUSTOMDRAW cd) {
    switch (cd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;
    case CDDS_ITEMPREPAINT: {
        const int row = static_cast<int>(cd->nmcd.dwItemSpec);
        if (row < 0 || row >= g_viewCount) return CDRF_DODEFAULT;
        const UINT state = static_cast<UINT>(
            SendMessageA(g_list, LVM_GETITEMSTATE, static_cast<WPARAM>(row), LVIS_SELECTED));
        if (state & LVIS_SELECTED) return CDRF_DODEFAULT;
        if (leakCountFor(g_view[row]) >= 2) cd->clrTextBk = C_LEAK_ROW;
        else cd->clrTextBk = (row % 2) ? C_BAND_ROW : RGB(255, 255, 255);
        cd->clrText = C_TEXT;
        return CDRF_DODEFAULT;
    }
    default:
        return CDRF_DODEFAULT;
    }
}

LRESULT onNotify(NMHDR* header, LPARAM lp) {
    if (header->hwndFrom != g_list) return 0;
    switch (header->code) {
    case LVN_GETDISPINFOA:
        handleListDisplay(reinterpret_cast<NMLVDISPINFOA*>(lp));
        return 0;
    case NM_CUSTOMDRAW:
        return handleListCustomDraw(reinterpret_cast<LPNMLVCUSTOMDRAW>(lp));
    case NM_DBLCLK:
        doEdit();
        return 0;
    case LVN_COLUMNCLICK: {
        const NMLISTVIEW* nm = reinterpret_cast<const NMLISTVIEW*>(lp);
        if (nm->iSubItem >= 0 && nm->iSubItem <= 2) {
            if (g_sortKey == nm->iSubItem) g_sortDesc = !g_sortDesc;
            else { g_sortKey = nm->iSubItem; g_sortDesc = false; }
            refreshView();
        }
        return 0;
    }
    case LVN_KEYDOWN: {
        const NMLVKEYDOWN* key = reinterpret_cast<const NMLVKEYDOWN*>(lp);
        if (key->wVKey == VK_DELETE) doDelete();
        else if (key->wVKey == VK_RETURN) doEdit();
        return 0;
    }
    default:
        return 0;
    }
}

LRESULT CALLBACK mainProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        g_main = hwnd;
        createMainControls(hwnd);
        updateColumnHeaders();
        return 0;
    case WM_SIZE:
        layoutMain(hwnd);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_GETMINMAXINFO: {
        MINMAXINFO* mm = reinterpret_cast<MINMAXINFO*>(lp);
        mm->ptMinTrackSize.x = S(1060);
        mm->ptMinTrackSize.y = S(640);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
        paintMain(hwnd);
        return 0;
    case WM_CTLCOLORSTATIC: {
        HDC dc = reinterpret_cast<HDC>(wp);
        SetBkColor(dc, C_BG);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, C_MUTED);
        return reinterpret_cast<LRESULT>(g_brBg);
    }
    case WM_DRAWITEM: {
        const DRAWITEMSTRUCT* d = reinterpret_cast<const DRAWITEMSTRUCT*>(lp);
        if (d->CtlType == ODT_BUTTON) { drawOwnerButton(d); return TRUE; }
        break;
    }
    case WM_NOTIFY:
        return onNotify(reinterpret_cast<NMHDR*>(lp), lp);
    case WM_COMMAND: {
        const int id = LOWORD(wp);
        const int code = HIWORD(wp);
        switch (id) {
        case IDC_ADD: doAdd(); break;
        case IDC_EDIT: doEdit(); break;
        case IDC_DELETE: doDelete(); break;
        case IDC_UNDO: doUndo(); break;
        case IDC_LEAK: doLeak(); break;
        case IDC_FORECAST: showForecastDialog(hwnd); break;
        case IDC_EXPORT: doExport(); break;
        case IDC_FINDGO: doFind(); break;
        case IDC_FINDID:
            break;
        case IDC_FILTER:
            if (code == EN_CHANGE) {
                GetWindowTextA(g_filterEdit, g_filter, sizeof(g_filter));
                GuiLogic::trim(g_filter);
                refreshView();
                if (g_filter[0])
                    setStatus(false, "Showing %d of %d expenses matching \"%s\".",
                              g_viewCount, g_recordCount, g_filter);
                else
                    setStatus(false, "Showing all %d expenses.", g_recordCount);
            }
            break;
        case IDC_SORTKEY:
            if (code == CBN_SELCHANGE) {
                const int pick = static_cast<int>(SendMessageA(g_sortCombo, CB_GETCURSEL, 0, 0));
                if (pick >= 0 && pick <= 2) { g_sortKey = pick; refreshView(); }
            }
            break;
        case IDC_SORTDIR:
            g_sortDesc = !g_sortDesc;
            refreshView();
            break;
        default:
            break;
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

} // namespace

// ----------------------------------------------------------------------------
// Entry point
// ----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmdLine, int showCommand) {
    (void)hPrev;
    (void)cmdLine;
    g_inst = hInstance;

    // Crisp text on high-DPI screens (looked up at run time for old Windows).
    typedef BOOL (WINAPI *SetDpiAwareFn)(void);
    HMODULE user32 = GetModuleHandleA("user32.dll");
    if (user32) {
        SetDpiAwareFn setAware = reinterpret_cast<SetDpiAwareFn>(
            reinterpret_cast<void*>(GetProcAddress(user32, "SetProcessDPIAware")));
        if (setAware) setAware();
    }
    HDC screen = GetDC(NULL);
    if (screen) {
        g_dpi = GetDeviceCaps(screen, LOGPIXELSX);
        ReleaseDC(NULL, screen);
    }
    if (g_dpi < 96) g_dpi = 96;

    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    // Data file lives beside the executable, exactly like the console build.
    char expensePath[MAX_PATH];
    DWORD length = GetModuleFileNameA(NULL, expensePath, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        MessageBoxA(NULL, "Could not determine the application folder.", "Expense Leak Detector",
                    MB_OK | MB_ICONERROR);
        return 1;
    }
    char* slash = std::strrchr(expensePath, '\\');
    char* fwd = std::strrchr(expensePath, '/');
    if (fwd != NULL && (slash == NULL || fwd > slash)) slash = fwd;
    if (slash == NULL) {
        MessageBoxA(NULL, "Could not determine the application folder.", "Expense Leak Detector",
                    MB_OK | MB_ICONERROR);
        return 1;
    }
    std::strcpy(slash + 1, "expense.txt");

    ExpenseManager manager;
    FileManager fileManager(expensePath);
    LeakAnalyzer leakAnalyzer;
    Search search;
    Sort sort;
    SavingsAdvisor advisor;
    UndoStack undoStack;
    g_manager = &manager;
    g_fileManager = &fileManager;
    g_leak = &leakAnalyzer;
    g_search = &search;
    g_sort = &sort;
    g_advisor = &advisor;
    g_undo = &undoStack;

    if (!fileManager.loadExpenses(manager)) {
        char msg[700];
        std::snprintf(msg, sizeof(msg), "Could not load expenses:\n\n%s", fileManager.getLastError());
        MessageBoxA(NULL, msg, "Expense Leak Detector", MB_OK | MB_ICONERROR);
        return 1;
    }

    g_fBody = makeFont(10, FW_NORMAL, "Segoe UI");
    g_fBold = makeFont(10, FW_SEMIBOLD, "Segoe UI");
    g_fSmall = makeFont(9, FW_NORMAL, "Segoe UI");
    g_fTitle = makeFont(20, FW_BOLD, "Segoe UI");
    g_fBig = makeFont(20, FW_BOLD, "Segoe UI");
    g_fHead = makeFont(14, FW_SEMIBOLD, "Segoe UI");
    g_fMono = makeFont(10, FW_NORMAL, "Consolas");
    g_brBg = CreateSolidBrush(C_BG);
    g_brWhite = CreateSolidBrush(RGB(255, 255, 255));
    currentDate();

    WNDCLASSEXA wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = mainProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = g_brBg;
    wc.lpszClassName = "ELDMain";
    wc.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    if (!RegisterClassExA(&wc)) return 1;

    WNDCLASSEXA dc;
    std::memset(&dc, 0, sizeof(dc));
    dc.cbSize = sizeof(dc);
    dc.lpfnWndProc = dialogProc;
    dc.hInstance = hInstance;
    dc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    dc.hbrBackground = g_brBg;
    dc.lpszClassName = "ELDDialog";
    if (!RegisterClassExA(&dc)) return 1;

    int winW = S(1320);
    int winH = S(800);
    const int screenW = GetSystemMetrics(SM_CXSCREEN);
    const int screenH = GetSystemMetrics(SM_CYSCREEN);
    if (winW > screenW - 40) winW = screenW - 40;
    if (winH > screenH - 80) winH = screenH - 80;

    HWND hwnd = CreateWindowExA(0, "ELDMain", "Expense Leak Detector",
                                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                                CW_USEDEFAULT, CW_USEDEFAULT, winW, winH,
                                NULL, NULL, hInstance, NULL);
    if (!hwnd) return 1;

    layoutMain(hwnd);
    refreshAll();
    setStatus(false, "Loaded %d expense%s from %s", g_recordCount,
              g_recordCount == 1 ? "" : "s", expensePath);
    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    // Final safety save, same as option 11 in the console version.
    if (!fileManager.saveExpenses(manager)) {
        char text[700];
        std::snprintf(text, sizeof(text), "Could not save expenses:\n\n%s", fileManager.getLastError());
        MessageBoxA(NULL, text, "Expense Leak Detector", MB_OK | MB_ICONERROR);
    }
    delete[] g_view;
    DeleteObject(g_fBody);
    DeleteObject(g_fBold);
    DeleteObject(g_fSmall);
    DeleteObject(g_fTitle);
    DeleteObject(g_fBig);
    DeleteObject(g_fHead);
    DeleteObject(g_fMono);
    DeleteObject(g_brBg);
    DeleteObject(g_brWhite);
    return static_cast<int>(msg.wParam);
}

#include <wx/wx.h>
#include <wx/filepicker.h>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <wx/protocol/http.h>
#include <wx/uri.h>

class FuzzerFrame : public wxFrame {
public:
    FuzzerFrame();
    ~FuzzerFrame();

private:
    wxTextCtrl* urlEntry;
    wxFilePickerCtrl* filePicker;
    wxTextCtrl* extEntry;
    wxTextCtrl* logArea;
    wxButton* startBtn;
    wxButton* stopBtn;
    wxStaticText* statusLabel;

    std::atomic<bool> isScanning{false};
    std::thread workerThread;

    void OnStart(wxCommandEvent& evt);
    void OnStop(wxCommandEvent& evt);
    void AppendLog(const wxString& text);
    void UpdateStatus(const wxString& status);
    void RunFuzzer(std::string targetUrl, std::string wordlistPath, std::string rawExts);

    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_START_BTN = 1001,
    ID_STOP_BTN = 1002
};

wxBEGIN_EVENT_TABLE(FuzzerFrame, wxFrame)
    EVT_BUTTON(ID_START_BTN, FuzzerFrame::OnStart)
    EVT_BUTTON(ID_STOP_BTN, FuzzerFrame::OnStop)
wxEND_EVENT_TABLE()

class FuzzerApp : public wxApp {
public:
    virtual bool OnInit() {
        FuzzerFrame* frame = new FuzzerFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(FuzzerApp);

FuzzerFrame::FuzzerFrame() : wxFrame(NULL, wxID_ANY, "Endpoint & Directory Fuzzer v2.0 (C++)", wxDefaultPosition, wxSize(700, 620)) {
    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Title
    wxStaticText* title = new wxStaticText(panel, wxID_ANY, "Endpoint & Directory Fuzzer");
    wxFont titleFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    mainSizer->Add(title, 0, wxALIGN_CENTER | wxALL, 10);

    // Form Grid
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(3, 2, 10, 10);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, "Target URL:"), 0, wxALIGN_CENTER_VERTICAL);
    urlEntry = new wxTextCtrl(panel, wxID_ANY, "http://httpbin.org", wxDefaultPosition, wxSize(400, -1));
    gridSizer->Add(urlEntry, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, "Wordlist File:"), 0, wxALIGN_CENTER_VERTICAL);
    filePicker = new wxFilePickerCtrl(panel, wxID_ANY, "", "Select Wordlist File", "Text files (*.txt)|*.txt|All files (*.*)|*.*", wxDefaultPosition, wxSize(400, -1));
    gridSizer->Add(filePicker, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, "Extensions:"), 0, wxALIGN_CENTER_VERTICAL);
    extEntry = new wxTextCtrl(panel, wxID_ANY, ".php, .json, .bak, .env, .txt, .html", wxDefaultPosition, wxSize(400, -1));
    gridSizer->Add(extEntry, 1, wxEXPAND);

    mainSizer->Add(gridSizer, 0, wxALL | wxEXPAND, 15);

    // Buttons Sizer
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    startBtn = new wxButton(panel, ID_START_BTN, "Start Scan");
    stopBtn = new wxButton(panel, ID_STOP_BTN, "Stop Scan");
    stopBtn->Enable(false);

    btnSizer->Add(startBtn, 0, wxRIGHT, 10);
    btnSizer->Add(stopBtn, 0, wxRIGHT, 10);
    mainSizer->Add(btnSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 15);

    // Status Label
    statusLabel = new wxStaticText(panel, wxID_ANY, "Status: Ready");
    mainSizer->Add(statusLabel, 0, wxLEFT | wxBOTTOM, 15);

    // Log Output Box
    logArea = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    logArea->SetBackgroundColour(wxColour(30, 30, 30));
    logArea->SetForegroundColour(wxColour(212, 212, 212));
    wxFont codeFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    logArea->SetFont(codeFont);

    mainSizer->Add(logArea, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 15);

    panel->SetSizer(mainSizer);
}

FuzzerFrame::~FuzzerFrame() {
    if (workerThread.joinable()) {
        isScanning = false;
        workerThread.join();
    }
}

void FuzzerFrame::AppendLog(const wxString& text) {
    wxTheApp->CallAfter([this, text]() {
        logArea->AppendText(text + "\n");
    });
}

void FuzzerFrame::UpdateStatus(const wxString& status) {
    wxTheApp->CallAfter([this, status]() {
        statusLabel->SetLabel("Status: " + status);
    });
}

void FuzzerFrame::OnStart(wxCommandEvent& evt) {
    if (isScanning) return;

    std::string targetUrl = urlEntry->GetValue().ToStdString();
    std::string wordlistPath = filePicker->GetPath().ToStdString();
    std::string rawExts = extEntry->GetValue().ToStdString();

    if (targetUrl.empty()) {
        wxMessageBox("Please enter a valid target URL.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    logArea->Clear();
    isScanning = true;
    startBtn->Enable(false);
    stopBtn->Enable(true);

    workerThread = std::thread(&FuzzerFrame::RunFuzzer, this, targetUrl, wordlistPath, rawExts);
}

void FuzzerFrame::OnStop(wxCommandEvent& evt) {
    if (isScanning) {
        isScanning = false;
        UpdateStatus("Stopping scan...");
        AppendLog("\n[-] Scan cancellation requested...");
    }
}

void FuzzerFrame::RunFuzzer(std::string targetUrl, std::string wordlistPath, std::string rawExts) {
    if (targetUrl.back() != '/') targetUrl += "/";

    std::vector<std::string> baseWords;
    if (!wordlistPath.empty()) {
        FILE* file = fopen(wordlistPath.c_str(), "r");
        if (file) {
            char line[256];
            while (fgets(line, sizeof(line), file)) {
                std::string word(line);
                word.erase(word.find_last_not_of("\r\n") + 1);
                if (!word.empty() && word[0] != '#') baseWords.push_back(word);
            }
            fclose(file);
        }
    }

    if (baseWords.empty()) {
        baseWords = {"admin", "login", "status", "hidden", "get", "post", "api", "robots.txt", ".env", "backup.zip", "dashboard", "user", "config"};
    }

    std::vector<std::string> extensions = {""};
    std::stringstream ss(rawExts);
    std::string ext;
    while (std::getline(ss, ext, ',')) {
        ext.erase(0, ext.find_first_not_of(" \t"));
        ext.erase(ext.find_last_not_of(" \t") + 1);
        if (!ext.empty()) {
            if (ext[0] != '.') ext = "." + ext;
            extensions.push_back(ext);
        }
    }

    std::vector<std::string> targetPaths;
    std::set<std::string> seen;

    for (const auto& word : baseWords) {
        for (const auto& extension : extensions) {
            std::string fullPath = word + extension;
            if (seen.find(fullPath) == seen.end()) {
                seen.insert(fullPath);
                targetPaths.push_back(fullPath);
            }
        }
    }

    UpdateStatus("Scanning...");
    AppendLog("=== Starting Scan on " + targetUrl + " ===");
    AppendLog("Total Requests: " + std::to_string(targetPaths.size()));
    AppendLog("-------------------------------------------------------");

    for (const auto& path : targetPaths) {
        if (!isScanning) break;

        std::string fullUrl = targetUrl + (path[0] == '/' ? path.substr(1) : path);

        wxHTTP http;
        http.SetHeader("User-Agent", "Mozilla/5.0 (Custom C++ GUI Security Fuzzer 2.0)");
        
        wxString url(fullUrl);
        wxURI uri(url);

        if (http.Connect(uri.GetServer(), uri.GetPort().empty() ? 80 : wxAtoi(uri.GetPort()))) {
            wxInputStream* stream = http.GetInputStream(uri.GetPath());
            int status = http.GetResponse();

            if (status == 200) {
                AppendLog("[200 OK]        -> " + fullUrl);
            } else if (status == 301 || status == 302) {
                AppendLog("[" + std::to_string(status) + " REDIRECT] -> " + fullUrl);
            } else if (status == 403) {
                AppendLog("[403 FORBIDDEN]-> " + fullUrl + " (Protected!)");
            } else if (status == 500) {
                AppendLog("[500 SERVER ERR]-> " + fullUrl);
            }
            wxDELETE(stream);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }

    std::string statusText = isScanning ? "Scan Complete!" : "Scan Stopped";
    AppendLog("\n=======================================================\n" + statusText);
    UpdateStatus(statusText);

    wxTheApp->CallAfter([this]() {
        startBtn->Enable(true);
        stopBtn->Enable(false);
        isScanning = false;
    });
}
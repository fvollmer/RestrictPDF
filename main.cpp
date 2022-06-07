#include <iostream>
#include <windows.h>
#include <shobjidl.h>

#include <iterator>
#include <string>
#include <vector>
#include <codecvt>
#include <filesystem>

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/Constants.h>

std::vector<std::filesystem::path> shellItemArrayToPathVector(IShellItemArray *psia)
{
    // create vector of filesystem paths from IShellItemArray
    std::vector<std::filesystem::path> file_list;
    HRESULT hr;
    DWORD cItems;
    hr = psia->GetCount(&cItems);
    if (FAILED(hr) || cItems == 0)
    {
        return file_list; // return empty
    }

    for (DWORD i = 0; i < cItems; ++i)
    {
        IShellItem* pShellItem;
        hr = psia->GetItemAt(i, &pShellItem);
        if (FAILED(hr))
        {
            continue;
        }

        PWSTR pszFilePath;
        hr = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

        // add to list
        if (SUCCEEDED(hr))
        {
            file_list.push_back(std::filesystem::path(pszFilePath));
            CoTaskMemFree(pszFilePath);
        }
    }

    return file_list;
}

std::vector<std::filesystem::path> openFileChooser(HWND parent_hwnd)
{
    // open a file chooser and return all selected files as a vector
    std::vector<std::filesystem::path> file_list;
    HRESULT hr;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog *pFileDialog;

        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
                IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileDialog));
        
        if ( FAILED(hr) )
            return file_list;

        IFileDialogCustomize *pfdc;
        hr = pFileDialog->QueryInterface(IID_PPV_ARGS(&pfdc));
        // todo check hr
        
        if ( !pfdc )
            return file_list;
        
        // ToDo use these settings
        pfdc->StartVisualGroup(1000, L"Restrictions");
        pfdc->AddComboBox(1100);
        pfdc->AddControlItem(1100, 1110, L"Print: None");
        pfdc->AddControlItem(1100, 1120, L"Print: High Resolution");
        pfdc->AddControlItem(1100, 1130, L"Print: Low Resolution");
        pfdc->SetSelectedControlItem(1100, 1110);
        pfdc->AddCheckButton(1200, L"allow accessibility", true);
        pfdc->AddCheckButton(1300, L"allow extract", true);
        pfdc->AddCheckButton(1400, L"allow assemble", false);
        pfdc->AddCheckButton(1500, L"allow annotate and form", false);
        pfdc->AddCheckButton(1600, L"allow form filling", false);
        pfdc->AddCheckButton(1700, L"allow modify other", false);
        pfdc->AddCheckButton(1800, L"encrypt metadata aes", true);
        pfdc->EndVisualGroup();
        pfdc->StartVisualGroup(2000, L"New owner password");
        pfdc->AddEditBox(2100, L"");
        pfdc->AddCheckButton(2200, L"random", true);
        pfdc->EndVisualGroup();
        pfdc->StartVisualGroup(3000, L"New user password");
        pfdc->AddEditBox(3100, L"");
        pfdc->EndVisualGroup();
        pfdc->StartVisualGroup(4000, L"Current user password");
        pfdc->AddEditBox(4100, L"");
        pfdc->EndVisualGroup();
        
        // ToDo has to be called after dialog creation
        // IOleWindow *pWindow;
        // hr = pFileDialog->QueryInterface(IID_PPV_ARGS(&pWindow));
        // HWND hwndDialog;
        // hr = pWindow->GetWindow(&hwndDialog);
        // HWND ownerPasswortEdit = GetDlgItem(hwndDialog, 2100);
        // EnableWindow(ownerPasswortEdit, false);
        // pWindow->Release();
        // // ToDo check hr

        FILEOPENDIALOGOPTIONS options;
        hr = pFileDialog->GetOptions(&options);
        if (SUCCEEDED(hr))
        {   
            pFileDialog->SetOptions(options|FOS_FORCEFILESYSTEM|FOS_ALLOWMULTISELECT);
        }

        pFileDialog->SetTitle(L"Choose PDF files to restrict");
        pFileDialog->SetOkButtonLabel(L"Start");

        COMDLG_FILTERSPEC rgSpec[] =
        { 
            { L"Portable document files", L"*.pdf" },
            { L"All files", L"*.*" },
        };

        pFileDialog->SetFileTypes(std::size(rgSpec), rgSpec);

        if (SUCCEEDED(hr))
        {
            hr = pFileDialog->Show(parent_hwnd);

            if (SUCCEEDED(hr))
            {
                IShellItemArray *psiaResults;
                hr = pFileDialog->GetResults(&psiaResults);

                if (SUCCEEDED(hr))
                {
                    file_list = std::move(shellItemArrayToPathVector(psiaResults));
                    psiaResults->Release();
                }

            }
            pFileDialog->Release();
            pfdc->Release();

        }
        
        CoUninitialize();
    }

    return file_list;
}

const wchar_t CLASS_NAME[]  = L"Restrict PDF Window Class";

HWND createBaseForm(HINSTANCE hInstance, int nCmdShow)
{
    // we need a window only to have a taskbar icon
    // so we create a transparent window
    WNDCLASS wc = { };

    wc.lpfnWndProc   = DefWindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED,  // optional window styles
        CLASS_NAME, // Window class
        L"RestrictPDF", // Window text
        0, // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, // Position
        0, 0, // Size
        NULL, // Parent window    
        NULL, // Menu
        hInstance, // Instance handle
        NULL // Additional application data
        );

    if (hwnd == NULL)
    {
        return hwnd;
    }

    SetLayeredWindowAttributes(hwnd, RGB(255,0,0), 0, LWA_ALPHA);
    ShowWindow(hwnd, nCmdShow);

    return hwnd;
}

std::string getRandomString(size_t len) {
    static const char chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";
    std::string str(len, ' ');

    for (size_t i = 0; i < len; ++i) {
        str[i] = chars[rand() % (ARRAYSIZE(chars) - 1)];
    }
    
    return str;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    HWND hwnd = createBaseForm(hInstance, nCmdShow);
    if (hwnd == NULL)
    {
        return 0;
    }

    std::vector<std::filesystem::path> file_path_list = openFileChooser(hwnd);

    // protect every file
    for (auto in_file: file_path_list)
    {
        std::filesystem::path out_file(in_file);
        out_file.replace_extension("protected.pdf");

        // ensure that we don't overwrite files
        if(std::filesystem::exists(out_file))
        {
            MessageBox(
                hwnd,
                in_file.wstring().append(L" does already exist. Skipping.").c_str(),
                L"File already exists",
                MB_ICONERROR
            );
            continue;
        }        

        try {
            QPDF inpdf;
            inpdf.processFile(in_file.u8string().c_str());

            QPDFWriter outpdfw(inpdf, out_file.u8string().c_str());
            outpdfw.setR6EncryptionParameters(
                "", // user_password
                // random owner password
                // not cryptographically secure, but the owner password
                // and all the restrictions are snake oil anyway
                // (only useful for programs that respect it)
                getRandomString(12).c_str(), // owner_password
                true, // allow_accessibility
                true, // allow_extract
                false, // allow_assemble
                false, // allow_annotate_and_form
                false, // allow_form_filling
                false, // allow_modify_other
                qpdf_r3p_none, // print
                true // encrypt_metadata_aes
            );
            outpdfw.write();

        } catch (std::exception& e){
            std::wstring errorw(strlen(e.what()) , L' ');
            mbstowcs(errorw.data(), e.what(), strlen(e.what()));

            MessageBox(
                hwnd,
                errorw.c_str(),
                in_file.c_str(),
                MB_ICONERROR
            );
        }
    }
    
    PostQuitMessage(0);
    return 0;
}




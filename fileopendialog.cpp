#include <windows.h>
#include <shobjidl.h>
#include <Shlwapi.h>
#include <comdef.h>
#include <Stringapiset.h>
#include <winerror.h>

#include "fileopendialog.hpp"
#include "util.hpp"

// create smart pointers
_COM_SMARTPTR_TYPEDEF(IFileDialogCustomize, __uuidof(IFileDialogCustomize));
_COM_SMARTPTR_TYPEDEF(IFileOpenDialog, __uuidof(IFileOpenDialog));
_COM_SMARTPTR_TYPEDEF(IOleWindow, __uuidof(IOleWindow));
_COM_SMARTPTR_TYPEDEF(IFileDialogEvents, __uuidof(IFileDialogEvents));
_COM_SMARTPTR_TYPEDEF(IShellItemArray, __uuidof(IShellItemArray));

#define GROUP_ENCRYPTION 1000
#define CHECK_ENCRYPT 1100
#define GROUP_RESTRICTIONS 2000
#define COMBOBOX_PRINT 2100
#define CHECK_ACCESSIBILITY 2200
#define CHECK_EXTRACT 2300
#define CHECK_ASSEMBLE 2400
#define CHECK_ANNOTATE_AND_FORM 2500
#define CHECK_FORM_FILLING 2600
#define CHECK_MODIFY_OTHER 2700
#define CHECK_ENCRYPT_METADATA_AES 2800
#define GROUP_OWNER_PASSWORD 3000
#define EDIT_NEW_OWNER_PASSWORD 3100
#define CHECK_NEW_RANDOM_OWNER 3200
#define GROUP_NEW_USER_PASSWORD 4000
#define EDIT_NEW_USER_PASSWORD 4100
#define GROUP_CURRENT_PASSWORD 5000
#define EDIT_CURRENT_PASSWORD 5100

// ToDo don't use a global variable
Parameters _parameters;

class DialogEventHandler : public IFileDialogEvents,
						   public IFileDialogControlEvents
{
public:
	// IUnknown methods
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		static const QITAB qit[] = {
			QITABENT(DialogEventHandler, IFileDialogEvents),
			QITABENT(DialogEventHandler, IFileDialogControlEvents),
			{nullptr, 0}};
		return QISearch(this, qit, riid, ppv);
	}

	IFACEMETHODIMP_(ULONG)
	AddRef()
	{
		return InterlockedIncrement(&_cRef);
	}

	IFACEMETHODIMP_(ULONG)
	Release()
	{
		long cRef = InterlockedDecrement(&_cRef);
		if (!cRef)
			delete this;
		return cRef;
	}

	IFACEMETHODIMP OnFileOk(IFileDialog *pfd);
	IFACEMETHODIMP OnFolderChange(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return S_OK; };
	IFACEMETHODIMP OnHelp(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnSelectionChange(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return S_OK; };
	IFACEMETHODIMP OnTypeChange(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return S_OK; };

	IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *, DWORD, DWORD) { return S_OK; };
	IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *, DWORD) { return S_OK; };
	IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL);
	IFACEMETHODIMP OnControlActivating(IFileDialogCustomize *, DWORD) { return S_OK; };

	DialogEventHandler() : _cRef(1){};

private:
	~DialogEventHandler(){};
	long _cRef;
};

HRESULT DialogEventHandler::OnCheckButtonToggled(IFileDialogCustomize *pfdc, DWORD, BOOL)
{
	// disable/enable controls depending on check box states
	BOOL new_random_owner_password;
	pfdc->GetCheckButtonState(CHECK_NEW_RANDOM_OWNER, &new_random_owner_password);
	CDCONTROLSTATEF random_owner_pwd_dwState = new_random_owner_password ? CDCS_VISIBLE : CDCS_ENABLEDVISIBLE;

	BOOL encrypt;
	pfdc->GetCheckButtonState(CHECK_ENCRYPT, &encrypt);
	CDCONTROLSTATEF encrypt_dwState = encrypt ? CDCS_ENABLEDVISIBLE : CDCS_VISIBLE;

	HRESULT hr;
	hr = pfdc->SetControlState(COMBOBOX_PRINT, encrypt_dwState);
	if (FAILED(hr))
		return hr;
	pfdc->SetControlState(CHECK_ACCESSIBILITY, encrypt_dwState);
	if (FAILED(hr))
		return hr;
	pfdc->SetControlState(CHECK_EXTRACT, encrypt_dwState);
	if (FAILED(hr))
		return hr;
	pfdc->SetControlState(CHECK_ASSEMBLE, encrypt_dwState);
	if (FAILED(hr))
		return hr;
	pfdc->SetControlState(CHECK_ANNOTATE_AND_FORM, encrypt_dwState);
	if (FAILED(hr))
		return hr;
	pfdc->SetControlState(CHECK_FORM_FILLING, encrypt_dwState);
	if (FAILED(hr))
		return hr;
	pfdc->SetControlState(CHECK_MODIFY_OTHER, encrypt_dwState);
	if (FAILED(hr))
		return hr;
	pfdc->SetControlState(CHECK_ENCRYPT_METADATA_AES, encrypt_dwState);
	if (FAILED(hr))
		return hr;
	pfdc->SetControlState(EDIT_NEW_USER_PASSWORD, encrypt_dwState);
	if (FAILED(hr))
		return hr;
	pfdc->SetControlState(EDIT_NEW_OWNER_PASSWORD, encrypt_dwState & random_owner_pwd_dwState);
	if (FAILED(hr))
		return hr;
	pfdc->SetControlState(CHECK_NEW_RANDOM_OWNER, encrypt_dwState);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT DialogEventHandler::OnFileOk(IFileDialog *pfd)
{
	IFileDialogCustomizePtr pfdc;
	pfdc = pfd;

	// get parameters
	WCHAR *current_password;
	pfdc->GetEditBoxText(EDIT_CURRENT_PASSWORD, &current_password);
	_parameters.current_password = utf8_encode(current_password);

	pfdc->GetCheckButtonState(CHECK_ENCRYPT, &_parameters.encrypt);

	WCHAR *user_password;
	pfdc->GetEditBoxText(EDIT_NEW_USER_PASSWORD, &user_password);
	_parameters.user_password = utf8_encode(user_password);

	WCHAR *owner_password;
	pfdc->GetEditBoxText(EDIT_NEW_OWNER_PASSWORD, &owner_password);
	_parameters.owner_password = utf8_encode(owner_password);
	pfdc->GetCheckButtonState(CHECK_NEW_RANDOM_OWNER, &_parameters.random_owner_password);

	pfdc->GetCheckButtonState(CHECK_ACCESSIBILITY, &_parameters.allow_accessibility);
	pfdc->GetCheckButtonState(CHECK_EXTRACT, &_parameters.allow_extract);
	pfdc->GetCheckButtonState(CHECK_ASSEMBLE, &_parameters.allow_assemble);
	pfdc->GetCheckButtonState(CHECK_ANNOTATE_AND_FORM, &_parameters.allow_annotate_and_form);
	pfdc->GetCheckButtonState(CHECK_FORM_FILLING, &_parameters.allow_form_filling);
	pfdc->GetCheckButtonState(CHECK_MODIFY_OTHER, &_parameters.allow_modify_other);
	DWORD print;
	pfdc->GetSelectedControlItem(COMBOBOX_PRINT, &print);
	_parameters.print = static_cast<qpdf_r3_print_e>(print);
	pfdc->GetCheckButtonState(CHECK_ENCRYPT_METADATA_AES, &_parameters.encrypt_metadata_aes);

	return S_OK;
}

HRESULT openFileChooser(HWND parent_hwnd, std::vector<std::filesystem::path> &file_list, Parameters &parameters)
{
	HRESULT hr = S_OK;
	IShellItemArrayPtr psiaResults;
	COMDLG_FILTERSPEC rgSpec[] =
		{
			{L"Portable document files", L"*.pdf"},
			{L"All files", L"*.*"},
		};

	// create file open dialog
	IFileOpenDialogPtr pFileDialog;
	hr = pFileDialog.CreateInstance(__uuidof(FileOpenDialog));
	if (FAILED(hr))
		return hr;

	// register file dialog events
	DialogEventHandler *pDialogEventHandler = new (std::nothrow) DialogEventHandler();
	hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;
	if (FAILED(hr))
		return hr;
	IFileDialogEventsPtr pfde;
	pfde = pDialogEventHandler;
	pDialogEventHandler->Release(); // is still used by our smart pointer and won't be destroyed (yet)
	if (pfde == nullptr)
		return hr;

	DWORD dwCookie;
	hr = pFileDialog->Advise(pfde, &dwCookie);
	if (SUCCEEDED(hr))
	{
		// customize file dialog
		IFileDialogCustomizePtr pfdc;
		pfdc = pFileDialog;
		if (pfdc == nullptr)
			goto unadvise;

		// ToDo don't hardcode the defaults, but remember last used settings (except for passwords)
		pfdc->StartVisualGroup(GROUP_CURRENT_PASSWORD, L"Current password");
		pfdc->AddEditBox(EDIT_CURRENT_PASSWORD, L"");
		pfdc->EndVisualGroup();
		pfdc->StartVisualGroup(GROUP_ENCRYPTION, L"Encryption");
		pfdc->AddCheckButton(CHECK_ENCRYPT, L" R6 AES256", true);
		pfdc->EndVisualGroup();
		pfdc->StartVisualGroup(GROUP_RESTRICTIONS, L"Restrictions");
		pfdc->AddComboBox(COMBOBOX_PRINT);
		pfdc->AddControlItem(COMBOBOX_PRINT, qpdf_r3p_none, L"Print: None");
		pfdc->AddControlItem(COMBOBOX_PRINT, qpdf_r3p_full, L"Print: High Resolution");
		pfdc->AddControlItem(COMBOBOX_PRINT, qpdf_r3p_low, L"Print: Low Resolution");
		pfdc->SetSelectedControlItem(COMBOBOX_PRINT, qpdf_r3p_none);
		pfdc->AddCheckButton(CHECK_ACCESSIBILITY, L"allow accessibility", true);
		pfdc->AddCheckButton(CHECK_EXTRACT, L"allow extract", true);
		pfdc->AddCheckButton(CHECK_ASSEMBLE, L"allow assemble", false);
		pfdc->AddCheckButton(CHECK_ANNOTATE_AND_FORM, L"allow annotate and form", true);
		pfdc->AddCheckButton(CHECK_FORM_FILLING, L"allow form filling", false);
		pfdc->AddCheckButton(CHECK_MODIFY_OTHER, L"allow modify other", true);
		pfdc->AddCheckButton(CHECK_ENCRYPT_METADATA_AES, L"encrypt metadata aes", true);
		pfdc->EndVisualGroup();
		pfdc->StartVisualGroup(GROUP_OWNER_PASSWORD, L"New owner password");
		pfdc->AddEditBox(EDIT_NEW_OWNER_PASSWORD, L"");
		pfdc->SetControlState(EDIT_NEW_OWNER_PASSWORD, CDCS_VISIBLE);
		pfdc->AddCheckButton(CHECK_NEW_RANDOM_OWNER, L"random", true);
		pfdc->EndVisualGroup();
		pfdc->StartVisualGroup(GROUP_NEW_USER_PASSWORD, L"New user password");
		pfdc->AddEditBox(EDIT_NEW_USER_PASSWORD, L"");
		pfdc->EndVisualGroup();

		hr = pFileDialog->SetTitle(L"Choose PDF files to restrict");
		if (FAILED(hr))
			goto unadvise;

		hr = pFileDialog->SetOkButtonLabel(L"Start");
		if (FAILED(hr))
			goto unadvise;

		hr = pFileDialog->SetFileTypes(std::size(rgSpec), rgSpec);
		if (FAILED(hr))
			goto unadvise;

		// allow selection of multiple files
		FILEOPENDIALOGOPTIONS options;
		hr = pFileDialog->GetOptions(&options);
		if (FAILED(hr))
			goto unadvise;
		hr = pFileDialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT);
		if (FAILED(hr))
			goto unadvise;

		hr = pFileDialog->Show(parent_hwnd);
		if (FAILED(hr))
			goto unadvise;

		// get path of selected items
		hr = pFileDialog->GetResults(&psiaResults);
		if (FAILED(hr))
			goto unadvise;
		file_list = std::move(shellItemArrayToPathVector(psiaResults));

	unadvise:
		// yep we use a goto do ensure that we unadvise the event handler
		// (lack of an RAII API)
		pFileDialog->Unadvise(dwCookie);
	}

	parameters = _parameters;

	return hr;
}
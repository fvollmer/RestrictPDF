#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/Constants.h>

#include "util.hpp"
#include "ccoinitializer.hpp"
#include "parameters.hpp"
#include "fileopendialog.hpp"

const wchar_t CLASS_NAME[] = L"RestrictPDF Window Class";

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
	// initialization
	HWND hwnd = nullptr; // no parent window

	WNDCLASS wc = {};
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	RegisterClass(&wc);

	CCoInitializer coinit(COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	std::vector<std::filesystem::path> file_path_list;
	Parameters parameters;

	// get file list and parameters
	HRESULT hr = openFileChooser(hwnd, file_path_list, parameters);

	if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
	{
		// user canceled
		PostQuitMessage(0);
		return 0;
	}
	else if (FAILED(hr))
	{
		// something went wrong. Shouldn't happen. Could happen e.g. out of memory.
		// but there isn't a lot that we can do about this
		std::wstringstream stream;
		stream << L"Unexpected Error: 0x";
		stream << std::hex << hr;

		MessageBox(
			hwnd,
			stream.str().c_str(),
			L"Error",
			MB_ICONERROR);

		PostQuitMessage(0);
		return 0;
	}

	if (parameters.user_password.empty() &&
		parameters.owner_password.empty() &&
		!parameters.random_owner_password)
	{
		MessageBox(
			hwnd,
			L"Warning",
			L"An empty user and owner password may lead to compatibility issues. "
			L"Use a random owner password to avoid theses issues.",
			MB_ICONWARNING);
	}

	// process each file
	for (auto in_file : file_path_list)
	{
		// ToDo Allow the user to select the file suffix
		std::filesystem::path out_file(in_file);
		out_file.replace_extension("out.pdf");

		// ensure that we don't overwrite files
		if (std::filesystem::exists(out_file))
		{
			MessageBox(
				hwnd,
				in_file.wstring().append(L" does already exist. Skipping.").c_str(),
				L"File already exists",
				MB_ICONERROR);
			continue;
		}

		if (parameters.random_owner_password)
		{
			// generate random owner password
			// not cryptographically secure, but the owner password
			// and all the restrictions are snake oil anyway
			// (only useful for programs that respect it)
			parameters.owner_password = getRandomString(12);
		}

		try
		{
			QPDF inpdf;
			inpdf.processFile(reinterpret_cast<const char *>(in_file.u8string().c_str()), parameters.current_password.c_str());

			QPDFWriter outpdfw(inpdf, reinterpret_cast<const char *>(out_file.u8string().c_str()));
			outpdfw.setPreserveEncryption(false);
			if (parameters.encrypt)
			{
				outpdfw.setR6EncryptionParameters(
					parameters.user_password.c_str(),					   // user_password
					parameters.owner_password.c_str(),					   // owner_password
					static_cast<bool>(parameters.allow_accessibility),	   // allow_accessibility
					static_cast<bool>(parameters.allow_extract),		   // allow_extract
					static_cast<bool>(parameters.allow_assemble),		   // allow_assemble
					static_cast<bool>(parameters.allow_annotate_and_form), // allow_annotate_and_form
					static_cast<bool>(parameters.allow_form_filling),	   // allow_form_filling
					static_cast<bool>(parameters.allow_modify_other),	   // allow_modify_other
					parameters.print,									   // print
					static_cast<bool>(parameters.encrypt_metadata_aes)	   // encrypt_metadata_aes
				);
			}
			outpdfw.write();
		}
		catch (std::exception &e)
		{
			std::wstring errorw(strlen(e.what()), L' ');
			mbstowcs(errorw.data(), e.what(), strlen(e.what()));

			MessageBox(
				hwnd,
				errorw.c_str(),
				in_file.c_str(),
				MB_ICONERROR);
		}
	}

	MessageBox(
		hwnd,
		L"All Files were processed.",
		L"Done",
		MB_ICONINFORMATION);

	PostQuitMessage(0);
	return 0;
}

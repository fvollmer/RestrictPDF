#pragma once

#include <windows.h>
#include <string>
#include <qpdf/Constants.h>

struct Parameters
{
	std::string current_password;
	BOOL encrypt;
	std::string user_password;
	std::string owner_password;
	BOOL random_owner_password;
	BOOL allow_accessibility;
	BOOL allow_extract;
	BOOL allow_assemble;
	BOOL allow_annotate_and_form;
	BOOL allow_form_filling;
	BOOL allow_modify_other;
	qpdf_r3_print_e print;
	BOOL encrypt_metadata_aes;
};
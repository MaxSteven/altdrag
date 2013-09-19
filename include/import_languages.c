/*
	Copyright (C) 2013  Stefan Sundin (recover89@gmail.com)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
*/

#define UNICODE
#define _UNICODE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <shlwapi.h>

#define APP_NAME            "AltDrag"
#define APP_VERSION         "1.0"
#define HELP_URL            "https://code.google.com/p/altdrag/wiki/Translate"

wchar_t *inipath;

#include "../localization/strings.h"
#include "../localization/en-US/strings.h"
struct strings *l10n = &en_US;
#include "../include/localization.c"

char *languages[] = {
	"en-US",
	"fr-FR",
	"pl-PL",
	"pt-BR",
	"sk-SK",
	"ru-RU",
	"zh-CN",
	"it-IT",
	"de-DE",
	"es-ES",
	"gl-ES",
};

void wcscpy_escape(wchar_t *dest, wchar_t *source) {
	// Copy from source to dest, escaping \n to \\n and " to \"
	for (; *source != '\0'; source++,dest++) {
		if (*source == '\n' || *source == '"') {
			*dest = '\\';
			dest++;
			*dest = (*source == '\n')?'n':'"';
		}
		else {
			*dest = *source;
		}
	}
	*dest = '\0';
}

int main(int argc, char *argv[]) {
	char utf8_bom[3] = { 0xEF,0xBB,0xBF };
	char utf16le_bom[2] = { 0xFF, 0xFE };
	int i,j;
	for (i=0; i < ARRAY_SIZE(languages); i++) {
		char *code = languages[i];
		wchar_t code_utf16[10];
		int ret = MultiByteToWideChar(CP_UTF8, 0, code, -1, code_utf16, sizeof(code_utf16));
		if (ret == 0) {
			wprintf(L"MultiByteToWideChar() failed. Something is wrong...\n");
		}
		if (!strcmp(code,"en-US")) {
			continue;
		}

		wchar_t ini[MAX_PATH], path[MAX_PATH];
		GetModuleFileName(NULL, ini, ARRAY_SIZE(ini));
		PathRemoveFileSpec(ini);
		wcscat(ini, L"\\");
		wcscat(ini, code_utf16);
		wcscpy(path, ini);
		wcscat(ini, L"\\Translation.ini");
		wcscat(path, L"\\strings.h");
		LoadTranslation(ini);
		l10n = &l10n_ini;

		char code2[5];
		strcpy(code2, code);
		code2[2] = '_';

		char author[100];
		ret = WideCharToMultiByte(CP_UTF8, 0, l10n->author, -1, author, sizeof(author), NULL, NULL);

		FILE *f = _wfopen(path, L"wb");
		fwrite(utf8_bom, 1, sizeof(utf8_bom), f); // Write BOM
		fprintf(f, "/*\n\
	"APP_NAME" "APP_VERSION" - %s localization by %s\n\
\n\
	This program is free software: you can redistribute it and/or modify\n\
	it under the terms of the GNU General Public License as published by\n\
	the Free Software Foundation, either version 3 of the License, or\n\
	(at your option) any later version.\n\
\n\
	Do not edit this file! It is automatically generated from Translate.ini!\n\
*/\n\
\n\
struct strings %s = {\n\
", code, author, code2);

		for (j=0; j < ARRAY_SIZE(l10n_mapping); j++) {
			wchar_t txt[3000];
			// Get pointer to string
			wchar_t *str = *(wchar_t**) ((void*)l10n + ((void*)l10n_mapping[j].str - (void*)&l10n_ini));
			// Escape
			wcscpy_escape(txt, str);
			// Convert to UTF-8
			char str_utf8[3000];
			int ret = WideCharToMultiByte(CP_UTF8, 0, txt, -1, str_utf8, sizeof(str_utf8), NULL, NULL);
			if (ret == 0) {
				wprintf(L"WideCharToMultiByte() failed. Something is wrong...\n");
			}
			// Print
			fprintf(f, "\tL\"%s\",\n", str_utf8);
		}
		fprintf(f, "};\n");
		fclose(f);
	}

	wchar_t path[MAX_PATH];
	GetModuleFileName(NULL, path, ARRAY_SIZE(path));
	PathRemoveFileSpec(path);
	wcscat(path, L"\\languages.h");

	FILE *f = _wfopen(path, L"wb");
	fwrite(utf8_bom, 1, sizeof(utf8_bom), f); // Write BOM
	fprintf(f, "// Do not edit this file! It is automatically generated!\n\n");

	for (i=0; i < ARRAY_SIZE(languages); i++) {
		fprintf(f, "#include \"%s/strings.h\"\n", languages[i]);
	}
	fprintf(f, "\nstruct strings *languages[] = {\n");
	for (i=0; i < ARRAY_SIZE(languages); i++) {
		char code[5];
		strcpy(code, languages[i]);
		code[2] = '_';
		fprintf(f, "\t&%s,\n", code);
	}
	fprintf(f, "};\n\nstruct strings *l10n = &en_US;\n");

	return 0;
}

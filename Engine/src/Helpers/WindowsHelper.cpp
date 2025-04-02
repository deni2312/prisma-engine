#include "Helpers/WindowsHelper.h"
#include <filesystem>
#include <Windows.h>

Prisma::WindowsHelper::WindowsHelper() {

}

std::string Prisma::WindowsHelper::relativePath(const std::string& path) const 
{
	try {
		std::filesystem::path absolutePath = std::filesystem::absolute(path);
		std::filesystem::path basePath = std::filesystem::current_path();
		return std::filesystem::relative(absolutePath, basePath).string();
	}
	catch (const std::exception& e) {
		return path;
	}
}

std::string Prisma::WindowsHelper::openFolder(const std::string& stringFilter) const
{
	OPENFILENAME ofn;
	char szFile[260];

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = stringFilter.c_str();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	// Open the File Explorer dialog
	if (GetOpenFileName(&ofn) == TRUE)
	{
		return szFile;
	}
	return "";
}

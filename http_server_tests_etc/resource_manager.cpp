#include "resource_manager.h"

void resource_manager::load()
{
	BOOL set = EnumResourceTypes(NULL, EnumResTypeProc, reinterpret_cast<LONG_PTR>(this));
	if (set == FALSE)
	{
		return;
	}

	for (USHORT type : resources_types)
	{
		set = EnumResourceNames(NULL, reinterpret_cast<LPWSTR>(type), Enumresnameprocw, reinterpret_cast<LONG_PTR>(this));
	}

	for (const resource_t& r : resources)
	{
		auto value = get_resource_as_string(r.id, r.type);
		if (value.empty()) continue;
		pre_loaded_resources.emplace(std::pair<USHORT, std::string>({ r.id, value }));
	}
}

std::string resource_manager::get_resource_as_string(USHORT id, USHORT type_id)
{
	HRSRC p_src = FindResource(NULL, MAKEINTRESOURCE(id), reinterpret_cast<LPCWSTR>(type_id));
	if (p_src == nullptr)
	{
		DWORD error = GetLastError();
		return "";
	}

	HGLOBAL p_handle = LoadResource(NULL, p_src);
	if (p_handle == nullptr)
	{
		CloseHandle(p_src);
		return "";
	}

	LPVOID p_resource = LockResource(p_handle);
	if (p_resource == nullptr)
	{
		FreeResource(p_handle);
		return "";
	}

	DWORD size = SizeofResource(NULL, p_src);
	if (size == 0)
	{
		UnlockResource(p_resource);
		FreeResource(p_handle);
		return "";
	}

	std::vector<char> buffer(size + 1);
	strcpy_s(buffer.data(), buffer.size(), static_cast<char*>(p_resource));

	UnlockResource(p_resource);
	FreeResource(p_handle);

	std::string html = std::string(buffer.begin(), buffer.end());
	html.erase(std::find(html.begin(), html.end(), '\0'), html.end());
	return html;
}

BOOL CALLBACK Enumresnameprocw(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
{
#pragma warning (disable : 4302)
	if (IS_INTRESOURCE(lpType) == TRUE)
	{
		if (IS_INTRESOURCE(lpName) == TRUE)
		{
			auto name_id = reinterpret_cast<USHORT>(lpName);
			resource_manager* rm = reinterpret_cast<resource_manager*>(lParam);
			resource_t r;
			r.type = reinterpret_cast<USHORT>(lpType);
			r.id = reinterpret_cast<USHORT>(lpName);			
			rm->resources.push_back(r);
		}
	}
	return TRUE;
}

BOOL CALLBACK EnumResTypeProc(HMODULE  hModule, LPWSTR lpszType, LONG_PTR lParam)
{
	if (lpszType == nullptr) return TRUE;

	if (IS_INTRESOURCE(lpszType) == TRUE) //int
	{
		auto id = reinterpret_cast<USHORT>(lpszType);
		resource_manager* rm = reinterpret_cast<resource_manager*>(lParam);
		if (MAKEINTRESOURCE(id) != RT_MANIFEST)
		{
			rm->resources_types.push_back(id);
		}
		
	}
	return TRUE;
}
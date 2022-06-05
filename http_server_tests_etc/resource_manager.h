#pragma once

#include <Windows.h>
#include <vector>
#include <map>
#include <string>

// Callback resource type
BOOL CALLBACK EnumResTypeProc(HMODULE  hModule, LPWSTR lpszType, LONG_PTR lParam);

// Callback resource name
BOOL CALLBACK Enumresnameprocw(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam);

struct resource_t
{
	USHORT id;
	USHORT type;
};

class resource_manager
{
public:
	std::vector<USHORT> resources_types;
	
	std::vector<resource_t> resources;

	std::map<USHORT, std::string> pre_loaded_resources;

	void load();

	std::string get_resource_as_string(USHORT id, USHORT type_id);

	~resource_manager()
	{		
		resources_types.clear();
		resources.clear();
		pre_loaded_resources.clear();
	}

};
#include "resource_manager.h"
#include "resource.h"
#include "../offsets.h"

#include "../include/xorstr/xorstr.hpp"

#include <Windows.h>

namespace fx
{
ResourceManager* ResourceManager::getCurrent(bool fb)
{
	// Do not use the old +0x24AE0 offset: it points to a constructor in
	// current artifacts. GetCurrent is a public C++ export in
	// citizen-resources-core.dll and remains stable across ASLR relocations.
	using GetCurrentFn = ResourceManager* (*)(bool allowFallback);
	static GetCurrentFn getCurrentFn =
		reinterpret_cast<GetCurrentFn>(offsets::ResourceManagerGetCurrent());

	return getCurrentFn ? getCurrentFn(fb) : nullptr;
}

ResourceManagerImpl* ResourceManager::getCurrentIl(bool fb)
{
	ResourceManager* mgr = getCurrent(fb);
	if (!mgr)
	{
		return nullptr;
	}
	return (ResourceManagerImpl*)&((uint64_t*)mgr)[2];
}

fwRefContainer<Resource> ResourceManagerImpl::gResByName(const std::string& name)
{
	std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

	fwRefContainer<Resource> resource;

	auto it = m_resources.find(name);
	if (it != m_resources.end())
		resource = it->second;

	return resource;
}

std::vector<fwRefContainer<Resource>> ResourceManagerImpl::getAllResources()
{
	std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);
	std::vector<fwRefContainer<Resource>> resources;

	for (auto& resource : m_resources)
		resources.push_back(resource.second);

	return resources;
}

} // fx

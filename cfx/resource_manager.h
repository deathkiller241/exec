#pragma once
#include "ref_container.h"
#include "component.h"
#include "event.h"

#include "../offsets.h"

#include "../include/tbb/concurrent_unordered_map.h"
#include "../include/tbb/concurrent_unordered_set.h"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <optional>
#include "Windows.h"

namespace fx
{
class Resource;
class ResourceImpl;
class ResourceMounter;
class ResourceManagerImpl;
class ResourceEventManagerComponent;
class ResourceEventComponent;

class ResourceManager : public fwRefCountable, public ComponentHolderImpl<ResourceManager>
{
public:
	fwEvent<> OnTick;
	fwEvent<> OnAfterReset;

public:
	static ResourceManager* getCurrent(bool fb = true);

	static ResourceManagerImpl* getCurrentIl(bool allowFallback = true);

public:

	template<typename T>
	T* getComp()
	{

		if constexpr (std::is_same_v<T, ResourceEventManagerComponent>)
		{
			// Index is NOT a constant: sub_1800012C0/sub_1800014C0 store
			// RegisterComponent("fx::ResourceEventManagerComponent") into
			// core+RVA 0xA3578, and sub_18001D880 / HandleReceivedPacket /
			// AttachToObject consume it as m_instances[index]. Read it live.
			// Pattern value first, pure-RVA second; first index that yields
			// a live instance wins.
			const std::size_t cands[2] = {
				offsets::ResourceEventComponentIndex(),
				offsets::ResourceEventComponentIndexRva()
			};
			const auto& registry = GetInstanceRegistry();
			if (!registry.GetRef())
			{
				return nullptr;
			}
			const size_t count = registry->GetAllInstances().size();
			for (int c = 0; c < 2; ++c)
			{
				const std::size_t idx = cands[c];
				if (idx == static_cast<std::size_t>(-1) || idx >= count)
				{
					continue;
				}
				if (void* ref = registry->GetInstance(idx).GetRef())
				{
					return (T*)ref;
				}
			}
			return nullptr;
		}

		return 0;
	}
};

class ResourceManagerImpl : private ResourceManager
{
public:
	std::recursive_mutex m_resourcesMutex;

	std::unordered_map<std::string, fwRefContainer<ResourceImpl>> m_resources;

	std::unordered_multimap<std::string, fwRefContainer<Resource>> m_resourceProvides;

	std::recursive_mutex m_mountersMutex;

	std::vector<fwRefContainer<ResourceMounter>> m_mounters;

public:
	fwRefContainer<Resource> gResByName(const std::string& name);

	std::vector<fwRefContainer<Resource>> getAllResources();
};

class ResourceEventManagerComponent
{
private:
	char pad_0x0000[0x290]; 

public:
	fwEvent<const std::string&, const std::string&, const std::string&> OnQueueEvent;

};


} 
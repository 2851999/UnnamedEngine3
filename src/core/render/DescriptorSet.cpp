#include "DescriptorSet.h"

/*****************************************************************************
 * DescriptorSetLayout class
 *****************************************************************************/

DescriptorSetLayout::~DescriptorSetLayout() {
    if (instance)
        device->destroyDescriptorSetLayout(instance);
}

void DescriptorSetLayout::create() {
    // Obtain the bindings
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    for (const auto& bindingInfo : bindingInfos) {
        // Assign the current binding info
        VkDescriptorSetLayoutBinding binding{};
        binding.binding            = bindingInfo.binding;
        binding.descriptorType     = bindingInfo.descriptorType;
        binding.descriptorCount    = binding.descriptorCount;
        binding.stageFlags         = binding.stageFlags;
        binding.pImmutableSamplers = nullptr;

        bindings.push_back(binding);
    }

    // Create the layout
    device->createDescriptorSetLayout(static_cast<uint32_t>(bindings.size()), bindings.data(), &instance);
}
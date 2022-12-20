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
        binding.descriptorCount    = bindingInfo.descriptorCount;
        binding.stageFlags         = bindingInfo.stageFlags;
        binding.pImmutableSamplers = nullptr;

        bindings.push_back(binding);
    }

    // Create the layout
    device->createDescriptorSetLayout(static_cast<uint32_t>(bindings.size()), bindings.data(), &instance);
}

/*****************************************************************************
 * DescriptorSet class
 *****************************************************************************/

DescriptorSet::DescriptorSet(Renderer* renderer, DescriptorSetLayout* layout, bool updatable) : RendererResource(renderer), layout(layout), updatable(updatable) {
    // Obtain the device for creation
    VulkanDevice* device = renderer->getDevice();

    // Obtain the required number of descriptor sets needed
    unsigned int numSetsRequired = updatable ? Renderer::MAX_FRAMES_IN_FLIGHT : 1;

    // Go through all of the bindings in the layout and assign the pool sizes
    std::vector<DescriptorSetLayout::BindingInfo>& bindingInfos = layout->getBindingInfos();
    std::vector<VkDescriptorPoolSize> poolSizes(bindingInfos.size());

    for (unsigned int i = 0; i < bindingInfos.size(); ++i) {
        poolSizes[i].type            = bindingInfos[i].descriptorType;
        poolSizes[i].descriptorCount = static_cast<uint32_t>(numSetsRequired);
    }

    // Create the pool
    device->createDescriptorPool(0, static_cast<uint32_t>(numSetsRequired), static_cast<uint32_t>(poolSizes.size()), poolSizes.data(), &pool);

    // Copy descriptor set layouts (need one per set)
    std::vector<VkDescriptorSetLayout> layouts(numSetsRequired, layout->getVkLayout());

    // Descriptor set allocation info
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(numSetsRequired);
    allocInfo.pSetLayouts        = layouts.data();

    // Attempt allocation
    instances.resize(numSetsRequired);
    if (vkAllocateDescriptorSets(device->getVkLogical(), &allocInfo, instances.data()) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to allocate descriptor sets", "DescriptorSet");
}

DescriptorSet::~DescriptorSet() {
    // Obtain the device for destruction
    VulkanDevice* device = renderer->getDevice();

    device->destroyDescriptorPool(pool);
}

void DescriptorSet::setup(std::vector<DescriptorSetResource*> resources) {
    this->resources = resources;

    // Update all descriptor sets
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    // Obtain VkWriteDescriptorSet's
    for (unsigned int i = 0; i < instances.size(); ++i) {
        for (unsigned int j = 0; j < resources.size(); ++j) {
            DescriptorSetLayout::BindingInfo& bindingInfo = layout->getBindingInfos()[j];
            descriptorWrites.push_back(resources[j]->initWriteDescriptorSet(i, instances[i], bindingInfo.binding, bindingInfo.descriptorCount));
        }
    }

    // Update
    vkUpdateDescriptorSets(renderer->getDevice()->getVkLogical(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DescriptorSet::updateCurrentFrame() {
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    // Obtain VkWriteDescriptorSet's
    for (unsigned int j = 0; j < resources.size(); ++j) {
        DescriptorSetLayout::BindingInfo& bindingInfo = layout->getBindingInfos()[j];
        descriptorWrites.push_back(resources[j]->initWriteDescriptorSet(renderer->getCurrentFrame(), instances[renderer->getCurrentFrame()], bindingInfo.binding, bindingInfo.descriptorCount));
    }

    // Update
    vkUpdateDescriptorSets(renderer->getDevice()->getVkLogical(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
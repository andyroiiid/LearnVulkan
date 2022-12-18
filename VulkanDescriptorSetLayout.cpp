//
// Created by andyroiiid on 12/17/2022.
//

#include "VulkanDescriptorSetLayout.h"

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
        VulkanDevice *device,
        const std::initializer_list<VulkanDescriptorSetBindingInfo> &bindingInfos
) : m_device(device) {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(bindingInfos.size());
    for (const VulkanDescriptorSetBindingInfo &bindingInfo: bindingInfos) {
        VkDescriptorSetLayoutBinding &binding = bindings.emplace_back();
        binding.binding = bindingInfo.Binding;
        binding.descriptorType = bindingInfo.DescriptorType;
        binding.descriptorCount = 1;
        binding.stageFlags = bindingInfo.StageFlags;

    }

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = bindings.size();
    createInfo.pBindings = bindings.data();

    m_descriptorSetLayout = m_device->CreateDescriptorSetLayout(createInfo);
}

void VulkanDescriptorSetLayout::Release() {
    if (m_device) {
        m_device->DestroyDescriptorSetLayout(m_descriptorSetLayout);
    }

    m_device = nullptr;
    m_descriptorSetLayout = VK_NULL_HANDLE;
}

void VulkanDescriptorSetLayout::Swap(VulkanDescriptorSetLayout &other) noexcept {
    std::swap(m_device, other.m_device);
    std::swap(m_descriptorSetLayout, other.m_descriptorSetLayout);
}

VkDescriptorSet VulkanDescriptorSetLayout::AllocateDescriptorSet() {
    return m_device->AllocateDescriptorSet(m_descriptorSetLayout);
}

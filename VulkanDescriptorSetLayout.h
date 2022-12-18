//
// Created by andyroiiid on 12/17/2022.
//

#pragma once

#include "VulkanDevice.h"

struct VulkanDescriptorSetBindingInfo {
    uint32_t Binding;
    VkDescriptorType DescriptorType;
    VkShaderStageFlags StageFlags;
};

class VulkanDescriptorSetLayout {
public:
    VulkanDescriptorSetLayout() = default;

    VulkanDescriptorSetLayout(VulkanDevice *device, const std::initializer_list<VulkanDescriptorSetBindingInfo> &bindingInfos);

    ~VulkanDescriptorSetLayout() {
        Release();
    }

    VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &) = delete;

    VulkanDescriptorSetLayout &operator=(const VulkanDescriptorSetLayout &) = delete;

    VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&other) noexcept {
        Swap(other);
    }

    VulkanDescriptorSetLayout &operator=(VulkanDescriptorSetLayout &&other) noexcept {
        if (this != &other) {
            Release();
            Swap(other);
        }
        return *this;
    }

    void Release();

    void Swap(VulkanDescriptorSetLayout &other) noexcept;

    [[nodiscard]] const VkDescriptorSetLayout &Get() const { return m_descriptorSetLayout; }

    VkDescriptorSet AllocateDescriptorSet();

private:
    VulkanDevice *m_device = nullptr;

    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
};

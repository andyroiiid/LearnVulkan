//
// Created by andyroiiid on 12/11/2022.
//

#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

template<typename... Args>
void DebugLog(const spdlog::format_string_t<Args...> &format, Args... args) {
    spdlog::info(format, args...);
}

template<typename... Args>
bool DebugCheck(const bool succeeded, const spdlog::format_string_t<Args...> &failMessage, Args... args) {
    if (succeeded) return true;
    DebugLog(failMessage, args...);
    return false;
}

template<typename... Args>
void DebugCheckVk(const VkResult result, const spdlog::format_string_t<Args...> &failMessage, Args... args) {
    DebugCheck(result == VK_SUCCESS, failMessage, args...);
}

template<typename... Args>
void DebugCheckCritical(const bool succeeded, const spdlog::format_string_t<Args...> &failMessage, Args... args) {
    if (DebugCheck(succeeded, failMessage, args...)) return;
    exit(EXIT_FAILURE);
}

template<typename... Args>
void DebugCheckCriticalVk(const VkResult result, const spdlog::format_string_t<Args...> &failMessage, Args... args) {
    DebugCheckCritical(result == VK_SUCCESS, failMessage, args...);
}

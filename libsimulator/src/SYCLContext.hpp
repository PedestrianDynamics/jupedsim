// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <sycl/sycl.hpp>

#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>

/**
 * @brief SYCL Context Singleton
 *
 * Manages a single SYCL device and queue instance for the entire application.
 * Thread-safe lazy initialization on first access.
 *
 * Usage:
 *   auto& context = SYCLContext::Get();
 *   sycl::queue& q = context.GetQueue();
 *   sycl::device& dev = context.GetDevice();
 */
class SYCLContext
{
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the SYCLContext instance
     */
    static SYCLContext& Get()
    {
        static std::once_flag flag;
        static std::unique_ptr<SYCLContext> instance;

        std::call_once(flag, []() { instance = std::make_unique<SYCLContext>(); });

        return *instance;
    }

    /**
     * @brief Get the SYCL queue
     * @return Reference to the SYCL queue
     */
    sycl::queue& GetQueue() { return *queue_; }

    /**
     * @brief Get the SYCL device
     * @return Reference to the SYCL device
     */
    sycl::device& GetDevice() { return device_; }

    /**
     * @brief Get device information
     * @return String with device name and vendor
     */
    std::string GetDeviceInfo() const
    {
        std::string info = "Device: ";
        info += device_.get_info<sycl::info::device::name>();
        info += " | Vendor: ";
        info += device_.get_info<sycl::info::device::vendor>();
        info += " | Type: ";
        info += device_.is_gpu() ? "GPU" : (device_.is_cpu() ? "CPU" : "Other");
        return info;
    }

    /**
     * @brief Check if device is GPU
     * @return true if the device is a GPU
     */
    bool IsGPU() const { return device_.is_gpu(); }

    /**
     * @brief Check if device is CPU
     * @return true if the device is a CPU
     */
    bool IsCPU() const { return device_.is_cpu(); }

    // Delete copy/move operations for singleton
    SYCLContext(const SYCLContext&) = delete;
    SYCLContext& operator=(const SYCLContext&) = delete;
    SYCLContext(SYCLContext&&) = delete;
    SYCLContext& operator=(SYCLContext&&) = delete;

    /**
     * @brief Constructor - initializes SYCL context
     *
     * Attempts to use GPU if available, falls back to CPU.
     */
    SYCLContext()
    {
        try {
            // Try to use CPU selector first
            device_ = sycl::device(sycl::cpu_selector_v);
        } catch(const sycl::exception&) {
            // Fall back to GPU if CPU unavailable
            try {
                device_ = sycl::device(sycl::gpu_selector_v);
            } catch(const sycl::exception&) {
                // Use default device as last resort
                device_ = sycl::device();
            }
        }

        // Create queue with the selected device
        queue_ = std::make_unique<sycl::queue>(device_);

        // Log device information
        std::cout << "SYCL Context initialized: " << GetDeviceInfo() << "\n";
    }

    ~SYCLContext() = default;

private:
    sycl::device device_;
    std::unique_ptr<sycl::queue> queue_;
};

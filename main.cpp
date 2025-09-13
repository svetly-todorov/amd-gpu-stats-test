#include <amd_smi/amdsmi.h>
#include <iostream>
#include <cstring>

int main() {
    amdsmi_status_t ret;

    // Initialize AMD SMI library
    try {
        ret = amdsmi_init(AMDSMI_INIT_AMD_GPUS);
        if (ret != AMDSMI_STATUS_SUCCESS) {
            std::cerr << "Failed to initialize AMDSMI library (status=" << ret << ")" << std::endl;
            std::cerr << "This program needs to be run with sudo privileges." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        std::cerr << "This program needs to be run with sudo privileges." << std::endl;
        return 1;
    }

    uint32_t device_count;
    amdsmi_processor_handle devices[64];  // Assuming max 64 GPUs
    ret = amdsmi_get_processor_handles(nullptr, &device_count, devices);
    if (ret != AMDSMI_STATUS_SUCCESS) {
        std::cerr << "Failed to get processor handles" << std::endl;
        amdsmi_shut_down();
        return 1;
    }

    std::cout << "Found " << device_count << " GPU device(s)" << std::endl;

    for (uint32_t dev_idx = 0; dev_idx < device_count; dev_idx++) {
        amdsmi_processor_handle dev_handle = devices[dev_idx];

        // Get device name
        char name[256];
        ret = amdsmi_get_gpu_vendor_name(dev_handle, name, sizeof(name));
        if (ret == AMDSMI_STATUS_SUCCESS) {
            std::cout << "\nDevice " << dev_idx << ": " << name << std::endl;
        }

        // Get metrics info
        amdsmi_gpu_metrics_t metrics_info;
        ret = amdsmi_get_gpu_metrics_info(dev_handle, &metrics_info);
        if (ret != AMDSMI_STATUS_SUCCESS) {
            std::cerr << "Failed to get GPU metrics for device " << dev_idx << std::endl;
            continue;
        }

        // Get the partition id
        uint32_t partition_id;
        amdsmi_status_t status;
        amdsmi_kfd_info_t kfd_info;
    
        status = amdsmi_get_gpu_kfd_info(dev_handle, &kfd_info);
        if (status != AMDSMI_STATUS_SUCCESS) {
            std::cerr << "Failed to get partition id for device " << dev_idx << std::endl;
            exit(1);
        }
        partition_id = kfd_info.current_partition_id;

        // Print GFX activity stats
        std::cout << "Partition ID: " << partition_id << std::endl;
        std::cout << "GFX Activity: " << metrics_info.average_gfx_activity << "%" << std::endl;
        std::cout << "GFX Busy Inst 0: " << metrics_info.xcp_stats[partition_id].gfx_busy_inst[0] << "%" << std::endl;
        std::cout << "GFX Busy Inst 1: " << metrics_info.xcp_stats[partition_id].gfx_busy_inst[1] << "%" << std::endl;
        std::cout << "GFX Busy Inst 2: " << metrics_info.xcp_stats[partition_id].gfx_busy_inst[2] << "%" << std::endl;
        std::cout << "GFX Busy Inst 3: " << metrics_info.xcp_stats[partition_id].gfx_busy_inst[3] << "%" << std::endl;
    }

    amdsmi_shut_down();
    return 0;
}
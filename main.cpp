#include <amd_smi/amdsmi.h>
#include <iostream>
#include <cstring>

/// \brief function to get aga_obj_key_t for a given GPU
/// \param[in]  gpu_handle  GPU handle
/// \param[out] key         aga_obj_key_t of the GPU
int
smi_gpu_uuid_get (amdsmi_processor_handle gpu_handle)
{
    amdsmi_status_t status;
    char uuid[AMDSMI_GPU_UUID_SIZE];
    uint32_t uuid_len = AMDSMI_GPU_UUID_SIZE;

    // get uuid from amdsmi
    status = amdsmi_get_gpu_device_uuid(gpu_handle, &uuid_len, uuid);
    if (status != AMDSMI_STATUS_SUCCESS) {
        std::cerr << "Failed to get uuid of GPU " << gpu_handle << ", err " << status << std::endl;
        return -1;
    }

    std::cout << "UUID of GPU " << gpu_handle << " is " << uuid << std::endl;

    return 0;
}

int
smi_discover_gpus (uint32_t *num_gpus, amdsmi_processor_handle *processor_handles)
{
    uint32_t num_procs;
    uint32_t num_sockets;
    amdsmi_status_t status;
    processor_type_t proc_type;
    amdsmi_socket_handle socket_handles[64];
    amdsmi_processor_handle proc_handles[64];

    if (!num_gpus) {
        return -1;
    }
    *num_gpus = 0;
    // get the socket count available in the system
    status = amdsmi_get_socket_handles(&num_sockets, NULL);
    if (status != AMDSMI_STATUS_SUCCESS) {
        return -1;
    }
    // get the socket handles in the system
    status = amdsmi_get_socket_handles(&num_sockets, &socket_handles[0]);
    if (status != AMDSMI_STATUS_SUCCESS) {
        return -1;
    }
    for (uint32_t i = 0; i < num_sockets; i++) {
        // for each socket get the number of processors
        status = amdsmi_get_processor_handles(socket_handles[i],
                                              &num_procs, NULL);
        if (status != AMDSMI_STATUS_SUCCESS) {
            return -1;
        }
        // for each socket get the processor handles
        status = amdsmi_get_processor_handles(socket_handles[i],
                                              &num_procs, &processor_handles[0]);
        if (status != AMDSMI_STATUS_SUCCESS) {
            return -1;
        }
        // get uuids of each GPU
        for (uint32_t j = 0; j < num_procs; j++) {
            status = amdsmi_get_processor_type(processor_handles[j], &proc_type);
            if (status != AMDSMI_STATUS_SUCCESS) {
                return -1;
            }
            if (proc_type == AMDSMI_PROCESSOR_TYPE_AMD_GPU) {
                if (smi_gpu_uuid_get(processor_handles[j])) {
                    return -1;
                }
                (*num_gpus)++;
            }
        }
    }
    return 0;
}

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
    amdsmi_processor_handle handles[64];  // Assuming max 64 GPUs
    if (smi_discover_gpus(&device_count, handles)) {
        std::cerr << "Failed to discover GPUs" << std::endl;
        amdsmi_shut_down();
        return 1;
    }

    std::cout << "Found " << device_count << " GPU device(s)" << std::endl;

    for (uint32_t dev_idx = 0; dev_idx < device_count; dev_idx++) {
        amdsmi_processor_handle dev_handle = handles[dev_idx];

        // Get device name
        char name[256];
        ret = amdsmi_get_gpu_vendor_name(dev_handle, name, sizeof(name));
        if (ret == AMDSMI_STATUS_SUCCESS) {
            std::cout << "\nDevice " << dev_idx << ": " << name << std::endl;
        }

        amdsmi_virtualization_mode_t virtualization_mode;
        ret = amdsmi_get_gpu_virtualization_mode(dev_handle, &virtualization_mode);
        if (ret == AMDSMI_STATUS_SUCCESS) {
            std::cout << "Virtualization Mode: " << virtualization_mode << std::endl;
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
        std::cout << "GFX Busy, Partition " << partition_id << ", Inst 0: " << metrics_info.xcp_stats[partition_id].gfx_busy_inst[0] << "%" << std::endl;
        std::cout << "GFX Busy, Partition " << partition_id << ", Inst 1: " << metrics_info.xcp_stats[partition_id].gfx_busy_inst[1] << "%" << std::endl;
        std::cout << "GFX Busy, Partition " << partition_id << ", Inst 2: " << metrics_info.xcp_stats[partition_id].gfx_busy_inst[2] << "%" << std::endl;
        std::cout << "GFX Busy, Partition " << partition_id << ", Inst 3: " << metrics_info.xcp_stats[partition_id].gfx_busy_inst[3] << "%" << std::endl;
    }

    amdsmi_shut_down();
    return 0;
}
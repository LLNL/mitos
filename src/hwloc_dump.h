#include <hwloc.h>

int dump_hardware_xml(const char *filename)
{
    int err;
    unsigned long flags = 0; // don't show anything special
    hwloc_topology_t topology;

    err = hwloc_topology_init(&topology);
    if(err)
    {
        std::cerr << "hwloc: Failed to initialize" << std::endl;
        return 1;
    }

    err = hwloc_topology_set_flags(topology, flags);
    if(err)
    {
        std::cerr << "hwloc: Failed to set flags" << std::endl;
        return 1;
    }

    err = hwloc_topology_load (topology);
    if(err)
    {
        std::cerr << "hwloc: Failed to load topology" << std::endl;
        return 1;
    }

    err = hwloc_topology_export_xml(topology, "hardware.xml");
    if(err)
    {
        std::cerr << "hwloc: Failed to export xml" << std::endl;
        return 1;
    }

    return 0;
}

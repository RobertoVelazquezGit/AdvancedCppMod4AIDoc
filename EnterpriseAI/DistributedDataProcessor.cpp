/// @file DistributedDataProcessor.cpp
/// @brief Non-template implementations of the local processing mock.
#include "DistributedDataProcessor.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace DistributedComputing {

DistributedDataProcessor::DistributedDataProcessor(const ProcessingConfig& config)
    : config(config) {
    if (config.chunkSize == 0) throw std::invalid_argument("chunkSize must be positive");
}

bool DistributedDataProcessor::partitionData(const std::vector<std::uint8_t>& rawData,
    std::vector<DataPartition>& partitions) {
    std::vector<DataPartition> result;
    for (std::size_t offset = 0; offset < rawData.size();) {
        const auto length = (std::min)(config.chunkSize, rawData.size() - offset);
        DataPartition partition;
        partition.partitionId = "partition-" + std::to_string(result.size());
        partition.startOffset = offset;
        partition.endOffset = offset + length;
        partition.data.assign(rawData.begin() + offset, rawData.begin() + offset + length);
        /// @note DataPartition has an implicitly generated move constructor:
        /// its string, vector, and map members are moved, while scalar offsets
        /// are copied. No user-declared special member prevents its generation.
        /// std::move enables push_back to select this constructor instead of
        /// copying the partition and its byte buffer. It does not itself move data.
        /// The local partition is no longer needed after insertion; it remains
        /// valid after the move, but its previous contents must not be assumed.
        result.push_back(std::move(partition));
        offset += length;
    }
    /// @note This invokes vector's move assignment, not DataPartition's move
    /// constructor. With the standard allocator used here, partitions takes
    /// ownership of result's storage without moving each element individually.
    /// The previous contents of partitions are replaced. The local result remains
    /// valid but its contents are unspecified, and it is no longer used.
    partitions = std::move(result);
    return true;
}

std::vector<DistributedDataProcessor::DataPartition>
DistributedDataProcessor::redistributePartitions(const std::vector<DataPartition>& partitions,
    const std::map<std::string, double>& nodeCapacities) {
    std::vector<std::string> workers;
    for (const auto& entry : nodeCapacities)
        if (entry.second > 0.0) workers.push_back(entry.first);
    auto result = partitions;
    if (!workers.empty()) {
        for (std::size_t index = 0; index < result.size(); ++index)
            result[index].metadata["workerNode"] = workers[index % workers.size()];
    }
    return result;
}

void DistributedDataProcessor::enableFaultTolerance(bool enableReplication, int replicationFactor) {
    if (replicationFactor < 1) throw std::invalid_argument("replicationFactor must be positive");
    replicationEnabled = enableReplication;
    replicationCopies = replicationFactor;
}
}

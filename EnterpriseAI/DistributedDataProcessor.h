/// @file DistributedDataProcessor.h
/// @brief Local data-processing mock with a header-defined template method.
#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <map>
#include <string>
#include <vector>

namespace DistributedComputing {

/// @brief Demonstrates partitioning and processing without network operations.
/// @note Configuration and non-template methods are not synchronized for concurrent access.
class DistributedDataProcessor {
public:
    /// @brief Settings for local processing and reserved distributed features.
    struct ProcessingConfig {
        std::size_t chunkSize = 4; ///< Maximum bytes per partition; must be positive.
        int parallelism = 1; ///< Reserved; processing is sequential.
        bool enableCompression = false; ///< Reserved; bytes are never compressed.
        std::string serializationFormat = "raw"; ///< Descriptive label only.
        std::chrono::milliseconds processingTimeout{1000}; ///< Reserved; no timeout is enforced.
    };

    /// @brief A contiguous byte range and its simulated destination metadata.
    struct DataPartition {
        std::string partitionId; ///< Identifier assigned during partitioning.
        std::size_t startOffset = 0; ///< Inclusive offset in the original byte vector.
        std::size_t endOffset = 0; ///< Exclusive offset in the original byte vector.
        std::vector<std::uint8_t> data; ///< Uncompressed bytes in this range.
        std::map<std::string, std::string> metadata; ///< Optional simulated worker assignment.
    };

    /// @brief Stores the processing configuration.
    /// @param config Settings for this instance.
    /// @throws std::invalid_argument If chunkSize is zero.
    explicit DistributedDataProcessor(const ProcessingConfig& config);

    /// @brief Creates a deferred sequential transformation of a dataset copy.
    /// @tparam DataType Copyable element type; processor results must convert to it.
    /// @tparam ProcessorFunc Copyable callable accepting a const DataType reference.
    /// @param dataset Elements to copy and transform.
    /// @param processor Transformation applied once per element.
    /// @param processingHints Reserved hints; ignored by the mock.
    /// @return Future containing ordered results, or a processor exception.
    /// @note get()/wait() performs the work locally; no background task is started.
    /// @note The lambda is mutable so its captured processor can invoke a non-const
    /// operator(), allowing stateful function objects to modify their internal state.
    /// These changes affect the captured copy, not the caller's original processor.
    /// @note A function object's copyability depends on its type and members
    /// (or captures for a lambda). The by-value parameter can be initialized by
    /// copying or moving, but this implementation requires a copyable processor
    /// because the lambda subsequently copies it into its capture.
    template<typename DataType, typename ProcessorFunc>
    std::future<std::vector<DataType>> processDistributed(
        const std::vector<DataType>& dataset, ProcessorFunc processor,
        const std::map<std::string, std::string>& processingHints = {}) {
        (void)processingHints;
        return std::async(std::launch::deferred, [dataset, processor]() mutable {
            std::vector<DataType> results;
            results.reserve(dataset.size());
            for (const auto& value : dataset) results.push_back(processor(value));
            return results;
        });
    }

    /// @brief Splits bytes into contiguous chunks of at most chunkSize bytes.
    /// @param rawData Source bytes; an empty input produces no partitions.
    /// @param partitions Output replaced by the newly generated partitions.
    /// @return True after successful partitioning.
    bool partitionData(const std::vector<std::uint8_t>& rawData,
        std::vector<DataPartition>& partitions);

    /// @brief Copies partitions and assigns mock worker names cyclically.
    /// @param partitions Partitions to copy; source objects remain unchanged.
    /// @param nodeCapacities Workers with positive capacity, ordered by name.
    /// Capacity magnitudes are ignored; nonpositive and NaN entries are skipped.
    /// @return Copies with metadata["workerNode"] assigned, or unchanged copies if no worker qualifies.
    std::vector<DataPartition> redistributePartitions(
        const std::vector<DataPartition>& partitions,
        const std::map<std::string, double>& nodeCapacities);

    /// @brief Stores simulated replication settings; does not replicate data.
    /// @param enableReplication Whether simulated replication is enabled.
    /// @param replicationFactor Positive requested copy count.
    /// @throws std::invalid_argument If replicationFactor is less than one.
    void enableFaultTolerance(bool enableReplication = true, int replicationFactor = 3);

private:
    ProcessingConfig config; ///< Local processing settings.
    bool replicationEnabled = false; ///< Reserved replication flag.
    int replicationCopies = 1; ///< Reserved replication count.
};
}

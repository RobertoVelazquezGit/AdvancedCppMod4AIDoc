/// @file main.cpp
/// @brief Basic usage of the scheduler and data processor mocks.
#include "TaskScheduler.h"
#include "DistributedDataProcessor.h"

#include <exception>
#include <functional>
#include <iostream>

/// @brief Runs a small local simulation of scheduling and data processing.
/// @return Zero on success, or one if an exception is reported.
int main() {
    using namespace DistributedComputing;
    try {
        using Scheduler = TaskScheduler<std::function<int()>, int>;
        Scheduler scheduler(Scheduler::SchedulingPolicy{});
        Scheduler::WorkerNode worker;
        worker.nodeId = "worker-1";
        worker.ipAddress = "127.0.0.1";
        scheduler.registerWorkerNode(worker);
        worker.nodeId = "worker-2";
        scheduler.registerWorkerNode(worker);

        std::cout << "Local mock: no network operations\n";
        auto task = scheduler.scheduleTask([] { return 6 * 7; });
        std::cout << "Single task result: " << task.get() << '\n';
        auto batch = scheduler.scheduleBatch({[] { return 10; }, [] { return 20; }});
        std::cout << "Batch results:";
        for (auto& result : batch) std::cout << ' ' << result.get();
        std::cout << '\n';
        scheduler.updateNodeStatus("worker-1", 0.25, 0.0);
        scheduler.optimizeScheduling({});
        for (const auto& metric : scheduler.getClusterMetrics())
            std::cout << metric.first << ": " << metric.second << '\n';

        DistributedDataProcessor::ProcessingConfig config;
        config.chunkSize = 3;
        DistributedDataProcessor processor(config);
        auto processed = processor.processDistributed<int>(
            {1, 2, 3, 4}, [](const int& value) { return value * value; });
        std::cout << "Squared values:";
        for (int value : processed.get()) std::cout << ' ' << value;
        std::cout << '\n';

        std::vector<DistributedDataProcessor::DataPartition> partitions;
        processor.partitionData({1, 2, 3, 4, 5, 6, 7}, partitions);
        auto assigned = processor.redistributePartitions(partitions,
            {{"worker-1", 1.0}, {"worker-2", 1.0}});
        for (const auto& partition : assigned)
            std::cout << partition.partitionId << " [" << partition.startOffset << ", "
                << partition.endOffset << ") -> " << partition.metadata.at("workerNode") << '\n';
        processor.enableFaultTolerance();
        scheduler.unregisterWorkerNode("worker-2");
        std::cout << "Replication settings stored only; no data is replicated.\n";
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}

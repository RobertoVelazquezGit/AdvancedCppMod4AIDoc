/// @file TaskScheduler.h
/// @brief Header-only mock scheduler for local callable tasks.
#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

/// @brief Educational simulations of distributed computing components.
namespace DistributedComputing {

/// @brief Stores simulated workers and executes tasks locally on demand.
/// @tparam TaskType Copyable callable that takes no arguments.
/// @tparam ResultType Return type of the callable, including void.
/// @details Futures use deferred execution: get() or wait() executes the task
/// on the calling thread. No network communication or background workers exist.
template<typename TaskType, typename ResultType>
class TaskScheduler {
public:
    /// @brief Description of a simulated worker.
    struct WorkerNode {
        std::string nodeId; ///< Unique, nonempty identifier.
        std::string ipAddress; ///< Display-only address; no connection is made.
        int cpuCores = 1; ///< Positive number of simulated CPU cores.
        double memoryGB = 1.0; ///< Positive simulated memory capacity.
        double currentLoad = 0.0; ///< Simulated CPU load in the range [0, 1].
        std::chrono::system_clock::time_point lastHeartbeat{}; ///< Last local update.
    };

    /// @brief Configuration of the educational scheduler.
    struct SchedulingPolicy {
        /// @brief Available policy names; only RoundRobin is implemented.
        enum class LoadBalancing {
            RoundRobin, ///< Select workers in registration order, cyclically.
            LeastLoaded, ///< Reserved; the mock falls back to RoundRobin.
            WeightedRandom ///< Reserved; the mock falls back to RoundRobin.
        };
        /// @brief Reserved failure strategies; task errors always reach the future.
        enum class FailureHandling {
            Retry, ///< Reserved; no retry is performed.
            Redirect, ///< Reserved; no redirection is performed.
            Abort ///< Report the error through the future.
        };
        LoadBalancing balancingStrategy = LoadBalancing::RoundRobin; ///< Requested strategy.
        FailureHandling failureStrategy = FailureHandling::Abort; ///< Reserved failure setting.
        int maxRetries = 0; ///< Reserved; ignored by this mock.
        std::chrono::milliseconds timeout{1000}; ///< Reserved; no timeout is enforced.
    };

    /// @brief Creates an empty scheduler.
    /// @param policy Configuration retained by the mock.
    explicit TaskScheduler(const SchedulingPolicy& policy) : currentPolicy(policy) {}

    /// @brief Releases local state; returned futures own their callable copies.
    ~TaskScheduler() = default;

    /// @brief Registers a valid worker and records a local heartbeat.
    /// @param node Worker description.
    /// @return False for an invalid description or duplicate identifier.
    bool registerWorkerNode(const WorkerNode& node) {
        std::lock_guard<std::mutex> lock(nodesMutex);
        if (!validateNodeHealth(node) || std::any_of(availableNodes.begin(),
            availableNodes.end(), [&](const WorkerNode& item) { return item.nodeId == node.nodeId; }))
            return false;
        availableNodes.push_back(node);
        availableNodes.back().lastHeartbeat = std::chrono::system_clock::now();
        return true;
    }

    /// @brief Removes a worker; unknown identifiers are ignored.
    /// @param nodeId Identifier to remove.
    void unregisterWorkerNode(const std::string& nodeId) {
        std::lock_guard<std::mutex> lock(nodesMutex);
		availableNodes.erase(std::remove_if(availableNodes.begin(), availableNodes.end(),  // remove_if to work with predicate
            [&](const WorkerNode& node) { return node.nodeId == nodeId; }), availableNodes.end());
    }

    /// @brief Selects a mock worker and returns a deferred local task.
    /// @param task Callable copied into the future.
    /// @param constraints Reserved hints; ignored by the mock.
    /// @return Future holding the callable result or its exception.
    /// @throws std::runtime_error If no worker is registered.
    /// @note Selection is bookkeeping only; execution happens locally on get()/wait().
    std::future<ResultType> scheduleTask(const TaskType& task,
        const std::vector<std::string>& constraints = {}) {
        (void)constraints;
        {
            std::lock_guard<std::mutex> lock(nodesMutex);
            lastSelectedNode = selectOptimalNode();
            ++scheduledTasks;
        }
        return std::async(std::launch::deferred, task);
    }  

    // ToDo

    /// @brief Schedules each callable in input order.
    /// @param tasks Callables to copy into independent futures.
    /// @param globalConstraints Reserved hints; ignored by the mock.
    /// @return Futures in the same order as the input tasks.
    /// @throws std::runtime_error If tasks are nonempty and no worker exists.
    std::vector<std::future<ResultType>> scheduleBatch(const std::vector<TaskType>& tasks,
        const std::map<std::string, std::string>& globalConstraints = {}) {
        (void)globalConstraints;
        std::vector<std::future<ResultType>> results;
        results.reserve(tasks.size());
        for (const auto& task : tasks) results.push_back(scheduleTask(task));
        return results;
    }

    /// @brief Updates a known worker's CPU load and heartbeat.
    /// @param nodeId Worker identifier; unknown identifiers are ignored.
    /// @param cpuLoad New load in [0, 1].
    /// @param memoryUsage Reserved memory metric; ignored by the mock.
    /// @throws std::invalid_argument If cpuLoad is outside [0, 1] or NaN.
    void updateNodeStatus(const std::string& nodeId, double cpuLoad, double memoryUsage) {
        (void)memoryUsage;
        if (!(cpuLoad >= 0.0 && cpuLoad <= 1.0)) throw std::invalid_argument("Invalid CPU load");
        std::lock_guard<std::mutex> lock(nodesMutex);
        for (auto& node : availableNodes) {
            if (node.nodeId == nodeId) {
                node.currentLoad = cpuLoad;
                node.lastHeartbeat = std::chrono::system_clock::now();
                return;
            }
        }
    }

    /// @brief Returns a snapshot of local mock statistics.
    /// @return workerCount, averageLoad (zero when empty), and scheduledTasks.
    std::map<std::string, double> getClusterMetrics() const {
        std::lock_guard<std::mutex> lock(nodesMutex);
        double totalLoad = 0.0;
        for (const auto& node : availableNodes) totalLoad += node.currentLoad;
        return {{"workerCount", static_cast<double>(availableNodes.size())},
            {"averageLoad", availableNodes.empty() ? 0.0 : totalLoad / availableNodes.size()},
            {"scheduledTasks", static_cast<double>(scheduledTasks)}};
    }

    /// @brief Placeholder for future scheduling optimization; performs no action.
    /// @param performanceData Reserved performance measurements.
    void optimizeScheduling(const std::map<std::string, double>& performanceData) {
        (void)performanceData;
    }

private:
    /// @brief Selects the next worker in round-robin order.
    /// @return Selected identifier.
    /// @throws std::runtime_error If the worker list is empty.
    /// @pre nodesMutex is locked by the caller.
    std::string selectOptimalNode() {
        if (availableNodes.empty()) throw std::runtime_error("No worker nodes registered");
        nextWorker %= availableNodes.size();
        return availableNodes[nextWorker++].nodeId;
    }

    /// @brief Checks descriptive fields only; does not contact the worker.
    /// @param node Description to validate.
    /// @return Whether the identifier and resource values are valid.
    bool validateNodeHealth(const WorkerNode& node) const {
        return !node.nodeId.empty() && node.cpuCores > 0 && node.memoryGB > 0.0
            && node.currentLoad >= 0.0 && node.currentLoad <= 1.0;
    }

    std::vector<WorkerNode> availableNodes; ///< Locally registered worker descriptions.
    SchedulingPolicy currentPolicy; ///< Retained configuration; reserved settings are ignored.
    mutable std::mutex nodesMutex; ///< Protects worker state and statistics.
    std::size_t nextWorker = 0; ///< Next round-robin position.
    std::size_t scheduledTasks = 0; ///< Number of tasks submitted, not completed.
    std::string lastSelectedNode; ///< Most recently selected simulated worker.
};
}

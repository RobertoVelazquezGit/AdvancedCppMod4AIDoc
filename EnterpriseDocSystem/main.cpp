/// @file main.cpp
/// @brief Demonstrates the deployment mock, checks behavior and times six operations.
#include "EnterpriseDocSystem.h"

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <stdexcept>

using EnterpriseDocumentationSystem::DocumentationDeploymentManager;

namespace {
/// @brief Checks a condition in both Debug and Release builds.
/// @param condition Condition that must hold.
/// @param message Diagnostic used if the check fails.
/// @throws std::runtime_error If condition is false.
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

/// @brief Warms up and times repeated calls using std::chrono::steady_clock.
/// @tparam Operation Callable accepting an integer index and returning a size-compatible value.
/// @param name Label printed after the timed loop.
/// @param iterations Number of timed calls; must be positive.
/// @param operation Callable invoked for 1000 warm-up calls and then iterations timed calls.
/// @pre iterations > 0.
/// @details Warm-up changes the same state used by the timed loop. Output includes
/// total milliseconds, mean nanoseconds per call and a sum of returned values.
/// Printing is outside the timed region. Exceptions propagate to the caller.
/// @warning This is an illustrative microbenchmark: compiler optimization may remove
/// trivial work. The checksum does not guarantee that every state update is retained.
template <typename Operation>
void benchmark(const char* name, int iterations, Operation operation) {
    // Warm up allocations and code paths before taking a measurement.
    for (int i = 0; i < 1000; ++i) {
        operation(i);
    }
    std::size_t checksum = 0;
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        checksum += operation(i);
    }
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const double milliseconds = std::chrono::duration<double, std::milli>(elapsed).count();
    const double nanoseconds = std::chrono::duration<double, std::nano>(elapsed).count();
    std::cout << std::left << std::setw(26) << name
              << std::right << std::setw(12) << milliseconds << " ms total | "
              << std::setw(12) << nanoseconds / iterations << " ns/op | checksum="
              << checksum << '\n';
}
}

/// @brief Runs behavior checks, prints initial metrics and benchmarks the mock.
/// @details Uses instance bounds [2, 8], one mock cluster and two alert labels.
/// Checks rejection before deployment, invalid bounds, scaling, rollback, alerts
/// and the blue-green flag. Then performs 100000 timed iterations per case:
/// construction plus deployment, scaling, metric snapshots, alert configuration,
/// blue-green toggling, and scaling plus rollback.
/// @return 0 on completion; 1 when a caught standard exception reports failure.
/// @note Timings describe local mock operations only. Use Release builds when comparing runs.
int main() {
    try {
        const DocumentationDeploymentManager::DeploymentConfig deployment{
            "documentation", "mock://registry", {{"ENV", "benchmark"}},
            {"mock-cluster"}, true, 2, 8
        };
        const DocumentationDeploymentManager::MonitoringConfig monitoring{
            "mock://metrics", "info", {"mock-alerts"}, std::chrono::minutes{1}
        };
        DocumentationDeploymentManager manager(deployment, monitoring);
        const std::vector<std::string> alerts{"high_cpu", "unhealthy_instance"};

        // Check behavior explicitly, including in Release builds.
        require(!manager.scaleDocumentationProcessing(4), "Scaling before deployment must fail");
        require(!manager.rollbackToLastStableVersion(), "Rollback before deployment must fail");
        auto invalidDeployment = deployment;
        invalidDeployment.maxInstances = 1;
        DocumentationDeploymentManager invalidManager(invalidDeployment, monitoring);
        require(!invalidManager.deployDocumentationService(), "Invalid deployment must fail");
        require(manager.deployDocumentationService(), "Deployment failed");
        require(!manager.scaleDocumentationProcessing(9), "Out-of-range scaling must fail");
        require(manager.scaleDocumentationProcessing(6), "Scaling failed");
        require(manager.getSystemHealthMetrics().at("instances") == "6", "Unexpected scale state");
        require(manager.rollbackToLastStableVersion(), "Rollback failed");
        require(manager.getSystemHealthMetrics().at("instances") == "2", "Rollback did not restore state");
        manager.configurateAlerting(alerts);
        manager.enableBlueGreenDeployment();
        require(manager.getSystemHealthMetrics().at("blue_green") == "enabled", "Blue-green toggle failed");
        require(manager.getSystemHealthMetrics().at("alert_conditions") == "2", "Alerts not configured");
        std::cout << "Mock behavior checks passed.\nInitial metrics:\n";
        for (const auto& metric : manager.getSystemHealthMetrics()) {
            std::cout << "  " << metric.first << ": " << metric.second << '\n';
        }

        constexpr int iterations = 100000;
        std::cout << "\nIn-memory mock benchmark: " << iterations << " iterations per operation.\n"
                  << "No network, containers or real infrastructure are measured.\n"
                  << "Use Release builds for timing comparisons.\n\n" << std::fixed << std::setprecision(3);
        benchmark("Construct + deploy", iterations, [&](int) {
            DocumentationDeploymentManager fresh(deployment, monitoring);
            return static_cast<std::size_t>(fresh.deployDocumentationService());
        });
        benchmark("Scale", iterations, [&](int i) {
            return static_cast<std::size_t>(manager.scaleDocumentationProcessing(2 + i % 7));
        });
        benchmark("Read metrics", iterations, [&](int) {
            return manager.getSystemHealthMetrics().size();
        });
        benchmark("Configure alerts", iterations, [&](int) {
            manager.configurateAlerting(alerts);
            return alerts.size();
        });
        benchmark("Toggle blue-green", iterations, [&](int i) {
            manager.enableBlueGreenDeployment(i % 2 == 0);
            return std::size_t{1};
        });
        benchmark("Scale + rollback", iterations, [&](int) {
            const bool scaled = manager.scaleDocumentationProcessing(8);
            const bool restored = manager.rollbackToLastStableVersion();
            return static_cast<std::size_t>(scaled && restored);
        });
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Mock check failed: " << error.what() << '\n';
        return 1;
    }
}
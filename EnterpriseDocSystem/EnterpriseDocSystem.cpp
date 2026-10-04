/// @file EnterpriseDocSystem.cpp
/// @brief Implements the in-memory deployment state transitions documented in the header.
#include "EnterpriseDocSystem.h"

namespace EnterpriseDocumentationSystem {

DocumentationDeploymentManager::DocumentationDeploymentManager(
    const DeploymentConfig& deployConfig, const MonitoringConfig& monitorConfig)
    : deployConfig(deployConfig), monitorConfig(monitorConfig) {}

bool DocumentationDeploymentManager::validateDeploymentReadiness() {
    return !deployConfig.kubernetesNamespace.empty()
        && !deployConfig.containerRegistryUrl.empty()
        && !deployConfig.targetClusters.empty()
        && deployConfig.minInstances > 0
        && deployConfig.maxInstances >= deployConfig.minInstances
        && !monitorConfig.metricsEndpoint.empty()
        && monitorConfig.healthCheckInterval.count() > 0;
}

void DocumentationDeploymentManager::setupMonitoringInfrastructure() {
    monitoringReady = true;
}

void DocumentationDeploymentManager::configureLoadBalancing() {
    loadBalancingReady = currentInstances > 1;
}

bool DocumentationDeploymentManager::deployDocumentationService() {
    if (!validateDeploymentReadiness()) {
        return false;
    }
    // Repeated deployments are idempotent in this mock.
    if (deployed) {
        return true;
    }
    currentInstances = deployConfig.minInstances;
    stableInstances = currentInstances;
    setupMonitoringInfrastructure();
    configureLoadBalancing();
    deployed = true;
    return true;
}

bool DocumentationDeploymentManager::scaleDocumentationProcessing(int desiredInstances) {
    if (!deployed || desiredInstances < deployConfig.minInstances
        || desiredInstances > deployConfig.maxInstances) {
        return false;
    }
    // Manual scaling is allowed even when automatic scaling is disabled.
    currentInstances = desiredInstances;
    configureLoadBalancing();
    return true;
}

std::map<std::string, std::string>
DocumentationDeploymentManager::getSystemHealthMetrics() {
    return {
        {"status", deployed ? "healthy" : "not_deployed"},
        {"instances", std::to_string(currentInstances)},
        {"monitoring", monitoringReady ? "enabled" : "disabled"},
        {"load_balancing", loadBalancingReady ? "enabled" : "disabled"},
        {"blue_green", blueGreenEnabled ? "enabled" : "disabled"},
        {"autoscaling", deployConfig.enableAutoscaling ? "enabled" : "disabled"},
        {"alert_conditions", std::to_string(alertConditions.size())}
    };
}

void DocumentationDeploymentManager::configurateAlerting(
    const std::vector<std::string>& conditions) {
    alertConditions = conditions;
}

bool DocumentationDeploymentManager::rollbackToLastStableVersion() {
    if (!deployed) {
        return false;
    }
    // No real versions exist: restore the instance count from initial deployment.
    currentInstances = stableInstances;
    configureLoadBalancing();
    return true;
}

void DocumentationDeploymentManager::enableBlueGreenDeployment(bool enable) {
    blueGreenEnabled = enable;
}

} // namespace EnterpriseDocumentationSystem
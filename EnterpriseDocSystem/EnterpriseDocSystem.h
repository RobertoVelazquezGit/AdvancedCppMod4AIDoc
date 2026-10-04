#pragma once

#include <chrono>
#include <map>
#include <string>
#include <vector>

namespace EnterpriseDocumentationSystem {

    class DocumentationDeploymentManager {
    public:
        struct DeploymentConfig {
            std::string kubernetesNamespace;
            std::string containerRegistryUrl;
            std::map<std::string, std::string> environmentVariables;
            std::vector<std::string> targetClusters;
            bool enableAutoscaling;
            int minInstances;
            int maxInstances;
        };

        struct MonitoringConfig {
            std::string metricsEndpoint;
            std::string loggingLevel;
            std::vector<std::string> alertingChannels;
            std::chrono::minutes healthCheckInterval;
        };

    public:
        DocumentationDeploymentManager(const DeploymentConfig& deployConfig,
            const MonitoringConfig& monitorConfig);

        bool deployDocumentationService();
        bool scaleDocumentationProcessing(int desiredInstances);

        std::map<std::string, std::string> getSystemHealthMetrics();
        void configurateAlerting(const std::vector<std::string>& alertConditions);

        bool rollbackToLastStableVersion();
        void enableBlueGreenDeployment(bool enable = true);

    private:
        DeploymentConfig deployConfig;
        MonitoringConfig monitorConfig;

        bool deployed = false;
        bool monitoringReady = false;
        bool loadBalancingReady = false;
        bool blueGreenEnabled = false;
        int currentInstances = 0;
        int stableInstances = 0;
        std::vector<std::string> alertConditions;

        bool validateDeploymentReadiness();
        void setupMonitoringInfrastructure();
        void configureLoadBalancing();
    };

}
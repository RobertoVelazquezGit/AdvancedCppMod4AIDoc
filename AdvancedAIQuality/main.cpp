/// @file main.cpp
/// @brief Small example of both documentation quality mocks.
#include "DocumentationQualityFramework.h"

#include <iostream>

/// @brief Analyzes a fixture, processes one request and simulates publication.
/// @return Zero on success, one if pipeline initialization or processing fails.
int main() {
    using namespace DocumentationQualityFramework;

    AIDocumentationAnalyzer::ValidationConfig validation{};
    validation.enableSemanticAnalysis = true;
    validation.validateCodeExamples = true;
    validation.requiredSections = { "Overview", "Usage", "Examples" };
    AIDocumentationAnalyzer analyzer(validation);

    const auto report = analyzer.analyzeDocumentation("example.md");
    std::cout << "Mock completeness: " << report.overallMetrics.completenessScore << '\n';
    for (const auto& suggestion : report.improvementSuggestions)
        std::cout << suggestion << '\n';
    std::cout << std::boolalpha << "Mock technical validation: "
        << analyzer.validateTechnicalAccuracy("Example documentation", "example.cpp") << '\n';

    ContinuousDocumentationPipeline::PipelineConfig settings{};
    settings.repositoryPath = "sample-repository";
    settings.outputDirectory = "sample-docs";
    ContinuousDocumentationPipeline pipeline(settings);
    if (!pipeline.initializePipeline()) return 1;
    pipeline.startContinuousMonitoring();

    ContinuousDocumentationPipeline::GenerationTask task{};
    task.sourceFilePath = "example.cpp";
    task.documentationType = "API reference";
    std::cout << "Scheduled: " << pipeline.scheduleDocumentationGeneration(task) << '\n';
    if (!pipeline.processGenerationQueue()) return 1;
    pipeline.publishDocumentation("1.0-demo", { "local-preview" });
    pipeline.stopPipeline();
    return 0;
}

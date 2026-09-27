#include "MultiModalDoc.h"

#include <exception>
#include <iostream>

using namespace MultiModalDocumentation;

int main()
{
    try {
        // Generate mock documentation for one audience and one format.
        DocumentationOrchestrator orchestrator;
        orchestrator.registerAIProvider(
            DocumentationOrchestrator::AIProvider::GPT4, "mock-config");

        DocumentationOrchestrator::DocumentationRequest request{};
        request.sourceCodePath = "example.cpp";
        request.preferredProvider = DocumentationOrchestrator::AIProvider::GPT4;
        request.targetAudiences = { DocumentationOrchestrator::AudienceType::Developers };
        request.requiredFormats = { DocumentationOrchestrator::OutputFormat::Markdown };
        request.qualityThreshold = 70;

        const auto result = orchestrator.generateDocumentation(request).get();
        std::cout << "=== Mock documentation ===\n"
                  << result.generatedContent.at(DocumentationOrchestrator::OutputFormat::Markdown)
                  << "\nQuality: " << result.overallQualityScore << "/100\n";

        // Build a prompt using an in-memory mock template.
        IntelligentPromptManager promptManager;
        promptManager.loadPromptTemplates("mock-templates");

        const auto prompt = promptManager.generateContextualPrompt(
            "int add(int a, int b) { return a + b; }",
            DocumentationOrchestrator::AudienceType::Developers,
            { "Explain the parameters and return value." });

        std::cout << "\n=== Generated prompt ===\n" << prompt << '\n';
    }
    catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

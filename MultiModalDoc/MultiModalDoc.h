#pragma once

#include <chrono>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace MultiModalDocumentation {

    class DocumentationOrchestrator {
    public:
        enum class AIProvider {
            GPT4, Claude, Gemini, CodeLlama, Custom
        };

        enum class OutputFormat {
            Markdown, HTML, PDF, LaTeX, DocBook, Confluence, GitHubWiki
        };

        enum class AudienceType {
            Developers, Architects, QA, DevOps, EndUsers, Executives
        };

        struct DocumentationRequest {
            std::string sourceCodePath;
            std::vector<AudienceType> targetAudiences;
            std::vector<OutputFormat> requiredFormats;
            AIProvider preferredProvider;
            std::map<std::string, std::string> customParameters;
            int qualityThreshold; // 0-100
        };

        struct GenerationResult {
            std::string requestId;
            std::map<OutputFormat, std::string> generatedContent;
            std::map<AudienceType, double> audienceScores;
            double overallQualityScore;
            std::vector<std::string> warnings;
            std::chrono::milliseconds generationTime;
        };

    public:
        DocumentationOrchestrator();
        ~DocumentationOrchestrator();

        bool registerAIProvider(AIProvider provider, const std::string& configuration);

        std::future<GenerationResult> generateDocumentation(const DocumentationRequest& request);

        std::vector<GenerationResult> batchGenerate(
            const std::vector<DocumentationRequest>& requests);

        bool optimizeForAudience(const std::string& content,
            AudienceType audience,
            std::string& optimizedContent);

        void enableQualityGating(double minimumScore, bool blockLowQuality = true);

    private:
        std::map<AIProvider, std::unique_ptr<class AIDocumentationEngine>> engines;
        std::unique_ptr<class ContentOptimizer> optimizer;
        std::unique_ptr<class FormatConverter> converter;

        std::string selectOptimalProvider(const DocumentationRequest& request);
        bool validateOutputQuality(const GenerationResult& result);
        void applyAudienceOptimizations(GenerationResult& result);
    };

    class IntelligentPromptManager {
    public:
        struct PromptTemplate {
            std::string templateId;
            std::string basePrompt;
            std::map<std::string, std::string> variableSubstitutions;
            std::vector<std::string> contextualHints;
            AudienceType targetAudience;
            double effectivenessScore;
        };

        struct PromptOptimizationResult {
            std::string optimizedPrompt;
            double expectedQuality;
            std::vector<std::string> appliedOptimizations;
            std::map<std::string, double> confidenceMetrics;
        };

    public:
        bool loadPromptTemplates(const std::string& templatesPath);

        std::string generateContextualPrompt(const std::string& codeContext,
            AudienceType audience,
            const std::vector<std::string>& requirements);

        PromptOptimizationResult optimizePrompt(const std::string& basePrompt,
            const std::string& codeContext,
            const std::vector<std::string>& qualityFeedback);

        void updateTemplateEffectiveness(const std::string& templateId,
            double qualityScore,
            const std::string& feedback);

    private:
        std::vector<PromptTemplate> templates;
        std::unique_ptr<class PromptAnalyzer> analyzer;
        std::unique_ptr<class EffectivenessTracker> tracker;

        std::string selectBestTemplate(const std::string& context, AudienceType audience);
        std::vector<std::string> extractContextualElements(const std::string& code);
        double predictPromptEffectiveness(const std::string& prompt);
    };

}
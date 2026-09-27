#pragma once

#include <chrono>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

/// @brief In-memory mocks for documentation generation and prompt preparation.
namespace MultiModalDocumentation {

    class AIDocumentationEngine;
    class ContentOptimizer;
    class FormatConverter;
    class PromptAnalyzer;
    class EffectivenessTracker;

    /// @brief Coordinates simulated documentation generation for multiple audiences and formats.
    /// @details No files are read and no AI services are contacted. Scores are fixed at 80/100.
    /// All formats, including PDF, contain plain-text placeholders. Instances are not thread-safe.
    class DocumentationOrchestrator {
    public:
        /// @brief Provider identifiers used only to select registered mock engines.
        enum class AIProvider {
            GPT4, Claude, Gemini, CodeLlama, Custom
        };

        /// @brief Requested output labels; the mock does not produce actual encoded formats.
        enum class OutputFormat {
            Markdown, HTML, PDF, LaTeX, DocBook, Confluence, GitHubWiki
        };

        /// @brief Intended readership of generated documentation.
        enum class AudienceType {
            Developers, Architects, QA, DevOps, EndUsers, Executives
        };

        /// @brief Inputs for a simulated generation request.
        /// @note Value-initialize this structure and explicitly select the provider and threshold.
        struct DocumentationRequest {
            /// @brief Nonempty source label included in the output; the path is never opened.
            std::string sourceCodePath;
            /// @brief Audiences appended as labels to each output; may be empty.
            std::vector<AudienceType> targetAudiences;
            /// @brief Required format labels; at least one is required and duplicates are collapsed.
            std::vector<OutputFormat> requiredFormats;
            /// @brief Preferred registered provider; otherwise the first provider in map order is used.
            AIProvider preferredProvider;
            /// @brief Reserved parameters, ignored by the mock with a warning when nonempty.
            std::map<std::string, std::string> customParameters;
            /// @brief Request-specific minimum score in [0, 100].
            int qualityThreshold; // 0-100
        };

        /// @brief Simulated content, quality metrics, warnings, and elapsed generation time.
        struct GenerationResult {
            /// @brief Process-local identifier formed from a shared increasing mock counter.
            std::string requestId;
            /// @brief Plain-text placeholders by format; cleared when quality gating blocks output.
            std::map<OutputFormat, std::string> generatedContent;
            /// @brief Fixed score of 80 for each distinct requested audience.
            std::map<AudienceType, double> audienceScores;
            /// @brief Fixed simulated quality score of 80 on a 0-100 scale.
            double overallQualityScore;
            /// @brief Mock limitations, provider fallback, and quality-gating messages.
            std::vector<std::string> warnings;
            /// @brief Measured local generation duration, which may round down to zero.
            std::chrono::milliseconds generationTime;
        };

    public:
        /// @brief Creates mock helpers with no registered providers and nonblocking quality gating.
        DocumentationOrchestrator();
        /// @brief Releases registered engines and mock helpers.
        ~DocumentationOrchestrator();

        /// @brief Registers a mock engine without contacting an external service.
        /// @param provider Identifier to register.
        /// @param configuration Nonempty placeholder; its contents are neither parsed nor stored.
        /// @return True if inserted; false for empty configuration or an existing identifier.
        bool registerAIProvider(AIProvider provider, const std::string& configuration);

        /// @brief Computes mock documentation immediately and returns a ready future.
        /// @param request Source label, audiences, formats, provider, and quality threshold.
        /// @return A ready future containing the result or a captured exception.
        /// @details The preferred provider is used if registered; otherwise the first map entry is used.
        /// A score below either threshold adds a warning and clears content only if blocking is enabled.
        /// @note Calling get() rethrows std::invalid_argument for an empty path, no formats, or an
        /// out-of-range threshold, and std::runtime_error when no provider is registered.
        /// No background task is started, and the future does not retain this object.
        std::future<GenerationResult> generateDocumentation(const DocumentationRequest& request);

        /// @brief Generates requests sequentially in input order.
        /// @param requests Requests to execute; may be empty.
        /// @return One result per request on success.
        /// @throws std::exception Propagates the first generation failure; no partial vector is returned.
        std::vector<GenerationResult> batchGenerate(
            const std::vector<DocumentationRequest>& requests);

        /// @brief Appends an audience label to nonempty content.
        /// @param content Text to adapt.
        /// @param audience Audience whose label is appended.
        /// @param[out] optimizedContent Updated text on success; unchanged on failure.
        /// @return False for empty content, otherwise true.
        bool optimizeForAudience(const std::string& content,
            AudienceType audience,
            std::string& optimizedContent);

        /// @brief Configures the global mock quality threshold and blocking policy.
        /// @param minimumScore Finite minimum score in [0, 100].
        /// @param blockLowQuality Whether to clear content below either global or request threshold.
        /// @throws std::invalid_argument If minimumScore is nonfinite or outside [0, 100].
        void enableQualityGating(double minimumScore, bool blockLowQuality = true);

    private:
        /// @brief Owned mock engines indexed by provider identifier.
        std::map<AIProvider, std::unique_ptr<AIDocumentationEngine>> engines;
        /// @brief Audience-label helper and storage for the global quality policy.
        std::unique_ptr<ContentOptimizer> optimizer;
        /// @brief Helper that prefixes content with a mock format label.
        std::unique_ptr<FormatConverter> converter;

        /// @brief Selects a provider name without performing any optimization.
        /// @param request Request containing the preferred provider.
        /// @return Preferred registered provider name, first map entry name, or empty if none exist.
        std::string selectOptimalProvider(const DocumentationRequest& request);
        /// @brief Checks only the configured global quality threshold.
        /// @param result Result whose overall score is checked.
        /// @return True if the score meets the global minimum.
        bool validateOutputQuality(const GenerationResult& result);
        /// @brief Appends every requested audience label to every generated format.
        /// @param[in,out] result Content to update using its audienceScores keys.
        void applyAudienceOptimizations(GenerationResult& result);
    };

    /// @brief Builds prompts from in-memory templates using simple string concatenation.
    /// @details No template files or AI services are used. Quality and confidence are simulated.
    /// Instances are not thread-safe.
    class IntelligentPromptManager {
    public:
        /// @brief Creates mock helpers with an initially empty template collection.
        IntelligentPromptManager();
        /// @brief Releases the analyzer, tracker, and stored templates.
        ~IntelligentPromptManager();

        /// @brief In-memory prompt template and its manually assigned effectiveness score.
        struct PromptTemplate {
            /// @brief Template identifier, such as mock-Developers or mock-QA.
            std::string templateId;
            /// @brief Initial prompt text describing the intended audience.
            std::string basePrompt;
            /// @brief Reserved substitutions; unused by the mock.
            std::map<std::string, std::string> variableSubstitutions;
            /// @brief Reserved contextual hints; unused by the mock.
            std::vector<std::string> contextualHints;
            /// @brief Audience used when selecting this template.
            DocumentationOrchestrator::AudienceType targetAudience;
            /// @brief Selection score in [0, 100], initially 80 for loaded mock templates.
            double effectivenessScore;
        };

        /// @brief Prompt assembled from supplied text and simulated evaluation metrics.
        struct PromptOptimizationResult {
            /// @brief Base prompt with optional code context and feedback appended.
            std::string optimizedPrompt;
            /// @brief Simulated score: zero for empty output, otherwise 80.
            double expectedQuality;
            /// @brief Descriptions of the context and feedback additions performed.
            std::vector<std::string> appliedOptimizations;
            /// @brief Contains mockConfidence: zero for empty output, otherwise 0.8.
            std::map<std::string, double> confidenceMetrics;
        };

    public:
        /// @brief Replaces stored templates with one fixed template for each of the six audiences.
        /// @param templatesPath Nonempty placeholder path; no file is opened.
        /// @return False for an empty path, leaving state unchanged; otherwise true.
        /// @details Successful loading resets template scores to 80 and clears tracked feedback.
        bool loadPromptTemplates(const std::string& templatesPath);

        /// @brief Combines an audience template, code context, and requirements.
        /// @param codeContext Code text appended verbatim.
        /// @param audience Audience used to choose the highest-scoring matching template.
        /// @param requirements Requirement strings appended in input order.
        /// @return Assembled prompt; a generic audience prompt is used if no template matches.
        std::string generateContextualPrompt(const std::string& codeContext,
            DocumentationOrchestrator::AudienceType audience,
            const std::vector<std::string>& requirements);

        /// @brief Appends context and feedback to a prompt without semantic analysis.
        /// @param basePrompt Initial text; may be empty.
        /// @param codeContext Code text appended only when nonempty.
        /// @param qualityFeedback Feedback entries appended in input order.
        /// @return Assembled text, descriptions of additions, and fixed mock metrics.
        PromptOptimizationResult optimizePrompt(const std::string& basePrompt,
            const std::string& codeContext,
            const std::vector<std::string>& qualityFeedback);

        /// @brief Replaces a template score and stores its latest feedback.
        /// @param templateId Identifier of a loaded template.
        /// @param qualityScore Finite score in [0, 100].
        /// @param feedback Feedback stored in memory; not used for prompt generation.
        /// @throws std::invalid_argument If the score is invalid or the identifier is unknown.
        void updateTemplateEffectiveness(const std::string& templateId,
            double qualityScore,
            const std::string& feedback);

    private:
        /// @brief Templates available for audience-based selection.
        std::vector<PromptTemplate> templates;
        /// @brief Helper providing fixed mock quality scores.
        std::unique_ptr<PromptAnalyzer> analyzer;
        /// @brief Storage for the latest feedback associated with each template.
        std::unique_ptr<EffectivenessTracker> tracker;

        /// @brief Finds the highest-scoring template for an audience; ties keep the first match.
        /// @param context Reserved context, ignored by this mock.
        /// @param audience Audience to match.
        /// @return Matching base prompt, or an empty string if no template matches.
        std::string selectBestTemplate(const std::string& context, DocumentationOrchestrator::AudienceType audience);
        /// @brief Wraps nonempty code as a single contextual element without parsing.
        /// @param code Source text to wrap.
        /// @return Empty vector for empty code, otherwise a one-element vector containing code.
        std::vector<std::string> extractContextualElements(const std::string& code);
        /// @brief Retrieves a fixed mock effectiveness score.
        /// @param prompt Text to evaluate for emptiness only.
        /// @return Zero for an empty prompt, otherwise 80.
        double predictPromptEffectiveness(const std::string& prompt);
    };

}
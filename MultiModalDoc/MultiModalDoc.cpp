#include "MultiModalDoc.h"

#include <atomic>
#include <cmath>
#include <stdexcept>

namespace MultiModalDocumentation {

    // This unnamed namespace gives the helper functions internal linkage,
    // like static free functions: they can only be used within this translation unit.
    namespace {
        // These type aliases can also be used in the enclosing MultiModalDocumentation
        // namespace after their declarations, within this translation unit.
        // They do not create new types or change the linkage of the original types.
        using Audience = DocumentationOrchestrator::AudienceType;
        using Provider = DocumentationOrchestrator::AIProvider;

        std::string audienceName(Audience audience) {
            switch (audience) {
            case Audience::Developers: return "Developers";
            case Audience::Architects: return "Architects";
            case Audience::QA: return "QA";
            case Audience::DevOps: return "DevOps";
            case Audience::EndUsers: return "End users";
            case Audience::Executives: return "Executives";
            }
            return "Unknown audience";
        }

        std::string providerName(Provider provider) {
            switch (provider) {
            case Provider::GPT4: return "GPT4";
            case Provider::Claude: return "Claude";
            case Provider::Gemini: return "Gemini";
            case Provider::CodeLlama: return "CodeLlama";
            case Provider::Custom: return "Custom";
            }
            return "Unknown provider";
        }
    } // namespace (anonymous)

    // Mock helpers: no network access, file parsing, or real format conversion.
    /// @brief Produces a fixed source description without reading files or calling AI.
    class AIDocumentationEngine {
    public:
        /// @brief Creates placeholder documentation.
        /// @param path Source label embedded verbatim in the text.
        /// @return Fixed description prefixed with the supplied source label.
        std::string generate(const std::string& path) const {
            return "Mock documentation for: " + path + "\nOverview: simulated source description.\n";
        }
    };

    /// @brief Appends audience labels and stores the mock quality-gating policy.
    class ContentOptimizer {
    public:
        /// @brief Global minimum quality score, initially zero.
        double minimumScore = 0.0;
        /// @brief Whether failing either quality threshold clears generated content.
        bool blockLowQuality = false;

        /// @brief Adds an audience label without rewriting the content.
        /// @param content Original text.
        /// @param audience Audience to label.
        /// @return Original text followed by the audience name.
        std::string optimize(const std::string& content, Audience audience) const {
            return content + "\nAudience: " + audienceName(audience);
        }
    };

    /// @brief Adds format labels to plain text; does not create real PDF, HTML, or other formats.
    class FormatConverter {
    public:
        /// @brief Prefixes text with the numeric mock format identifier.
        /// @param content Text to label.
        /// @param format Requested format identifier.
        /// @return Labeled plain-text placeholder.
        std::string convert(const std::string& content,
            DocumentationOrchestrator::OutputFormat format) const {
            return "[Mock format " + std::to_string(static_cast<int>(format)) + "]\n" + content;
        }
    };

    /// @brief Returns a fixed score based only on whether a prompt is empty.
    class PromptAnalyzer {
    public:
        /// @brief Computes the mock score.
        /// @param prompt Text checked for emptiness.
        /// @return Zero for empty text, otherwise 80.
        double score(const std::string& prompt) const {
            return prompt.empty() ? 0.0 : 80.0;
        }
    };

    /// @brief Stores template feedback in memory without learning or persistence.
    class EffectivenessTracker {
    public:
        /// @brief Latest feedback string indexed by template identifier.
        std::map<std::string, std::string> feedback;
    };

    DocumentationOrchestrator::DocumentationOrchestrator()
        : optimizer(std::make_unique<ContentOptimizer>()),
          converter(std::make_unique<FormatConverter>()) {
    }

    DocumentationOrchestrator::~DocumentationOrchestrator() = default;

    bool DocumentationOrchestrator::registerAIProvider(
        AIProvider provider, const std::string& configuration) {
        if (configuration.empty()) return false;
        // emplace returns a pair: first is an iterator to the inserted or existing element;
        // second is true if a new element was inserted, or false if the provider already exists.
        // It does not test whether the pointer is non-null. An existing engine is preserved.
        return engines.emplace(provider, std::make_unique<AIDocumentationEngine>()).second;
    }

    std::future<DocumentationOrchestrator::GenerationResult>
        DocumentationOrchestrator::generateDocumentation(const DocumentationRequest& request) {
        const auto start = std::chrono::steady_clock::now();
        std::promise<GenerationResult> promise;
        auto future = promise.get_future();  // ToDo
        // Compute immediately and return a ready future; no background thread is needed.
        try {
            if (request.sourceCodePath.empty() || request.requiredFormats.empty())
                throw std::invalid_argument("A source path and at least one format are required.");
            if (request.qualityThreshold < 0 || request.qualityThreshold > 100)
                throw std::invalid_argument("Quality threshold must be between 0 and 100.");

            const auto provider = selectOptimalProvider(request);
            if (provider.empty()) throw std::runtime_error("No mock provider registered.");
            auto engine = engines.find(request.preferredProvider);
            if (engine == engines.end()) engine = engines.begin();

            static std::atomic<unsigned long long> nextId{ 0 };
            GenerationResult result{};
            result.requestId = "mock-" + std::to_string(++nextId);
            result.overallQualityScore = 80.0;
            result.warnings.push_back("Mock output: no source files or AI services were accessed.");
            result.warnings.push_back("Formats are text placeholders, including PDF; scores are simulated.");
            if (engine->first != request.preferredProvider)
                result.warnings.push_back("Preferred provider unavailable; using " + provider + ".");
            if (!request.customParameters.empty())
                result.warnings.push_back("Custom parameters are ignored by this mock.");

            const auto content = "Provider: " + provider + "\n" +
                engine->second->generate(request.sourceCodePath);
            for (const auto format : request.requiredFormats)
                result.generatedContent[format] = converter->convert(content, format);
            for (const auto audience : request.targetAudiences)
                result.audienceScores[audience] = 80.0;
            applyAudienceOptimizations(result);

            if (result.overallQualityScore < request.qualityThreshold || !validateOutputQuality(result)) {
                result.warnings.push_back("Simulated quality is below the required threshold.");
                if (optimizer->blockLowQuality) {
                    result.generatedContent.clear();
                    result.warnings.push_back("Generated content was blocked by quality gating.");
                }
            }
            result.generationTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);
            promise.set_value(std::move(result));
        }
        catch (...) {
            promise.set_exception(std::current_exception());
        }
        return future;
    }

    std::vector<DocumentationOrchestrator::GenerationResult>
        DocumentationOrchestrator::batchGenerate(const std::vector<DocumentationRequest>& requests) {
        std::vector<GenerationResult> results;
        for (const auto& request : requests)
            results.push_back(generateDocumentation(request).get());
        return results;
    }

    bool DocumentationOrchestrator::optimizeForAudience(
        const std::string& content, AudienceType audience, std::string& optimizedContent) {
        if (content.empty()) return false;
        optimizedContent = optimizer->optimize(content, audience);
        return true;
    }

    void DocumentationOrchestrator::enableQualityGating(double minimumScore, bool blockLowQuality) {
        if (!std::isfinite(minimumScore) || minimumScore < 0.0 || minimumScore > 100.0)
            throw std::invalid_argument("Minimum score must be between 0 and 100.");
        optimizer->minimumScore = minimumScore;
        optimizer->blockLowQuality = blockLowQuality;
    }

    std::string DocumentationOrchestrator::selectOptimalProvider(const DocumentationRequest& request) {
        if (engines.empty()) return {};
        auto engine = engines.find(request.preferredProvider);
        if (engine == engines.end()) engine = engines.begin();
        return providerName(engine->first);
    }

    bool DocumentationOrchestrator::validateOutputQuality(const GenerationResult& result) {
        return result.overallQualityScore >= optimizer->minimumScore;
    }

    void DocumentationOrchestrator::applyAudienceOptimizations(GenerationResult& result) {
        for (auto& content : result.generatedContent)
            for (const auto& audience : result.audienceScores)
                content.second = optimizer->optimize(content.second, audience.first);
    }

    IntelligentPromptManager::IntelligentPromptManager()
        : analyzer(std::make_unique<PromptAnalyzer>()),
          tracker(std::make_unique<EffectivenessTracker>()) {
    }

    IntelligentPromptManager::~IntelligentPromptManager() = default;

    bool IntelligentPromptManager::loadPromptTemplates(const std::string& templatesPath) {
        if (templatesPath.empty()) return false;
        // The path is only a mock input; templates are created in memory.
        templates.clear();
        tracker->feedback.clear();
        for (const auto audience : { Audience::Developers, Audience::Architects, Audience::QA,
            Audience::DevOps, Audience::EndUsers, Audience::Executives }) {
            PromptTemplate item{};
            item.templateId = "mock-" + audienceName(audience);
            item.basePrompt = "Document this code for " + audienceName(audience) + ".";
            item.targetAudience = audience;
            item.effectivenessScore = 80.0;
            templates.push_back(std::move(item));
        }
        return true;
    }

    std::string IntelligentPromptManager::generateContextualPrompt(
        const std::string& codeContext, Audience audience,
        const std::vector<std::string>& requirements) {
        auto prompt = selectBestTemplate(codeContext, audience);
        if (prompt.empty()) prompt = "Document this code for " + audienceName(audience) + ".";
        prompt += "\nCode context:\n" + codeContext;
        for (const auto& requirement : requirements) prompt += "\nRequirement: " + requirement;
        return prompt;
    }

    IntelligentPromptManager::PromptOptimizationResult IntelligentPromptManager::optimizePrompt(
        const std::string& basePrompt, const std::string& codeContext,
        const std::vector<std::string>& qualityFeedback) {
        PromptOptimizationResult result{};
        result.optimizedPrompt = basePrompt;
        for (const auto& element : extractContextualElements(codeContext)) {
            result.optimizedPrompt += "\nCode context:\n" + element;
            result.appliedOptimizations.push_back("Added code context (mock).");
        }
        for (const auto& feedback : qualityFeedback) {
            result.optimizedPrompt += "\nFeedback: " + feedback;
            result.appliedOptimizations.push_back("Appended feedback (mock).");
        }
        result.expectedQuality = predictPromptEffectiveness(result.optimizedPrompt);
        result.confidenceMetrics["mockConfidence"] = result.optimizedPrompt.empty() ? 0.0 : 0.8;
        return result;
    }

    void IntelligentPromptManager::updateTemplateEffectiveness(
        const std::string& templateId, double qualityScore, const std::string& feedback) {
        if (!std::isfinite(qualityScore) || qualityScore < 0.0 || qualityScore > 100.0)
            throw std::invalid_argument("Quality score must be between 0 and 100.");
        for (auto& item : templates) {
            if (item.templateId == templateId) {
                item.effectivenessScore = qualityScore;
                tracker->feedback[templateId] = feedback;
                return;
            }
        }
        throw std::invalid_argument("Unknown mock template: " + templateId);
    }

    std::string IntelligentPromptManager::selectBestTemplate(const std::string& context, Audience audience) {
        // Context is intentionally ignored by this simple audience-based selection.
        (void)context;
        const PromptTemplate* best = nullptr;
        for (const auto& item : templates)
            if (item.targetAudience == audience && (!best || item.effectivenessScore > best->effectivenessScore))
                best = &item;
        return best ? best->basePrompt : std::string{};
    }

    std::vector<std::string> IntelligentPromptManager::extractContextualElements(const std::string& code) {
        if (code.empty()) return {};
        return { code };
    }

    double IntelligentPromptManager::predictPromptEffectiveness(const std::string& prompt) {
        return analyzer->score(prompt);
    }
} // namespace MultiModalDocumentation

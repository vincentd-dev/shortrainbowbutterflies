#ifndef HELPER_H // Prevents multiple inclusion of helper.h
#define HELPER_H // Marks HELPER_H as defined

#include <string> // Provides std::string for optional model configuration

// AI INTEGRATION (RULE-BASED + LOCAL OLLAMA ASSISTANT):
// Answers engineering & study formula/concept prompts using a local Ollama model
// when available (http://localhost:11434/api/chat via curl) and falls back to
// built-in offline formula/study rules when offline or if Ollama is inactive.
void runFormulaAssistantModule(const std::string& model = "llama3.2");

#endif // End of HELPER_H
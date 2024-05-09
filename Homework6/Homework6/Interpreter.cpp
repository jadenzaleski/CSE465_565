//
//  Interpreter.cpp
//  Homework6
//
//  Created by Jaden Zaleski on 5/8/24.
//

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <variant>
#include "Lexer.cpp"

using VariableValue = std::variant<int, std::string>;

class Interpreter {
public:
    Interpreter(const std::vector<Token>& tokens) : tokens(tokens), structuredTokens(structureTokens(tokens)), currentIndex(0) {}
    
    void run() {
        size_t loopCount = 0;
        size_t i = 0;
        
        while (i < structuredTokens.size()) {
            const auto& line = structuredTokens[i];
            
            if (line.size() == 3 && line[0].type == TokenType::Print) {
                handlePrint(line);
            } else if (line.size() == 4 && (line[1].type == TokenType::Assign ||
                                            line[1].type == TokenType::PlusAssign ||
                                            line[1].type == TokenType::MinusAssign ||
                                            line[1].type == TokenType::MultAssign)) {
                handleAssignment(line);
            } else if (line.size() == 2 && line[0].type == TokenType::For) {
                // Capture the FOR loop's starting point and loop count
                if (structuredTokens[i][1].type == TokenType::Number) {
                    // If loop count is a number
                    loopCount = std::stoi(structuredTokens[i][1].value);
                } else if (structuredTokens[i][1].type == TokenType::Identifier) {
                    const std::string& varName = structuredTokens[i][1].value;
                    
                    if (variables.find(varName) != variables.end() && std::holds_alternative<int>(variables[varName])) {
                        loopCount = std::get<int>(variables[varName]); // Retrieve the loop count from a variable
                    } else {
                        std::cerr << "Error: Loop count variable '" << varName << "' not found or is not an integer." << std::endl;
                        break;
                    }
                } else {
                    std::cerr << "Error: Invalid loop count." << std::endl;
                    break;
                }
                std::vector<std::vector<Token>> loopBody(structuredTokens.begin() + i + 1, structuredTokens.begin() + i + loopCount);
                executeLoop(loopBody, loopCount); // Run the loop
                i += loopBody.size(); // Skip over the loop content
            } 
            
            ++i; // Move to the next line
        }
    }
    
    
    
private:
    const std::vector<Token>& tokens;
    size_t currentIndex;
    std::unordered_map<std::string, VariableValue> variables;
    std::vector<std::vector<Token>> structuredTokens;
    
    // Function to handle assignments
    void handleAssignment(const std::vector<Token>& line) {
        const std::string varName = line[0].value;
        const auto& operatorToken = line[1];
        const auto& valueToken = line[2];
        
        if (operatorToken.type == TokenType::Assign) {
            if (valueToken.type == TokenType::Number && isNumeric(valueToken.value)) {
                int newValue = std::stoi(valueToken.value);
                variables[varName] = newValue;
            } else if (valueToken.type == TokenType::Identifier && variables.find(valueToken.value) != variables.end()) {
                const VariableValue& sourceValue = variables.at(valueToken.value);
                variables[varName] = sourceValue;
            } else {
                std::cerr << "Error: Invalid assignment." << std::endl;
            }
        } else if (operatorToken.type == TokenType::PlusAssign) {
            if (variables.find(varName) != variables.end() && std::holds_alternative<int>(variables[varName])) {
                int existingValue = std::get<int>(variables[varName]);
                
                if (isNumeric(valueToken.value)) {
                    int additionValue = std::stoi(valueToken.value);
                    variables[varName] = existingValue + additionValue;
                } else if (variables.find(valueToken.value) != variables.end() && std::holds_alternative<int>(variables[valueToken.value])) {
                    int additionValue = std::get<int>(variables[valueToken.value]);
                    variables[varName] = existingValue + additionValue;
                } else {
                    std::cerr << "Error: Invalid addition value." << std::endl;
                }
            }
        } else if (operatorToken.type == TokenType::MinusAssign) {
            if (variables.find(varName) != variables.end() && std::holds_alternative<int>(variables[varName])) {
                int existingValue = std::get<int>(variables[varName]);
                if (isNumeric(valueToken.value)) {
                    int subtractionValue = std::stoi(valueToken.value);
                    variables[varName] = existingValue - subtractionValue;
                } else if (variables.find(valueToken.value) != variables.end() && std::holds_alternative<int>(variables[valueToken.value])) {
                    int subtractionValue = std::get<int>(variables[valueToken.value]);
                    variables[varName] = existingValue - subtractionValue;
                } else {
                    std::cerr << "Error: Invalid variable for subtraction." << std::endl;
                }
            }
        }
    }
    
    // Function to handle PRINT operations
    void handlePrint(const std::vector<Token>& line) {
        if (line[1].type == TokenType::Identifier) {
            const auto& variableName = line[1].value;
            if (variables.find(variableName) != variables.end()) {
                const auto& variableValue = variables.at(variableName);
                if (std::holds_alternative<int>(variableValue)) {
                    std::cout << variableName << " = " << std::get<int>(variableValue) << std::endl;
                } else if (std::holds_alternative<std::string>(variableValue)) {
                    std::cout << variableName << " = " << std::get<std::string>(variableValue) << std::endl;
                }
            } else {
                std::cerr << "Variable not found: " << variableName << std::endl;
            }
        }
    }
    
    // Function to execute a FOR loop
    void executeLoop(const std::vector<std::vector<Token>>& loopBody, size_t loopCount) {
        for (size_t j = 0; j < loopCount; ++j) {
            for (const auto& line : loopBody) {
                // Execute the loop body according to its structure
                if (line.size() == 4 && (line[1].type == TokenType::Assign ||
                                         line[1].type == TokenType::PlusAssign ||
                                         line[1].type == TokenType::MinusAssign ||
                                         line[1].type == TokenType::MultAssign)) {
                    handleAssignment(line);
                } else if (line.size() == 3 && line[0].type == TokenType::Print) {
                    handlePrint(line);
                }
            }
        }
    }
    
    std::vector<std::vector<Token>> structureTokens(const std::vector<Token>& tokens) {
        std::vector<std::vector<Token>> linedTokens; // 2D vector
        std::vector<Token> currentLine; // Temporary vector to hold current line of tokens
        
        for (size_t i = 0; i < tokens.size(); i++) {
            const auto& token = tokens[i];
            if (token.type == TokenType::Semicolon || token.type == TokenType::EndFor) {
                // Add the current line to the list of lines
                currentLine.push_back(token); // Add the token to the end of the line
                linedTokens.push_back(currentLine); // Store the current line
                currentLine.clear(); // Clear the current line for next usage
            } else if (token.type == TokenType::For) {
                // Add the current line to the list of lines
                currentLine.push_back(token); // Add the token to the end of the line
                currentLine.push_back(tokens[i + 1]); // the number of for
                linedTokens.push_back(currentLine); // Store the current line
                currentLine.clear(); // Clear the current line for next usage
                ++i;
            } else {
                // Add the token to the current line
                currentLine.push_back(token);
            }
        }
        
        // If there's any remaining line without a semicolon, add it to the list
        if (!currentLine.empty()) {
            linedTokens.push_back(currentLine);
        }
        
        return linedTokens;
    }
    
    std::vector<Token> unrollForLoops(const std::vector<Token>& tokens) {
        std::vector<Token> unrolledTokens;
        std::vector<Token> loopBody;
        bool inLoop = false;
        int loopCount = 0;
        
        for (size_t i = 0; i < tokens.size(); i++) {
            const auto& token = tokens[i];

            if (token.type == TokenType::For) {
                if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::Number) {
                    loopCount = std::stoi(tokens[i + 1].value); // Get the loop count
                    inLoop = true; // Start collecting the loop body
                    i++; // Skip the loop count token
                }
            } else if (inLoop && token.type == TokenType::EndFor) {
                // If we've reached the end of the loop, duplicate the loop body
                for (int j = 0; j < loopCount; j++) {
                    unrolledTokens.insert(unrolledTokens.end(), loopBody.begin(), loopBody.end());
                }
                loopBody.clear();
                inLoop = false; // Exit loop collection
            } else if (inLoop) {
                // If in a loop, add tokens to the loop body
                loopBody.push_back(token);
            } else {
                // If not in a loop, add tokens normally
                unrolledTokens.push_back(token);
            }
            
            
        }
        
        return unrolledTokens;
    }
    
    // Function to print the variable's value
    void printVariable(const std::string& variableName) {
        const auto& variableValue = variables.at(variableName);
        if (std::holds_alternative<int>(variableValue)) {
            std::cout << variableName << "=" << std::get<int>(variableValue) << std::endl;
        } else if (std::holds_alternative<std::string>(variableValue)) {
            std::cout << variableName << "=\"" << std::get<std::string>(variableValue) << "\"" << std::endl;
        }
    }
    
    // Function to check if a string is numeric
    bool isNumeric(const std::string& str) {
        if (str.empty()) return false;
        
        size_t start = 0;
        if (str[0] == '-' || str[0] == '+') {
            start = 1; // Account for negative/positive signs
        }
        
        for (size_t i = start; i < str.size(); i++) {
            if (!std::isdigit(str[i])) {
                return false; // If any non-digit is found, it's not numeric
            }
        }
        
        return true;
    }
};

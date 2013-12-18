/*
 * Seccheck - A tool for security C/C++ code analysis
 * Copyright (C) 2013 Wang Anyu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
// Unsafe functions
//---------------------------------------------------------------------------

#include "checkunsafefunctions.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckUnsafeFunctions instance;
}

void CheckUnsafeFunctions::unsafeFunctions()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // Functions defined somewhere?
    for (unsigned int i = 0; i < symbolDatabase->functionScopes.size(); i++) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        _unsafeStandardFunctions.erase(scope->className);
        _unsafePosixFunctions.erase(scope->className);
        _unsafeC99Functions.erase(scope->className);
    }

    for (unsigned int i = 0; i < symbolDatabase->functionScopes.size(); i++) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (tok->isName() && tok->varId()==0 && (tok->next() && tok->next()->str() == "(") &&
                (!Token::Match(tok->previous(), ".|::") || Token::simpleMatch(tok->tokAt(-2), "std ::"))) {

                std::map<std::string,std::string>::const_iterator it = _unsafeStandardFunctions.find(tok->str());
                if (it != _unsafeStandardFunctions.end()) {
                    // If checking an old code base it might be uninteresting to update unsafe functions.
                    reportError(tok, Severity::style, "unsafeFunctions"+it->first, it->second);
                } else {
                    if (_settings->standards.posix) {
                        it = _unsafePosixFunctions.find(tok->str());
                        if (it != _unsafePosixFunctions.end()) {
                            // If checking an old code base it might be uninteresting to update unsafe functions.
                            reportError(tok, Severity::style, "unsafeFunctions"+it->first, it->second);
                        }
                    }
                    if (_settings->standards.c >= Standards::C99) {
                        // alloca : this function is unsafe in C but not in C++ (#4382)
                        it = _unsafeC99Functions.find(tok->str());
                        if (it != _unsafeC99Functions.end() && !(tok->str() == "alloca" && _tokenizer->isCPP())) {
                            reportError(tok, Severity::style, "unsafeFunctions"+it->first, it->second);
                        }
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
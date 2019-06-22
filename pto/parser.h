#ifndef LUA_PTO_PARSER_H
#define LUA_PTO_PARSER_H
#include <ctype.h>
#include <string.h>
#include <map>
#include <vector>
#include "pto.h"
#include "lua.hpp"

namespace LuaPto {
	struct Parser;
	struct ParserPto;

	struct ParserField {
		char* name_;
		bool array_;
		eTYPE type_;
		ParserPto* pto_;

		ParserField(const char* name, bool array, eTYPE type, ParserPto* pto) {
			name_ = _strdup(name);
			array_ = array;
			type_ = type;
			pto_ = pto;
		}

		~ParserField() {
			free(name_);
		}
	};

	struct ParserPto {
		std::string file_;
		std::string name_;
		int line_;
		std::vector<ParserField*> fields_;
		std::map<std::string, ParserPto*> childs_;
		ParserPto* last_;
		ParserPto(const char* path, const char* name, int line, ParserPto* last) : file_(path), name_(name), line_(line) {
			last_ = last;
		}

		~ParserPto()  {
			std::map<std::string, ParserPto*>::iterator itPto = childs_.begin();
			for ( ; itPto != childs_.end(); itPto++ ) {
				ParserPto* pto = itPto->second;
				delete pto;
			}

			for ( uint32_t i = 0; i < fields_.size(); ++i ) {
				ParserField* field = fields_[i];
				delete field;
			}
		}

		void AddField(struct ParserField* field) {
			fields_.push_back(field);
		}

		inline ParserField* GetField(int index) {
			return fields_[index];
		}

		ParserPto* GetPto(std::string& name) {
			std::map<std::string, ParserPto*>::iterator it = childs_.find(name);
			if ( it == childs_.end() ) {
				return NULL;
			}
			return it->second;
		}

		void AddPto(std::string& name, ParserPto* pto) {
			childs_[name] = pto;
		}

		void Export(lua_State* L);
	};

	struct ParserContext {
		std::string path_;
		std::map<std::string, ParserPto*> ptos_;
		std::map<std::string, Parser*> parsers_;

		ParserContext(std::string path);

		~ParserContext();

		ParserPto* GetPto(std::string& name) {
			std::map<std::string, ParserPto*>::iterator it = ptos_.find(name);
			if ( it == ptos_.end() ) {
				return NULL;
			}
			return it->second;
		}

		void AddPto(std::string& name, ParserPto* pto) {
			ptos_[name] = pto;
		}

		Parser* GetParser(std::string& name) {
			std::map<std::string, Parser*>::iterator it = parsers_.find(name);
			if ( it == parsers_.end() ) {
				return NULL;
			}
			return it->second;
		}

		bool Import(std::string name);

		void Export(lua_State* L);
	};

	struct Parser {
		ParserContext* ctx_;
		std::string path_;
		char* cursor_;
		char* data_;
		int line_;

		Parser(ParserContext* ctx, std::string& dir, std::string& name);

		~Parser();

		inline bool Eos(int n) {
			if ( *(cursor_ + n) == 0 ) {
				return true;
			}
			return false;
		}

		inline void Skip(int n) {
			char* c = cursor_;
			int index = 0;
			while ( !Eos(0) && index < n ) {
				c++;
				index++;
			}
			cursor_ = c;
		}

		inline void SkipSpace() {
			char *c = cursor_;
			while ( isspace(*c) && *c ) {
				if ( *c == '\n' ) {
					line_++;
				}
				c++;
			}

			cursor_ = c;
			if ( *c == '#' && *c ) {
				NextLine();
			}
		}

		inline void NextLine() {
			char* c = cursor_;
			while ( *c != '\n' && *c ) {
				c++;
			}
			if ( *c == '\n' ) {
				c++;
			}
			line_++;
			cursor_ = c;
		}

		inline std::string NextToken() {
			std::string token;
			char ch = *cursor_;
			int index = 0;
			while ( ch != 0 && (ch == '_' || isalpha(ch) || isdigit(ch)) ) {
				token += ch;
				++cursor_;
				ch = *cursor_;
			}
			SkipSpace();
			return token;
		}

		inline bool Expect(char c) {
			return *cursor_ == c;
		}

		inline bool ExpectSpace() {
			return isspace(*cursor_);
		}

		void ThrowError(std::string reason);

		void ParsePto(ParserPto* last);

		void Init();

		void Run();

	};
}

#endif

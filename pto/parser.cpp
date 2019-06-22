#include "pto.h"
#include "parser.h"
#include "lua.hpp"
#include "format.h"
#include <fstream>
#include <iostream>

using namespace LuaPto;

struct BadParse : public std::exception {
	std::string error_;
	BadParse(std::string error) : error_(error) {
	}

	virtual const char* what() const {
		return error_.c_str();
	}
};


ParserContext::ParserContext(const char* path) : path_(path) {
}

ParserContext::~ParserContext() {
	std::map<std::string, Parser*>::iterator itParser = parsers_.begin();
	for ( ; itParser != parsers_.end(); itParser++ ) {
		Parser* parser = itParser->second;
		delete parser;
	}


	std::map<std::string, ParserPto*>::iterator itPto = ptos_.begin();
	for ( ; itPto != ptos_.end(); itPto++ ) {
		ParserPto* pto = itPto->second;
		delete pto;
	}
}

bool ParserContext::Import(std::string& name) {
	if ( GetParser(name) ) {
		return true;
	}

	try {
		Parser* parser = new Parser(this, path_, name);
		parser->Run();
		parsers_[name] = parser;
	}
	catch ( BadParse& e ) {
		std::cout << e.what() << std::endl;
		return false;
	}
	return true;
}

void ParserContext::Export(lua_State* L) {

}

Parser::Parser(ParserContext* ctx, std::string& dir, std::string& name) {
	ctx_ = ctx;

	path_ = fmt::format("{}/{}", dir, name);

	std::ifstream file;
	file.open(path_.c_str());

	file.seekg(0, std::ios::end);
	int length = file.tellg();
	file.seekg(0, std::ios::beg);

	data_ = (char*)malloc(length + 1);
	memset(data_, 0, length + 1);
	file.read(data_, length);

	file.close();

	cursor_ = data_;
	line_ = 1;
}

Parser::~Parser() {
	free(data_);
}

void Parser::ThrowError(std::string reason) {
	std::string error = fmt::format("{}@line:{}:{}\n", path_, line_, reason);
	throw BadParse(error);
}

void Parser::ParsePto(ParserPto* last) {
	std::string name = NextToken();
	if ( name.size() == 0 ) {
		ThrowError("syntax error");
	}

	int line = line_;

	if ( !Expect('{') ) {
		ThrowError("protocol must start with {");
	}

	if ( last ) {
		ParserPto* parent = last;
		while ( parent ) {
			ParserPto* pto = parent->GetPto(name);
			if ( pto ) {
				ThrowError(fmt::format("protocol:{} already define in {}@{}", name, pto->file_, pto->line_));
			}
			parent = parent->last_;
		}

		ParserPto* pto = ctx_->GetPto(name);
		if ( pto ) {
			ThrowError(fmt::format("protocol:{} already define in {}@{}", name, pto->file_, pto->line_));
		}
	}

	ParserPto* pto = new ParserPto(path_.c_str(), name.c_str(), line, last);
	if ( last ) {
		last->AddPto(name, pto);
	}
	else {
		ctx_->AddPto(name, pto);
	}

	Skip(1);
	SkipSpace();

	while ( !Expect('}') ) {
		name = NextToken();
		if ( name.size() == 0 ) {
			ThrowError("syntax error");
		}

		if ( name == "protocol" ) {
			ParsePto(pto);
			continue;
		}

		int type = eTYPE::Pto;
		for ( int i = 0; i < sizeof(kTYPE_NAME) / sizeof(void*); ++i ) {
			if ( name == kTYPE_NAME[i] ) {
				type = i;
				break;
			}
		}

		bool array = false;
		if ( strncmp(cursor_, "[]", 2) == 0 ) {
			array = true;
			Skip(2);
			if ( !ExpectSpace() ) {
				ThrowError("syntax error,expect space");
			}
			SkipSpace();
		}

		ParserPto* reference = NULL;
		if ( type == eTYPE::Pto ) {
			ParserPto* last = pto;
			while ( last ) {
				reference = last->GetPto(name);
				if ( reference ) {
					break;
				}
				last = last->last_;
			}

			if ( !reference ) {
				reference = ctx_->GetPto(name);
				if ( !reference ) {
					ThrowError(fmt::format("unknown protocol:{}", name));
				}
			}
		}

		name = NextToken();
		if ( name.size() == 0 ) {
			ThrowError("syntax error");
		}

		ParserField* field = new ParserField(name.c_str(), array, (eTYPE)type, reference);
		pto->AddField(field);
	}
	Skip(1);
	SkipSpace();
}

void Parser::Run() {
	SkipSpace();
	while ( !Eos(0) ) {
		std::string name = NextToken();
		if ( name.size() == 0 ) {
			ThrowError("syntax error");
		}

		if ( name == "protocol" ) {
			ParsePto(NULL);
		}
		else if ( name == "import" ) {
			SkipSpace();
			if ( !Expect('\"') ) {
				ThrowError("import format must start with \"");
			}

			Skip(1);

			name = NextToken();
			if ( name.size() == 0 ) {
				ThrowError("import protocol name empty");
			}

			if ( !Expect('\"') ) {
				ThrowError("import format must end with \"");
			}

			Skip(1);

			if ( !ctx_->GetParser(name) ) {
				ctx_->Import(name);
			}

			SkipSpace();
		}
	}
}

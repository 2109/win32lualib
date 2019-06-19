#include <iostream>
#include "../linenoise.hpp"

using namespace std;



void CompletionCallback(const char* buffer, std::vector<std::string>& completions) {

	if ( buffer[0] == 'h' ) {
		completions.push_back("hello");
		completions.push_back("hello there");
	} else if (buffer[0] == 'f')
	{
		completions.push_back("fuck");
	}
}

int main(int argc, const char** argv)
{
    const auto path = "history.txt";

    // Enable the multi-line mode
    linenoise::SetMultiLine(true);

    // Set max length of the history
    linenoise::SetHistoryMaxLen(4);

	linenoise::SetCompletionCallback(CompletionCallback);
    // Load history
    linenoise::LoadHistory(path);

    while (true) {
        std::string line;

        bool quit = linenoise::Readline("hello> ", line);

        if (quit) {
            break;
        }

        cout <<  "echo: '" << line << "'" << endl;

        // Add line to history
        linenoise::AddHistory(line.c_str());

        // Save history
        linenoise::SaveHistory(path);
    }

    return 0;
}

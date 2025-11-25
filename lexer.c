
#include "lexer.h"
#include <ctype.h>

Token getNextToken(FILE *fp)
{
	int ch = fgetc(fp);

	if(isdigit(ch) ) {
		return TOKEN_NUMBER;
	}

	if(isalpha(ch) ) {
		return TOKEN_IDENTIFIER;
	}

	if (ch == '+') return TOKEN_PLUS;
	if (ch == '+') return TOKEN_PLUS;

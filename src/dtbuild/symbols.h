#pragma once

namespace dtbuild
{
	enum symbol_type: short
	{
		NO_TYPE,

		// Non-terminal types used for parsing
		FUNC,
		STMT,
		STMT1,
		STMTS,
		ID,
		FUNCBDY,
		FUNC_CALL,
		DECL_ID,
		TYPE_INIT,
		INIT_LIST,
		INIT_ARGS,
		LIST,
		ARG,
		ARGS,
		EXPR,
		EXPR1,
		TERM,
		TERM1,
		FACTOR,
		CONST,
		TYPENAME,
		OP,
		LP_OP,
		HP_OP,
		ELIPSIS,
		COMMA_ELIPSIS,
		PROGRAM,
		
		NT_CUTOFF = PROGRAM, //transition from non-terminal to terminal, nothing should ever == NT_CUTOFF

		// terminal types used for tokenizing and parsing
		IDENTIFIER,
		NUM_LITERAL,
		STRING_LITERAL,
		CHAR_LITERAL,
		POUND_SIGN,
		DOLLAR_SIGN,
		QUESTION_MARK,
		TAB,
		SPACE,
		NEW_LINE,
		SQUOTE,
		DQUOTE,
		LPAREN,
		RPAREN,
		LBRACE,
		RBRACE,
		LBRACK,
		RBRACK,
		INT_TYPE,
		DOUBLE_TYPE,
		ALGORITHM_TYPE,
		INDICATOR_TYPE,
		RETURN,
		LANGBRACK,
		RANGBRACK,
		LINE_COMMENT,
		COMMENT_START,
		COMMENT_END,
		COMMA,
		PERIOD,
		SEMICOLON,
		COLON,
		AND,
		AND_COMP,
		OR,
		OR_COMP,
		PLUS,
		SUB_ASGN,
		ADD_ASGN,
		EQUALS_ASGN,
		EQUALS_COMP,
		NEQUALS_COMP,
		MINUS,
		ASTERISK,
		SLASH,
		MODULUS,
		NOT,
		TILDE,
		REQUIRE_PREPRO,
		INCLUDE_PREPRO,
		BUY_CALL,
		SELL_CALL,
		NOTHING_CALL
	};
}
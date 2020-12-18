#include "generator.h"

namespace dtbuild
{
	namespace generator
	{
		//typedef void (*gen_func)(const parser::Node&, std::string&, int);
		//std::vector<gen_func> gen_funcs;

		std::string tabs(int amt)
		{
			std::string out;
			for (int i = 0; i < amt; i++)
			{
				out += '\t';
			}
			return out;
		}

		// dtbuild specific rules for c++ generation
		std::string generate(const parser::Node& tree, int tab_depth = 0)
		{
			std::string out;
			using namespace parser;

			switch(tree.type)
			{
			case PROGRAM:
				for (const Node& arg : tree.args)
				{
					out += generate(arg, tab_depth);
				}
				break;

			case INDI_INIT:
				out += generate(tree.args[0]);
				out += "(_candles, ";
				for (const Node& arg : tree.args[1].args)
				{
					out += generate(arg);
				}
				out += ')';
				break;

			case SEL_STMT:
				out += tabs(tab_depth) + "if (";
				out += generate(tree.args[0]);
				out += ")\n";
				out += generate(tree.args[1], tab_depth);
				break;

			case JUMP_STMT:
				out += tabs(tab_depth) + "return ";
				out += generate(tree.args[0]);
				out += ";\n";
				break;

			case DECL_STMT:
				out += tabs(tab_depth);
				out += generate(tree.args[0]);
				out += "= ";
				out += generate(tree.args[1]);
				out += ";\n";
				break;

			case EXPR_STMT:
				out += tabs(tab_depth);
				for (const Node& arg : tree.args)
				{
					out += generate(arg);
				}
				out += ";\n";
				break;

			case INDEX_OP:
				out += '[';
				out += generate(tree.args[0]);
				out += ']';
				break;

			case ALGO_ARGS:
				out += tabs(tab_depth) + "out.dataset.resize(" + std::to_string(tree.args.size()) + ");\n\n";
				for (int i = 0; i < tree.args.size(); i++)
				{

					const Node& n = tree.args[i];
					out += tabs(tab_depth) + "out.dataset[" + std::to_string(i) + "] = ";
					out += generate(n.args[0], tab_depth);
					out += ";\n";
					out += tabs(tab_depth) + "const std::vector<double>& _" + generate(n.args[1], tab_depth) + " = out.dataset[" + std::to_string(i) + "];\n\n"; // the identifier
				}
				break;

			case INDI:
				out += tabs(tab_depth) + "std::vector<double> ";
				out += generate(tree.args[0]);
				out += "(const candleset& _candles, ";
				out += generate(tree.args[1], tab_depth);
				out += ")\n";
				out += generate(tree.args[2], tab_depth);
				out += '\n';
				break;
				
			case ALGO:
				out += tabs(tab_depth) + "extern \"C\" void ";
				out += generate(tree.args[0]);
				out += "(const candleset& _candles, algorithm_data& out)\n";
				out += tabs(tab_depth) + "{\n";
				out += generate(tree.args[1], tab_depth+1);
				out += generate(tree.args[2], tab_depth+1);
				out += tabs(tab_depth) + "}\n";
				break;

			case FUNC:
				out += tabs(tab_depth);
				out += generate(tree.args[0], tab_depth + 1);
				out += '(';
				out += generate(tree.args[1]);
				out += ")\n";
				out += generate(tree.args[2], tab_depth);
				out += '\n';
				std::cout << "GENERATING: " << tree.args[2].type << std::endl;
				break;

			case CALL_OP:
				out += '(';
				out += generate(tree.args[0]);
				out += ')';
				break;

			case FACTOR:
				if (tree.args[0].type == EXPR)
				{
					out += '(';
					for (const Node& arg : tree.args) out += generate(arg, tab_depth);
					out += ") ";
				}
				else
				{
					for (const Node& arg : tree.args) out += generate(arg, tab_depth);
				}
				break;

			case FUNCBDY:
				for (const Node& arg : tree.args)
				{
					out += generate(arg, tab_depth);
				}
				break;

			case ACCESSOR:
				out += '.';
				out += generate(tree.args[0]);
				break;

			case PACCESSOR:
				out += "->";
				out += generate(tree.args[0]);
				break;

			case COMPOUND_STMT:
				out += tabs(tab_depth) + "{\n";
				for (const Node& arg : tree.args)
				{
					out += generate(arg, tab_depth + 1);
				}
				out += tabs(tab_depth) + "}\n";
				break;

			case LIST:
			case ARGS:
			case INIT_LIST:
				for (int i = 0; i < tree.args.size(); i++)
				{
					out += generate(tree.args[i], tab_depth);
					if (i < tree.args.size() - 1)
					{
						out += ", ";
					}
				}
				break;

			case DECLARATION:
				out += tabs(tab_depth);
				out += generate(tree.args[0]);
				out += "= ";
				out += generate(tree.args[1]);
				out += ";\n";
				break;
			
			case CONST:
				if (tree.subtype == STRING_LITERAL)
				{
					out += "\"";
					out += tree.value;
					out += "\"";
				}
				else
				{
					out += tree.value;
				}
				break;

			default:
				if (tree.args.empty())
				{
					out += tree.value + ' ';
				}
				else
				{
					for (const Node& arg : tree.args)
					{
						out += generate(arg, tab_depth);
					}
				}
				break;
			}

			return out;
		}

		std::string generate_code(const parser::Node& tree)
		{
			std::string out = "#include <candle.h>\n#include <algodefs.h>\n\n";
			out += generate(tree, 1);

			return out;
		}
	} // namespace generator
} // namespace dtbuild
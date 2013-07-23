#include "Buttons.h"
#include "Bitmap.h"

static char * button_label[] = {
	"int", "sin", "sin¯¹",
	"frac", "cos", "cos¯¹",
    "sign", "tan", "tan¯¹",
    "abs", "sinh", "sinh¯¹",
    "nCr", "cosh", "cosh¯¹",
    "nPr", "tanh", "tanh¯¹",
    "cot", "coth",  "cot¯¹",
    "coth¯¹",
    
    "nand", "nor", "cnt", 
    "mod", "cbit", "tbit",
    "div", "sbit", "neg",
    "or", "shl", "shr",
    "and", "rol", "ror",
    "xor", "not", "asr",
    
    "»x", "»x;y", "CS", 
    "Avg(x)", "Avg(y)", "∑xy",
    "∑x", "∑y", "Sx", 
    "∑x²", "∑y²", "Sy", 
    "Dev(x)", "Dev(y)", "n",
    
    "Exp", "x¯¹", "π", "x!", "y*", "x²",
    "A", "B", "C", "D", "x³", "e*",
    "7", "8", "9", "E", "÷", "M",
    "4", "5", "6", "F", "×", ";",
    "1", "2", "3", "i", "−", "(",
    "0", "±", "·", "=", "+", ")",
    
    "√", "*√", "CE",
    "³√", "log", "rand",
    "10*", "log10", "min",
    "re", "im", "max",
    "ø", "rect", "conj",
    "Enter", "x", "log2",
    "fib", "gcd"
};

char * button_strs[] = {
	"int ", "sin ", "sin¯¹ ",
	"frac ", "cos ", "cos¯¹ ",
    "sign ", "tan ", "tan¯¹ ",
    "abs ", "sinh ", "sinh¯¹ ",
    " nCr ", "cosh ", "cosh¯¹ ",
    " nPr ", "tanh ", "tanh¯¹ ",
    "cot ", "coth ",  "cot¯¹ ",
    "coth¯¹ ",
    
    " nand ", " nor ", "cnt ",
    " mod ", " cbit ", " tbit ",
    " div ", " sbit ", "neg ",
    " or ", " shl ", " shr ",
    " and ", " rol ", " ror ",
    " xor ", "not ", " asr ",
    
    "", "", "", 
    "", "", "", 
    "", "", "", 
    "", "", "", 
    "", "", "",
    
    "E", "¯¹", "π", "!", "^", "²",
    "A", "B", "C", "D", "³", "e^",
    "7", "8", "9", "E", " ÷ ", "M",
    "4", "5", "6", "F", " × ", "; ",
    "1", "2", "3", "i", " - ", "(",
    "0", "", ".", " = ", " + ", ")",
    
    "√", "*√", "",
    "³√", "log ", "rand",
    "10^", "log10 ", "min ",
    "re ", "im ", "max ",
    "ø ", "rect ", "conj ",
    "", "x", "log2 ",
    "fib ", "gcd "
};

bool IsNumber (int32 id)
{
	return (	(id == ID_0) || (id == ID_1) || (id == ID_2) || (id == ID_3) || (id == ID_4) ||
				(id == ID_5) || (id == ID_6) || (id == ID_7) || (id == ID_8) || (id == ID_9) ||
				(id == ID_A) || (id == ID_B) || (id == ID_C) || (id == ID_D) || (id == ID_E) ||
				(id == ID_F) || (id == ID_DP) || (id == ID_I) || (id == ID_EXPON) || (id == ID_PI) );
}

bool IsFunction (int32 id)
{
	char * str = button_strs[id];
	int32 len = strlen(str)-1;
	return (((str[len] == ' ') && (str[0] != ' ')) || ((str[len] == '^') && (str[0] != '^')));
}

MButton * MakeButton (int32 id, minimax size) {
	MButton *b;
	b = new MButton(button_label[id], id, size);
	switch (id) {
		case ID_0: case ID_1: case ID_2: case ID_3: case ID_4: case ID_5: case ID_6:
		case ID_7: case ID_8: case ID_9: case ID_A: case ID_B: case ID_C: case ID_D:
		case ID_E: case ID_F:
			b->flags |= (M_REPORT_REPEAT);
			break;
		case ID_ENTER:
			b->MakeDefault(true);
			break;
		default: ;
	}
	return b;
}

char * ButtonIs (int32 id) {
	return button_strs[id];
}

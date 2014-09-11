/*
 * =====================================================================================
 *
 *       Filename:  md2html.h
 *
 *    Description:	a C implementation of the Markdown to HTML system. 
 *
 *        Version:  0.66
 *        Created:  08/17/2014 16:48:02
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Jackie Kuo (http://jackiekuo.com), j.kuo2012@gmail.com
 *
 * =====================================================================================
 */

#ifndef MD2HTML_H
#define MD2HTML_H

#include <stdio.h>
#include <string.h>

#define CSS_PATH "./style.css"
#define MAX 1000

char title[257];
int isHr = 0, ERROR_CODE = 0;

/*
 * ERROR_CODE
 *
 * 0 Everything is OK
 * 1 cannot open input file
 * 2 cannot create output file
 * 3 cannot find style.css
 *
 */

static const char *head1 =
	"<!DOCTYPE html>\n"
	"<html>\n"
	"<head>\n"
	"<meta charset=\"utf-8\">\n"
	"<meta name=\"viewport\" "
	"content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n\n";

static const char *head2 =
	"<title>";

static const char *head3 =
	"</title>\n"
	"</head>\n\n"
	"<body>\n";

static const char *foot =
	"\n</body>\n"
	"</html>\n";


int onUrl(FILE *out_fp, FILE *in_fp) {
	// this function deals with url
	// if what it deals with is a url
	// it will return 0
	// if what it deals with is not a url
	// it will return from 1 to 5
	// if what it deals with is not a url and onUrl gets a '\n'
	// it will return 2 or 4

	int i = 0;
	char ch, name[MAX], url[MAX];

	while ((ch = fgetc(in_fp)) != EOF) {
		if (ch == ']' || ch == '\n' || i > 999) {
			break;
		}
		name[i++] = ch;
	}
	name[i] = '\0';

	if (ch != ']') {
		fprintf(out_fp, "[%s%c", name, ch);
		return 1;
	}

	while ((ch = fgetc(in_fp)) == ' ') {}

	if (ch != '(') {
		if (ch == '\n') {
			fprintf(out_fp, "[%s]", name);
			return 2;
		}
		else {
			fprintf(out_fp, "[%s]%c", name, ch);
			return 3;
		}
	}

	i = 0;
	while ((ch = fgetc(in_fp)) != EOF) {
		if (ch == ')' || ch == '\n' || i > 999) {
			break;
		}
		url[i++] = ch;
	}
	url[i] = '\0';

	if (ch != ')') {
		if (ch == '\n') {
			fprintf(out_fp, "[%s](%s", name, url);
			return 4;
		}
		else {
			fprintf(out_fp, "[%s](%s%c", name, url, ch);
			return 5;
		}
	}

	fprintf(out_fp, "<a href=\"%s\">%s</a>", url, name);
	return 0;
}

int onHeader(FILE *out_fp, FILE *in_fp) {
	// this function deals with Header
	// from <h1> to <h6>
	// if what it deals with is a header
	// it will return 0
	// if what it deals with is not a header
	// it will return 1

	int count = 1, onUrlstate;
	char ch;

	while ((ch = fgetc(in_fp)) == '#') {
		++count;
	}

	if (count > 6) {
		while (count--) {
			fprintf(out_fp, "#");
		}
		fprintf(out_fp, "%c", ch);
		return 1;
	}

	fprintf(out_fp, "<h%d>", count);

	while (ch != '\n' && ch != EOF) {
		if (ch == '[') {
			onUrlstate = onUrl(out_fp, in_fp);
			if (onUrlstate == 2 || onUrlstate == 4) {
				// there is a '\n'
				break;
			}
		}
		else {
			fprintf(out_fp, "%c", ch);
		}
		ch = fgetc(in_fp);
	}

	fprintf(out_fp, "</h%i>\n", count);
	return 0;
}

int onHr(FILE *out_fp, FILE *in_fp) {
	int count = 1;
	char ch;

	ch = fgetc(in_fp);
	while (ch != '\n' && ch == '-') {
		++count;
		ch = fgetc(in_fp);
	}

	if (count >= 3) {
		fprintf(out_fp, "\n<hr/>\n");
		isHr = 1;
	}
	else {
		while (count--) {
			fprintf(out_fp, "-");
		}
		fputc(ch, out_fp);
		return 1;
	}
	return 0;
}

int onImg(FILE *out_fp, FILE *in_fp) {
	int i = 0;
	char ch, name[MAX], url[MAX];

	ch = fgetc(in_fp);
	if (ch != '[') {
		fprintf(out_fp, "!%c", ch);
		return 1;
	}

	while ((ch = fgetc(in_fp)) != EOF) {
		if (ch == ']' || ch == '\n' || i > 999) {
			break;
		}
		name[i++] = ch;
	}

	name[i] = '\0';
	if (ch == '\n' || ch == EOF) {
		fprintf(out_fp, "![%s%c", name, ch);
		return 2;
	}
	
	while ((ch = fgetc(in_fp)) == ' ') {}

	if (ch != '(') {
		fprintf(out_fp, "![%s]%c", name, ch);
		return 3;
	}

	i = 0;
	while ((ch = fgetc(in_fp)) != EOF) {
		if (ch == ')' || ch == '\n' || i > 999) {
			break;
		}
		url[i++] = ch;
	}
	url[i] = '\0';

	if (ch != ')') {
		fprintf(out_fp, "![%s](%s%c", name, url, ch);
		return 4;
	}

	fprintf(out_fp, "<img src=\"%s\" alt=\"%s\">", url, name);
	return 0;
}

int onBold(FILE *out_fp, FILE *in_fp) {
	int i = 0;
	char ch, content[MAX];

	while ((ch = fgetc(in_fp)) == ' ') {}
	
	if (ch == '*') {
		ch = fgetc(in_fp);
		if (ch == '*') {
			fprintf(out_fp, "<strong></strong>");
			return 1;
		}
		else {
			fprintf(out_fp, "<i>*</i>");
			return 2;
		}
	}
	else if (ch == '\n') {
		fputc(ch, out_fp);
		return 3;
	}

	content[i++] = ch;
	while ((ch = fgetc(in_fp)) != EOF) {
		if (ch == '*' || ch == '\n' || i > 999) {
			break;
		}
		content[i++] = ch;
	}
	content[i] = '\0';

	if (ch != '*') {
		fprintf(out_fp, "%s%c", content, ch);
		return 4;
	}
	
	ch = fgetc(in_fp);
	if (ch != '*') {
		fprintf(out_fp, "<i>*%s</i>%c", content, ch);
		return 5;
	}

	fprintf(out_fp, "<strong>%s</strong>", content);
	return 0;
}

int onIorB(FILE *out_fp, FILE *in_fp) {
	// this function deal with italic or bold style
	// if what it deals with is bold style
	// it will return -1
	// if what it deals with is italic style
	// it will return -2

	int i = 0, state, isSpace;
	char ch, content[MAX];
	
	ch = fgetc(in_fp);
	if (ch == '*') {
		state = onBold(out_fp, in_fp);
		if (state != 0) {
			return state;
		}
		else {
			return -1;
		}
	}
	else if (ch == ' ') {
		isSpace = 1;
	}
	else {
		isSpace = 0;
		content[i++] = ch;
	}
	if (isSpace) {
		while ((ch = fgetc(in_fp)) == ' ') {}
		content[i++] = ch;
	}
	while ((ch = fgetc(in_fp)) != EOF) {
		if (ch == '*' || ch == '\n' || i> 999) {
			break;
		}
		content[i++] = ch;
	}
	content[i] = '\0';

	if (ch != '*') {
		fprintf(out_fp, "*%s%c", content, ch);
		return 10;
	}

	fprintf(out_fp, "<i>%s</i>", content);
	return -2;
}

int onList(FILE *out_fp, FILE *in_fp,const int sign) {
	int onUrlstate;
	char ch;

	if (sign == 1) {
		fprintf(out_fp, "<ul>\n");
	}
	
	if (sign == 1 || sign == 2) {
		fprintf(out_fp, "<li>");
		//while ((ch = fgetc(in_fp)) == ' ') {}

		ch = fgetc(in_fp);
		while (ch != '\n' && ch != EOF) {
		
			if (ch == '[') {
				onUrlstate = onUrl(out_fp, in_fp);
				if (onUrlstate == 2 || onUrlstate == 4) {
					// there is a '\n'
					break;
				}
			}
			else {
				fprintf(out_fp, "%c", ch);
			}
			ch = fgetc(in_fp);
		}

		fprintf(out_fp, "</li>\n");
	}

	if (sign == 3) {
		fprintf(out_fp, "</ul>\n");
	}

	return 0;
}

int onAster(FILE *out_fp, FILE *in_fp, const int sign) {
	// if what it deal with is bold style
	// it will return -1
	// and if what it deal with is italic style
	// it will return -2
	// and if what it deal with is a list
	// it will return -3

	int state, i = 0;
	char ch, content[MAX];

	ch = fgetc(in_fp);
	if (ch == ' ') {
		state = onList(out_fp, in_fp, sign);
		if (state == 0) {
			return -3;
		}
		else {
			return state;
		}
	}
	else if (ch == '*') {
		fputc('\n', out_fp);
		state = onBold(out_fp, in_fp);
		if (state == 0) {
			return -1;
		}
		else {
			return state;
		}
	}
	else if (ch == '\n') {
		fprintf(out_fp, "<p>*</p>\n");
	}
	content[i++] = ch;
	
	while ((ch = fgetc(in_fp)) != EOF) {
		if (ch == '*' || ch == '\n' || i> 999) {
			break;
		}
		content[i++] = ch;
	}
	content[i] = '\0';

	if (ch != '*') {
		if (ch == '\n') {
			fprintf(out_fp, "<p>*%s</p>\n", content);
		}
		else {
			fprintf(out_fp, "<p>*%s%c", content, ch);
		}
		return 10;
	}

	fprintf(out_fp, "<p><i>%s</i>", content);
	return -2;

}

void onSpecialChar(FILE *out_fp, const char ch) {
	if (ch == '&') {
		fprintf(out_fp, "&amp;");
	}
	else if (ch == '<') {
		fprintf(out_fp, "&lt;");
	}
}

int onCode(FILE *out_fp, FILE *in_fp) {
	int i = 0, j, isSpace, isNewLine;
	char ch, content[MAX];

	ch = fgetc(in_fp);
	if (ch == ' ') {
		isSpace = 1;
	}
	else {
		isSpace = 0;
		content[i++] = ch;
	}
	if (isSpace) {
		while ((ch = fgetc(in_fp)) == ' ') {}
		content[i++] = ch;
	}

	while ((ch = fgetc(in_fp)) != EOF) {
		if (isNewLine && ch == '\n') {
			break;
		}
		else if (!isNewLine && ch == '\n') {
			isNewLine = 1;
		}
		else {
			isNewLine = 0;
			if (ch == '`' || i > 999) {
				break;
			}
		}
		content[i++] = ch;
	}
	content[i] = '\0';

	if (ch != '`') {
		fprintf(out_fp, "`%s%c", content, ch);
		return 1;
	}

	fprintf(out_fp, "<code>");
	for (j=0; j<i; j++) {
		if (content[j] == '&' || content[j] == '<') {
			onSpecialChar(out_fp, content[j]);
		}
		else {
			fputc(content[j], out_fp);
		}
	}

	fprintf(out_fp, "</code>");
	return 0;
}

int onQuote(FILE *out_fp, const int sign) {
	// this function deals with blockquote
	
	if (sign == 1) {
		fprintf(out_fp, "<blockquote>\n");
	}
	if (sign == 2) {
		fprintf(out_fp, "\n</blockquote>\n");
	}
	return 0;
}

int onBlock(FILE *out_fp, FILE *in_fp, const int sign) {
	// this function deals with code block
	// it works the same as onList
	int i, j;
	char ch, content[MAX];

	if (sign == 1) {
		fprintf(out_fp, "<pre>\n<code>\n");
	}
	
	if (sign == 1 || sign == 2) {
		i = 0;
		while ((ch = fgetc(in_fp)) != EOF) {
			if (ch == '\n' || i > 999) {
				break;
			}
			content[i++] = ch;
		}
		content[i] = '\0';

		for (j=0; j<i; j++) {
			if (content[j] == '&' || content[j] == '<') {
				onSpecialChar(out_fp, content[j]);
			}
			else {
				fputc(content[j], out_fp);
			}
		}
		fputc('\n', out_fp);
	}

	if (sign == 3) {
		fprintf(out_fp, "</code>\n</pre>\n");
	}

	return 0;
}

void add_style(FILE *out_fp, const char *css_file) {
	FILE *css_fp;
	char ch;

	fprintf(out_fp, "%s", "<style>\n");

	if ((css_fp = fopen(css_file, "rw+")) == NULL) {
		ERROR_CODE = 3;
		printf("\nerror!\nERROR_CODE:%d\tcan't open style file(%s)\n", ERROR_CODE, css_file);
	}
	else {
		while ((ch = fgetc(css_fp)) != EOF) {
			fputc(ch, out_fp);
		}
	}

	fprintf(out_fp, "%s", "\n</style>\n");
}

void add_head(FILE *out_fp, const char *css_file) {
	fprintf(out_fp, "%s", head1);
	add_style(out_fp, css_file);
	fprintf(out_fp, "%s%s%s", head2, title, head3);
}

void add_foot(FILE *out_fp) {
	fprintf(out_fp, "%s", foot);
}

void mdparser(FILE *out_fp, FILE *in_fp) {
	char ch;
	int isNewLine, isQuote, isCode, isList, isBlock;
	isNewLine = 1;
	isQuote = isCode = isList = isBlock = 0;

	add_head(out_fp, CSS_PATH);
	
	while ((ch = fgetc(in_fp)) != EOF) {
		if (isNewLine && ch != '*' && isList) {
			onList(out_fp, in_fp, 3);
			isList = 0;
		}
		if (isNewLine && ch != '\t' && isBlock) {
			onBlock(out_fp, in_fp, 3);
			isBlock = 0;
		}
		if (isNewLine && ch != '>' && isQuote) {
			onQuote(out_fp, 2);
			isQuote = 0;
		}

		if (isNewLine &&
			(ch == '>' ||  ch == '#' || ch == '-' || ch == '*' || ch == '\t')) {
			if (ch == '>') {
				if (isQuote == 0) {
					onQuote(out_fp, 1);
					isQuote = 1;
				}
			}
			else if (ch == '#') {
				onHeader(out_fp, in_fp);
				ch = '\n';
			}
			else if (ch == '-') {
				onHr(out_fp, in_fp);
			}
			else if (ch == '*') {
				int state;
				if (isList == 0) {
					state = onAster(out_fp, in_fp, 1);
				}
				else {
					state = onAster(out_fp, in_fp, 2);
				}

				if (state == -3) { // if it is a list
					ch ='\n';
					isList = 1;
				}
				// if ch is a aterisk in a new line,
				// it can be a list,
				// or italic style
				// or bold style
				// or just common text
			}
			else if (ch == '\t') {
				if (isBlock == 0) {
					onBlock(out_fp, in_fp, 1);
				}
				else {
					onBlock(out_fp, in_fp, 2);
				}
				ch = '\n';
				isBlock = 1;
				isNewLine = 1;
			}
			else {
				fputc(ch, out_fp);
			}
		}
		else if (ch == '[') {
			if (isNewLine) {
				fprintf(out_fp, "<p>");
			}
            onUrl(out_fp, in_fp);
        }
		else if (ch == '!') {
			if (isNewLine) {
				fprintf(out_fp, "<p>");
			}
			onImg(out_fp, in_fp);
		}
		else if (ch == '`') {
			if (isNewLine) {
				fprintf(out_fp, "<p>");
			}
			onCode(out_fp, in_fp);
		}
		else if (ch == '*') {
			if (isNewLine) {
				fprintf(out_fp, "<p>%c", ch);
			}
			onIorB(out_fp, in_fp);
			// if ch is a asterish and not in a new line,
			// it can be italic style
			// or it can be bold style
		}
        else if (ch != '\n') {
			if (isNewLine && !isQuote) {
				fprintf(out_fp, "<p>%c", ch);
			}
			else {
            	fputc(ch, out_fp);
			}
        }

        if (ch != '\n') {
            isNewLine = 0;
        }
        else {
			if (!isHr && !isQuote && !isNewLine) {
				fprintf(out_fp, "</p>\n");
			}
			isNewLine = 1;
			isHr = 0;
        }
	}
	add_foot(out_fp);
}


void get_input(char *file_dest) {
	printf("Please input the directory of the target file:\n");
	scanf("%s", file_dest);
}

//get the filename of output file
void name_output(char *new, char *old, const char *format) {
	int dot, len = (int)strlen(old);

	strcpy(new, old);
	
	for (dot = len-1; new[dot] != '.'; dot--) {}
	new[dot] = '\0';

	//get filename
	for (--dot; new[dot] != '/' && dot >= 0; dot--) {}
	strcpy(title, new + dot + 1);
	
	strcat(new, format);
}

int open_file(FILE **in_fp, const char *file) {
	if ((*in_fp = fopen(file, "r")) == NULL) {
		ERROR_CODE = 1;
		printf("\nerror!\nERROR_CODE:%d\tcan't open input file(%s)", ERROR_CODE, file);
		return 1;
	}
	return 0;
}

int create_file(FILE **out_fp, const char *file) {
	if ((*out_fp = fopen(file, "w")) == NULL) {
		ERROR_CODE = 2;
		printf("\nerror!\nERROR_CODE:%d\tcan't create output file(%s)", ERROR_CODE, file);
		return 1;
	}
	return 0;
}

void close_files(FILE **fpa, FILE **fpb) {
	if (!ERROR_CODE) {
		printf("\nDone!\n");
	}
	fclose(*fpa);
	fclose(*fpb);
	*fpa = NULL;
	*fpb = NULL;
}

#endif // MD2HTML_H

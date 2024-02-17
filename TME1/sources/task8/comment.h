#ifndef COMMENT_H
#define COMMENT_H


struct title {
	int size;
	char title[];
};

struct author {
	int size;
	char author[];
};

struct text {
	int size;
	char text[];
};

struct comment {
	struct title* title;
	struct author* author;
	struct text* text;
};

struct comment *new_comment(int title_size, char *title, int author_size,
			    char *author, int text_size, char *text);

void display_comment(struct comment *v);
void free_comment(struct comment *c);

#endif

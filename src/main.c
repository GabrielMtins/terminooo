/*
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>
	
	Copyright 2024 Gabriel Martins
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <ncurses.h>

#define COLOR_THEREISNOT 1
#define COLOR_POSITION_CORRECT 2
#define COLOR_THEREIS 3
#define COLOR_DEFAULT 4

#define WARNING_WINDOW_WIDTH 60

#define LIST_SIZE 5087
#define WORDS_SIZE 1753

#define is_on_list(str) is_on_list_binary(str, 0, LIST_SIZE - 1)

static char dictionary[LIST_SIZE][10];
static char words[WORDS_SIZE][10];

void read_list(void);

int has_five_characters(const char *str);

int is_on_list_binary(const char *str, int start, int end);

void print_with_color(WINDOW *buffer_window, int current_buffer, const char *try, const char *word);

void read_list(void){
	FILE *file = fopen("dicionario.txt", "r");

	if(file == NULL){
		printf("Failed to open file: dicionario.txt\n");
		exit(1);
	}

	for(int i = 0; i < LIST_SIZE; i++){
		fscanf(file, "%s", dictionary[i]);
	}

	fclose(file);

	file = fopen("palavras.txt", "r");

	if(file == NULL){
		printf("Failed to open file: palavras.txt\n");
		exit(1);
	}

	for(int i = 0; i < WORDS_SIZE; i++){
		fscanf(file, "%s", words[i]);
	}

	fclose(file);
}

int has_five_characters(const char *str){
	for(int i = 0; i < 5; i++){
		if(isspace(str[i])) return 0;
	}

	return 1;
}

int is_on_list_binary(const char *str, int start, int end){
	if(start > end) return 0;

	int index = (start + end) / 2;
	int value = strcmp(str, dictionary[index]);

	if(value == 0){
		return 1;
	}
	else if(value < 0){
		return is_on_list_binary(str, start, index - 1);
	}
	else{
		return is_on_list_binary(str, index + 1, end);
	}
}

void print_with_color(WINDOW *buffer_window, int current_buffer, const char *try, const char *word){
	int char_num[26];

	for(int i = 0; i < 26; i++) char_num[i] = 0;

	for(int i = 0; i < 5; i++){
		char_num[word[i] - 'a']++;
		char_num[try[i] - 'a']--;
	}

	for(int i = 0; i < 5; i++){
		wmove(buffer_window, 1 + current_buffer, 1 + i);

		if(try[i] == word[i]){
			wattron(buffer_window, COLOR_PAIR(COLOR_POSITION_CORRECT));
		}
		else if(char_num[try[i] - 'a'] >= 0){
			wattron(buffer_window, COLOR_PAIR(COLOR_THEREIS));
		}
		else{
			wattron(buffer_window, COLOR_PAIR(COLOR_THEREISNOT));
		}

		waddch(buffer_window, try[i]);
	}

	wattron(buffer_window, COLOR_PAIR(COLOR_DEFAULT));
}

int main(void){
	int ch;
	int quit = 0;
	int current_character = 0;
	int current_buffer = 0;
	char buffer_try[5][6];
	int reset = 0;
	WINDOW *buffer_window;
	WINDOW *warning_window;

	srand(time(NULL));
	read_list();

	char *word_chosen = words[rand() % WORDS_SIZE];

	initscr();

	if(has_colors() == FALSE){
		endwin();
		printf("No color supported\n");
		exit(1);
	}

	keypad(stdscr, TRUE);
	start_color();

	init_pair(COLOR_POSITION_CORRECT, COLOR_BLACK, COLOR_GREEN);
	init_pair(COLOR_THEREIS, COLOR_BLACK, COLOR_YELLOW);
	init_pair(COLOR_THEREISNOT, COLOR_BLACK, COLOR_RED);
	init_pair(COLOR_DEFAULT, COLOR_WHITE, COLOR_BLACK);

	noecho();
	//raw();

	buffer_window = newwin(7, 7, LINES/2 - 3, COLS/2 - 3);
	warning_window = newwin(3, WARNING_WINDOW_WIDTH, LINES - 4, COLS/2 - WARNING_WINDOW_WIDTH/2 - 1);

	box(buffer_window, 0, 0);
	box(warning_window, 0, 0);

	attron(COLOR_PAIR(COLOR_DEFAULT));
	wattron(buffer_window, COLOR_PAIR(COLOR_DEFAULT));
	wattron(warning_window, COLOR_PAIR(COLOR_DEFAULT));

	mvwprintw(warning_window, 1, 16, "Adivinhe a palavra de 5 letras.");
	mvwprintw(stdscr, 3, COLS/2 - 4, "TERMINOOO");
	wmove(buffer_window, 1, 1);

	refresh();
	wrefresh(warning_window);
	wrefresh(buffer_window);

	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 6; j++){
			buffer_try[i][j] = '\0';
		}
	}

	while(!quit){
		ch = getch();

		switch(ch){
			case KEY_BACKSPACE:
				current_character--;

				if(current_character < 0) current_character = 0;

				mvwaddch(buffer_window, 1 + current_buffer, 1 + current_character, ' ');
				buffer_try[current_buffer][current_character] = ' ';
				wmove(buffer_window, 1 + current_buffer, 1 + current_character);
				break;

			case KEY_F(1):
				quit = 1;
				break;

			case KEY_LEFT:
				current_character--;
				if(current_character < 0) current_character = 0;
				wmove(buffer_window, 1 + current_buffer, 1 + current_character);

				break;

			case KEY_RIGHT:
				current_character++;
				if(current_character > 5) current_character = 5;
				wmove(buffer_window, 1 + current_buffer, 1 + current_character);

				break;

			case '\n':
				werase(warning_window);
				box(warning_window, 0, 0);

				if(reset){
					word_chosen = words[rand() % WORDS_SIZE];
					reset = 0;
					current_buffer = 0;
					current_character = 0;

					werase(buffer_window);

					box(buffer_window, 0, 0);

					wmove(buffer_window, 1, 1);
				}
				else if(has_five_characters(buffer_try[current_buffer])){
					if(is_on_list(buffer_try[current_buffer])){
						print_with_color(buffer_window, current_buffer, buffer_try[current_buffer], word_chosen);

						if(!strcmp(buffer_try[current_buffer], word_chosen)){
							mvwprintw(warning_window, 1, 7, "Voce ganhou. Aperte enter para jogar novamente.");
							reset = 1;
						}
						else if(current_buffer == 4){
							mvwprintw(warning_window, 1, 5, "A palavra era: %s. Aperte enter para continuar.", word_chosen);
							reset = 1;
						}
						else{
							current_buffer++;
							current_character = 0;

							wmove(buffer_window, current_buffer + 1, current_character + 1);
						}
					}
					else{
						mvwprintw(warning_window, 1, 13, "Essa palavra nao esta no dicionario.");
					}
				}
				break;

			default:
				if(ch >= 'a' && ch <= 'z'){
					if(current_character < 5){
						mvwaddch(buffer_window, 1 + current_buffer, 1 + current_character, ch);
						buffer_try[current_buffer][current_character] = ch;
					}
	
					current_character++;

					if(current_character > 5) current_character = 5;
				}

				break;
		}


		wrefresh(warning_window);
		wrefresh(buffer_window);
	}

	delwin(warning_window);
	delwin(buffer_window);
	endwin();

	return 0;
}

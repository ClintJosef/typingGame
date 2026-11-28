# Terminal Typing Game I made in C!
https://github.com/user-attachments/assets/f8bb5b1b-c2c2-4903-b508-f3b78fb06745

## To run yourself:
Clone the repo:

```
git clone https://github.com/ClintJosef/typingGame.git && cd typingGame
```

Build with make:

```
make
```

Then run the game!

```
build/typingGame
```

## Usage:
Run with ™¡'-h':
```
> build/typingGame -h
Usage: typingGame [-p paragraphs] [-s sentences] [-S sentences] [-w words] [-W words] [-x seed] [-h]

A typing game in the terminal.
Navigate the text via arrow keys (cursor) and '[' or ']' (pages).

Options:
    -p <paragraphs>	:no. of paragraphs			(default: 2)
    -s <sentences>	:minimum no. of sentences per paragraph	(default: 3)
    -S <sentences>	:maximum no. of sentences per paragraph	(default: 5)
    -w <words>		:minimum no. of words per sentence	(default: 4)
    -W <words>		:maximum no. of words per sentence	(default: 6)
    -x <seed>		:seed to generate text			(default: system time)
    -h 			:shows this help message

Options are for the built-in text generator.
Alternatively, you can pipe your own text through stdin
```

## Examples:
```
# 4-paragraph text
build/typingGame -p 4

# 4-paragraph text, 5-10 sentences each
build/typingGame -p 4 -s 5 -S 20

# 4-paragraph text, 5-10 sentences each, with 10-20 words per sentence
build/typingGame -p 4 -s 5 -S 10 -w 10 -W 20

# Generate text with a seed of 100: (can be used with other options, will always result in the same text)
build/typingGame -x 100

# Pipe output from another command to use as text (ex. fortune)
fortune | build/typingGame

# Use text from a file
build/typingGame < example.txt
```

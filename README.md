# Terminal Typing Game I made in C!
https://github.com/user-attachments/assets/8ce34ca0-ff01-408b-a784-9d1f48b042dc

## To run yourself:
Clone the repo:

```
git clone https://github.com/ClintJosef/typingGame.git && cd typingGame
```

Build with make:

```
make
```

Then just run the game!

```
output/typingGame
```

## Usage:
Just run `--help' or '-h':
```
> output/typingGame -h
Usage: typingGame [-pg -smin -smax -wmin -wmax -h -help]

Options:
    -pg 	:no. of paragraphs			(default: 2)
    -smin 	:minimum no. of sentences per paragraph	(default: 3)
    -smax 	:maximum no. of sentences per paragraph	(default: 5)
    -wmin 	:minimum no. of words per sentence	(default: 4)
    -wmax 	:maximum no. of words per sentence	(default: 6)
    -h/-help 	:prints this help message

Each arg must be passed either as '-arg=value' or '--arg=value'
You can also pipe another program's output into this, but it won't be formatted
```
